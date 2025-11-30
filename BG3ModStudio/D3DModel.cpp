#include "stdafx.h"
#include "D3DModel.h"

#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

// Simple vertex shader
static constexpr auto* VERTEX_SHADER = R"(
// Constant buffer with model transform
cbuffer ModelConstants : register(b0) {
    float3 modelCenter;
    float modelScale;
};

struct VS_INPUT {
    float3 position : POSITION;
    float4 boneWeights : BONEWEIGHTS;
    uint4 boneIndices : BONEINDICES;
    float4 qtangent : QTANGENT;
    float4 color : COLOR;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;

    // Use precomputed center and scale from constant buffer
    float3 pos = input.position;
    pos -= modelCenter;        // Center at origin
    pos *= modelScale;         // Scale to fit viewport

    output.position = float4(pos.x, pos.y, -pos.z, 1.0);
    output.color = input.color;
    
    return output;
}
)";

// Simple pixel shader
static constexpr auto* PIXEL_SHADER = R"(
struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target {
    // Return white for now (ignore vertex color)
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";

D3DModel::~D3DModel()
{
    Release();
}

BOOL D3DModel::Create(Direct3D& d3d, const GR2Model& model)
{
    Release();

    auto* device = d3d.Device();
    if (!device) {
        ATLTRACE("D3DModel::Create: Invalid D3D device.\n");
        return FALSE;
    }

    m_constants.centerX = model.bounds.center.x;
    m_constants.centerY = model.bounds.center.y;
    m_constants.centerZ = model.bounds.center.z;

    // Calculate scale to fit in viewp  (normalize to radius 2.0)
    m_constants.scale = model.bounds.radius > 0.0f ? (2.0f / model.bounds.radius) : 1.0f;

    if (!CreateShaders(device)) {
        ATLTRACE("D3DModel::Create: Failed to create shaders.\n");
        return FALSE;
    }

    if (!CreateConstantBuffer(device)) {
        ATLTRACE("D3DModel::Create: Failed to create constant buffer.\n");
        return FALSE;
    }

    if (!CreateBuffers(device, model)) {
        ATLTRACE("D3DModel::Create: Failed to create buffers.\n");
        return FALSE;
    }

    return !m_meshes.empty();
}

BOOL D3DModel::IsValid() const
{
    return !m_meshes.empty() &&
        m_vertexShader &&
        m_pixelShader &&
        m_inputLayout &&
        m_constantBuffer;
}

void D3DModel::Render(Direct3D& d3d)
{
    auto* context = d3d.Context();
    if (!context) {
        ATLTRACE("D3DModel::Render: Invalid D3D context.\n");
        return;
    }

    // Bind constant buffer
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // Set shaders
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // Set input layout
    context->IASetInputLayout(m_inputLayout.Get());

    // Set primitive topology
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Render each mesh
    for (const auto& mesh : m_meshes) {
        auto stride = static_cast<UINT>(sizeof(GR2Vertex));
        auto offset = 0u;
        context->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(mesh.indexCount, 0, 0);
    }
}

void D3DModel::Release()
{
    m_meshes.clear();
    m_inputLayout.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_constantBuffer.Reset();
}

BOOL D3DModel::CreateBuffers(ID3D11Device* device, const GR2Model& model)
{
    m_meshes.clear();

    m_meshes.reserve(model.meshes.size());
    for (const auto& mesh : model.meshes) {
        MeshBuffers meshBuffers;
        meshBuffers.indexCount = static_cast<uint32_t>(mesh.indices.size());

        // Create vertex buffer
        D3D11_BUFFER_DESC vbDesc{};
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.ByteWidth = static_cast<UINT>(sizeof(GR2Vertex) * mesh.vertices.size());
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vbData{};
        vbData.pSysMem = mesh.vertices.data();

        auto hr = device->CreateBuffer(&vbDesc, &vbData, &meshBuffers.vertexBuffer);
        if (FAILED(hr)) {
            ATLTRACE("D3DModel::CreateBuffers: Failed to create vertex buffer. HRESULT=0x%08X\n", hr);
            return FALSE;
        }

        // Create index buffer
        D3D11_BUFFER_DESC ibDesc{};
        ibDesc.Usage = D3D11_USAGE_DEFAULT;
        ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA ibData{};
        ibData.pSysMem = mesh.indices.data();

        hr = device->CreateBuffer(&ibDesc, &ibData, &meshBuffers.indexBuffer);
        if (FAILED(hr)) {
            ATLTRACE("D3DModel::CreateBuffers: Failed to create index buffer. HRESULT=0x%08X\n", hr);
            return FALSE;
        }

        m_meshes.emplace_back(std::move(meshBuffers));
    }

    return TRUE;
}

BOOL D3DModel::CreateShaders(ID3D11Device* device)
{
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;

    // Compile vertex shader
    auto hr = D3DCompile(
        VERTEX_SHADER,
        strlen(VERTEX_SHADER),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr)) {
        if (errorBlob) {
            ATLTRACE("D3DModel::CreateShaders: Vertex shader compilation error: %s\n",
                     static_cast<const char*>(errorBlob->GetBufferPointer()));
        } else {
            ATLTRACE("D3DModel::CreateShaders: Failed to compile vertex shader. HRESULT=0x%08X\n", hr);
        }
        return FALSE;
    }

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
                                    &m_vertexShader
    );
    if (FAILED(hr)) {
        ATLTRACE("D3DModel::CreateShaders: Failed to create vertex shader. HRESULT=0x%08X\n", hr);
        return FALSE;
    }

    // Create input layout
    if (!CreateInputLayout(device, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize())) {
        return FALSE;
    }

    // Compile pixel shader
    ComPtr<ID3DBlob> psBlob;
    hr = D3DCompile(
        PIXEL_SHADER,
        strlen(PIXEL_SHADER),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr)) {
        if (errorBlob) {
            ATLTRACE("D3DModel::CreateShaders: Pixel shader compilation error: %s\n",
                     static_cast<const char*>(errorBlob->GetBufferPointer()));
        } else {
            ATLTRACE("D3DModel::CreateShaders: Failed to compile pixel shader. HRESULT=0x%08X\n", hr);
        }
        return FALSE;
    }

    hr = device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &m_pixelShader
    );

    return SUCCEEDED(hr);
}

BOOL D3DModel::CreateInputLayout(ID3D11Device* device, const void* vsBytecode, size_t vsBytecodeSize)
{
    constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONEWEIGHTS", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"QTANGENT", 0, DXGI_FORMAT_R16G16B16A16_SNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R16G16_UNORM, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R16G16_UNORM, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    auto hr = device->CreateInputLayout(layout,
                                        ARRAYSIZE(layout),
                                        vsBytecode,
                                        vsBytecodeSize,
                                        &m_inputLayout);

    return SUCCEEDED(hr);
}

BOOL D3DModel::CreateConstantBuffer(ID3D11Device* device)
{
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(ModelConstants);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = &m_constants;

    auto hr = device->CreateBuffer(&desc, &data, &m_constantBuffer);
    if (FAILED(hr)) {
        ATLTRACE("D3DModel::CreateConstantBuffer: Failed to create constant buffer. HRESULT=0x%08X\n", hr);
        return FALSE;
    }

    return TRUE;
}

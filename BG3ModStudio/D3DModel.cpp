#include "stdafx.h"
#include "D3DModel.h"

#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

// Simple vertex shader
static constexpr auto* VERTEX_SHADER = R"(
// Constant buffer with model transform
cbuffer ModelConstants : register(b0) {
    float centerX;
    float centerY;
    float centerZ;
    float scale;
    float aspectRatio;
    float zoom;
    float yaw;
    float pitch;
    float panX;
    float panY;
    float padding[2];
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
    float3 normal : NORMAL;
    float4 color : COLOR;
};

// Decode normal from QTangent (quaternion-based tangent space)
float3 DecodeNormal(float4 qtangent) {
    // QTangent stores a quaternion that encodes the tangent space
    // We extract the normal (Z-axis of tangent space)
    
    float x = qtangent.x;
    float y = qtangent.y;
    float z = qtangent.z;
    float w = qtangent.w;
    
    // Quaternion to normal conversion
    // This extracts the Z-axis (normal) from the tangent space quaternion
    float3 normal;
    normal.x = 2.0 * (x * z + w * y);
    normal.y = 2.0 * (y * z - w * x);
    normal.z = 1.0 - 2.0 * (x * x + y * y);
    
    return normalize(normal);
}

// Rotation matrices
float3 rotateY(float3 v, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return float3(
        v.x * c + v.z * s,
        v.y,
        -v.x * s + v.z * c
    );
}

float3 rotateX(float3 v, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return float3(
        v.x,
        v.y * c - v.z * s,
        v.y * s + v.z * c
    );
}

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;

    // Center the model at origin
    float3 pos = input.position;
    pos.x -= centerX;
    pos.y -= centerY;
    
    // Scale to fit viewport
    pos *= scale;
    
    // Apply camera rotation
    pos = rotateY(pos, yaw);
    pos = rotateX(pos, pitch);
    
    // Apply zoom
    pos *= zoom;

    // Apply pan offset (in screen space)
    pos.x += panX;
    pos.y += panY;

    // Project to screen space with proper depth
    // Map Z to [0, 1] range for depth testing
    float zNear = -10.0;  // Near plane
    float zFar = 10.0;    // Far plane
    float z = (pos.z - zNear) / (zFar - zNear);  // Normalize Z to [0, 1]
    
    output.position = float4(
        pos.x / aspectRatio,  // X with aspect correction
        pos.y,                 // Y
        z,                     // Z in [0, 1] range for depth testing
        1.0                    // W
    );
    
    // Rotate normal for lighting
    float3 normal = DecodeNormal(input.qtangent);
    normal = rotateY(normal, yaw);
    normal = rotateX(normal, pitch);
    output.normal = normal;
    
    output.color = input.color;
    
    return output;
}
)";

// Simple pixel shader
static constexpr auto* PIXEL_SHADER = R"(
struct PS_INPUT {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target {
    // Normalize the interpolated normal
    float3 normal = normalize(input.normal);

    // Define a light direction (coming from upper-right-front)
    float3 lightDir = normalize(float3(0.5, 0.7, -0.5));

    // Calculate diffuse lighting (N dot L)
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Add ambient light (so it's not completely black)
    float ambient = 0.4;
    float lighting = ambient + diffuse * 0.8;

    // Use vertex color RED channel as ambient occlusion
    float ao = input.color.r;

    // Combine lighting with ambient occlusion
    float finalColor = lighting * ao;

    return float4(finalColor, finalColor, finalColor, 1.0);
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

    // Calculate scale to fit in viewport  (normalize to radius 1.0)
    m_constants.scale = model.bounds.radius > 0.0f ? (1.0f / model.bounds.radius) : 1.0f;

    m_constants.aspectRatio = 1.0f; // Will be set during rendering
    m_constants.zoom = 1.0f;
    m_constants.yaw = 0.0f;
    m_constants.pitch = 0.0f;

    auto worldWidth = (model.bounds.max.x - model.bounds.min.x) * m_constants.scale;
    auto worldHeight = (model.bounds.max.y - model.bounds.min.y) * m_constants.scale;

    m_modelScreenWidth = worldWidth;
    m_modelScreenHeight = worldHeight;

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

    d3d.BeginRender();

    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    context->RSGetViewports(&numViewports, &viewport);

    auto aspectRatio = viewport.Height <= 0.0f ? 1.0f : (viewport.Width / viewport.Height);

    if (abs(m_constants.aspectRatio - aspectRatio) > 0.001f) {
        m_constants.aspectRatio = aspectRatio;
    }

    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &m_constants, 0, 0);

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

void D3DModel::SetRotation(float yaw, float pitch)
{
    m_constants.yaw = yaw;
    m_constants.pitch = pitch;
}

void D3DModel::SetZoom(float zoom)
{
    m_constants.zoom = zoom;
}

void D3DModel::SetPan(float panX, float panY)
{
    m_constants.panX = panX;
    m_constants.panY = panY;
}

float D3DModel::GetScreenWidth() const
{
    return m_modelScreenWidth * m_constants.zoom;
}

float D3DModel::GetScreenHeight() const
{
    return m_modelScreenHeight * m_constants.zoom;
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

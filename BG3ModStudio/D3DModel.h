#pragma once

#include "Direct3D.h"
#include "GR2Model.h"

#include <d3d11.h>
#include <wrl/client.h>

class D3DModel
{
public:
    D3DModel() = default;
    ~D3DModel();

    BOOL Create(Direct3D& d3d, const GR2Model& model);
    BOOL IsValid() const;
    void Release();
    void Render(Direct3D& d3d);

private:
    BOOL CreateBuffers(ID3D11Device* device, const GR2Model& model);
    BOOL CreateConstantBuffer(ID3D11Device* device);
    BOOL CreateInputLayout(ID3D11Device* device, const void* vsBytecode, size_t vsBytecodeSize);
    BOOL CreateShaders(ID3D11Device* device);

    struct MeshBuffers
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        uint32_t indexCount = 0;
    };

    struct ModelConstants
    {
        float centerX{}, centerY{}, centerZ{};
        float scale{};
    };

    ModelConstants m_constants{};
    std::vector<MeshBuffers> m_meshes;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
};

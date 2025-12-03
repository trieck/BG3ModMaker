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
    void SetRotation(float yaw, float pitch);
    void SetZoom(float zoom);
    void SetPan(float panX, float panY);

    float GetScreenWidth() const;
    float GetScreenHeight() const;

private:
    BOOL CreateBuffers(ID3D11Device* device, const GR2Model& model);
    BOOL CreateConstantBuffer(ID3D11Device* device);
    BOOL CreateInputLayout(ID3D11Device* device, const void* vsBytecode, size_t vsBytecodeSize);
    BOOL CreateShaders(ID3D11Device* device);
    BOOL CreateRasterizerState(ID3D11Device* device);

    struct MeshBuffers
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        uint32_t indexCount = 0;
    };

    struct ModelConstants
    {
        float centerX{0.0f};
        float centerY{0.0f};
        float centerZ{0.0f};
        float scale{0.0f};
        float aspectRatio{0.0f};
        float zoom{1.0f};
        float yaw{0.0f};
        float pitch{0.0f};
        float panX{0.0f};
        float panY{0.0f};
        float padding[2];
    };

    ModelConstants m_constants{};
    std::vector<MeshBuffers> m_meshes;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    
    float m_modelScreenWidth{0.0f};
    float m_modelScreenHeight{0.0f};
};

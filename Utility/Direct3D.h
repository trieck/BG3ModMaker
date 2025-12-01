#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Direct3D
{
public:
    Direct3D() = default;
    ~Direct3D();

    HRESULT Initialize(HWND hWnd);

    void Release();

    HRESULT Resize(UINT width, UINT height);
    void Clear(const float clearColor[4]);
    void BeginRender();
    HRESULT Present();

    ID3D11Device* Device() const;
    ID3D11DeviceContext* Context() const;
    IDXGISwapChain* SwapChain() const;
    ID3D11RenderTargetView* RenderTargetView() const;
    ID3D11DepthStencilView* DepthStencilView() const;

private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
};

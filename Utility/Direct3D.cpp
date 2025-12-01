#include "pch.h"
#include "Direct3D.h"

#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi.h>

using Microsoft::WRL::ComPtr;

Direct3D::~Direct3D()
{
    Release();
}

HRESULT Direct3D::Initialize(HWND hWnd)
{
    if (!IsWindow(hWnd)) {
        return E_INVALIDARG;
    }

    RECT rc;
    GetClientRect(hWnd, &rc);
    auto width = rc.right - rc.left;
    auto height = rc.bottom - rc.top;

    width = std::max<LONG>(width, 8);
    height = std::max<LONG>(height, 8);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL featureLevel;
    auto hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &sd,
        &m_swapChain,
        &m_d3dDevice,
        &featureLevel,
        &m_d3dContext
    );
    if (FAILED(hr)) {
        return hr;
    }

    // Create render target view
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) {
        return hr;
    }

    // Set render target
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

    // Setup viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_d3dContext->RSSetViewports(1, &vp);

    return S_OK;
}

void Direct3D::Release()
{
    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();
}

HRESULT Direct3D::Resize(UINT width, UINT height)
{
    if (!m_swapChain) {
        return E_FAIL;
    }

    // Release old views
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();

    // Resize buffers
    auto hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        return hr;
    }

    // Recreate render target view
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) {
        return hr;
    }

    // Set render target
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

    // Update viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_d3dContext->RSSetViewports(1, &vp);

    return S_OK;
}

void Direct3D::Clear(const float clearColor[4])
{
    if (!m_d3dContext || !m_renderTargetView) {
        return;
    }

    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

    if (m_depthStencilView) {
        m_d3dContext->ClearDepthStencilView(
            m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0
        );
    }
}

void Direct3D::BeginRender()
{
    if (!m_d3dContext || !m_renderTargetView) {
        return;
    }

    // Rebind render target view (gets unbound after Present with flip model)
    ID3D11RenderTargetView* rtv = m_renderTargetView.Get();
    m_d3dContext->OMSetRenderTargets(1, &rtv, nullptr);
}

HRESULT Direct3D::Present()
{
    if (!m_swapChain) {
        return E_FAIL;
    }

    return m_swapChain->Present(0, 0);
}

ID3D11Device* Direct3D::Device() const
{
    return m_d3dDevice.Get();
}

ID3D11DeviceContext* Direct3D::Context() const
{
    return m_d3dContext.Get();
}

IDXGISwapChain* Direct3D::SwapChain() const
{
    return m_swapChain.Get();
}

ID3D11RenderTargetView* Direct3D::RenderTargetView() const
{
    return m_renderTargetView.Get();
}

ID3D11DepthStencilView* Direct3D::DepthStencilView() const
{
    return m_depthStencilView.Get();
}

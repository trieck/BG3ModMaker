#pragma once

class CoMemory : public IUnknown
{
public:
    CoMemory();
    virtual ~CoMemory();

    // IUknown
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

    void* Alloc(size_t size);
    void* Realloc(size_t size);
    void Free();
    uint8_t* Data() const;

private:
    void* m_pData;
    ULONG m_cRef;
};

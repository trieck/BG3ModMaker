#pragma once
#include "UtilityBase.h"

/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE UTF8Stream : public CComObjectRoot, public IStream
{
public:
    BEGIN_COM_MAP(UTF8Stream)
        COM_INTERFACE_ENTRY(IStream)
    END_COM_MAP()
    virtual ~UTF8Stream();

    HRESULT FinalConstruct();
    void FinalRelease();

    HRESULT Write(LPCWSTR text) const;
    HRESULT Write(LPCSTR text) const;
    HRESULT Write(LPCSTR text, size_t length) const;
    HRESULT Reset() const;
    CStringA ReadString();
    CStringW ReadUTF16String();
    ByteBuffer ReadBytes(size_t size = -1);

    // IStream members
    STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override;
    STDMETHODIMP Write(const void* pv, ULONG cb, ULONG* pcbWritten) override;
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override;
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) override;
    STDMETHODIMP CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override;
    STDMETHODIMP Commit(DWORD grfCommitFlags) override;
    STDMETHODIMP Revert() override;
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
    STDMETHODIMP Stat(STATSTG* pstatstg, DWORD grfStatFlag) override;
    STDMETHODIMP Clone(IStream** ppstm) override;

private:
    CComPtr<IStream> m_pImpl;
};

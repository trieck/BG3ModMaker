#pragma once

class PAKWizard
{
public:
    explicit PAKWizard(const CString& root);
    ~PAKWizard();

    void Execute();
    const CString& GetRoot() const;

    BOOL GetGenerateLoca() const;
    void SetGenerateLoca(BOOL generateLoca);

    BOOL GetGenerateLSF() const;
    void SetGeneateLSF(BOOL generateLoca);

    const CString& GetPAKFile() const;
    void SetPAKFile(const CString& file);

private:
    CString m_root;
    CString m_PAKFile;
    BOOL m_generateLoca = TRUE;
    BOOL m_generateLSF = TRUE;
};

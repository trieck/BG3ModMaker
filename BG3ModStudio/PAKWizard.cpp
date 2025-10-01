#include "stdafx.h"
#include "PAKWizSheet.h"
#include "PAKWizard.h"

PAKWizard::PAKWizard(const CString& root) : m_root(root)
{
}

PAKWizard::~PAKWizard()
{
}

void PAKWizard::Execute()
{
    PAKWizSheet sheet(this);
    sheet.DoModal();
}

const CString& PAKWizard::GetRoot() const
{
    return m_root;
}

BOOL PAKWizard::GetGenerateLoca() const
{
    return m_generateLoca;
}

void PAKWizard::SetGenerateLoca(BOOL generateLoca)
{
    m_generateLoca = generateLoca;
}

BOOL PAKWizard::GetGenerateLSF() const
{
    return m_generateLSF;
}

void PAKWizard::SetGeneateLSF(BOOL generateLoca)
{
    m_generateLSF = generateLoca;
}

const CString& PAKWizard::GetPAKFile() const
{
    return m_PAKFile;
}

void PAKWizard::SetPAKFile(const CString& file)
{
    m_PAKFile = file;
}

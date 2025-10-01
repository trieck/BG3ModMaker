#include "stdafx.h"
#include "PAKWizSheet.h"

PAKWizSheet::PAKWizSheet(PAKWizard* pWiz)
    : BaseSheet(_T("BG3 PAK Builder"), IDB_PAK_HEADER, IDB_PAK_LARGE, 0, nullptr),
      m_pWiz(pWiz), m_welcomePage(pWiz), m_filePage(pWiz), m_buildPage(pWiz)
{
    AddPage(m_welcomePage);
    AddPage(m_filePage);
    AddPage(m_buildPage);
}

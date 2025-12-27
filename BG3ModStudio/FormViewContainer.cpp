#include "stdafx.h"
#include "FormViewContainer.h"
#include "OsiViewFactory.h"

BOOL FormViewContainer::LoadView(OsiViewType viewType, LPCVOID pv)
{
    if (!IsWindow()) {
        return FALSE;
    }

    if (m_pFormView) {
        m_pFormView->Destroy();
        m_pFormView.reset();
    }

    m_pFormView = OsiViewFactory::CreateView(*this, viewType, pv);
    if (m_pFormView) {
        ::ShowWindow(*m_pFormView, SW_SHOW);
    }

    UpdateLayout();

    return m_pFormView != nullptr;
}

void FormViewContainer::DestroyView()
{
    if (m_pFormView) {
        m_pFormView->Destroy();
    }

    m_pFormView.reset();

    UpdateLayout();
}

LRESULT FormViewContainer::OnCreate(LPCREATESTRUCT pcs)
{
    return 0;
}

void FormViewContainer::OnSize(UINT nType, CSize size)
{
    UpdateLayout();
}

void FormViewContainer::UpdateLayout()
{
    if (IsWindow() && m_pFormView && ::IsWindow(*m_pFormView)) {
        CRect rc;
        GetClientRect(&rc);
        ::SetWindowPos(*m_pFormView, nullptr, 0, 0, rc.Width(), rc.Height(),
                       SWP_NOZORDER | SWP_NOACTIVATE);
    }

    Invalidate();
}

void FormViewContainer::OnDestroy()
{
    if (m_pFormView) {
        m_pFormView->Destroy();
    }

    m_pFormView.reset();
}

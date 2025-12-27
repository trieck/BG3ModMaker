#include "stdafx.h"
#include "GoalFormView.h"
#include "OsiViewFactory.h"

IFormView::Ptr OsiViewFactory::CreateView(HWND hWndParent, OsiViewType type, LPCVOID pv)
{
    IFormView::Ptr formView;

    if (type == OVT_GOAL) {
        formView = std::make_unique<GoalFormView>();
    }

    if (formView) {
        formView->Create(hWndParent, reinterpret_cast<LPARAM>(pv));
    }

    return formView;
}

#include "stdafx.h"
#include "FunctionFormView.h"
#include "GoalFormView.h"
#include "OsiViewFactory.h"
#include "TypeFormView.h"

IFormView::Ptr OsiViewFactory::CreateView(HWND hWndParent, OsiViewType type, LPCVOID pv)
{
    IFormView::Ptr formView;

    switch (type) {
    case OVT_FUNCTION:
        formView = std::make_unique<FunctionFormView>();
        break;
    case OVT_GOAL:
        formView = std::make_unique<GoalFormView>();
        break;
    case OVT_TYPE:
        formView = std::make_unique<TypeFormView>();
        break;
    default:
        break;
    }

    if (formView) {
        formView->Create(hWndParent, reinterpret_cast<LPARAM>(pv));
    }

    return formView;
}

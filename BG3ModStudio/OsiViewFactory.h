#pragma once

#include "FormView.h"
#include "OsiData.h"

class OsiViewFactory
{
public:
    static IFormView::Ptr CreateView(HWND hWndParent, OsiViewType type, LPCVOID pv = nullptr);
};


#include "stdafx.h"
#include "GoalFormView.h"
#include "OsiData.h"
#include "StringHelper.h"
#include "Util.h"

HWND GoalFormView::Create(HWND hWndParent, LPARAM dwInitParam)
{
    if (!Base::Create(hWndParent, dwInitParam)) {
        return nullptr;
    }

    auto* pData = reinterpret_cast<OsiData*>(dwInitParam);
    ATLASSERT(pData != nullptr);

    m_pStory = pData->pStory;
    ATLASSERT(m_pStory != nullptr);

    m_pGoal = static_cast<const OsiGoal*>(pData->pdata);
    ATLASSERT(m_pGoal != nullptr);

    m_initCalls = GetDlgItem(IDC_INIT_CALLS);
    ATLASSERT(m_initCalls.IsWindow());

    m_exitCalls = GetDlgItem(IDC_EXIT_CALLS);
    ATLASSERT(m_exitCalls.IsWindow());

    m_signature = GetDlgItem(IDC_E_SIGNATURE);
    ATLASSERT(m_signature.IsWindow());

    m_font.Attach(Util::CreateFixedWidthFont(110));
    m_signature.SetFont(m_font);

    static constexpr auto icons = {
        IDI_CALL
    };

    m_imageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, static_cast<uint32_t>(icons.size()), 0);
    ATLASSERT(m_imageList);

    for (auto icon : icons) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(icon));
        ATLASSERT(hIcon);
        m_imageList.AddIcon(hIcon);
    }

    m_initCalls.SetImageList(m_imageList, TVSIL_NORMAL);
    m_exitCalls.SetImageList(m_imageList, TVSIL_NORMAL);

    DlgResize_Init();

    Populate();

    return this->m_hWnd;
}

LRESULT GoalFormView::OnSelChanged(LPNMHDR pnmh)
{
    CTreeItem item;
    if (pnmh->hwndFrom == m_initCalls) {
        item = MAKE_TREEITEM(pnmh, &m_initCalls);
    } else if (pnmh->hwndFrom == m_exitCalls) {
        item = MAKE_TREEITEM(pnmh, &m_exitCalls);
    }

    if (item.IsNull()) {
        return 0;
    }

    auto pCall = reinterpret_cast<const OsiCall*>(item.GetData());
    ATLASSERT(pCall != nullptr);

    auto callString = CallString(*pCall);

    m_signature.SetWindowText(callString);

    return 0;
}

void GoalFormView::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);
}

CString GoalFormView::CallString(const OsiCall& call)
{
    CString result;

    if (call.negate) {
        result.Format(L"NOT ");
    }

    result += StringHelper::fromUTF8(call.name.c_str());
    result += L"(";

    for (auto i = 0u; i < call.parameters.size(); ++i) {
        const auto& param = call.parameters[i];
        auto paramString = ParameterString(call, i, param);
        result += paramString;

        if (i + 1 < call.parameters.size()) {
            result += L",\r\n";
        }
    }

    result += L")";

    return result;
}

CString GoalFormView::ParameterString(const OsiCall& call, uint32_t index, const OsiTypedValue::Ptr& param)
{
    CString paramValue(L"<unknown>");
    CString paramType(L"<unknown>");

    auto it = m_pStory->functionNames.find(call.name);
    if (it != m_pStory->functionNames.end()) {
        const auto* func = it->second;
        const auto& sig = func->name;
        if (index < sig.parameters.types.size()) {
            auto type = sig.parameters.types[index];

            CString outParam;
            if (sig.isOutParam(index)) {
                outParam.Format(L"out ");
            }
            paramType = outParam + StringHelper::fromUTF8(m_pStory->typeName(type).c_str());
        }
    }

    const auto& value = param->value;
    std::visit([&]<typename T0>(const T0& arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, int32_t>) {
            paramValue.Format(L"%d", arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            paramValue.Format(L"%lld", static_cast<long long>(arg));
        } else if constexpr (std::is_same_v<T, float>) {
            paramValue.Format(L"%g", arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            paramValue = L"\"" + StringHelper::fromUTF8(arg.c_str()) + L"\"";
        }
    }, value);

    CString result;
    result.Format(L"%s %s", paramType.GetString(), paramValue.GetString());

    return result;
}

void GoalFormView::Populate()
{
    ATLASSERT(m_pGoal != nullptr);

    m_initCalls.DeleteAllItems();
    m_exitCalls.DeleteAllItems();
    m_signature.SetWindowText(L"");

    for (const auto& call : m_pGoal->initCalls) {
        CString callString;
        if (call.negate) {
            callString.Format(L"NOT ");
        }
        callString += StringHelper::fromUTF8(call.name.c_str());

        TVINSERTSTRUCT tvis{};
        tvis.hParent = TVI_ROOT;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        tvis.item.pszText = const_cast<LPWSTR>(callString.GetString());
        tvis.item.lParam = reinterpret_cast<LPARAM>(&call);

        m_initCalls.InsertItem(&tvis);
    }

    for (const auto& call : m_pGoal->exitCalls) {
        CString callString;
        if (call.negate) {
            callString.Format(L"NOT ");
        }
        callString += StringHelper::fromUTF8(call.name.c_str());

        TVINSERTSTRUCT tvis{};
        tvis.hParent = TVI_ROOT;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        tvis.item.pszText = const_cast<LPWSTR>(callString.GetString());
        tvis.item.lParam = reinterpret_cast<LPARAM>(&call);

        m_exitCalls.InsertItem(&tvis);
    }
}

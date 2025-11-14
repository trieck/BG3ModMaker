#include "stdafx.h"
#include "Exception.h"
#include "GameObjectDlg.h"
#include "IconDlg.h"
#include "Settings.h"
#include "StringHelper.h"
#include "Util.h"
#include "ValueViewDlg.h"

static constexpr auto COLUMN_PADDING = 12;

struct NodeData
{
    CString uuid;
    nlohmann::json data;
};

BOOL GameObjectDlg::OnIdle()
{
    UIUpdateChildWindows(TRUE);

    return FALSE;
}

BOOL GameObjectDlg::OnInitDialog(HWND, LPARAM)
{
    Settings settings;
    m_dbPath = settings.GetString("Settings", "GameObjectPath");

    auto wndFrame = GetDlgItem(IDC_ST_GAMEOBJECT);
    ATLASSERT(wndFrame.IsWindow());

    CRect rcDlg;
    GetClientRect(&rcDlg);

    CRect rcFrame;
    wndFrame.GetWindowRect(&rcFrame);
    ScreenToClient(&rcFrame);

    m_marginLeft = rcFrame.left;
    m_marginTop = rcFrame.top;
    m_marginRight = rcDlg.right - rcFrame.right;
    m_marginBottom = rcDlg.bottom - rcFrame.bottom;

    wndFrame.DestroyWindow(); // Needed only for margin calculation

    m_splitter.Create(m_hWnd, rcFrame, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    ATLASSERT(m_splitter.IsWindow());

    if (!m_tree.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                       WS_EX_CLIENTEDGE)) {
        ATLTRACE("Unable to create tree view window.\n");
        return -1;
    }

    m_font = AtlCreateControlFont();
    m_tree.SetFont(m_font);

    static constexpr auto icons = {
        IDI_GAME_OBJECT
    };

    m_imageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, static_cast<int>(icons.size()), 0);
    for (auto icon : icons) {
        auto hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(icon));
        ATLASSERT(hIcon);
        m_imageList.AddIcon(hIcon);
    }

    m_tree.SetImageList(m_imageList, TVSIL_NORMAL);

    auto style = TVS_HASBUTTONS | TVS_HASLINES | TVS_FULLROWSELECT | TVS_INFOTIP
        | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_EDITLABELS;

    m_tree.ModifyStyle(0, style);

    m_attributes.Create(m_splitter, rcDefault, nullptr,
                        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                        LVS_REPORT | LVS_SINGLESEL, WS_EX_CLIENTEDGE, ID_ATTRIBUTE_LIST);

    m_attributes.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_attributes.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100);
    m_attributes.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 150);
    m_attributes.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 150);

    m_splitter.SetSplitterPane(0, m_tree);
    m_splitter.SetSplitterPane(1, m_attributes);
    m_splitter.SetSplitterPosPct(50);

    Populate();

    UIAddChildWindowContainer(m_hWnd);

    DlgResize_Init();

    CenterWindow(GetParent());

    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddIdleHandler(this);

    auto icon = Util::LoadBitmapAsIcon(ID_TOOL_GAMEOBJECT, 32, 32);
    if (icon != nullptr) {
        SetIcon(icon, TRUE);
        SetIcon(icon, FALSE);
    }

    return TRUE; // Let the system set the focus
}

LRESULT GameObjectDlg::OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    auto pia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    if (!pia || pia->iItem < 0) {
        return 0;
    }

    CString id, value;
    m_attributes.GetItemText(pia->iItem, 1, value);
    if (value.IsEmpty()) {
        return 0;
    }

    CWaitCursor cursor;
    auto pDlg = std::make_unique<IconDlg>(value);
    if (!pDlg->HasImage()) {
        return 0;
    }

    pDlg->Run(*this);
    pDlg.release();

    return 0;
}

LRESULT GameObjectDlg::OnItemExpanding(LPNMHDR pnmh)
{
    const auto item = MAKE_TREEITEM(pnmh, &m_tree);

    ExpandNode(item);

    return 0;
}

LRESULT GameObjectDlg::OnTVSelChanged(LPNMHDR pnmh)
{
    m_attributes.DeleteAllItems();

    const auto item = MAKE_TREEITEM(pnmh, &m_tree);

    auto* data = reinterpret_cast<NodeData*>(item.GetData());
    if (data == nullptr) {
        return 0;
    }

    auto jsonData = data->data;

    for (const auto& attr : jsonData["attributes"]) {
        CString name = attr.value("id", "").c_str();
        CString value = attr.value("value", "").c_str();
        CString type = attr.value("type", "").c_str();
        auto row = m_attributes.InsertItem(m_attributes.GetItemCount(), name);
        m_attributes.SetItemText(row, 1, value);
        m_attributes.SetItemText(row, 2, type);
    }

    AutoAdjustAttributes();

    return 0;
}

CString GameObjectDlg::GetAttribute(const nlohmann::json& obj, const CString& key)
{
    const auto& attributes = obj["attributes"];
    ATLASSERT(attributes.is_array());

    for (const auto& attr : attributes) {
        auto id = attr["id"].get<std::string>();
        auto wideId = StringHelper::fromUTF8(id.c_str());
        if (wideId == key) {
            auto value = attr["value"].get<std::string>();
            return value.c_str();
        }
    }

    return "";
}

void GameObjectDlg::OnContextMenu(const CWindow& wnd, const CPoint& point)
{
    CRect rc;
    m_attributes.GetWindowRect(&rc);

    if (rc.PtInRect(point)) {
        OnContextAttributes(point);
    }
}

void GameObjectDlg::OnContextAttributes(const CPoint& point)
{
    CMenu menu;
    menu.LoadMenu(IDR_ATTRIBUTE_CONTEXT);

    CMenuHandle popup = menu.GetSubMenu(0);
    auto cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, *this);
    if (cmd == 0) {
        return; // No command selected
    }

    auto selectedRow = m_attributes.GetSelectedIndex();
    if (selectedRow < 0) {
        return; // No item selected
    }

    CString text;
    switch (cmd) {
    case ID_ATTRIBUTE_COPYNAME: // Copy Name
        m_attributes.GetItemText(selectedRow, 0, text);
        break;
    case ID_ATTRIBUTE_COPYVALUE: // Copy Value
        m_attributes.GetItemText(selectedRow, 1, text);
        break;
    case ID_ATTRIBUTE_COPYTYPE: // Copy Type
        m_attributes.GetItemText(selectedRow, 2, text);
        break;
    case ID_ATTRIBUTE_VIEW_VALUE: // View Value
        ViewValue();
        return;
    default:
        return; // Unknown command
    }

    if (text.IsEmpty()) {
        return; // Nothing to copy
    }

    Util::CopyToClipboard(*this, text);
}

void GameObjectDlg::ViewValue()
{
    auto selectedRow = m_attributes.GetSelectedIndex();
    if (selectedRow < 0) {
        return; // No item selected
    }

    CString name, value;
    m_attributes.GetItemText(selectedRow, 0, name);
    m_attributes.GetItemText(selectedRow, 1, value);
    if (value.IsEmpty()) {
        return; // No value to view
    }

    auto* pDlg = new ValueViewDlg();
    pDlg->SetTitle(name);
    pDlg->SetValue(value);
    pDlg->Run(*this);
}

void GameObjectDlg::OnSize(UINT, const CSize& size)
{
    DlgResize_UpdateLayout(size.cx, size.cy);

    if (m_splitter.IsWindow()) {
        CRect rc;
        rc.left = m_marginLeft;
        rc.top = m_marginTop;
        rc.right = size.cx - m_marginRight;
        rc.bottom = size.cy - m_marginBottom;

        m_splitter.MoveWindow(&rc);
    }
}

void GameObjectDlg::PopulateTypes()
{
    m_tree.DeleteAllItems();
    m_attributes.DeleteAllItems();

    auto it = m_cataloger.getTypes();

    for (; it->isValid(); it->next()) {
        auto type = it->value();
        auto wideType = StringHelper::fromUTF8(type.c_str());
        InsertNode(TVI_ROOT, wideType, reinterpret_cast<LPARAM>(new NodeData()));
    }
}

void GameObjectDlg::AutoAdjustAttributes()
{
    CClientDC dc(m_attributes);
    auto hFont = m_attributes.GetFont();
    auto hOldFont = dc.SelectFont(hFont);

    auto header = m_attributes.GetHeader();

    for (auto col = 0; col < header.GetItemCount(); ++col) {
        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;

        TCHAR textBuf[256]{};
        lvc.pszText = textBuf;
        lvc.cchTextMax = _countof(textBuf);
        m_attributes.GetColumn(col, &lvc);

        CString headerText = lvc.pszText;

        CSize sz;
        dc.GetTextExtent(headerText, headerText.GetLength(), &sz);
        auto maxWidth = sz.cx;

        auto rowCount = m_attributes.GetItemCount();
        for (auto row = 0; row < rowCount; ++row) {
            CString cellText;
            m_attributes.GetItemText(row, col, cellText);

            dc.GetTextExtent(cellText, cellText.GetLength(), &sz);
            maxWidth = std::max(sz.cx, maxWidth);
        }

        maxWidth += COLUMN_PADDING;
        m_attributes.SetColumnWidth(col, maxWidth);
    }

    dc.SelectFont(hOldFont);
}

void GameObjectDlg::ExpandNode(const CTreeItem& node)
{
    auto child = node.GetChild();

    if (m_tree.GetItemState(node.m_hTreeItem, TVIS_EXPANDED)) {
        if (child.IsNull()) {
            TVITEMEX item{};
            item.mask = TVIF_CHILDREN;
            item.hItem = node.m_hTreeItem;
            item.cChildren = 0; // no children
            m_tree.SetItem(&item);
        }

        return; // already expanded
    }

    if (!child.IsNull()) {
        return; // already have children
    }

    auto data = std::bit_cast<NodeData*>(node.GetData());
    if (data == nullptr) {
        return;
    }

    CWaitCursor cursor;

    if (data->uuid.IsEmpty()) { // type node
        CString type;
        m_tree.GetItemText(node.m_hTreeItem, type);

        auto utf8Type = StringHelper::toUTF8(type).GetString();

        auto it = m_cataloger.getRoots(utf8Type);
        if (!it->isValid()) {
            TVITEMEX item{};
            item.mask = TVIF_CHILDREN;
            item.hItem = node.m_hTreeItem;
            item.cChildren = 0; // no children
            m_tree.SetItem(&item);
            return;
        }

        for (; it->isValid(); it->next()) {
            auto uuid = it->value();
            auto wideUuid = StringHelper::fromUTF8(uuid.c_str());
            auto value = m_cataloger.get(uuid);

            CString wideName(wideUuid);
            auto name = GetAttribute(value, "Name");
            if (!name.IsEmpty()) {
                wideName = name;
            }

            auto* pNodeData = new NodeData();
            pNodeData->uuid = wideUuid;
            pNodeData->data = std::move(value);
            InsertNode(node.m_hTreeItem, wideName, reinterpret_cast<LPARAM>(pNodeData));
        }
    } else {
        auto utfUuid = StringHelper::toUTF8(data->uuid);
        auto children = m_cataloger.getChildren(utfUuid);

        if (!children->isValid()) {
            TVITEMEX item{};
            item.mask = TVIF_CHILDREN;
            item.hItem = node.m_hTreeItem;
            item.cChildren = 0; // no children
            m_tree.SetItem(&item);
            return;
        }

        for (; children->isValid(); children->next()) {
            auto childUuid = children->value();
            auto wideChild = StringHelper::fromUTF8(childUuid.c_str());
            auto* pNodeData = new NodeData();
            pNodeData->uuid = wideChild;
            pNodeData->data = m_cataloger.get(childUuid);

            CString wideName(wideChild);
            auto name = GetAttribute(pNodeData->data, "Name");
            if (!name.IsEmpty()) {
                wideName = name;
            }

            InsertNode(node.m_hTreeItem, wideName, reinterpret_cast<LPARAM>(pNodeData));
        }
    }
}

HTREEITEM GameObjectDlg::InsertNode(HTREEITEM hParent, const CString& key, LPARAM lparam)
{
    TVINSERTSTRUCT tvis{};
    tvis.hParent = hParent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
        TVIF_CHILDREN | TVIF_PARAM;

    tvis.item.pszText = const_cast<LPWSTR>(key.GetString());
    tvis.item.cChildren = 1;
    tvis.item.lParam = lparam;

    return m_tree.InsertItem(&tvis);
}

LRESULT GameObjectDlg::OnDelete(LPNMHDR pnmh)
{
    const auto item = MAKE_OLDTREEITEM(pnmh, &m_tree);

    auto* pdata = reinterpret_cast<NodeData*>(item.GetData());

    delete pdata;

    return 0;
}

void GameObjectDlg::OnClose()
{
    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveIdleHandler(this);

    m_cataloger.close();

    Destroy();
}

void GameObjectDlg::OnDestroy()
{
    auto* pLoop = _Module.GetMessageLoop();
    if (pLoop != nullptr) {
        pLoop->RemoveIdleHandler(this);
    }
}

void GameObjectDlg::Populate()
{
    CWaitCursor cursor;

    m_tree.DeleteAllItems();
    m_attributes.DeleteAllItems();

    try {
        if (!m_cataloger.isOpen()) {
            m_cataloger.openReadOnly(StringHelper::toUTF8(m_dbPath).GetString());
        }
        PopulateTypes();
    } catch (const Exception& ex) {
        CString msg;
        msg.Format(_T("Failed to open game object database: %s"), CString(ex.what()));
        AtlMessageBox(*this, msg.GetString(), nullptr, MB_ICONERROR);
    }
}

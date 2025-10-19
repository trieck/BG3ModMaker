#pragma once

#include "Scintilla.h"
#include "ILexer.h"

extern "C" Scintilla::ILexer5* __stdcall CreateLexer(const char* name);

template <class TBase>
class ScintillaCtrlT : public TBase
{
public:
    explicit ScintillaCtrlT(HWND hWnd = nullptr) : TBase(hWnd)
    {
    }

    ScintillaCtrlT& operator=(HWND hWnd)
    {
        this->m_hWnd = hWnd;
        return *this;
    }

    static LPCTSTR GetWndClassName()
    {
        return L"Scintilla";
    }

    HWND Create(HWND hWndParent, _U_RECT rect = nullptr, LPCTSTR szWindowName = nullptr,
                DWORD dwStyle = CControlWinTraits::GetWndStyle(0),
                DWORD dwExStyle = CControlWinTraits::GetWndExStyle(0), _U_MENUorID MenuOrID = 0U,
                LPVOID lpCreateParam = nullptr)
    {
        ATLASSERT(hWndParent != nullptr);
        ATLASSERT(this->m_hWnd == nullptr);

        if (!TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle,
                           MenuOrID.m_hMenu, lpCreateParam)) {
            ATLTRACE("Failed to create Scintilla control.\n");
            return nullptr;
        }

        return this->m_hWnd;
    }

    LRESULT SetDefaultFontFace(LPCSTR faceName)
    {
        return SendMessage(this->m_hWnd, SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>(faceName));
    }

    LRESULT SetDefaultFontSize(int size)
    {
        return SendMessage(this->m_hWnd, SCI_STYLESETSIZE, STYLE_DEFAULT, size);
    }

    LRESULT ClearAllStyles()
    {
        return SendMessage(this->m_hWnd, SCI_STYLECLEARALL, 0, 0);
    }

    LRESULT SetLexer(LPCSTR name)
    {
        if (name == nullptr) {
            return SendMessage(this->m_hWnd, SCI_SETILEXER, 0, 0);
        }

        return SendMessage(this->m_hWnd, SCI_SETILEXER, 0, reinterpret_cast<LPARAM>(CreateLexer(name)));
    }

    LRESULT SetMarginType(int margin /* 0 = line numbers */, int type)
    {
        return SendMessage(this->m_hWnd, SCI_SETMARGINTYPEN, margin, type);
    }

    LRESULT SetMarginWidth(int margin /* 0 = line numbers */, int width)
    {
        return SendMessage(this->m_hWnd, SCI_SETMARGINWIDTHN, margin, width);
    }

    LRESULT SetForeStyle(int style, COLORREF color)
    {
        return SendMessage(this->m_hWnd, SCI_STYLESETFORE, style, color);
    }

    LRESULT SetBoldStyle(int style, BOOL bold)
    {
        return SendMessage(this->m_hWnd, SCI_STYLESETBOLD, style, bold);
    }

    LRESULT SetTabWidth(int width)
    {
        return SendMessage(this->m_hWnd, SCI_SETTABWIDTH, 0, width);
    }

    LRESULT SetUseTabs(BOOL useTabs)
    {
        return SendMessage(this->m_hWnd, SCI_SETUSETABS, useTabs, 0);
    }

    LRESULT SetIdleStyling()
    {
        return SendMessage(this->m_hWnd, SCI_SETIDLESTYLING, 0, 0);
    }

    LRESULT SetScrollWidthTracking(BOOL enable)
    {
        return SendMessage(this->m_hWnd, SCI_SETSCROLLWIDTHTRACKING, enable, 0);
    }

    LRESULT SetIndent(int indent)
    {
        return SendMessage(this->m_hWnd, SCI_SETINDENT, indent, 0);
    }

    LRESULT SetIndentationGuides(int indentView)
    {
        return SendMessage(this->m_hWnd, SCI_SETINDENTATIONGUIDES, indentView, 0);
    }

    LRESULT SetText(LPCSTR text)
    {
        return SendMessage(this->m_hWnd, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(text));
    }

    CStringA GetText()
    {
        int length = static_cast<int>(SendMessage(this->m_hWnd, SCI_GETTEXTLENGTH, 0, 0));
        if (length == 0) {
            return CStringA();
        }

        CStringA contents;
        contents.GetBufferSetLength(length + 1);
        SendMessage(this->m_hWnd, SCI_GETTEXT, length, reinterpret_cast<LPARAM>(contents.GetBuffer()));
        contents.ReleaseBuffer();

        return contents;
    }

    LRESULT SetReadOnly(BOOL readOnly)
    {
        return SendMessage(this->m_hWnd, SCI_SETREADONLY, readOnly, 0);
    }

    LRESULT SetKeywords(int keywordSet, LPCSTR keywords)
    {
        return SendMessage(this->m_hWnd, SCI_SETKEYWORDS, keywordSet, reinterpret_cast<LPARAM>(keywords));
    }
};

using ScintillaCtrl = ScintillaCtrlT<CWindow>;

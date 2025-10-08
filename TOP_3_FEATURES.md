# Top 3 Recommended Features (Detailed Implementation Guide)

## 🥇 #1: Lua Script Syntax Highlighting

### Why This Feature?
- **High Value:** Lua is the primary scripting language for BG3 game logic
- **Low Effort:** Scintilla already has Lua lexer support built-in
- **Quick Win:** Can be implemented in ~1-2 hours

### Implementation Steps

1. **Add LuaStyler class** (`BG3ModStudio/DocStyler.h`):
```cpp
class LuaStyler : public DocStyler
{
public:
    void Apply(ScintillaCtrl& ctrl) override;
};
```

2. **Implement styling** (`BG3ModStudio/DocStyler.cpp`):
```cpp
void LuaStyler::Apply(ScintillaCtrl& ctrl)
{
    ctrl.SetLexer("lua");
    ctrl.SetIndent(4);
    ctrl.SetTabWidth(4);
    ctrl.SetUseTabs(FALSE);
    ctrl.SetScrollWidthTracking(TRUE);
    ctrl.SetMarginType(0, SC_MARGIN_NUMBER);
    ctrl.SetMarginWidth(0, 40);

    // Lua syntax colors
    ctrl.SetForeStyle(SCE_LUA_COMMENT, RGB(0, 128, 0));        // Green comments
    ctrl.SetForeStyle(SCE_LUA_COMMENTLINE, RGB(0, 128, 0));
    ctrl.SetForeStyle(SCE_LUA_NUMBER, RGB(43, 145, 175));      // Blue numbers
    ctrl.SetForeStyle(SCE_LUA_WORD, RGB(0, 0, 255));           // Blue keywords
    ctrl.SetForeStyle(SCE_LUA_STRING, RGB(163, 21, 21));       // Red strings
    ctrl.SetForeStyle(SCE_LUA_CHARACTER, RGB(163, 21, 21));
    ctrl.SetForeStyle(SCE_LUA_LITERALSTRING, RGB(163, 21, 21));
    ctrl.SetForeStyle(SCE_LUA_OPERATOR, RGB(0, 0, 0));         // Black operators
    ctrl.SetForeStyle(SCE_LUA_WORD2, RGB(43, 145, 175));       // Teal for functions
}
```

3. **Register in DocStylerRegistry** (`BG3ModStudio/DocStyler.cpp`):
```cpp
DocStylerRegistry::DocStylerRegistry()
{
    m_stylers[".xml"] = std::make_shared<XmlStyler>();
    m_stylers[".lsx"] = std::make_shared<XmlStyler>();
    m_stylers[".json"] = std::make_shared<JsonStyler>();
    m_stylers[".lua"] = std::make_shared<LuaStyler>();    // ADD THIS LINE
    m_default = std::make_shared<PlainTextStyler>();
}
```

**Testing:** Open any `.lua` file and verify syntax highlighting works correctly.

---

## 🥈 #2: Find/Replace Dialog

### Why This Feature?
- **Essential Functionality:** Basic text editing feature currently missing
- **High User Value:** Needed for editing large Stats.txt, LSX, and script files
- **Moderate Effort:** ~4-6 hours implementation

### Implementation Steps

1. **Create dialog resource** (`BG3ModStudio/resources/BG3ModStudio.rc`):
```rc
IDD_FINDREPLACE DIALOGEX 0, 0, 320, 120
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Find and Replace"
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    LTEXT           "Find what:",IDC_STATIC,7,10,40,8
    EDITTEXT        IDC_E_FIND,50,8,200,14,ES_AUTOHSCROLL
    LTEXT           "Replace with:",IDC_STATIC,7,30,45,8
    EDITTEXT        IDC_E_REPLACE,50,28,200,14,ES_AUTOHSCROLL
    
    CONTROL         "Match case",IDC_CHK_MATCHCASE,"Button",BS_AUTOCHECKBOX,7,50,60,10
    CONTROL         "Whole word",IDC_CHK_WHOLEWORD,"Button",BS_AUTOCHECKBOX,7,65,60,10
    CONTROL         "Use regex",IDC_CHK_REGEX,"Button",BS_AUTOCHECKBOX,7,80,60,10
    
    PUSHBUTTON      "Find Next",IDC_B_FINDNEXT,260,8,50,14
    PUSHBUTTON      "Replace",IDC_B_REPLACE,260,28,50,14
    PUSHBUTTON      "Replace All",IDC_B_REPLACEALL,260,48,50,14
    PUSHBUTTON      "Close",IDCANCEL,260,98,50,14
END
```

2. **Create FindReplaceDlg class** (`BG3ModStudio/FindReplaceDlg.h`):
```cpp
#pragma once

#include "ModelessDialog.h"
#include "ScintillaCtrl.h"
#include "resources/resource.h"

class FindReplaceDlg : public ModelessDialog<FindReplaceDlg>
{
public:
    enum { IDD = IDD_FINDREPLACE };
    
    void SetTarget(ScintillaCtrl* ctrl) { m_target = ctrl; }
    
    BEGIN_MSG_MAP(FindReplaceDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        COMMAND_ID_HANDLER3(IDC_B_FINDNEXT, OnFindNext)
        COMMAND_ID_HANDLER3(IDC_B_REPLACE, OnReplace)
        COMMAND_ID_HANDLER3(IDC_B_REPLACEALL, OnReplaceAll)
        COMMAND_ID_HANDLER3(IDCANCEL, OnClose)
    END_MSG_MAP()
    
private:
    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
    void OnClose();
    void OnFindNext();
    void OnReplace();
    void OnReplaceAll();
    
    int GetSearchFlags();
    bool FindText(const std::string& text, bool forward = true);
    
    ScintillaCtrl* m_target = nullptr;
    CEdit m_findEdit;
    CEdit m_replaceEdit;
    CButton m_matchCase;
    CButton m_wholeWord;
    CButton m_useRegex;
};
```

3. **Implement search logic** (`BG3ModStudio/FindReplaceDlg.cpp`):
```cpp
bool FindReplaceDlg::FindText(const std::string& text, bool forward)
{
    if (!m_target || text.empty()) return false;
    
    int flags = GetSearchFlags();
    
    Sci_Position currentPos = m_target->GetCurrentPos();
    Sci_Position docLength = m_target->GetLength();
    
    m_target->SetSearchFlags(flags);
    m_target->SetTargetRange(forward ? currentPos : 0, 
                            forward ? docLength : currentPos);
    
    Sci_Position pos = m_target->SearchInTarget(text.length(), text.c_str());
    
    if (pos >= 0) {
        m_target->SetSel(m_target->GetTargetStart(), m_target->GetTargetEnd());
        m_target->ScrollCaret();
        return true;
    }
    
    return false;
}
```

4. **Add keyboard shortcuts** (`BG3ModStudio/MainFrame.h`):
```cpp
// In MainFrame message map:
COMMAND_ID_HANDLER3(ID_EDIT_FIND, OnEditFind)
COMMAND_ID_HANDLER3(ID_EDIT_REPLACE, OnEditReplace)

// Add accelerators for Ctrl+F and Ctrl+H
```

**Testing:** Test find/replace with various options on LSX, Stats.txt, and Lua files.

---

## 🥉 #3: Stats.txt Validator

### Why This Feature?
- **Prevents Errors:** Catches syntax errors before in-game testing
- **Learning Aid:** Helps modders understand Stats.txt format
- **Moderate Effort:** ~6-8 hours including parser

### Implementation Steps

1. **Create Stats parser** (`Utility/StatsParser.h`):
```cpp
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct StatsEntry
{
    std::string type;           // "new entry", "data"
    std::string name;           // Entry name
    int lineNumber;
    std::unordered_map<std::string, std::string> fields;
};

struct StatsError
{
    int lineNumber;
    std::string message;
    enum class Severity { Error, Warning, Info } severity;
};

class StatsParser
{
public:
    bool Parse(const std::string& content);
    const std::vector<StatsEntry>& GetEntries() const { return m_entries; }
    const std::vector<StatsError>& GetErrors() const { return m_errors; }
    
private:
    void ParseLine(const std::string& line, int lineNumber);
    void ValidateEntry(const StatsEntry& entry);
    bool IsValidStatType(const std::string& type);
    
    std::vector<StatsEntry> m_entries;
    std::vector<StatsError> m_errors;
    StatsEntry m_currentEntry;
};
```

2. **Implement validation rules** (`Utility/StatsParser.cpp`):
```cpp
void StatsParser::ValidateEntry(const StatsEntry& entry)
{
    // Check for required fields based on type
    static const std::unordered_map<std::string, std::vector<std::string>> requiredFields = {
        {"Weapon", {"Damage", "Weight"}},
        {"Armor", {"ArmorType", "ArmorClass"}},
        {"StatusData", {"StatusType"}},
        // Add more types...
    };
    
    auto it = requiredFields.find(entry.type);
    if (it != requiredFields.end()) {
        for (const auto& field : it->second) {
            if (entry.fields.find(field) == entry.fields.end()) {
                m_errors.push_back({
                    entry.lineNumber,
                    "Missing required field: " + field,
                    StatsError::Severity::Error
                });
            }
        }
    }
    
    // Validate GUID format
    auto usingField = entry.fields.find("using");
    if (usingField != entry.fields.end()) {
        if (!IsValidGUID(usingField->second)) {
            m_errors.push_back({
                entry.lineNumber,
                "Invalid GUID format: " + usingField->second,
                StatsError::Severity::Warning
            });
        }
    }
}
```

3. **Create validator dialog** (`BG3ModStudio/StatsValidatorDlg.h`):
```cpp
class StatsValidatorDlg : public ModelessDialog<StatsValidatorDlg>
{
public:
    enum { IDD = IDD_STATS_VALIDATOR };
    
    void ValidateFile(const CString& path);
    
private:
    CListViewCtrl m_listErrors;
    // Display errors with line numbers, click to jump to line
};
```

4. **Integrate with TextFileView** (`BG3ModStudio/TextFileView.cpp`):
```cpp
// Add menu item or toolbar button to validate current Stats.txt file
void TextFileView::OnValidateStats()
{
    if (!m_path.Right(9).CompareNoCase(L"Stats.txt")) {
        StatsParser parser;
        if (parser.Parse(GetText())) {
            // Show errors in validator dialog
            ShowValidationResults(parser.GetErrors());
        }
    }
}
```

**Testing:** Create test Stats.txt files with various errors and verify they're caught correctly.

---

## Next Steps

1. **Start with Lua highlighting** - Quick implementation, immediate value
2. **Add Find/Replace** - Essential feature that users will appreciate
3. **Implement Stats validator** - Significant value for mod developers

Each feature builds on the existing architecture and follows established patterns in the codebase. They can be implemented independently and tested incrementally.

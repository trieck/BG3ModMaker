# Feature Suggestions for BG3ModMaker

Based on analysis of the current codebase and the existing feature set, here are recommended features to add next, prioritized by value and implementation complexity:

## High Priority Features

### 1. **Lua Script Syntax Highlighting and Editor**
**Rationale:** BG3 uses Lua extensively for scripting. Adding Lua syntax highlighting would greatly improve the modding experience.

**Implementation:**
- Add `LuaStyler` class in `DocStyler.cpp/h` (similar to existing `XmlStyler` and `JsonStyler`)
- Scintilla already supports Lua lexer (`SCE_LUA_*` constants)
- Register `.lua` extension in `DocStylerRegistry`
- Minimal code changes required (~50-100 lines)

**Value:** High - Lua is core to BG3 modding for game logic and scripting

---

### 2. **Find/Replace Dialog for Text Editor**
**Rationale:** The text editor (TextFileView with Scintilla) lacks basic find/replace functionality that's essential for editing large files.

**Implementation:**
- Create `FindReplaceDlg` class (similar to existing `SearchDlg`, `UUIDDlg`)
- Use Scintilla's built-in search APIs (`SCI_FINDTEXT`, `SCI_SEARCHINTARGET`)
- Add menu items and keyboard shortcuts (Ctrl+F, Ctrl+H)
- Implement find next/previous, replace, replace all

**Value:** High - Essential text editing feature, currently missing

---

### 3. **Stats.txt Validator/Linter**
**Rationale:** BG3's Stats.txt files have specific syntax requirements. A validator would help modders catch errors before testing.

**Implementation:**
- Parse Stats.txt format and validate structure
- Check for common errors (missing fields, invalid types, duplicate entries)
- Display errors in OutputWindow with line numbers
- Integrate with existing file views

**Value:** High - Reduces trial-and-error in mod development

---

## Medium Priority Features

### 4. **Diff Viewer for LSX/LSF Files**
**Rationale:** When modding, comparing your changes to vanilla files is crucial. A visual diff would help understand what changed.

**Implementation:**
- Create `DiffView` class with side-by-side or unified diff display
- Use existing LSX/LSF parsing from `LibLS`
- Compare two files selected from folder tree
- Highlight additions, deletions, modifications

**Value:** Medium - Useful for understanding mod conflicts and changes

---

### 5. **Bookmark System for Files**
**Rationale:** Modders often work with the same set of files. Quick access to bookmarked files would improve workflow.

**Implementation:**
- Add bookmark menu to file tabs context menu
- Store bookmarks in settings file
- Add bookmarks panel or dropdown in toolbar
- Persist across sessions

**Value:** Medium - Quality of life improvement for workflow

---

### 6. **PAK File Comparison Tool**
**Rationale:** Compare contents of two PAK files to see what's different between mod versions or between mod and vanilla.

**Implementation:**
- Extend existing `PAKReader` functionality
- Create dialog to select two PAK files
- List added/removed/modified files
- Allow extracting differences

**Value:** Medium - Helps with mod compatibility and debugging

---

## Lower Priority Features

### 7. **LSX/LSF Format Conversion in Context Menu**
**Rationale:** Currently requires using Tools menu. Context menu access would be more convenient.

**Implementation:**
- Add context menu items to tree view for .lsx/.lsf files
- Use existing conversion functions from `Compress` class
- Show result in output window

**Value:** Low - Convenience improvement, functionality already exists

---

### 8. **Auto-complete for Common BG3 Identifiers**
**Rationale:** Speed up editing by suggesting common GUIDs, stat names, spell names from the index.

**Implementation:**
- Query Xapian index for common identifiers
- Hook into Scintilla's autocompletion (`SCI_AUTOCSHOW`)
- Trigger on typing patterns (e.g., "GUID_" or stat types)

**Value:** Low-Medium - Nice to have, but requires index to be built

---

### 9. **Localization File Editor (Enhanced)**
**Rationale:** Improve the current basic loca support with a dedicated editor for translations.

**Implementation:**
- Create dedicated `LocalizationEditor` dialog
- Table view with columns for handle, version, text
- Filter by language
- Export/import from XML

**Value:** Low - Only useful for mods with custom text

---

### 10. **Git Integration**
**Rationale:** Many modders use version control. Basic Git operations from the UI would be helpful.

**Implementation:**
- Add Git status icons to folder tree
- Basic commit/push/pull operations
- Show changes in output window
- Use libgit2 library

**Value:** Low - Many users prefer external Git tools

---

## Recommendations

**Suggested Priority Order:**
1. **Lua Script Editor** - Quick win with high value
2. **Find/Replace Dialog** - Essential missing feature
3. **Stats.txt Validator** - Reduces modding errors significantly

These three features provide the best value-to-effort ratio and address current gaps in the tool's functionality. They align with the existing architecture and UI patterns, making them relatively straightforward to implement while providing substantial improvements to the modding workflow.

---

## Implementation Notes

All features should:
- Follow existing code patterns (WTL, ATL, Modern C++)
- Use existing infrastructure (Scintilla, Xapian, RocksDB)
- Maintain consistency with current UI design
- Include appropriate error handling
- Update documentation in README.md

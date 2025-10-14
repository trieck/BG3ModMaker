# BG3ModMaker

**BG3ModMaker** is a suite of tools for creating mods in **Baldur’s Gate 3**.
It’s written in modern C++ (WTL/DirectX/DirectXTex) with a focus on speed, transparency, and being fully self-contained (no dependency on LSLib or .NET).

![image](assets/bg3-mod-studio.png)

---

## ✨ Features
- **Mod Editor**
  - Multi-tab interface for low-level mod editing
  - Explorer-type folder interface

- **Stats + LSX/LSF Indexing**
  - Full-text indexing of Gustav.pak and other content with **Xapian**
  - Fast search across stats, passives, spells, icons, and more

- **Icon Explorer**
  - Maps `IconUV` atlas references to cropped icons
  - Stores icons in **RocksDB** for fast lookup
  - Supports DDS and PNG (via DirectXTex + WIC)

- **Image Viewer**
  - Zoomable, scrollable Direct2D-based viewer
  - Supports DDS, PNG, JPG, BMP, TGA, GIF, HDR (and more)

- **GameObject Explorer**
  - Explore root templates, stats bindings, localization, and icons by UUID
  - Quickly see how items and abilities are constructed

- **PAK Explorer**
  - Explore PAK file contents without requiring exploding to the filesystem

---

## 📂 Project Structure

- `BG3ModStudio/` — main GUI application (WTL/Direct2D)
- `Catalog/` — constructs a game object rocksdb database from PAK files
- `Iconizer` — constructs an icon rocksdb database from PAK files 
- `LibLS/` — library for reading/writing LSX, LSF, and PAK formats
- `Index/` — Xapian-based indexer for BG3 resources
- `Utility/` — shared utility code`

---

## 🛠️ Building

### Requirements
- Visual Studio 2022 (C++20 or later)
- DirectXTex (for DDS/WIC image handling)
- Xapian (for indexing)
- RocksDB (for icon storage)

### Steps
```bash
git clone https://github.com/trieck/BG3ModMaker.git
cd BG3ModMaker
# open the solution in Visual Studio and build
```

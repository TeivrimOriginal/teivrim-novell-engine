# Novell Engine

Visual novel editor with timeline, script panel, hierarchy, scene editor, inspector, and asset browser.

## Controls

| Key | Action |
|-----|--------|
| F1 | Toggle debug overlay |
| F5 | Toggle preview mode |
| F11 | Fullscreen |
| Shift+drag | Snap-to-grid (20px) |
| Ctrl+S | Save project |
| Ctrl+O | Open project |
| Scroll wheel | Scroll panels / zoom scene |
| Right-click | Context menu (add objects) |
| Drag files from Explorer | Import assets |

## Build

```bat
build.bat
```

Requires: MinGW g++ (14.2.0+), Windows SDK (gdi32, gdiplus, comctl32, comdlg32)

## Build Status

Working features:
- Drag-and-drop from Windows Explorer
- Editor panels: Hierarchy, Scene, Inspector, Script, Asset Browser, Timeline
- Object lifespan with tick system
- Snap-to-grid while holding Shift
- Color picker in Inspector (RGB fields)
- Script line creation from hierarchy objects
- Save/Load project files
- Preview mode with dialogue playback per tick

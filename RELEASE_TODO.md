# Release TODO

## Critical Bugs
- [x] Fix: objects added via context menu use wrong coordinates (ctxMenu.mx/my)
- [x] Fix: asset drop overlaps with object selection (check existing first)
- [x] Fix: panel click dispatch order (rect vs contentRect)
- [x] Fix: HandleEvents called before Layout (panels had zero size)
- [x] Fix: dragSplitter never reset (stuck on forever)

## Editor Features
- [x] Add "Add Object" button in Hierarchy panel ([R][E][T][+])
- [ ] Make top menu File > Import Assets open file dialog
- [x] Proper drag-and-drop from Asset Browser (selectedAsset + click-to-place)
- [ ] Undo/Redo system
- [ ] Save/Load project scenes
- [ ] Multiple selection in Hierarchy (Ctrl+click)

## Scene Panel
- [x] Proper ellipse rendering (raster ellipse via horizontal strips)
- [ ] Image/sprite rendering from file
- [ ] Snap-to-grid option
- [x] Pan with left-click drag on empty space

## Inspector
- [ ] Color picker for objects
- [ ] Add/remove components

## Script Panel
- [ ] Script saving/loading
- [x] Script line editing (inline text edit)
- [ ] Character/sprite/background picker per line

## Asset Browser
- [x] Folder navigation (subdirectories)
- [ ] Image preview thumbnails
- [x] Delete/rename assets
- [ ] File drag from Explorer into window

## Stability & Debug
- [x] Debug overlay (F1 toggles bounds/coords/stats/todo)
- [ ] Memory leak check
- [ ] Handle window resize properly
- [ ] Crash on missing assets/Images folder

## Release
- [ ] Installer/packaged build
- [ ] Clean up old/unused source files (removed Panels.cpp, ObjectUI.cpp, etc.)
- [x] Build script (build.bat)
- [ ] README with controls

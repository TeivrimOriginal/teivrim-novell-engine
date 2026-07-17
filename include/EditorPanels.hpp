#pragma once
#include "GraphicsAPI.hpp"
#include "ProjectManager.hpp"
#include <string>
#include <vector>
#include <functional>
#include <cstdlib>

struct Rect {
    int x, y, w, h;
    bool Contains(int px, int py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
};

struct ContextMenu {
    bool active;
    int mx, my;
    std::vector<std::string> items;
    int hovered;
    ContextMenu() : active(false), mx(0), my(0), hovered(-1) {}
    void Open(int x, int y, const std::vector<std::string>& opts) { active = true; mx = x; my = y; items = opts; hovered = -1; }
    void Close() { active = false; items.clear(); }
    int HitTest(int x, int y) {
        if (!active) return -1;
        if (x < mx || x > mx + 160 || y < my || y > my + (int)items.size() * 24) return -1;
        return (y - my) / 24;
    }
    void Draw(GraphicsAPI* gfx) {
        if (!active) return;
        gfx->DrawRect(mx, my, 160, (int)items.size() * 24, 45, 45, 55);
        for (int i = 0; i < (int)items.size(); i++) {
            int iy = my + i * 24;
            if (i == hovered) gfx->DrawRect(mx + 2, iy, 156, 22, 55, 80, 120);
            gfx->RenderText(items[i], mx + 8, iy + 3, 200, 200, 220);
        }
    }
};

struct EditorObject {
    std::string name, type, spritePath;
    int posX, posY, sizeW, sizeH;
    std::string text;
    bool visible;
    int colorR, colorG, colorB;
    int startTick, endTick;
    EditorObject() : posX(50), posY(50), sizeW(100), sizeH(100), visible(true), colorR(70), colorG(80), colorB(100), startTick(0), endTick(9999) {}
};

struct ScriptLine {
    std::string text, characterName, spritePath, backgroundPath;
    int tick;
    ScriptLine() : tick(0) {}
};

enum GizmoPart { GIZMO_NONE, GIZMO_MOVE, GIZMO_RESIZE_TL, GIZMO_RESIZE_TR, GIZMO_RESIZE_BL, GIZMO_RESIZE_BR };

struct EditorPanel {
    std::string title;
    Rect rect, contentRect;
    bool visible;
    EditorPanel(const std::string& t) : title(t), visible(true) {}
    virtual ~EditorPanel() = default;
    virtual void Draw(GraphicsAPI* gfx) = 0;
    virtual bool HandleClick(int mx, int my) = 0;
    void SetRect(const Rect& r) { rect = r; contentRect = {r.x + 2, r.y + 22, r.w - 4, r.h - 24}; }
    void DrawTitleBar(GraphicsAPI* gfx) {
        gfx->DrawRect(rect.x, rect.y, rect.w, 20, 50, 50, 60);
        gfx->RenderText(title, rect.x + 6, rect.y + 2, 200, 200, 220);
        gfx->DrawRect(rect.x, rect.y + 20, rect.w, 1, 70, 70, 80);
    }
    void DrawFrame(GraphicsAPI* gfx) {
        DrawTitleBar(gfx);
        gfx->DrawRect(rect.x, rect.y, 1, rect.h, 60, 60, 70);
        gfx->DrawRect(rect.x + rect.w - 1, rect.y, 1, rect.h, 60, 60, 70);
        gfx->DrawRect(rect.x, rect.y + rect.h - 1, rect.w, 1, 60, 60, 70);
    }
};

struct HierarchyPanel : EditorPanel {
    std::vector<EditorObject> objects;
    int selectedIndex, scrollOffset;
    std::vector<int> multiSel;
    HierarchyPanel();
    void Draw(GraphicsAPI* gfx) override;
    bool HandleClick(int mx, int my) override;
    int AddObject(const std::string& name, const std::string& type, int px = -1, int py = -1);
    void DeleteSelected();
    void ToggleVisible();
    void SyncMulti();
    bool IsMulti(int i) const;
};

struct InspectorPanel : EditorPanel {
    EditorObject* target;
    int editingField;
    std::string editBuf;
    InspectorPanel();
    void Draw(GraphicsAPI* gfx) override;
    bool HandleClick(int mx, int my) override;
    void CommitEdit();
    void SetTarget(EditorObject* obj);
};

struct AssetBrowserPanel : EditorPanel {
    std::vector<std::string> files;
    int scrollOffset;
    bool selectedAsset;
    int selAssetIndex;
    std::string selAssetPath;
    std::string currentDir;
    int editingField, editingIndex;
    std::string editBuf;
    AssetBrowserPanel();
    void ScanDirectory(const std::string& dir);
    void ScanDirectory();
    void Draw(GraphicsAPI* gfx) override;
    bool HandleClick(int mx, int my) override;
    int GetFileIndexAt(int mx, int my) const;
    void CancelSelect();
    void HandleKey(int key);
};

struct ScenePanel : EditorPanel {
    int selectedIndex;
    GizmoPart activeGizmo;
    int gizmoStartX, gizmoStartY, gizmoObjX, gizmoObjY, gizmoObjW, gizmoObjH;
    float zoom;
    int panX, panY;
    ScenePanel();
    void Draw(GraphicsAPI* gfx) override;
    bool HandleClick(int mx, int my) override;
    void HandleDrag(int mx, int my);
    void HandleScroll(int delta);
    void DrawGrid(GraphicsAPI* gfx);
    GizmoPart HitTestGizmo(int mx, int my, EditorObject* obj);
    void DrawGizmo(GraphicsAPI* gfx, EditorObject* obj);
};

struct ScriptPanel : EditorPanel {
    std::vector<ScriptLine> lines;
    int selectedLine, scrollOffset;
    int editingLine, editingField;
    std::string editBuf;
    ScriptPanel();
    void Draw(GraphicsAPI* gfx) override;
    bool HandleClick(int mx, int my) override;
    void AddLine();
    void DeleteLine();
    void MoveUp();
    void MoveDown();
    void HandleKey(int key);
};

struct TimelinePanel : EditorPanel {
    int scrollOffset;
    int dragMode, dragObj;
    int dragStartTick, dragEndTick, dragMouseStartX;
    TimelinePanel();
    void Draw(GraphicsAPI* gfx) override;
    void Draw(GraphicsAPI* gfx, const std::vector<EditorObject>& objects, int selectedIndex, int currentTick);
    bool HandleClick(int mx, int my) override;
    bool HandleClick(int mx, int my, std::vector<EditorObject>& objects, int& selectedIndex, int& currentTick);
    void HandleDrag(int mx, int my, std::vector<EditorObject>& objects);
};

struct EditorSnapshot {
    std::vector<EditorObject> objects;
    std::vector<ScriptLine> lines;
};

struct EditorWindow {
    HierarchyPanel hierarchy;
    InspectorPanel inspector;
    AssetBrowserPanel assetBrowser;
    ScenePanel scenePanel;
    ScriptPanel scriptPanel;
    TimelinePanel timeline;

    int currentTick;

    Rect splitterR1, splitterR2, splitterR3, splitterR4;
    int dragSplitter;
    float leftRatio, rightRatio, bottomRatio, timelineRatio;

    ContextMenu ctxMenu;

    int activeMenu;
    std::vector<std::string> menuItems;
    int menuHovered;

    bool showDebug;
    bool scenePanning;
    int panStartX, panStartY, panStartPanX, panStartPanY;
    std::string lastMsg;
    int lastMsgTimer;

    // Preview mode
    bool previewMode;
    int previewLine;

    int snapSize;
    std::vector<EditorSnapshot> undoStack, redoStack;

    EditorWindow();
    void Layout(int winW, int winH);
    void Draw(GraphicsAPI* gfx);
    void DrawPreview(GraphicsAPI* gfx);
    void HandleEvents(const std::vector<WindowEvent>& events);
    void HandleClick(int mx, int my);
    void HandleRClick(int mx, int my);
    void HandleDrag(int mx, int my);
    void HandleScroll(int delta, int mouseY);
    void HandleKey(int key);
    int HitTestSplitter(int mx, int my);
    void AddShape(const std::string& type, int mx, int my);
    void SyncSelection();
    void DrawMenuBar(GraphicsAPI* gfx);
    void HandleMenuClick(int mx, int my);
    void DrawSceneObjects(GraphicsAPI* gfx);
    void DrawSplitters(GraphicsAPI* gfx);
    void DrawDebugOverlay(GraphicsAPI* gfx);
    void HandleFileDrop(const std::string& filePath);
    void PushUndo();
    void Undo();
    void Redo();
};

#include "EditorPanels.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif
namespace fs = std::filesystem;

// ========== HIERARCHY ==========
HierarchyPanel::HierarchyPanel() : EditorPanel("Hierarchy"), selectedIndex(-1), scrollOffset(0) {}

bool HierarchyPanel::IsMulti(int i) const {
    for (auto& s : multiSel) if (s == i) return true;
    return false;
}

void HierarchyPanel::SyncMulti() {
    multiSel.clear();
    if (selectedIndex >= 0 && selectedIndex < (int)objects.size()) multiSel.push_back(selectedIndex);
}

void HierarchyPanel::Draw(GraphicsAPI* gfx) {
    DrawFrame(gfx);
    int y = contentRect.y + 4;
    int maxY = rect.y + rect.h - 26;
    for (int i = scrollOffset; i < (int)objects.size(); i++) {
        if (y + 24 > maxY) break;
        int rowX = contentRect.x + 2, rowW = contentRect.w - 4;
        if (i == selectedIndex) gfx->DrawRect(rowX, y, rowW, 22, 55, 80, 120);
        else if (IsMulti(i)) gfx->DrawRect(rowX, y, rowW, 22, 50, 60, 90);
        std::string label = objects[i].name;
        if (label.size() > 20) label = label.substr(0, 19) + ".";
        gfx->RenderText(label, rowX + 4, y + 2, objects[i].visible ? 200 : 100, objects[i].visible ? 200 : 100, objects[i].visible ? 220 : 120);
        y += 24;
    }
    if (objects.empty()) gfx->RenderText("No objects", contentRect.x + 8, contentRect.y + 6, 80, 80, 100);
    int tby = rect.y + rect.h - 24;
    gfx->DrawRect(rect.x + 1, tby, rect.w - 2, 23, 45, 45, 55);
    int bw = std::max(40, (rect.w - 6) / 4);
    gfx->RenderText("[R]", rect.x + 4, tby + 3, 150, 200, 150);
    gfx->RenderText("[E]", rect.x + 6 + bw, tby + 3, 200, 200, 150);
    gfx->RenderText("[T]", rect.x + 8 + bw*2, tby + 3, 150, 150, 200);
    gfx->RenderText("[+]", rect.x + 10 + bw*3, tby + 3, 200, 150, 150);
}

bool HierarchyPanel::HandleClick(int mx, int my) {
    int tby = rect.y + rect.h - 24;
    if (my >= tby && my <= tby + 23 && mx >= rect.x && mx <= rect.x + rect.w) {
        int bw = std::max(40, (rect.w - 6) / 4);
        if (mx < rect.x + 6 + bw) { AddObject("Rectangle", "rectangle"); SyncMulti(); return true; }
        if (mx < rect.x + 8 + bw*2) { AddObject("Ellipse", "ellipse"); SyncMulti(); return true; }
        if (mx < rect.x + 10 + bw*3) { AddObject("Text", "text"); SyncMulti(); return true; }
        AddObject("Sprite", "sprite"); SyncMulti(); return true;
    }
    if (!contentRect.Contains(mx, my)) return false;
    int y = contentRect.y + 4;
    int maxY = rect.y + rect.h - 26;
#ifdef _WIN32
    bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
#else
    bool ctrl = false;
#endif
    for (int i = scrollOffset; i < (int)objects.size(); i++) {
        if (y + 24 > maxY) break;
        int rowX = contentRect.x + 2, rowW = contentRect.w - 4;
        if (mx >= rowX && mx <= rowX + rowW && my >= y && my <= y + 22) {
            int rw = rowW / 3;
            if (mx >= rowX + rowW - rw && mx < rowX + rowW) { objects.erase(objects.begin() + i); multiSel.clear(); if (selectedIndex >= (int)objects.size()) selectedIndex = (int)objects.size() - 1; return true; }
            if (mx >= rowX + rowW - rw*2 && mx < rowX + rowW - rw) { objects[i].visible = !objects[i].visible; return true; }
            if (ctrl) {
                if (IsMulti(i)) { multiSel.erase(std::remove(multiSel.begin(), multiSel.end(), i), multiSel.end()); }
                else { multiSel.push_back(i); selectedIndex = i; }
            } else {
                selectedIndex = i; SyncMulti();
            }
            return true;
        }
        y += 24;
    }
    return true;
}

int HierarchyPanel::AddObject(const std::string& name, const std::string& type, int px, int py) {
    EditorObject obj;
    obj.name = name; obj.type = type;
    obj.posX = (px >= 0) ? px : (50 + ((int)objects.size() * 30) % 400);
    obj.posY = (py >= 0) ? py : (50 + ((int)objects.size() * 30) % 300);
    obj.sizeW = 100; obj.sizeH = 100; obj.visible = true;
    if (type == "text") { obj.sizeW = 200; obj.sizeH = 30; obj.text = "Text"; obj.colorR = 100; obj.colorG = 100; obj.colorB = 130; }
    if (type == "ellipse") { obj.sizeW = 100; obj.sizeH = 100; obj.colorR = 80; obj.colorG = 70; obj.colorB = 90; }
    if (type == "rectangle") { obj.colorR = 120; obj.colorG = 100; obj.colorB = 80; }
    if (type == "sprite") { obj.colorR = 60; obj.colorG = 80; obj.colorB = 100; }
    if (type == "background") { obj.colorR = 30; obj.colorG = 40; obj.colorB = 50; obj.sizeW = 800; obj.sizeH = 600; }
    objects.push_back(obj);
    selectedIndex = (int)objects.size() - 1;
    SyncMulti();
    return selectedIndex;
}

void HierarchyPanel::DeleteSelected() {
    if (objects.empty()) return;
    // Delete all multi-selected (including primary)
    if (!multiSel.empty()) {
        std::sort(multiSel.begin(), multiSel.end(), std::greater<int>());
        for (int idx : multiSel) if (idx >= 0 && idx < (int)objects.size()) objects.erase(objects.begin() + idx);
        multiSel.clear();
    } else if (selectedIndex >= 0 && selectedIndex < (int)objects.size()) {
        objects.erase(objects.begin() + selectedIndex);
    }
    if (selectedIndex >= (int)objects.size()) selectedIndex = (int)objects.size() - 1;
}

void HierarchyPanel::ToggleVisible() {
    if (!multiSel.empty()) {
        for (int idx : multiSel) if (idx >= 0 && idx < (int)objects.size()) objects[idx].visible = !objects[idx].visible;
    } else if (selectedIndex >= 0 && selectedIndex < (int)objects.size()) {
        objects[selectedIndex].visible = !objects[selectedIndex].visible;
    }
}

// ========== INSPECTOR ==========
InspectorPanel::InspectorPanel() : EditorPanel("Inspector"), target(nullptr), editingField(0) {}

void InspectorPanel::Draw(GraphicsAPI* gfx) {
    DrawFrame(gfx);
    if (!target) { gfx->RenderText("Select an object", contentRect.x + 6, contentRect.y + 6, 100, 100, 120); return; }
    int y = contentRect.y + 6, lx = contentRect.x + 6;
    auto fld = [&](int id, const std::string& label, const std::string& val, int x, int y, int fw) {
        gfx->RenderText(label, x, y, 140, 140, 160);
        int fx = x + 55, fy = y, fw2 = fw > 0 ? fw : contentRect.w - 65;
        if (editingField == id) {
            gfx->DrawRect(fx, fy, fw2, 20, 80, 60, 40);
            gfx->RenderText(editBuf, fx + 4, fy + 2, 255, 255, 200);
        } else {
            gfx->DrawRect(fx, fy, fw2, 20, 40, 40, 50);
            gfx->RenderText(val, fx + 4, fy + 2, 200, 200, 200);
        }
    };
    fld(1, "Name:", target->name, lx, y, 0); y += 26;
    gfx->RenderText("Type:", lx, y, 140, 140, 160); gfx->RenderText(target->type, lx + 55, y + 2, 180, 180, 200); y += 26;
    fld(2, "PosX:", std::to_string(target->posX), lx, y, 60); y += 26;
    fld(3, "PosY:", std::to_string(target->posY), lx, y, 60); y += 26;
    fld(4, "W:", std::to_string(target->sizeW), lx, y, 60); y += 26;
    fld(5, "H:", std::to_string(target->sizeH), lx, y, 60); y += 26;
    fld(6, "Sprite:", target->spritePath, lx, y, 0); y += 26;
    gfx->RenderText("Color:", lx, y, 140, 140, 160);
    gfx->DrawRect(lx + 55, y, 20, 20, target->colorR, target->colorG, target->colorB);
    fld(7, "R:", std::to_string(target->colorR), lx + 80, y, 30);
    fld(8, "G:", std::to_string(target->colorG), lx + 130, y, 30);
    fld(9, "B:", std::to_string(target->colorB), lx + 180, y, 30); y += 26;
    fld(10, "StartT:", std::to_string(target->startTick), lx, y, 50); y += 26;
    fld(11, "EndT:", std::to_string(target->endTick), lx, y, 50); y += 26;
    gfx->RenderText("Vis: " + std::string(target->visible ? "Yes" : "No"), lx, y, 140, 140, 160);
}

bool InspectorPanel::HandleClick(int mx, int my) {
    if (!contentRect.Contains(mx, my)) { editingField = 0; return false; }
    if (!target) return true;
    int y = contentRect.y + 6, lx = contentRect.x + 6;
    auto fldR = [&](int id, int y) -> Rect { return {lx + 61, y, contentRect.w - 70, 20}; };
    if (fldR(1, y).Contains(mx, my)) { editingField = 1; editBuf = target->name; return true; } y += 26;
    y += 26;
    if (fldR(2, y).Contains(mx, my)) { editingField = 2; editBuf = std::to_string(target->posX); return true; } y += 26;
    if (fldR(3, y).Contains(mx, my)) { editingField = 3; editBuf = std::to_string(target->posY); return true; } y += 26;
    if (fldR(4, y).Contains(mx, my)) { editingField = 4; editBuf = std::to_string(target->sizeW); return true; } y += 26;
    if (fldR(5, y).Contains(mx, my)) { editingField = 5; editBuf = std::to_string(target->sizeH); return true; } y += 26;
    if (fldR(6, y).Contains(mx, my)) { editingField = 6; editBuf = target->spritePath; return true; } y += 26;
    // R/G/B are on same line as "Color:" label
    Rect rf7 = {lx + 80, y, 50, 20}; Rect rf8 = {lx + 130, y, 50, 20}; Rect rf9 = {lx + 180, y, 50, 20};
    if (rf7.Contains(mx, my)) { editingField = 7; editBuf = std::to_string(target->colorR); return true; }
    if (rf8.Contains(mx, my)) { editingField = 8; editBuf = std::to_string(target->colorG); return true; }
    if (rf9.Contains(mx, my)) { editingField = 9; editBuf = std::to_string(target->colorB); return true; }
    y += 26;
    if (fldR(10, y).Contains(mx, my)) { editingField = 10; editBuf = std::to_string(target->startTick); return true; } y += 26;
    if (fldR(11, y).Contains(mx, my)) { editingField = 11; editBuf = std::to_string(target->endTick); return true; } y += 26;
    editingField = 0;
    return true;
}

void InspectorPanel::CommitEdit() {
    if (editingField == 0 || !target) return;
    if (editingField == 1) target->name = editBuf;
    else if (editingField == 2) target->posX = atoi(editBuf.c_str());
    else if (editingField == 3) target->posY = atoi(editBuf.c_str());
    else if (editingField == 4) target->sizeW = atoi(editBuf.c_str());
    else if (editingField == 5) target->sizeH = atoi(editBuf.c_str());
    else if (editingField == 6) target->spritePath = editBuf;
    else if (editingField == 7) { target->colorR = std::max(0, std::min(255, atoi(editBuf.c_str()))); }
    else if (editingField == 8) { target->colorG = std::max(0, std::min(255, atoi(editBuf.c_str()))); }
    else if (editingField == 9) { target->colorB = std::max(0, std::min(255, atoi(editBuf.c_str()))); }
    else if (editingField == 10) { target->startTick = std::max(0, atoi(editBuf.c_str())); }
    else if (editingField == 11) { target->endTick = std::max(1, atoi(editBuf.c_str())); }
    editingField = 0;
}

void InspectorPanel::SetTarget(EditorObject* obj) { target = obj; editingField = 0; }

// ========== ASSET BROWSER ==========
AssetBrowserPanel::AssetBrowserPanel() : EditorPanel("Asset Browser"), scrollOffset(0), selectedAsset(false), selAssetIndex(-1), currentDir("assets/Images"), editingField(0), editingIndex(-1) {}

void AssetBrowserPanel::CancelSelect() { selectedAsset = false; selAssetIndex = -1; selAssetPath.clear(); }

void AssetBrowserPanel::ScanDirectory(const std::string& dir) {
    printf("[ScanDir] %s cwd=", dir.c_str());
    char cwd[260]; GetCurrentDirectoryA(260, cwd); printf("%s\n", cwd);
    files.clear();
    if (!fs::exists(dir)) {
        printf("[ScanDir] dir does not exist! Attempting to create...\n");
        fs::create_directories(dir);
        if (!fs::exists(dir)) { printf("[ScanDir] could not create directory!\n"); return; }
    }
    // Add ".." if not at root
    if (dir != "assets/Images") files.push_back("..");
    int count = 0;
    for (auto& entry : fs::directory_iterator(dir)) {
        std::string name = entry.path().filename().string();
        files.push_back(name);
        count++;
    }
    std::sort(files.begin() + (files.size() - count), files.end());
    printf("[ScanDir] found %d entries in %s:\n", count, dir.c_str());
    for (auto& f : files) printf("  %s\n", f.c_str());
}

void AssetBrowserPanel::ScanDirectory() { ScanDirectory(currentDir); }

int AssetBrowserPanel::GetFileIndexAt(int mx, int my) const {
    if (!contentRect.Contains(mx, my)) return -1;
    int cols = std::max(1, contentRect.w / 90), cellW = contentRect.w / cols;
    int y = contentRect.y + 6, idx = scrollOffset;
    while (idx < (int)files.size()) {
        for (int c = 0; c < cols && idx < (int)files.size(); c++, idx++) {
            int x = contentRect.x + c * cellW + 4;
            if (mx >= x && mx <= x + cellW - 8 && my >= y && my <= y + 70) return idx;
        }
        y += 78;
        if (y > contentRect.y + contentRect.h) break;
    }
    return -1;
}

void AssetBrowserPanel::Draw(GraphicsAPI* gfx) {
    DrawFrame(gfx);
    // Show current directory path
    gfx->RenderText(currentDir, contentRect.x + 4, contentRect.y + 2, 140, 140, 160);
    int startY = contentRect.y + 18;
    int cols = std::max(1, contentRect.w / 90), cellW = contentRect.w / cols;
    int y = startY, idx = scrollOffset;
    while (idx < (int)files.size()) {
        for (int c = 0; c < cols && idx < (int)files.size(); c++, idx++) {
            int x = contentRect.x + c * cellW + 4;
            // Check if this entry is a directory
            std::string fullPath = currentDir + "/" + files[idx];
            bool isDir = (files[idx] == "..") || fs::is_directory(fullPath);
            if (isDir) {
                gfx->DrawRect(x, y, cellW - 8, 70, 40, 50, 60);
                gfx->RenderText("[DIR]", x + 4, y + 4, 100, 180, 255);
            } else {
                gfx->DrawRect(x, y, cellW - 8, 70, 35, 35, 45);
            }
            bool sel = (selectedAsset && idx == selAssetIndex);
            if (sel && !isDir) { gfx->DrawRect(x - 1, y - 1, cellW - 6, 72, 100, 200, 100); gfx->DrawRect(x - 1, y - 1, cellW - 6, 72, 255, 255, 255); }
            // Image thumbnail for image files
            if (!isDir && files[idx] != "..") {
                std::string ext = files[idx].substr(files[idx].find_last_of(".") + 1);
                for (auto& c : ext) c = (char)tolower(c);
                if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif")
                    gfx->DrawImage(x + 2, y + 2, cellW - 12, 48, fullPath);
            }
            // Rename editing
            if (editingField == 1 && editingIndex == idx) {
                gfx->DrawRect(x + 2, y + 28, cellW - 12, 20, 80, 60, 40);
                gfx->RenderText(editBuf + "|", x + 6, y + 30, 255, 255, 200);
            } else {
                std::string n = files[idx]; if (n.size() > 12) n = n.substr(0, 11) + ".";
                int tc = isDir ? 180 : 180;
                gfx->RenderText(n, x + 4, y + (isDir ? 24 : 52), 180, 180, 200);
            }
        }
        y += 78;
        if (y > contentRect.y + contentRect.h) break;
    }
    if (files.empty()) gfx->RenderText("No assets in " + currentDir, contentRect.x + 6, contentRect.y + 6, 80, 80, 100);
}

bool AssetBrowserPanel::HandleClick(int mx, int my) {
    int idx = GetFileIndexAt(mx, my);
    if (idx >= 0) {
        std::string fullPath = currentDir + "/" + files[idx];
        if (files[idx] == "..") {
            size_t pos = currentDir.find_last_of("/\\");
            if (pos != std::string::npos) { currentDir = currentDir.substr(0, pos); ScanDirectory(); }
            return true;
        }
        if (fs::is_directory(fullPath)) {
            currentDir = fullPath;
            ScanDirectory();
            return true;
        }
        selectedAsset = true; selAssetIndex = idx; selAssetPath = fullPath; return true;
    }
    CancelSelect();
    return contentRect.Contains(mx, my);
}

void AssetBrowserPanel::HandleKey(int key) {
    if (editingField == 0) return;
    if (key == VK_RETURN || key == VK_ESCAPE) {
        if (editingField == 1 && editingIndex >= 0 && editingIndex < (int)files.size() && !editBuf.empty()) {
            std::string oldPath = currentDir + "/" + files[editingIndex];
            std::string newPath = currentDir + "/" + editBuf;
            if (key == VK_RETURN && oldPath != newPath) {
                fs::rename(oldPath, newPath);
                ScanDirectory();
            }
        }
        editingField = 0; editingIndex = -1;
        return;
    }
    if (key == VK_BACK && !editBuf.empty()) { editBuf.pop_back(); }
}

// ========== SCENE PANEL ==========
ScenePanel::ScenePanel() : EditorPanel("Scene"), selectedIndex(-1), activeGizmo(GIZMO_NONE), zoom(1.0f), panX(0), panY(0) {}

void ScenePanel::DrawGrid(GraphicsAPI* gfx) {
    int gridSize = std::max(10, (int)(40 * zoom));
    int ox = contentRect.x + (panX % gridSize), oy = contentRect.y + (panY % gridSize);
    for (int x = ox; x < contentRect.x + contentRect.w; x += gridSize)
        gfx->DrawRect(x, contentRect.y, 1, contentRect.h, 35, 35, 45);
    for (int y = oy; y < contentRect.y + contentRect.h; y += gridSize)
        gfx->DrawRect(contentRect.x, y, contentRect.w, 1, 35, 35, 45);
}

GizmoPart ScenePanel::HitTestGizmo(int mx, int my, EditorObject* obj) {
    if (!obj) return GIZMO_NONE;
    int dx = contentRect.x + (int)(obj->posX * zoom) + panX, dy = contentRect.y + (int)(obj->posY * zoom) + panY;
    int sw = (int)(obj->sizeW * zoom), sh = (int)(obj->sizeH * zoom), hs = 8;
    if (Rect{dx + sw/2 - 12, dy - 20, 24, 16}.Contains(mx, my)) return GIZMO_MOVE;
    if (Rect{dx - hs, dy - hs, hs*2, hs*2}.Contains(mx, my)) return GIZMO_RESIZE_TL;
    if (Rect{dx + sw - hs, dy - hs, hs*2, hs*2}.Contains(mx, my)) return GIZMO_RESIZE_TR;
    if (Rect{dx - hs, dy + sh - hs, hs*2, hs*2}.Contains(mx, my)) return GIZMO_RESIZE_BL;
    if (Rect{dx + sw - hs, dy + sh - hs, hs*2, hs*2}.Contains(mx, my)) return GIZMO_RESIZE_BR;
    if (mx >= dx && mx <= dx + sw && my >= dy && my <= dy + sh) return GIZMO_MOVE;
    return GIZMO_NONE;
}

void ScenePanel::DrawGizmo(GraphicsAPI* gfx, EditorObject* obj) {
    if (!obj) return;
    int dx = contentRect.x + (int)(obj->posX * zoom) + panX, dy = contentRect.y + (int)(obj->posY * zoom) + panY;
    int sw = (int)(obj->sizeW * zoom), sh = (int)(obj->sizeH * zoom), hs = 6;
    // Outline (4 thin rects)
    gfx->DrawRect(dx - 2, dy - 2, sw + 4, 2, 255, 200, 50);     // top
    gfx->DrawRect(dx - 2, dy + sh, sw + 4, 2, 255, 200, 50);     // bottom
    gfx->DrawRect(dx - 2, dy, 2, sh, 255, 200, 50);              // left
    gfx->DrawRect(dx + sw, dy, 2, sh, 255, 200, 50);             // right
    // Move handle
    gfx->DrawRect(dx + sw/2 - 10, dy - 16, 20, 12, 255, 200, 50);
    // Resize handles
    gfx->DrawRect(dx - hs, dy - hs, hs*2, hs*2, 255, 200, 50);
    gfx->DrawRect(dx + sw - hs, dy - hs, hs*2, hs*2, 255, 200, 50);
    gfx->DrawRect(dx - hs, dy + sh - hs, hs*2, hs*2, 255, 200, 50);
    gfx->DrawRect(dx + sw - hs, dy + sh - hs, hs*2, hs*2, 255, 200, 50);
}

void ScenePanel::Draw(GraphicsAPI* gfx) {
    DrawFrame(gfx);
    gfx->DrawRect(contentRect.x, contentRect.y, contentRect.w, contentRect.h, 20, 20, 30);
    DrawGrid(gfx);
}

void ScenePanel::HandleScroll(int delta) {
    if (delta > 0) zoom *= 1.1f; else zoom /= 1.1f;
    if (zoom < 0.1f) zoom = 0.1f; if (zoom > 5.0f) zoom = 5.0f;
}

bool ScenePanel::HandleClick(int mx, int my) {
    if (!contentRect.Contains(mx, my)) { activeGizmo = GIZMO_NONE; return false; }
    return true;
}

void ScenePanel::HandleDrag(int mx, int my) {
}

// ========== SCRIPT PANEL ==========
ScriptPanel::ScriptPanel() : EditorPanel("Script"), selectedLine(-1), scrollOffset(0), editingLine(-1), editingField(0) {}

void ScriptPanel::Draw(GraphicsAPI* gfx) {
    DrawFrame(gfx);
    int y = contentRect.y + 4;
    for (int i = scrollOffset; i < (int)lines.size(); i++) {
        int ih = 64;
        if (y + ih > contentRect.y + contentRect.h) break;
        if (i == selectedLine) {
            gfx->DrawRect(contentRect.x + 2, y, contentRect.w - 4, ih, 50, 60, 90);
            gfx->DrawRect(contentRect.x + 2, y, 3, ih, 100, 150, 220);
        } else gfx->DrawRect(contentRect.x + 2, y, contentRect.w - 4, ih, 35, 35, 45);
        // Character name (clickable top area)
        int chw = std::min(200, contentRect.w / 2);
        bool editingChar = (editingField == 1 && editingLine == i);
        if (editingChar) {
            gfx->DrawRect(contentRect.x + 12, y + 2, chw, 20, 80, 60, 40);
            gfx->RenderText(editBuf + "|", contentRect.x + 16, y + 3, 255, 255, 200);
        } else {
            gfx->RenderText(lines[i].characterName.empty() ? "(narrator)" : lines[i].characterName, contentRect.x + 12, y + 2, 220, 200, 150);
        }
        // Dialogue text (lower area)
        bool editingText = (editingField == 2 && editingLine == i);
        if (editingText) {
            gfx->DrawRect(contentRect.x + 12, y + 22, contentRect.w - 28, 36, 80, 60, 40);
            gfx->RenderText(editBuf + "|", contentRect.x + 16, y + 24, 255, 255, 200);
        } else {
            std::string t = lines[i].text; if (t.size() > 40) t = t.substr(0, 39) + "...";
            gfx->RenderText(t, contentRect.x + 12, y + 22, 200, 200, 200);
        }
        y += ih + 2;
    }
    if (lines.empty()) {
        gfx->RenderText("No script lines. Click +Add or", contentRect.x + 8, contentRect.y + 6, 80, 80, 100);
        gfx->RenderText("RClick object > Create Script Line", contentRect.x + 8, contentRect.y + 24, 80, 80, 100);
    }
    int tby = rect.y + rect.h - 24, bw2 = rect.w / 4;
    gfx->DrawRect(rect.x + 1, tby, rect.w - 2, 23, 45, 45, 55);
    gfx->RenderText("+Add", rect.x + 4, tby + 3, 140, 200, 140);
    gfx->RenderText("Del", rect.x + 4 + bw2, tby + 3, 200, 140, 140);
    gfx->RenderText("^Up", rect.x + 4 + bw2*2, tby + 3, 140, 140, 200);
    gfx->RenderText("vDn", rect.x + 4 + bw2*3, tby + 3, 140, 140, 200);
}

bool ScriptPanel::HandleClick(int mx, int my) {
    int tby = rect.y + rect.h - 24, bw2 = rect.w / 4;
    if (my >= tby && my <= tby + 23 && mx >= rect.x && mx <= rect.x + rect.w) {
        if (mx < rect.x + bw2) { editingField = 0; AddLine(); }
        else if (mx < rect.x + bw2*2) { editingField = 0; DeleteLine(); }
        else if (mx < rect.x + bw2*3) { editingField = 0; MoveUp(); }
        else { editingField = 0; MoveDown(); }
        return false;
    }
    if (!contentRect.Contains(mx, my)) { editingField = 0; return false; }
    int y = contentRect.y + 4;
    for (int i = scrollOffset; i < (int)lines.size(); i++) {
        int ih = 64;
        if (y + ih > contentRect.y + contentRect.h) break;
        if (mx >= contentRect.x + 2 && mx <= contentRect.x + contentRect.w - 2 && my >= y && my <= y + ih) {
            selectedLine = i;
            int chw = std::min(200, contentRect.w / 2);
            // Character name area (top)
            if (my >= y + 2 && my <= y + 20 && mx >= contentRect.x + 12 && mx <= contentRect.x + 12 + chw) {
                editingLine = i; editingField = 1; editBuf = lines[i].characterName;
            } else if (my >= y + 22 && my <= y + 58) {
                editingLine = i; editingField = 2; editBuf = lines[i].text;
            } else {
                editingField = 0;
            }
            return true;
        }
        y += ih + 2;
    }
    selectedLine = -1; editingField = 0; return true;
}

void ScriptPanel::AddLine() {
    ScriptLine sl; sl.text = "New dialogue"; sl.characterName = "Char";
    sl.tick = (int)lines.size();
    lines.push_back(sl); selectedLine = (int)lines.size() - 1;
}

void ScriptPanel::DeleteLine() {
    if (selectedLine < 0 || selectedLine >= (int)lines.size()) return;
    lines.erase(lines.begin() + selectedLine);
    if (selectedLine >= (int)lines.size()) selectedLine = (int)lines.size() - 1;
}

void ScriptPanel::MoveUp() {
    if (selectedLine <= 0) return;
    std::swap(lines[selectedLine], lines[selectedLine - 1]); selectedLine--;
}

void ScriptPanel::MoveDown() {
    if (selectedLine < 0 || selectedLine >= (int)lines.size() - 1) return;
    std::swap(lines[selectedLine], lines[selectedLine + 1]); selectedLine++;
}

void ScriptPanel::HandleKey(int key) {
    if (editingField == 0) return;
    if (key == VK_RETURN || key == VK_ESCAPE) {
        if (editingLine >= 0 && editingLine < (int)lines.size()) {
            if (editingField == 1) lines[editingLine].characterName = editBuf;
            else if (editingField == 2) lines[editingLine].text = editBuf;
        }
        editingField = 0;
        return;
    }
    if (key == VK_BACK && !editBuf.empty()) {
        editBuf.pop_back();
        if (editingLine >= 0 && editingLine < (int)lines.size()) {
            if (editingField == 1) lines[editingLine].characterName = editBuf;
            else if (editingField == 2) lines[editingLine].text = editBuf;
        }
    }
}

// ========== TIMELINE ==========
TimelinePanel::TimelinePanel() : EditorPanel("Timeline"), scrollOffset(0), dragMode(0), dragObj(-1), dragStartTick(0), dragEndTick(0), dragMouseStartX(0) {}

void TimelinePanel::Draw(GraphicsAPI* gfx) { DrawFrame(gfx); }

void TimelinePanel::Draw(GraphicsAPI* gfx, const std::vector<EditorObject>& objects, int selectedIndex, int currentTick) {
    DrawFrame(gfx);
    int headerH = 20, rowH = 22, tickW = 24;
    int y = contentRect.y + headerH;
    int viewTicks = contentRect.w / tickW;
    gfx->DrawRect(contentRect.x, contentRect.y, contentRect.w, headerH, 40, 40, 50);
    for (int t = 0; t < viewTicks + 2; t++) {
        gfx->RenderText(std::to_string(t + scrollOffset), contentRect.x + 4 + t * tickW, contentRect.y + 3, 120, 120, 140);
    }
    int cx = contentRect.x + 4 + (currentTick - scrollOffset) * tickW;
    if (cx >= contentRect.x && cx <= contentRect.x + contentRect.w)
        gfx->DrawRect(cx, contentRect.y, 2, contentRect.h, 255, 200, 50);
    for (int i = 0; i < (int)objects.size(); i++) {
        int rowY = y + i * rowH;
        if (rowY + rowH > contentRect.y + contentRect.h) break;
        gfx->DrawRect(contentRect.x, rowY, contentRect.w, rowH - 1, i == selectedIndex ? 50 : 35, i == selectedIndex ? 50 : 35, i == selectedIndex ? 70 : 45);
        std::string label = objects[i].name; if (label.size() > 12) label = label.substr(0, 11) + ".";
        gfx->RenderText(label, contentRect.x + 4, rowY + 2, 180, 180, 200);
        int barX = contentRect.x + 4 + (objects[i].startTick - scrollOffset) * tickW;
        int barW = (objects[i].endTick - objects[i].startTick) * tickW;
        if (barW < 4) barW = 4;
        if (barX + barW >= contentRect.x && barX <= contentRect.x + contentRect.w) {
            int clipX = barX < contentRect.x ? contentRect.x : barX;
            int clipW = (barX + barW > contentRect.x + contentRect.w) ? contentRect.x + contentRect.w - clipX : barW - (clipX - barX);
            gfx->DrawRect(clipX, rowY + 10, clipW, rowH - 14, objects[i].colorR, objects[i].colorG, objects[i].colorB);
            gfx->DrawRect(clipX, rowY + 10, clipW, rowH - 14, 200, 200, 200);
        }
    }
}

bool TimelinePanel::HandleClick(int mx, int my) { return contentRect.Contains(mx, my); }

bool TimelinePanel::HandleClick(int mx, int my, std::vector<EditorObject>& objects, int& selectedIndex, int& currentTick) {
    if (!contentRect.Contains(mx, my)) { dragMode = 0; return false; }
    int headerH = 20, rowH = 22, tickW = 24, y = contentRect.y + headerH;
    if (my < contentRect.y + headerH) {
        int tick = (mx - contentRect.x - 4) / tickW + scrollOffset;
        if (tick >= 0) currentTick = tick;
        return true;
    }
    for (int i = 0; i < (int)objects.size(); i++) {
        int rowY = y + i * rowH;
        if (rowY + rowH > contentRect.y + contentRect.h) break;
        if (my >= rowY && my <= rowY + rowH - 1 && mx >= contentRect.x && mx <= contentRect.x + contentRect.w) {
            selectedIndex = i;
            int clickTick = (mx - contentRect.x - 4) / tickW + scrollOffset;
            if (clickTick >= 0) currentTick = clickTick;
            int barX = contentRect.x + 4 + (objects[i].startTick - scrollOffset) * tickW;
            int barW = (objects[i].endTick - objects[i].startTick) * tickW;
            if (mx >= barX && mx <= barX + 6) { dragMode = 2; dragObj = i; dragStartTick = objects[i].startTick; dragEndTick = objects[i].endTick; dragMouseStartX = mx; }
            else if (mx >= barX + barW - 6 && mx <= barX + barW) { dragMode = 3; dragObj = i; dragStartTick = objects[i].startTick; dragEndTick = objects[i].endTick; dragMouseStartX = mx; }
            else if (mx >= barX && mx <= barX + barW) { dragMode = 1; dragObj = i; dragStartTick = objects[i].startTick; dragEndTick = objects[i].endTick; dragMouseStartX = mx; }
            return true;
        }
    }
    return true;
}

void TimelinePanel::HandleDrag(int mx, int my, std::vector<EditorObject>& objects) {
    if (dragMode == 0 || dragObj < 0 || dragObj >= (int)objects.size()) { dragMode = 0; return; }
    int tickW = 24, dTick = (mx - dragMouseStartX) / tickW;
    auto& obj = objects[dragObj];
    if (dragMode == 1) {
        int newStart = dragStartTick + dTick; if (newStart < 0) newStart = 0;
        obj.startTick = newStart; obj.endTick = newStart + (dragEndTick - dragStartTick);
    } else if (dragMode == 2) {
        int newStart = dragStartTick + dTick; if (newStart < 0) newStart = 0;
        if (newStart < obj.endTick - 1) obj.startTick = newStart;
    } else if (dragMode == 3) {
        int newEnd = dragEndTick + dTick;
        if (newEnd <= obj.startTick + 1) newEnd = obj.startTick + 1;
        obj.endTick = newEnd;
    }
}

// ========== EDITOR WINDOW ==========
EditorWindow::EditorWindow()
    : hierarchy(), inspector(), assetBrowser(), scenePanel(), scriptPanel(), timeline(),
      currentTick(0),
      dragSplitter(-1), leftRatio(0.20f), rightRatio(0.80f), bottomRatio(0.50f), timelineRatio(0.12f),
      activeMenu(-1), menuHovered(-1), showDebug(false),
      scenePanning(false), panStartX(0), panStartY(0), panStartPanX(0), panStartPanY(0), lastMsgTimer(0),
      previewMode(false), previewLine(0), snapSize(20) { lastMsg = "EDITOR LOADED"; lastMsgTimer = 9999; }

void EditorWindow::Layout(int winW, int winH) {
    int visSpl = 4, hitSpl = 8, menuH = 24;
    int timelineH = std::max(40, (int)(winH * timelineRatio));
    int vPos = (int)(winW * leftRatio);
    int vPos2 = (int)(winW * rightRatio);
    int hPos = (int)(winH * bottomRatio);
    if (vPos < 80) vPos = 80;
    if (vPos2 > winW - 80) vPos2 = winW - 80;
    if (vPos2 - vPos < 100) vPos2 = vPos + 100;
    if (hPos < 120) hPos = 120;
    if (hPos > winH - 80 - timelineH) hPos = winH - 80 - timelineH;
    int bottomH = winH - hPos - visSpl - timelineH;
    hierarchy.SetRect({0, menuH, vPos, hPos - menuH});
    scriptPanel.SetRect({0, hPos + visSpl, vPos, bottomH});
    scenePanel.SetRect({vPos + visSpl, menuH, vPos2 - vPos - visSpl, hPos - menuH});
    inspector.SetRect({vPos2 + visSpl, menuH, winW - vPos2 - visSpl, winH - menuH - timelineH});
    assetBrowser.SetRect({vPos + visSpl, hPos + visSpl, vPos2 - vPos - visSpl, bottomH});
    timeline.SetRect({0, winH - timelineH, winW, timelineH});
    splitterR1 = {vPos - 2, menuH, hitSpl, hPos - menuH};
    splitterR2 = {vPos2 - 2, menuH, hitSpl, winH - menuH - timelineH};
    splitterR3 = {0, hPos - 2, winW, hitSpl};
    splitterR4 = {0, winH - timelineH - 2, winW, hitSpl};
}

int EditorWindow::HitTestSplitter(int mx, int my) {
    if (splitterR1.Contains(mx, my)) return 1;
    if (splitterR2.Contains(mx, my)) return 2;
    if (splitterR3.Contains(mx, my)) return 3;
    if (splitterR4.Contains(mx, my)) return 4;
    return -1;
}

void EditorWindow::SyncSelection() {
    if (hierarchy.selectedIndex >= 0 && hierarchy.selectedIndex < (int)hierarchy.objects.size()) {
        scenePanel.selectedIndex = hierarchy.selectedIndex;
        inspector.SetTarget(&hierarchy.objects[hierarchy.selectedIndex]);
    } else if (scenePanel.selectedIndex >= 0 && scenePanel.selectedIndex < (int)hierarchy.objects.size()) {
        hierarchy.selectedIndex = scenePanel.selectedIndex;
        inspector.SetTarget(&hierarchy.objects[scenePanel.selectedIndex]);
    } else {
        inspector.SetTarget(nullptr);
    }
}

void EditorWindow::AddShape(const std::string& type, int mx, int my) {
    int sx = (int)((mx - scenePanel.contentRect.x - scenePanel.panX) / scenePanel.zoom);
    int sy = (int)((my - scenePanel.contentRect.y - scenePanel.panY) / scenePanel.zoom);
    int idx = hierarchy.AddObject(type, type, sx, sy);
    printf("[AddShape] type=%s mx=%d my=%d -> sx=%d sy=%d idx=%d objs=%zu\n", type.c_str(), mx, my, sx, sy, idx, hierarchy.objects.size());
    scenePanel.selectedIndex = idx;
    inspector.SetTarget(&hierarchy.objects[idx]);
}

// ========== DRAWING ==========
void EditorWindow::DrawMenuBar(GraphicsAPI* gfx) {
    int mh = 24, ww = gfx->GetWidth();
    gfx->DrawRect(0, 0, ww, mh, 40, 40, 50);
    gfx->DrawRect(0, mh - 1, ww, 1, 55, 55, 65);
    const char* labels[] = {"  File", "  Edit", "  Windows", "  Help"};
    int mpos[] = {0, 50, 100, 170};
    for (int i = 0; i < 4; i++) {
        int mw = (i < 3) ? (mpos[i+1] - mpos[i]) : 60;
        if (activeMenu == i) {
            gfx->DrawRect(mpos[i], 0, mw, mh, 60, 60, 80);
            gfx->DrawRect(mpos[i], mh - 1, mw, 1, 100, 150, 220);
        }
        gfx->RenderText(labels[i], mpos[i] + 4, 3, activeMenu == i ? 255 : 180, activeMenu == i ? 220 : 180, activeMenu == i ? 180 : 190);
    }
    if (activeMenu < 0) return;
    static const char* items[4][10] = {
        {"New Project", "Open", "Save", "---", "Import Assets...", "---", "Preview...", "Exit"},
        {"Undo", "Redo", "---", "Delete Selected", "Duplicate"},
        {"Toggle Hierarchy", "Toggle Inspector", "Toggle Asset Browser", "Toggle Scene", "Toggle Script"},
        {"About", "Controls"}
    };
    int dy = mh, dw = 160;
    for (int mi = 0; items[activeMenu][mi] != nullptr; mi++) {
        gfx->DrawRect(mpos[activeMenu], dy, dw, 22, 45, 45, 55);
        if (mi == menuHovered) gfx->DrawRect(mpos[activeMenu] + 1, dy, dw - 2, 22, 55, 80, 120);
        std::string s = items[activeMenu][mi];
        if (s == "---") gfx->DrawRect(mpos[activeMenu] + 4, dy + 10, dw - 8, 1, 70, 70, 80);
        else gfx->RenderText(s, mpos[activeMenu] + 6, dy + 2, mi == menuHovered ? 255 : 200, mi == menuHovered ? 220 : 200, mi == menuHovered ? 200 : 200);
        dy += 22;
    }
}

void EditorWindow::DrawSceneObjects(GraphicsAPI* gfx) {
    auto& cp = scenePanel.contentRect;
    gfx->DrawRect(cp.x, cp.y, cp.w, cp.h, 20, 20, 30);
    scenePanel.DrawGrid(gfx);
    gfx->SetClipRect(cp.x, cp.y, cp.w, cp.h);
    for (int i = 0; i < (int)hierarchy.objects.size(); i++) {
        auto& obj = hierarchy.objects[i];
        if (!obj.visible) continue;
        int dx = cp.x + (int)(obj.posX * scenePanel.zoom) + scenePanel.panX;
        int dy = cp.y + (int)(obj.posY * scenePanel.zoom) + scenePanel.panY;
        int sw = (int)(obj.sizeW * scenePanel.zoom), sh = (int)(obj.sizeH * scenePanel.zoom);
        if (obj.type == "ellipse") {
            int cx = dx + sw/2, cy = dy + sh/2, rx = sw/2, ry = sh/2;
            for (int row = -ry; row <= ry; row++) {
                int hw = (int)(rx * sqrt(1.0 - ((double)row*row)/((double)ry*ry)));
                gfx->DrawRect(cx - hw, cy + row, hw*2, 1, obj.colorR, obj.colorG, obj.colorB);
            }
        } else if (obj.type == "text") {
            gfx->DrawRect(dx, dy, sw, sh, obj.colorR, obj.colorG, obj.colorB);
            gfx->RenderText(obj.text.empty() ? obj.name : obj.text, dx + 4, dy + 4, 200, 200, 220);
        } else if (obj.type == "sprite" && !obj.spritePath.empty()) {
            gfx->DrawImage(dx, dy, sw, sh, obj.spritePath);
        } else {
            gfx->DrawRect(dx, dy, sw, sh, obj.colorR, obj.colorG, obj.colorB);
            gfx->RenderText(obj.name, dx + 4, dy + 4, 200, 200, 220);
        }
        if (i == scenePanel.selectedIndex) scenePanel.DrawGizmo(gfx, &obj);
    }
    gfx->ResetClip();
}

void EditorWindow::DrawPreview(GraphicsAPI* gfx) {
    int w = gfx->GetWidth(), h = gfx->GetHeight();
    gfx->DrawRect(0, 0, w, h, 15, 15, 25);
    int maxTick = 0;
    for (auto& obj : hierarchy.objects) if (obj.endTick > maxTick) maxTick = obj.endTick;
    // Find dialogue line for current tick
    ScriptLine* sl = nullptr;
    for (auto& l : scriptPanel.lines) { if (l.tick == currentTick) { sl = &l; break; } }
    // Find active objects at current tick
    auto isActive = [&](EditorObject& obj) { return obj.visible && currentTick >= obj.startTick && currentTick < obj.endTick; };
    // Background
    for (auto& obj : hierarchy.objects)
        if (obj.type == "background" && isActive(obj) && !obj.spritePath.empty()) { gfx->DrawImage(0, 0, w, h, obj.spritePath); break; }
    if (sl && !sl->backgroundPath.empty()) gfx->DrawImage(0, 0, w, h, sl->backgroundPath);
    // Sprites
    for (auto& obj : hierarchy.objects) {
        if (obj.type == "sprite" && isActive(obj) && !obj.spritePath.empty()) {
            int sw = w / 3, sh = h * 2 / 3; if (sh > h - 140) sh = h - 140;
            gfx->DrawImage(40, h - sh - 120, sw, sh, obj.spritePath);
        }
    }
    // Dialogue box
    int boxH = 110, boxY = h - boxH - 10;
    gfx->DrawRect(10, boxY, w - 20, boxH, 0, 0, 0);
    gfx->DrawRect(11, boxY + 1, w - 22, boxH - 2, 180, 180, 200);
    if (sl) {
        if (!sl->characterName.empty()) gfx->RenderText(sl->characterName, 24, boxY + 6, 255, 220, 100);
        int tx = 24, ty = boxY + 30, lineH = 20, maxW2 = w - 68;
        std::string remaining = sl->text;
        while (!remaining.empty()) {
            if (ty + lineH > boxY + boxH - 8) break;
            std::string line; size_t brk = remaining.find('\n');
            if (brk != std::string::npos) { line = remaining.substr(0, brk); remaining = remaining.substr(brk + 1); }
            else { line = remaining; remaining.clear(); }
            while ((int)line.size() * 8 > maxW2 && line.size() > 1) {
                size_t sp = line.rfind(' ', maxW2 / 8);
                if (sp == std::string::npos) sp = maxW2 / 8 - 1;
                gfx->RenderText(line.substr(0, sp), tx, ty, 240, 240, 240); ty += lineH;
                if (ty + lineH > boxY + boxH - 8) break;
                line = line.substr(sp + 1);
            }
            gfx->RenderText(line, tx, ty, 240, 240, 240); ty += lineH;
        }
    }
    gfx->RenderText("Tick " + std::to_string(currentTick), w - 120, 10, 120, 120, 140);
    gfx->RenderText("[Click/Space/Enter = next tick | Esc = exit]", 10, h - 18, 80, 80, 100);
}

void EditorWindow::DrawSplitters(GraphicsAPI* gfx) {
    gfx->DrawRect(splitterR1.x + 2, splitterR1.y, 4, splitterR1.h, 50, 50, 60);
    gfx->DrawRect(splitterR2.x + 2, splitterR2.y, 4, splitterR2.h, 50, 50, 60);
    gfx->DrawRect(splitterR3.x, splitterR3.y + 2, splitterR3.w, 4, 50, 50, 60);
    gfx->DrawRect(splitterR4.x, splitterR4.y + 2, splitterR4.w, 4, 50, 50, 60);
}

void EditorWindow::DrawDebugOverlay(GraphicsAPI* gfx) {
    if (!showDebug) return;
    int ww = gfx->GetWidth(), wh = gfx->GetHeight();
    gfx->DrawRect(0, 0, ww, wh, 0, 0, 0);
    gfx->DrawRect(ww / 2, 0, ww / 2, wh, 20, 20, 30);
    auto outline = [&](Rect r, int r2, int g2, int b2) { gfx->DrawRect(r.x, r.y, r.w, r.h, r2, g2, b2); };
    outline(hierarchy.rect, 255, 0, 0); outline(hierarchy.contentRect, 255, 100, 0);
    outline(inspector.rect, 0, 255, 0); outline(inspector.contentRect, 100, 255, 0);
    outline(scenePanel.rect, 0, 0, 255); outline(scenePanel.contentRect, 0, 100, 255);
    outline(assetBrowser.rect, 255, 255, 0); outline(assetBrowser.contentRect, 200, 200, 0);
    outline(scriptPanel.rect, 255, 0, 255); outline(scriptPanel.contentRect, 200, 0, 200);
    gfx->DrawRect(splitterR1.x, splitterR1.y, splitterR1.w, splitterR1.h, 255, 255, 255);
    gfx->DrawRect(splitterR2.x, splitterR2.y, splitterR2.w, splitterR2.h, 255, 255, 255);
    gfx->DrawRect(splitterR3.x, splitterR3.y, splitterR3.w, splitterR3.h, 255, 255, 255);
    gfx->DrawRect(splitterR4.x, splitterR4.y, splitterR4.w, splitterR4.h, 255, 255, 255);
    outline(timeline.rect, 200, 100, 200);
    int ly = 4;
    auto info = [&](const std::string& l, const std::string& v) { gfx->RenderText(l + ": " + v, 4, ly, 200, 200, 200); ly += 14; };
    info("Objects", std::to_string(hierarchy.objects.size()));
    info("Hier.sel", std::to_string(hierarchy.selectedIndex));
    info("Scene.sel", std::to_string(scenePanel.selectedIndex));
    info("zoom", std::to_string(scenePanel.zoom));
    info("pan", std::to_string(scenePanel.panX) + "," + std::to_string(scenePanel.panY));
    info("dragSpl", std::to_string(dragSplitter));
    info("ctxMenu", ctxMenu.active ? "active" : "off");
    info("selAsset", assetBrowser.selectedAsset ? "true" : "false");
    info("activeMenu", std::to_string(activeMenu));
    for (int i = 0; i < std::min((int)hierarchy.objects.size(), 15); i++) {
        auto& o = hierarchy.objects[i];
        info("[" + std::to_string(i) + "]", o.name + " " + o.type + " v=" + (o.visible ? "1" : "0"));
    }
    gfx->RenderText("F1=close debug", ww / 2 + 4, 4, 255, 100, 100);
}

void EditorWindow::Draw(GraphicsAPI* gfx) {
    if (previewMode) { DrawPreview(gfx); return; }
    gfx->Clear(30, 30, 40);
    DrawMenuBar(gfx);
    hierarchy.Draw(gfx);
    scriptPanel.Draw(gfx);
    timeline.Draw(gfx, hierarchy.objects, hierarchy.selectedIndex, currentTick);
    DrawSceneObjects(gfx);
    inspector.Draw(gfx);
    assetBrowser.Draw(gfx);
    DrawSplitters(gfx);
    ctxMenu.Draw(gfx);
    DrawDebugOverlay(gfx);
    if (lastMsgTimer > 0) {
        int y = gfx->GetHeight() - 36;
        gfx->DrawRect(0, y, gfx->GetWidth(), 18, 0, 0, 0);
        gfx->RenderText(lastMsg, 10, y + 1, 255, 255, 0);
        lastMsgTimer--;
    }
}

// ========== EVENT HANDLING ==========
void EditorWindow::HandleMenuClick(int mx, int my) {
    int mh = 24;
    int mpos[] = {0, 50, 100, 170};
    // Click on menu bar
    if (my < mh) {
        for (int i = 0; i < 4; i++) {
            int nextX = (i < 3) ? mpos[i+1] : mpos[i] + 60;
            if (mx >= mpos[i] && mx < nextX) {
                activeMenu = (activeMenu == i) ? -1 : i;
                menuHovered = -1; return;
            }
        }
        activeMenu = -1; menuHovered = -1; return;
    }
    // Menu dropdown is open - check if click is within bounds
    if (activeMenu >= 0) {
        int dx = mpos[activeMenu];
        if (mx < dx || mx > dx + 160) { activeMenu = -1; menuHovered = -1; return; }
        static const char* actions[4][10] = {
            {"New Project", "Open", "Save", "", "Import Assets...", "", "Preview...", "Exit"},
            {"Undo", "Redo", "", "Delete Selected", "Duplicate"},
            {"Toggle Hierarchy", "Toggle Inspector", "Toggle Asset Browser", "Toggle Scene", "Toggle Script"},
            {"About", "Controls"}
        };
        int idx = (my - mh) / 22;
        if (idx >= 0 && idx < 10 && actions[activeMenu][idx] != nullptr && strlen(actions[activeMenu][idx]) > 0) {
            std::string act = actions[activeMenu][idx];
            printf("[MenuAct] %s\n", act.c_str());
            activeMenu = -1; menuHovered = -1;
            if (act == "Import Assets...") {
                printf("  -> Import Assets dialog\n");
                char fileBuf[260] = {}; OPENFILENAMEA ofn = {};
                ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = GetActiveWindow();
                ofn.lpstrFile = fileBuf; ofn.nMaxFile = 260;
                ofn.lpstrFilter = "Images (*.png;*.jpg;*.bmp;*.gif)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif\0All\0*.*\0";
                ofn.Flags = OFN_FILEMUSTEXIST;
                if (GetOpenFileNameA(&ofn)) {
                    std::string src(fileBuf);
                    size_t sep = src.find_last_of("\\/");
                    std::string fn = (sep != std::string::npos) ? src.substr(sep+1) : src;
                    printf("  Copy %s -> %s/%s\n", src.c_str(), assetBrowser.currentDir.c_str(), fn.c_str());
                    CopyFileA(src.c_str(), (assetBrowser.currentDir + "/" + fn).c_str(), FALSE);
                    assetBrowser.ScanDirectory();
                }
            } else if (act == "Save") {
                printf("  -> Save\n");
                std::ofstream ofs("project.vne");
                ofs << hierarchy.objects.size() << "\n";
                for (auto& o : hierarchy.objects) {
                ofs << o.name << "|" << o.type << "|" << o.posX << "|" << o.posY << "|"
                    << o.sizeW << "|" << o.sizeH << "|" << o.visible << "|" << o.spritePath << "|" << o.text << "|"
                    << o.colorR << "|" << o.colorG << "|" << o.colorB << "|" << o.startTick << "|" << o.endTick << "\n";
                }
                ofs << scriptPanel.lines.size() << "\n";
                for (auto& l : scriptPanel.lines) {
                    ofs << l.text << "|" << l.characterName << "|" << l.spritePath << "|" << l.backgroundPath << "|" << l.tick << "\n";
                }
                printf("  Saved %zu objects, %zu lines\n", hierarchy.objects.size(), scriptPanel.lines.size());
            } else if (act == "Open") {
                printf("  -> Open\n");
                std::ifstream ifs("project.vne");
                if (ifs) {
                    hierarchy.objects.clear(); scriptPanel.lines.clear();
                    int n; ifs >> n; ifs.ignore();
                    for (int i = 0; i < n; i++) {
                        EditorObject o;
                        std::getline(ifs, o.name, '|'); std::getline(ifs, o.type, '|');
                        std::string sx, sy, sw, sh, sv;
                        std::getline(ifs, sx, '|'); std::getline(ifs, sy, '|');
                        std::getline(ifs, sw, '|'); std::getline(ifs, sh, '|');
                        std::getline(ifs, sv, '|');
                        o.posX = atoi(sx.c_str()); o.posY = atoi(sy.c_str());
                        o.sizeW = atoi(sw.c_str()); o.sizeH = atoi(sh.c_str()); o.visible = (sv != "0");
                        std::getline(ifs, o.spritePath, '|'); std::getline(ifs, o.text);
                        std::string sR, sG, sB;
                        std::getline(ifs, sR, '|'); std::getline(ifs, sG, '|'); std::getline(ifs, sB);
                        o.colorR = atoi(sR.c_str()); o.colorG = atoi(sG.c_str()); o.colorB = atoi(sB.c_str());
                        std::string sST, sET;
                        std::getline(ifs, sST, '|'); std::getline(ifs, sET, '|');
                        o.startTick = atoi(sST.c_str()); o.endTick = atoi(sET.c_str());
                        hierarchy.objects.push_back(o);
                    }
                    ifs >> n; ifs.ignore();
                    for (int i = 0; i < n; i++) {
                        ScriptLine l;
                        std::getline(ifs, l.text, '|'); std::getline(ifs, l.characterName, '|');
                        std::getline(ifs, l.spritePath, '|'); std::getline(ifs, l.backgroundPath);
                        std::string sTK; std::getline(ifs, sTK, '|');
                        l.tick = atoi(sTK.c_str());
                        scriptPanel.lines.push_back(l);
                    }
                    SyncSelection(); printf("  Loaded %zu objects, %zu lines\n", hierarchy.objects.size(), scriptPanel.lines.size());
                }
            } else if (act == "New Project") {
                printf("  -> New Project\n");
                hierarchy.objects.clear(); scriptPanel.lines.clear(); assetBrowser.files.clear();
                hierarchy.selectedIndex = -1; scenePanel.selectedIndex = -1; inspector.SetTarget(nullptr);
                hierarchy.AddObject("Background", "background"); SyncSelection();
            } else if (act == "Delete Selected" && hierarchy.selectedIndex >= 0) { printf("  -> DeleteSelected\n"); hierarchy.DeleteSelected(); SyncSelection(); }
            else if (act == "Duplicate" && hierarchy.selectedIndex >= 0) { printf("  -> Duplicate\n");
                auto& src = hierarchy.objects[hierarchy.selectedIndex];
                int ni = hierarchy.AddObject(src.name + "_copy", src.type, src.posX + 20, src.posY + 20);
                hierarchy.objects[ni].sizeW = src.sizeW; hierarchy.objects[ni].sizeH = src.sizeH; hierarchy.objects[ni].visible = src.visible;
                SyncSelection();
            } else if (act == "Undo") { printf("  -> Undo\n"); Undo(); }
            else if (act == "Redo") { printf("  -> Redo\n"); Redo(); }
            else if (act == "Preview...") {
                printf("  -> Preview\n");
                if (!scriptPanel.lines.empty()) { previewMode = true; previewLine = 0; }
            } else if (act == "Exit") { exit(0); }
        }
        activeMenu = -1; menuHovered = -1;
    }
}

void EditorWindow::HandleClick(int mx, int my) {
    PushUndo();
    if (previewMode) {
        currentTick++;
        return;
    }
    printf("[Click] at %d,%d activeMenu=%d ctxMenu.active=%d\n", mx, my, activeMenu, ctxMenu.active);
    // Menu bar or dropdown - always return after handling
    if (activeMenu >= 0 || my < 24) { HandleMenuClick(mx, my); return; }
    // Context menu
    if (ctxMenu.active) {
        int hit = ctxMenu.HitTest(mx, my);
        printf("[Click] ctxMenu.active=true hit=%d mx=%d my=%d\n", hit, mx, my);
        if (hit >= 0) {
            std::string act = ctxMenu.items[hit];
            printf("[Click] ctxMenu action=%s\n", act.c_str());
            ctxMenu.Close();
            printf("[CtxAct] %s\n", act.c_str());
            if (act == "Add Rectangle") { printf("  -> AddShape rect\n"); AddShape("rectangle", ctxMenu.mx, ctxMenu.my); }
            else if (act == "Add Ellipse") { printf("  -> AddShape ellipse\n"); AddShape("ellipse", ctxMenu.mx, ctxMenu.my); }
            else if (act == "Add Text") { printf("  -> AddShape text\n"); AddShape("text", ctxMenu.mx, ctxMenu.my); }
            else if (act == "Add Image") { printf("  -> AddShape sprite\n"); AddShape("sprite", ctxMenu.mx, ctxMenu.my); }
            else if (act == "Create Script Line" && hierarchy.selectedIndex >= 0) {
                printf("  -> CreateScriptLine\n");
                auto& obj = hierarchy.objects[hierarchy.selectedIndex];
                ScriptLine sl; sl.characterName = obj.name; sl.text = obj.text;
                if (obj.type == "sprite") sl.spritePath = obj.spritePath;
                sl.tick = (int)scriptPanel.lines.size();
                scriptPanel.lines.push_back(sl);
                scriptPanel.selectedLine = (int)scriptPanel.lines.size() - 1;
            }
            else if (act == "Delete" && hierarchy.selectedIndex >= 0) { printf("  -> DeleteSelected\n"); hierarchy.DeleteSelected(); SyncSelection(); }
            else if (act == "Hide" && hierarchy.selectedIndex >= 0) { printf("  -> ToggleVisible\n"); hierarchy.ToggleVisible(); }
            else if (act == "Bring to Front" && scenePanel.selectedIndex >= 0 && scenePanel.selectedIndex < (int)hierarchy.objects.size() - 1) {
                printf("  -> BringToFront\n");
                std::swap(hierarchy.objects[scenePanel.selectedIndex], hierarchy.objects.back());
                scenePanel.selectedIndex = (int)hierarchy.objects.size() - 1; SyncSelection();
            }
            else if (act == "New Folder") { printf("  -> NewFolder\n"); bool ok = fs::create_directories(assetBrowser.currentDir + "/NewFolder"); printf("  create_directories=%d\n", ok); assetBrowser.ScanDirectory(); }
            else if (act == "Add Empty File") { printf("  -> AddEmptyFile\n"); std::ofstream ofs(assetBrowser.currentDir + "/new_file.txt"); ofs.close(); assetBrowser.ScanDirectory(); }
            else if (act == "Add Material") { printf("  -> AddMaterial\n"); std::ofstream ofs(assetBrowser.currentDir + "/new_material.mat"); ofs.close(); assetBrowser.ScanDirectory(); }
            else if (act == "Refresh") { printf("  -> Refresh\n"); assetBrowser.ScanDirectory(); }
            else if (act == "Rename" && assetBrowser.selectedAsset && assetBrowser.selAssetIndex >= 0) {
                printf("  -> Rename\n");
                assetBrowser.editingField = 1; assetBrowser.editingIndex = assetBrowser.selAssetIndex;
                assetBrowser.editBuf = assetBrowser.files[assetBrowser.selAssetIndex];
            }
            else if (act == "Open" && assetBrowser.selectedAsset && assetBrowser.selAssetIndex >= 0) {
                printf("  -> Open dir\n");
                assetBrowser.HandleClick(ctxMenu.mx, ctxMenu.my);
            }
            else if (act == "Delete" && assetBrowser.selectedAsset && assetBrowser.selAssetIndex >= 0) {
                printf("  -> Delete\n");
                std::string delPath = assetBrowser.currentDir + "/" + assetBrowser.files[assetBrowser.selAssetIndex];
                fs::remove(delPath);
                assetBrowser.CancelSelect();
                assetBrowser.ScanDirectory();
            }
            return;
        }
        ctxMenu.Close();
    }
    // Clear transient state
    scenePanning = false;
    dragSplitter = -1;
    // Splitter
    int spl = HitTestSplitter(mx, my);
    if (spl > 0) { dragSplitter = spl; return; }
    // Asset placement
    if (assetBrowser.selectedAsset) {
        std::string fp = assetBrowser.selAssetPath;
        std::string fn = fp; size_t p = fn.find_last_of("/\\"); if (p != std::string::npos) fn = fn.substr(p + 1);
        if (hierarchy.rect.Contains(mx, my)) {
            int hidx = hierarchy.AddObject(fn, "sprite");
            hierarchy.objects[hidx].spritePath = fp;
            SyncSelection(); assetBrowser.CancelSelect(); return;
        }
        if (scenePanel.contentRect.Contains(mx, my)) {
            for (int i = (int)hierarchy.objects.size() - 1; i >= 0; i--) {
                auto& obj = hierarchy.objects[i];
                if (!obj.visible) continue;
                int dx = scenePanel.contentRect.x + (int)(obj.posX * scenePanel.zoom) + scenePanel.panX;
                int dy = scenePanel.contentRect.y + (int)(obj.posY * scenePanel.zoom) + scenePanel.panY;
                int sw = (int)(obj.sizeW * scenePanel.zoom), sh = (int)(obj.sizeH * scenePanel.zoom);
                if (mx >= dx && mx <= dx + sw && my >= dy && my <= dy + sh) {
                    scenePanel.selectedIndex = i; hierarchy.selectedIndex = i;
                    inspector.SetTarget(&hierarchy.objects[i]); assetBrowser.CancelSelect(); return;
                }
            }
            int sx = (int)((mx - scenePanel.contentRect.x - scenePanel.panX) / scenePanel.zoom);
            int sy = (int)((my - scenePanel.contentRect.y - scenePanel.panY) / scenePanel.zoom);
            int idx = hierarchy.AddObject(fn, "sprite", sx, sy);
            hierarchy.objects[idx].spritePath = fp;
            scenePanel.selectedIndex = idx; hierarchy.selectedIndex = idx;
            inspector.SetTarget(&hierarchy.objects[idx]); assetBrowser.CancelSelect(); return;
        }
        assetBrowser.CancelSelect();
    }
    // Panel dispatch (per-panel, early return)
    if (hierarchy.rect.Contains(mx, my)) { hierarchy.HandleClick(mx, my); SyncSelection(); return; }
    if (inspector.contentRect.Contains(mx, my)) { inspector.HandleClick(mx, my); return; }
    if (scriptPanel.rect.Contains(mx, my)) { scriptPanel.HandleClick(mx, my); return; }
    if (timeline.rect.Contains(mx, my)) { timeline.HandleClick(mx, my, hierarchy.objects, hierarchy.selectedIndex, currentTick); SyncSelection(); return; }
    if (assetBrowser.contentRect.Contains(mx, my)) { assetBrowser.HandleClick(mx, my); return; }
    // Scene
    if (scenePanel.contentRect.Contains(mx, my)) {
        scenePanel.HandleClick(mx, my);
        // Gizmo on selected object
        int si = scenePanel.selectedIndex;
        if (si >= 0 && si < (int)hierarchy.objects.size()) {
            GizmoPart gp = scenePanel.HitTestGizmo(mx, my, &hierarchy.objects[si]);
            if (gp != GIZMO_NONE) {
                scenePanel.activeGizmo = gp;
                scenePanel.gizmoStartX = mx; scenePanel.gizmoStartY = my;
                scenePanel.gizmoObjX = hierarchy.objects[si].posX;
                scenePanel.gizmoObjY = hierarchy.objects[si].posY;
                scenePanel.gizmoObjW = hierarchy.objects[si].sizeW;
                scenePanel.gizmoObjH = hierarchy.objects[si].sizeH;
                return;
            }
        }
        // Pick object
        for (int i = (int)hierarchy.objects.size() - 1; i >= 0; i--) {
            auto& obj = hierarchy.objects[i];
            if (!obj.visible) continue;
            int dx = scenePanel.contentRect.x + (int)(obj.posX * scenePanel.zoom) + scenePanel.panX;
            int dy = scenePanel.contentRect.y + (int)(obj.posY * scenePanel.zoom) + scenePanel.panY;
            int sw = (int)(obj.sizeW * scenePanel.zoom), sh = (int)(obj.sizeH * scenePanel.zoom);
            if (mx >= dx && mx <= dx + sw && my >= dy && my <= dy + sh) {
                scenePanel.selectedIndex = i; hierarchy.selectedIndex = i;
                inspector.SetTarget(&hierarchy.objects[i]); return;
            }
        }
        // Empty space - pan mode
        hierarchy.selectedIndex = -1; scenePanel.selectedIndex = -1; inspector.SetTarget(nullptr);
        scenePanning = true;
        panStartX = mx; panStartY = my;
        panStartPanX = scenePanel.panX; panStartPanY = scenePanel.panY;
    }
}

void EditorWindow::HandleRClick(int mx, int my) {
    printf("[RClick] at %d,%d\n", mx, my);
    lastMsg = "RClick at " + std::to_string(mx) + "," + std::to_string(my);
    lastMsgTimer = 9999;
    if (ctxMenu.active) { ctxMenu.Close(); return; }
    bool inHier = hierarchy.contentRect.Contains(mx, my);
    bool inScene = scenePanel.contentRect.Contains(mx, my);
    bool inAsset = assetBrowser.contentRect.Contains(mx, my);
    lastMsg += " H=" + std::to_string(inHier) + " S=" + std::to_string(inScene) + " A=" + std::to_string(inAsset);
    if (inHier) {
        for (int i = 0; i < (int)hierarchy.objects.size(); i++) {
            int y = hierarchy.contentRect.y + 4 + i * 24;
            if (y + 24 > hierarchy.rect.y + hierarchy.rect.h - 26) break;
            if (i >= hierarchy.scrollOffset && mx >= hierarchy.contentRect.x + 2 && mx <= hierarchy.contentRect.x + hierarchy.contentRect.w - 2 && my >= y && my <= y + 22) {
                hierarchy.selectedIndex = i;
                ctxMenu.Open(mx, my, {"Create Script Line", "---", "Delete", "Hide"});
                lastMsg += " HIER_ITEM";
                return;
            }
        }
        lastMsg += " HIER_EMPTY";
        return;
    }
    if (inScene) {
        for (int i = (int)hierarchy.objects.size() - 1; i >= 0; i--) {
            auto& obj = hierarchy.objects[i];
            int dx = scenePanel.contentRect.x + (int)(obj.posX * scenePanel.zoom) + scenePanel.panX;
            int dy = scenePanel.contentRect.y + (int)(obj.posY * scenePanel.zoom) + scenePanel.panY;
            int sw = (int)(obj.sizeW * scenePanel.zoom), sh = (int)(obj.sizeH * scenePanel.zoom);
            if (mx >= dx && mx <= dx + sw && my >= dy && my <= dy + sh) {
                scenePanel.selectedIndex = i;
                ctxMenu.Open(mx, my, {"Delete", "Hide", "Bring to Front"});
                lastMsg += " SCENE_OBJ";
                return;
            }
        }
        ctxMenu.Open(mx, my, {"Add Rectangle", "Add Ellipse", "Add Text", "Add Image"});
        lastMsg += " SCENE_ADD";
        return;
    }
    if (inAsset) {
        printf("[RClick] AssetBrowser hit! files=%zu\n", assetBrowser.files.size());
        int fi = assetBrowser.GetFileIndexAt(mx, my);
        printf("[RClick] GetFileIndexAt=%d\n", fi);
        if (fi >= 0) {
            assetBrowser.selectedAsset = true; assetBrowser.selAssetIndex = fi; assetBrowser.selAssetPath = assetBrowser.currentDir + "/" + assetBrowser.files[fi];
            std::string fullPath = assetBrowser.currentDir + "/" + assetBrowser.files[fi];
            if (fs::is_directory(fullPath)) {
                ctxMenu.Open(mx, my, {"Open"});
            } else {
                ctxMenu.Open(mx, my, {"Place on Scene", "Place in Hierarchy", "---", "Rename", "Delete"});
            }
            lastMsg += " ASSET_PLACE";
            printf("[RClick] ctxMenu opened: asset actions at %d,%d\n", mx, my);
        } else {
            ctxMenu.Open(mx, my, {"New Folder", "Add Empty File", "Add Material", "Refresh"});
            lastMsg += " ASSET_ADD";
            printf("[RClick] ctxMenu opened: New Folder etc at %d,%d\n", mx, my);
        }
        return;
    }
    lastMsg += " NOWHERE";
}

void EditorWindow::HandleDrag(int mx, int my) {
    timeline.HandleDrag(mx, my, hierarchy.objects);
    int winW = inspector.rect.x + inspector.rect.w;
    int winH = timeline.rect.y + timeline.rect.h;
    if (winW <= 0 || winH <= 0) return;
    if (dragSplitter == 1) {
        leftRatio = (float)mx / winW;
        if (leftRatio < 0.05f) leftRatio = 0.05f;
        if (leftRatio > 0.50f) leftRatio = 0.50f;
        return;
    }
    if (dragSplitter == 2) {
        rightRatio = (float)mx / winW;
        if (rightRatio < leftRatio + 0.10f) rightRatio = leftRatio + 0.10f;
        if (rightRatio > 0.95f) rightRatio = 0.95f;
        return;
    }
    if (dragSplitter == 3) {
        bottomRatio = (float)my / winH;
        if (bottomRatio < 0.30f) bottomRatio = 0.30f;
        if (bottomRatio > 0.90f) bottomRatio = 0.90f;
        return;
    }
    if (dragSplitter == 4) {
        timelineRatio = (float)(winH - my) / winH;
        if (timelineRatio < 0.04f) timelineRatio = 0.04f;
        if (timelineRatio > 0.50f) timelineRatio = 0.50f;
        return;
    }
    if (scenePanning && scenePanel.contentRect.Contains(mx, my)) {
        scenePanel.panX = panStartPanX + (mx - panStartX);
        scenePanel.panY = panStartPanY + (my - panStartY);
        return;
    }
    if (scenePanel.activeGizmo != GIZMO_NONE && scenePanel.selectedIndex >= 0 && scenePanel.selectedIndex < (int)hierarchy.objects.size()) {
        auto& obj = hierarchy.objects[scenePanel.selectedIndex];
        int dx = mx - scenePanel.gizmoStartX, dy = my - scenePanel.gizmoStartY;
        int adx = (int)(dx / scenePanel.zoom), ady = (int)(dy / scenePanel.zoom);
        int snap = 1;
#ifdef _WIN32
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) snap = snapSize;
#endif
        switch (scenePanel.activeGizmo) {
            case GIZMO_MOVE:
                obj.posX = scenePanel.gizmoObjX + adx;
                obj.posY = scenePanel.gizmoObjY + ady;
                if (snap > 1) {
                    obj.posX = (obj.posX / snap) * snap;
                    obj.posY = (obj.posY / snap) * snap;
                }
                break;
            case GIZMO_RESIZE_TL: obj.posX = scenePanel.gizmoObjX + adx; obj.posY = scenePanel.gizmoObjY + ady; obj.sizeW = scenePanel.gizmoObjW - adx; obj.sizeH = scenePanel.gizmoObjH - ady; break;
            case GIZMO_RESIZE_TR: obj.posY = scenePanel.gizmoObjY + ady; obj.sizeW = scenePanel.gizmoObjW + adx; obj.sizeH = scenePanel.gizmoObjH - ady; break;
            case GIZMO_RESIZE_BL: obj.posX = scenePanel.gizmoObjX + adx; obj.sizeW = scenePanel.gizmoObjW - adx; obj.sizeH = scenePanel.gizmoObjH + ady; break;
            case GIZMO_RESIZE_BR: obj.sizeW = scenePanel.gizmoObjW + adx; obj.sizeH = scenePanel.gizmoObjH + ady; break;
            default: break;
        }
        if (obj.sizeW < 10) obj.sizeW = 10; if (obj.sizeH < 10) obj.sizeH = 10;
    }
}

void EditorWindow::HandleScroll(int delta, int mouseY) {
    int mouseX = 0;
#ifdef _WIN32
    POINT pt; GetCursorPos(&pt);
    HWND hw = GetActiveWindow(); if (hw) { ScreenToClient(hw, &pt); mouseX = pt.x; }
#endif
    if (hierarchy.rect.Contains(mouseX, mouseY)) {
        hierarchy.scrollOffset += delta > 0 ? -1 : 1;
        if (hierarchy.scrollOffset < 0) hierarchy.scrollOffset = 0;
        int maxOff = std::max(0, (int)hierarchy.objects.size() - 1);
        if (hierarchy.scrollOffset > maxOff) hierarchy.scrollOffset = maxOff;
    } else if (scriptPanel.rect.Contains(mouseX, mouseY)) {
        scriptPanel.scrollOffset += delta > 0 ? -1 : 1;
        if (scriptPanel.scrollOffset < 0) scriptPanel.scrollOffset = 0;
        int maxOff = std::max(0, (int)scriptPanel.lines.size() - 1);
        if (scriptPanel.scrollOffset > maxOff) scriptPanel.scrollOffset = maxOff;
    } else if (timeline.rect.Contains(mouseX, mouseY)) {
        timeline.scrollOffset += delta > 0 ? -1 : 1;
        if (timeline.scrollOffset < 0) timeline.scrollOffset = 0;
    } else {
        scenePanel.HandleScroll(delta);
    }
}

void EditorWindow::HandleKey(int key) {
    if (previewMode) {
        if (key == VK_ESCAPE) { previewMode = false; return; }
        if (key == VK_SPACE || key == VK_RETURN) { currentTick++; }
        return;
    }
    printf("[Key] key=%d (0x%x) inspField=%d scriptField=%d assetField=%d\n", key, key, inspector.editingField, scriptPanel.editingField, assetBrowser.editingField);
    if (inspector.editingField) {
        if (key == VK_RETURN || key == VK_ESCAPE) { inspector.CommitEdit(); }
        else if (key == VK_BACK && !inspector.editBuf.empty()) { inspector.editBuf.pop_back(); }
        return;
    }
    if (scriptPanel.editingField) { scriptPanel.HandleKey(key); return; }
    if (assetBrowser.editingField) { assetBrowser.HandleKey(key); return; }
    if (key == VK_F1) { showDebug = !showDebug; return; }
    if (key == VK_DELETE && hierarchy.selectedIndex >= 0) { PushUndo(); hierarchy.DeleteSelected(); SyncSelection(); return; }
    if (key == 'R' || key == 'r') { AddShape("rectangle", scenePanel.contentRect.x + 100, scenePanel.contentRect.y + 100); return; }
    if (key == 'E' || key == 'e') { AddShape("ellipse", scenePanel.contentRect.x + 200, scenePanel.contentRect.y + 150); return; }
    if (key == 'T' || key == 't') { AddShape("text", scenePanel.contentRect.x + 300, scenePanel.contentRect.y + 200); return; }
    if (key == 'S' || key == 's') { AddShape("sprite", scenePanel.contentRect.x + 100, scenePanel.contentRect.y + 300); return; }
    if (key == 'Z' && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) { Undo(); return; }
    if (key == 'Y' && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) { Redo(); return; }
    if (key == VK_OEM_4) { snapSize = std::max(1, snapSize - 5); lastMsg = "Snap: " + std::to_string(snapSize); lastMsgTimer = 60; return; } // [
    if (key == VK_OEM_6) { snapSize += 5; lastMsg = "Snap: " + std::to_string(snapSize); lastMsgTimer = 60; return; } // ]
    if (key == 'A' || key == 'a') {
        std::ofstream("assets/Images/test_asset.png").close();
        assetBrowser.ScanDirectory();
        lastMsg = "Created test_asset.png";
        lastMsgTimer = 120;
        return;
    }
}

void EditorWindow::PushUndo() {
    if ((int)undoStack.size() > 50) undoStack.erase(undoStack.begin());
    EditorSnapshot s;
    s.objects = hierarchy.objects;
    s.lines = scriptPanel.lines;
    undoStack.push_back(s);
    redoStack.clear();
}

void EditorWindow::Undo() {
    if (undoStack.empty()) return;
    EditorSnapshot cur; cur.objects = hierarchy.objects; cur.lines = scriptPanel.lines;
    redoStack.push_back(cur);
    hierarchy.objects = undoStack.back().objects;
    scriptPanel.lines = undoStack.back().lines;
    undoStack.pop_back();
    SyncSelection();
    lastMsg = "Undo"; lastMsgTimer = 60;
}

void EditorWindow::Redo() {
    if (redoStack.empty()) return;
    EditorSnapshot cur; cur.objects = hierarchy.objects; cur.lines = scriptPanel.lines;
    undoStack.push_back(cur);
    hierarchy.objects = redoStack.back().objects;
    scriptPanel.lines = redoStack.back().lines;
    redoStack.pop_back();
    SyncSelection();
    lastMsg = "Redo"; lastMsgTimer = 60;
}

void EditorWindow::HandleFileDrop(const std::string& filePath) {
    printf("[Drop] %s\n", filePath.c_str());
    lastMsg = "Dropped: " + filePath;
    lastMsgTimer = 9999;
    // Copy file to current asset dir
    size_t sep = filePath.find_last_of("/\\");
    std::string fn = (sep != std::string::npos) ? filePath.substr(sep + 1) : filePath;
    std::string dest = assetBrowser.currentDir + "/" + fn;
    CopyFileA(filePath.c_str(), dest.c_str(), FALSE);
    assetBrowser.ScanDirectory();
    // Place sprite in scene
    int sx = 50 + ((int)hierarchy.objects.size() * 30) % 400;
    int sy = 50 + ((int)hierarchy.objects.size() * 30) % 300;
    int idx = hierarchy.AddObject(fn, "sprite", sx, sy);
    hierarchy.objects[idx].spritePath = dest;
    scenePanel.selectedIndex = idx;
    hierarchy.selectedIndex = idx;
    inspector.SetTarget(&hierarchy.objects[idx]);
}

void EditorWindow::HandleEvents(const std::vector<WindowEvent>& events) {
    for (auto& e : events) {
        switch (e.type) {
            case WindowEvent::MOUSE_CLICK: HandleClick(e.param1, e.param2); break;
            case WindowEvent::MOUSE_RCLICK: HandleRClick(e.param1, e.param2); break;
            case WindowEvent::MOUSE_DRAG: HandleDrag(e.param1, e.param2); break;
            case WindowEvent::MOUSE_SCROLL: HandleScroll(e.param1, e.param2); break;
            case WindowEvent::CHAR_INPUT: {
                int c = e.param1;
                if (c <= 32) break;
                // Convert UTF-16 code unit to UTF-8
                char utf8Buf[4] = {};
                int utf8Len = 0;
#ifdef _WIN32
                wchar_t wc = (wchar_t)c;
                utf8Len = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8Buf, 4, nullptr, nullptr);
                if (utf8Len <= 0) break;
#endif
                std::string utf8Str(utf8Buf, utf8Len > 0 ? utf8Len : 1);
                if (utf8Len <= 0) { utf8Str = std::string(1, (char)c); }
                if (scriptPanel.editingField && scriptPanel.editBuf.size() < 80) {
                    scriptPanel.editBuf += utf8Str;
                    if (scriptPanel.editingLine >= 0 && scriptPanel.editingLine < (int)scriptPanel.lines.size()) {
                        if (scriptPanel.editingField == 1) scriptPanel.lines[scriptPanel.editingLine].characterName = scriptPanel.editBuf;
                        else if (scriptPanel.editingField == 2) scriptPanel.lines[scriptPanel.editingLine].text = scriptPanel.editBuf;
                    }
                } else if (inspector.editingField && inspector.editBuf.size() < 40 && utf8Len == 1) {
                    inspector.editBuf += (char)c;
                } else if (assetBrowser.editingField && assetBrowser.editBuf.size() < 60) {
                    assetBrowser.editBuf += utf8Str;
                }
                break;
            }
            case WindowEvent::KEY_DOWN: HandleKey(e.param1); break;
            default: break;
        }
    }
}

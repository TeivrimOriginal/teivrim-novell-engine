#include "WinAPIGraphics.hpp"
#include "ProjectManager.hpp"
#include "EditorPanels.hpp"
#include <ctime>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif
namespace fs = std::filesystem;

static std::string TimeStr() {
    time_t t = time(nullptr);
    std::string s = ctime(&t); s.pop_back();
    return s;
}

static void Button(GraphicsAPI* gfx, const std::string& text, int x, int y, int w, int h, bool hovered = false) {
    int r = hovered ? 100 : 70, g = hovered ? 110 : 80, b = hovered ? 130 : 100;
    gfx->DrawRect(x, y, w, h, r, g, b);
    gfx->DrawRect(x + 1, y + 1, w - 2, h - 2, r + 10, g + 10, b + 10);
    gfx->RenderText(text, x + 10, y + 4, 220, 220, 230);
}

int main() {
#ifdef _WIN32
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif
    printf("=== VNE v3.0 STARTED ===\n");
    // Set working directory to exe's parent directory (project root)
    char exePath[260]; GetModuleFileNameA(NULL, exePath, 260);
    std::string exeDir(exePath); size_t pos = exeDir.find_last_of("\\/");
    if (pos != std::string::npos) exeDir = exeDir.substr(0, pos);
    pos = exeDir.find_last_of("\\/"); // go up from build\ to project root
    if (pos != std::string::npos) exeDir = exeDir.substr(0, pos);
    SetCurrentDirectoryA(exeDir.c_str());
    char cwd[260]; GetCurrentDirectoryA(260, cwd); printf("CWD=%s\n", cwd);

    WinAPIGraphics gfx;
    if (!gfx.Init("VNE v3.0 - Rewrite", 960, 640)) return 1;

    ProjectManager pm;
    pm.Load("projects.dat");

    enum Screen { PROJECT_MANAGER, EDITOR, INPUT_DIALOG };
    Screen screen = PROJECT_MANAGER;
    int selProj = -1;
    if (pm.GetCount() > 0) selProj = 0;

    std::string inputText, inputPrompt;
    bool inputRename = false;

    // Auto-create test project for quick testing
    if (pm.GetCount() == 0) {
        Project p; p.name = "Test Project"; p.path = "projects/Test Project";
        p.description = "Auto-created"; p.createdAt = TimeStr(); pm.Add(p);
    }
    selProj = 0;

    EditorWindow editor;
    fs::create_directories("assets/Images");

    while (gfx.IsRunning()) {
        std::vector<WindowEvent> events;
        gfx.PollEvents(events);

        for (auto& e : events) {
            if (e.type == WindowEvent::CLOSE) { gfx.Shutdown(); return 0; }

            if (screen == PROJECT_MANAGER) {
                if (e.type == WindowEvent::MOUSE_CLICK) {
                    int mx = e.param1, my = e.param2;
                    int listY = 120;
                    for (int i = 0; i < pm.GetCount(); i++) {
                        if (mx >= 40 && mx <= gfx.GetWidth() - 40 && my >= listY && my <= listY + 40) selProj = i;
                        listY += 44;
                    }
                    int btnY = gfx.GetHeight() - 70, btnW = 140, btnH = 36, startX = 60;
                    if (mx >= startX && mx <= startX + btnW && my >= btnY && my <= btnY + btnH) {
                        screen = INPUT_DIALOG; inputText.clear(); inputPrompt = "Enter project name:"; inputRename = false;
                    }
                    if (mx >= startX + 160 && mx <= startX + 160 + btnW && my >= btnY && my <= btnY + btnH && selProj >= 0 && selProj < pm.GetCount()) {
                        pm.Delete(pm.List()[selProj].name); selProj = std::min(selProj, pm.GetCount() - 1);
                    }
                    if (mx >= startX + 320 && mx <= startX + 320 + btnW && my >= btnY && my <= btnY + btnH && selProj >= 0 && selProj < pm.GetCount()) {
                        screen = INPUT_DIALOG; inputText = pm.List()[selProj].name; inputPrompt = "Rename project to:"; inputRename = true;
                    }
                    if (mx >= startX + 480 && mx <= startX + 480 + btnW && my >= btnY && my <= btnY + btnH && selProj >= 0 && selProj < pm.GetCount()) {
                        editor = EditorWindow();
                        editor.hierarchy.AddObject("Background", "background");
                        editor.hierarchy.selectedIndex = 0;
                        editor.scenePanel.selectedIndex = 0;
                        editor.inspector.SetTarget(&editor.hierarchy.objects[0]);
                        editor.assetBrowser.ScanDirectory();
                        editor.scriptPanel.AddLine();
                        screen = EDITOR;
                    }
                }
                if (e.type == WindowEvent::MOUSE_DBLCLICK && selProj >= 0 && selProj < pm.GetCount()) {
                    editor = EditorWindow();
                    editor.hierarchy.AddObject("Background", "background");
                    editor.hierarchy.selectedIndex = 0;
                    editor.scenePanel.selectedIndex = 0;
                    editor.inspector.SetTarget(&editor.hierarchy.objects[0]);
                    editor.assetBrowser.ScanDirectory();
                    editor.scriptPanel.AddLine();
                    screen = EDITOR;
                }
                if (e.type == WindowEvent::KEY_DOWN && e.param1 == VK_ESCAPE) { gfx.Shutdown(); return 0; }
                if (e.type == WindowEvent::KEY_DOWN && e.param1 == VK_RETURN && selProj >= 0) {
                    editor = EditorWindow(); editor.hierarchy.AddObject("Background", "background");
                    editor.hierarchy.selectedIndex = 0; editor.scenePanel.selectedIndex = 0;
                    editor.inspector.SetTarget(&editor.hierarchy.objects[0]);
                    editor.assetBrowser.ScanDirectory(); editor.scriptPanel.AddLine(); screen = EDITOR;
                }
            }
            else if (screen == INPUT_DIALOG) {
                if (e.type == WindowEvent::CHAR_INPUT) {
                    char c = (char)e.param1;
                    if (c >= 32 && c <= 126 && inputText.size() < 40) inputText += c;
                }
                if (e.type == WindowEvent::KEY_DOWN) {
                    if (e.param1 == VK_RETURN && !inputText.empty()) {
                        if (inputRename && selProj >= 0 && selProj < pm.GetCount()) pm.Rename(pm.List()[selProj].name, inputText);
                        else if (!inputRename) {
                            Project p; p.name = inputText; p.path = "projects/" + inputText; p.description = "Created from Project Manager"; p.createdAt = TimeStr();
                            pm.Add(p); selProj = pm.GetCount() - 1;
                        }
                        screen = PROJECT_MANAGER;
                    }
                    if (e.param1 == VK_ESCAPE) { screen = PROJECT_MANAGER; }
                    if (e.param1 == VK_BACK && !inputText.empty()) { inputText.pop_back(); }
                }
            }
            else if (screen == EDITOR) {
                if (e.type == WindowEvent::KEY_DOWN && e.param1 == VK_ESCAPE && !editor.previewMode) { screen = PROJECT_MANAGER; }
            }
        }

        if (screen == EDITOR) {
            editor.Layout(gfx.GetWidth(), gfx.GetHeight());
            editor.HandleEvents(events);
            std::vector<std::string> drops; gfx.GetDroppedFiles(drops);
            for (auto& path : drops) editor.HandleFileDrop(path);
        }

        if (screen == PROJECT_MANAGER) {
            gfx.BeginFrame(); gfx.Clear(25, 25, 35);
            gfx.RenderText("=== VISUAL NOVEL ENGINE ===", 40, 20, 255, 255, 255);
            gfx.RenderText("Project Manager", 40, 44, 180, 180, 200);
            gfx.DrawRect(30, 75, gfx.GetWidth() - 60, 1, 60, 60, 75);
            gfx.RenderText("Your Projects:", 40, 88, 140, 140, 160);
            int listY = 120;
            if (pm.GetCount() == 0) gfx.RenderText("(No projects. Click 'Add Project' to create one.)", 50, listY, 90, 90, 110);
            else for (int i = 0; i < pm.GetCount(); i++) {
                if (i == selProj) {
                    gfx.DrawRect(40, listY, gfx.GetWidth() - 80, 40, 50, 60, 90);
                    gfx.DrawRect(40, listY, 3, 40, 100, 150, 220);
                    gfx.RenderText(pm.List()[i].name, 55, listY + 4, 255, 220, 100);
                    gfx.RenderText("Path: " + pm.List()[i].path, 55, listY + 22, 140, 140, 160);
                } else {
                    gfx.DrawRect(40, listY, gfx.GetWidth() - 80, 40, 35, 35, 45);
                    gfx.RenderText(pm.List()[i].name, 55, listY + 4, 200, 200, 200);
                    gfx.RenderText("Path: " + pm.List()[i].path, 55, listY + 22, 100, 100, 120);
                }
                listY += 44;
            }
            int btnY = gfx.GetHeight() - 70, btnW = 140, startX = 60;
            Button(&gfx, "Add Project", startX, btnY, btnW, 36);
            Button(&gfx, "Delete", startX + 160, btnY, btnW, 36);
            Button(&gfx, "Rename", startX + 320, btnY, btnW, 36);
            Button(&gfx, "Open Editor", startX + 480, btnY, btnW, 36);
            gfx.EndFrame();
        }
        else if (screen == INPUT_DIALOG) {
            gfx.BeginFrame(); gfx.Clear(25, 25, 35);
            int dlgW = 400, dlgH = 120, dlgX = (gfx.GetWidth() - dlgW) / 2, dlgY = (gfx.GetHeight() - dlgH) / 2;
            gfx.DrawRect(dlgX, dlgY, dlgW, dlgH, 45, 45, 55);
            gfx.DrawRect(dlgX + 1, dlgY + 1, dlgW - 2, dlgH - 2, 55, 55, 65);
            gfx.RenderText(inputPrompt, dlgX + 15, dlgY + 15, 220, 220, 230);
            gfx.DrawRect(dlgX + 15, dlgY + 40, dlgW - 30, 28, 30, 30, 40);
            gfx.RenderText(inputText + (inputText.size() < 40 ? "_" : ""), dlgX + 20, dlgY + 44, 255, 255, 200);
            gfx.RenderText("Enter = confirm   Escape = cancel", dlgX + 15, dlgY + dlgH - 22, 120, 120, 140);
            gfx.EndFrame();
        }
        else if (screen == EDITOR) {
            gfx.BeginFrame();
            editor.Layout(gfx.GetWidth(), gfx.GetHeight());
            editor.Draw(&gfx);
            gfx.RenderText("ESC=back R=rect E=ellipse T=text S=sprite [=snap- ]=snap+ F1=debug", 10, gfx.GetHeight() - 16, 100, 100, 120);
            gfx.EndFrame();
        }

        Sleep(16);
    }
    return 0;
}

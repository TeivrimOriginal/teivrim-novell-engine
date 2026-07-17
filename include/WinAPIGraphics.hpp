#pragma once
#include "GraphicsAPI.hpp"
#include <windows.h>
#include <gdiplus.h>
#include <map>

class WinAPIGraphics : public GraphicsAPI {
public:
    WinAPIGraphics();
    ~WinAPIGraphics();

    bool Init(const std::string& title, int width, int height) override;
    void Shutdown() override;
    bool IsRunning() override;
    void PollEvents(std::vector<WindowEvent>& outEvents) override;
    void BeginFrame() override;
    void EndFrame() override;
    void DrawRect(int x, int y, int w, int h,
                  unsigned char r, unsigned char g, unsigned char b) override;
    void DrawImage(int x, int y, int w, int h, const std::string& filePath) override;
    void RenderText(const std::string& text, int x, int y,
                    unsigned char r, unsigned char g, unsigned char b) override;
    void Clear(unsigned char r, unsigned char g, unsigned char b) override;
    int GetWidth() const override;
    int GetHeight() const override;
    void SetClipRect(int x, int y, int w, int h) override;
    void ResetClip() override;

    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void GetDroppedFiles(std::vector<std::string>& out);

private:
    HWND hwnd;
    HINSTANCE hInstance;
    bool running;
    int width, height;
    std::vector<WindowEvent> eventQueue;
    HDC hdcBuffer;
    HBITMAP hbmBuffer;
    HFONT hFont;
    int fontHeight;
    int mouseX, mouseY;
    bool mouseDown;
    ULONG_PTR gdiplusToken;
    std::map<std::string, Gdiplus::Bitmap*> imageCache;
    std::vector<std::string> droppedFiles;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

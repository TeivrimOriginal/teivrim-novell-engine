#include "WinAPIGraphics.hpp"
#include <windowsx.h>
#include <shellapi.h>
#include <map>
#include <algorithm>
using namespace Gdiplus;

WinAPIGraphics::WinAPIGraphics()
    : hwnd(nullptr), hInstance(GetModuleHandleW(nullptr)),
      running(false), width(800), height(600),
      hdcBuffer(nullptr), hbmBuffer(nullptr), hFont(nullptr), fontHeight(16),
      mouseX(0), mouseY(0), mouseDown(false) {}

WinAPIGraphics::~WinAPIGraphics() { Shutdown(); }

LRESULT CALLBACK WinAPIGraphics::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WinAPIGraphics* self = reinterpret_cast<WinAPIGraphics*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self) {
        return self->HandleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT WinAPIGraphics::HandleMessage(HWND h, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hwnd = h;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            return 0;

        case WM_KEYDOWN:
            eventQueue.push_back({WindowEvent::KEY_DOWN, (int)wParam, 0});
            return 0;

        case WM_CLOSE:
            running = false;
            DestroyWindow(hwnd);
            return 0;

        case WM_CHAR:
            eventQueue.push_back({WindowEvent::CHAR_INPUT, (int)wParam, 0});
            return 0;

        case WM_DESTROY:
            running = false;
            hwnd = nullptr;
            PostQuitMessage(0);
            return 0;

        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            wchar_t fileName[MAX_PATH];
            UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
            for (UINT i = 0; i < count; i++) {
                DragQueryFileW(hDrop, i, fileName, MAX_PATH);
                int len = WideCharToMultiByte(CP_UTF8, 0, fileName, -1, nullptr, 0, nullptr, nullptr);
                char* mbPath = new char[len];
                WideCharToMultiByte(CP_UTF8, 0, fileName, -1, mbPath, len, nullptr, nullptr);
                droppedFiles.push_back(std::string(mbPath));
                delete[] mbPath;
            }
            DragFinish(hDrop);
            return 0;
        }

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);
            if (hdcBuffer) {
                HDC hdc = GetDC(hwnd);
                HBITMAP old = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);
                DeleteObject(old);
                hbmBuffer = CreateCompatibleBitmap(hdc, width, height);
                SelectObject(hdcBuffer, hbmBuffer);
                ReleaseDC(hwnd, hdc);
            }
            return 0;

        case WM_LBUTTONDOWN:
            mouseDown = true;
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);
            eventQueue.push_back({WindowEvent::MOUSE_CLICK, mouseX, mouseY});
            return 0;

        case WM_LBUTTONDBLCLK:
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);
            eventQueue.push_back({WindowEvent::MOUSE_DBLCLICK, mouseX, mouseY});
            return 0;

        case WM_RBUTTONDOWN:
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);
            eventQueue.push_back({WindowEvent::MOUSE_RCLICK, mouseX, mouseY});
            return 0;

        case WM_MOUSEWHEEL:
            eventQueue.push_back({WindowEvent::MOUSE_SCROLL, GET_WHEEL_DELTA_WPARAM(wParam), mouseY});
            return 0;

        case WM_LBUTTONUP:
            mouseDown = false;
            return 0;

        case WM_MOUSEMOVE:
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);
            if (mouseDown) {
                eventQueue.push_back({WindowEvent::MOUSE_DRAG, mouseX, mouseY});
            }
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool WinAPIGraphics::Init(const std::string& title, int w, int hParam) {
    width = w;
    height = hParam;

    const wchar_t* CLASS_NAME = L"NovellEngineWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    if (!RegisterClassW(&wc)) return false;

    int len = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
    wchar_t* wTitle = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, wTitle, len);

    HWND hw = CreateWindowExW(
        0, CLASS_NAME, wTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr
    );

    delete[] wTitle;

    if (!hw) return false;

    hwnd = hw;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    HDC hdc = GetDC(hwnd);
    hdcBuffer = CreateCompatibleDC(hdc);
    hbmBuffer = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(hdcBuffer, hbmBuffer);
    ReleaseDC(hwnd, hdc);

    hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    DragAcceptFiles(hwnd, TRUE);

    running = true;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return true;
}

void WinAPIGraphics::Shutdown() {
    running = false;
    // Cleanup image cache
    for (auto& pair : imageCache) delete pair.second;
    imageCache.clear();
    GdiplusShutdown(gdiplusToken);
    if (hFont) { DeleteObject(hFont); hFont = nullptr; }
    if (hbmBuffer) { DeleteObject(hbmBuffer); hbmBuffer = nullptr; }
    if (hdcBuffer) { DeleteDC(hdcBuffer); hdcBuffer = nullptr; }
    if (hwnd) { DestroyWindow(hwnd); hwnd = nullptr; }
    UnregisterClassW(L"NovellEngineWindow", hInstance);
}

bool WinAPIGraphics::IsRunning() { return running; }

void WinAPIGraphics::PollEvents(std::vector<WindowEvent>& outEvents) {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    outEvents = eventQueue;
    eventQueue.clear();
}

void WinAPIGraphics::GetDroppedFiles(std::vector<std::string>& out) {
    out = droppedFiles;
    droppedFiles.clear();
}

void WinAPIGraphics::BeginFrame() {
    RECT rect;
    GetClientRect(hwnd, &rect);
    HBRUSH brush = CreateSolidBrush(RGB(20, 20, 30));
    FillRect(hdcBuffer, &rect, brush);
    DeleteObject(brush);
}

void WinAPIGraphics::EndFrame() {
    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, 0, 0, width, height, hdcBuffer, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void WinAPIGraphics::Clear(unsigned char r, unsigned char g, unsigned char b) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
    FillRect(hdcBuffer, &rect, brush);
    DeleteObject(brush);
}

void WinAPIGraphics::DrawRect(int x, int y, int w, int h,
                              unsigned char r, unsigned char g, unsigned char b) {
    HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
    RECT rect = {x, y, x + w, y + h};
    FillRect(hdcBuffer, &rect, brush);
    DeleteObject(brush);
}

void WinAPIGraphics::RenderText(const std::string& text, int x, int y,
                                unsigned char r, unsigned char g, unsigned char b) {
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hFont);
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(r, g, b));

    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    wchar_t* wText = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wText, len);

    TextOutW(hdcBuffer, x, y, wText, len - 1);
    delete[] wText;
    SelectObject(hdcBuffer, oldFont);
}

int WinAPIGraphics::GetWidth() const { return width; }
int WinAPIGraphics::GetHeight() const { return height; }

void WinAPIGraphics::DrawImage(int x, int y, int w, int h, const std::string& filePath) {
    auto it = imageCache.find(filePath);
    Gdiplus::Bitmap* bmp = nullptr;
    if (it != imageCache.end()) {
        bmp = it->second;
    } else {
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0);
        wchar_t* widePath = new wchar_t[wideLen];
        MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, widePath, wideLen);
        bmp = Gdiplus::Bitmap::FromFile(widePath);
        delete[] widePath;
        if (!bmp || bmp->GetLastStatus() != Ok) { if (bmp) delete bmp; return; }
        imageCache[filePath] = bmp;
    }
    Graphics g(hdcBuffer);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(bmp, x, y, w, h);
}

void WinAPIGraphics::SetClipRect(int x, int y, int w, int h) {
    HRGN rgn = CreateRectRgn(x, y, x + w, y + h);
    SelectClipRgn(hdcBuffer, rgn);
    DeleteObject(rgn);
}

void WinAPIGraphics::ResetClip() {
    SelectClipRgn(hdcBuffer, NULL);
}

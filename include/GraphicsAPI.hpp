#pragma once
#include <string>
#include <vector>

struct WindowEvent {
    enum Type { KEY_DOWN, CLOSE, CHAR_INPUT, MOUSE_CLICK, MOUSE_DBLCLICK, MOUSE_DRAG, MOUSE_RCLICK, MOUSE_SCROLL };
    Type type;
    int param1;
    int param2;
};

class GraphicsAPI {
public:
    virtual ~GraphicsAPI() = default;
    virtual bool Init(const std::string& title, int width, int height) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsRunning() = 0;
    virtual void PollEvents(std::vector<WindowEvent>& outEvents) = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void DrawRect(int x, int y, int w, int h,
                          unsigned char r, unsigned char g, unsigned char b) = 0;
    virtual void DrawImage(int x, int y, int w, int h, const std::string& filePath) = 0;
    virtual void RenderText(const std::string& text, int x, int y,
                            unsigned char r, unsigned char g, unsigned char b) = 0;
    virtual void Clear(unsigned char r, unsigned char g, unsigned char b) = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual void SetClipRect(int x, int y, int w, int h) = 0;
    virtual void ResetClip() = 0;
};

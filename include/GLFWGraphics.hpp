#pragma once
#include "GraphicsAPI.hpp"

#if defined(USE_GLFW) && USE_GLFW
#include <GLFW/glfw3.h>
#include <GL/gl.h>

class GLFWGraphics : public GraphicsAPI {
public:
    GLFWGraphics();
    ~GLFWGraphics();

    bool Init(const std::string& title, int width, int height) override;
    void Shutdown() override;
    bool IsRunning() override;
    void PollEvents(std::vector<WindowEvent>& outEvents) override;
    void BeginFrame() override;
    void EndFrame() override;
    void DrawRect(int x, int y, int w, int h,
                  unsigned char r, unsigned char g, unsigned char b) override;
    void RenderText(const std::string& text, int x, int y,
                    unsigned char r, unsigned char g, unsigned char b) override;
    void Clear(unsigned char r, unsigned char g, unsigned char b) override;
    int GetWidth() const override;
    int GetHeight() const override;

private:
    GLFWwindow* window;
    bool running;
    int width, height;
};
#endif

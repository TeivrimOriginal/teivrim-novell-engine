#if defined(USE_GLFW) && USE_GLFW

#include "GLFWGraphics.hpp"
#include <cstring>

GLFWGraphics::GLFWGraphics() : window(nullptr), running(false), width(800), height(600) {}

GLFWGraphics::~GLFWGraphics() { Shutdown(); }

bool GLFWGraphics::Init(const std::string& title, int w, int h) {
    width = w;
    height = h;

    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    running = true;
    return true;
}

void GLFWGraphics::Shutdown() {
    running = false;
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool GLFWGraphics::IsRunning() {
    return running && !glfwWindowShouldClose(window);
}

void GLFWGraphics::PollEvents(std::vector<WindowEvent>& outEvents) {
    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        running = false;
        outEvents.push_back({WindowEvent::CLOSE, 0, 0});
    }
}

void GLFWGraphics::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLFWGraphics::EndFrame() {
    glfwSwapBuffers(window);
}

void GLFWGraphics::Clear(unsigned char r, unsigned char g, unsigned char b) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLFWGraphics::DrawRect(int x, int y, int w, int h,
                            unsigned char r, unsigned char g, unsigned char b) {
    glColor3ub(r, g, b);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
}

void GLFWGraphics::RenderText(const std::string& text, int x, int y,
                              unsigned char r, unsigned char g, unsigned char b) {
    glColor3ub(r, g, b);
    glRasterPos2i(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
}

int GLFWGraphics::GetWidth() const { return width; }
int GLFWGraphics::GetHeight() const { return height; }

#endif

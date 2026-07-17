#pragma once
#include <string>
#include <map>
#include "EngineCore.hpp"
#include "Scene.cpp"

class Interface {
public:
    void DrawText(std::string text, float x, float y) {
        // Simplified text rendering
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(x, y);
        for (char c : text) {
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
        }
    }
    
    void DrawButton(std::string text, float x, float y, float width, float height) {
        // Draw button background
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y - height);
        glVertex2f(x, y - height);
        glEnd();
        
        // Draw button text
        DrawText(text, x + 5, y - 15);
    }
};

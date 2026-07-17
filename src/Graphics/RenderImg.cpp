#pragma once
#include <string>
#include <GL/gl.h>
#include "parserImg.cpp"
#include "EngineCore.hpp"

class RenderImg {
public:
    void DrawImage(Image* img, float x, float y, float scale) {
        if (!img) return;
        
        img->Bind();
        
        // Draw quad with texture
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex2f(x, y);
        glTexCoord2f(1, 1);
        glVertex2f(x + (img->width * scale), y);
        glTexCoord2f(1, 0);
        glVertex2f(x + (img->width * scale), y - (img->height * scale));
        glTexCoord2f(0, 0);
        glVertex2f(x, y - (img->height * scale));
        glEnd();
    }
    
    void BeginFrame() {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    void EndFrame() {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
};

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <GL/gl.h>

class Image {
public:
    std::string path;
    int width;
    int height;
    unsigned char* data;
    GLuint textureId;
    
    Image(std::string p) {
        path = p;
        data = nullptr;
        textureId = 0;
        Load();
    }
    
    ~Image() {
        if (data) delete[] data;
        if (textureId) glDeleteTextures(1, &textureId);
    }
    
    void Load() {
        // Simplified image loading - in real implementation would use stb_image or similar
        // For testing, we just set dummy values
        width = 100;
        height = 100;
        data = new unsigned char[width * height * 3];
        // Generate texture ID
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    void Bind() {
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
};

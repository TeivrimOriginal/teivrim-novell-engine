#pragma once
#include <vector>
#include "EngineCore.hpp"
#include "RenderImg.cpp"

class ScenePosition {
public:
    float x;
    float y;
    float scale;
    std::string imageName;
    
    ScenePosition(float xPos, float yPos, float s, std::string name) {
        x = xPos;
        y = yPos;
        scale = s;
        imageName = name;
    }
};

class Scene {
public:
    std::string name;
    std::vector<ScenePosition> positions;
    RenderImg renderer;
    
    Scene(std::string sceneName) {
        name = sceneName;
    }
    
    void AddImage(float x, float y, float scale, std::string imageName) {
        positions.push_back(ScenePosition(x, y, scale, imageName));
    }
    
    void RenderAll() {
        renderer.BeginFrame();
        for (auto& pos : positions) {
            renderer.DrawImage(nullptr, pos.x, pos.y, pos.scale);
        }
        renderer.EndFrame();
    }
};

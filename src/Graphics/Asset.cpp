#pragma once
#include <string>
#include <map>
#include <memory>
#include "EngineCore.hpp"

class Asset {
private:
    static std::map<std::string, Image*> imageCache;
    
public:
    static Image* LoadImage(std::string path) {
        if (imageCache.find(path) == imageCache.end()) {
            imageCache[path] = new Image(path);
        }
        return imageCache[path];
    }
    
    static void UnloadImage(std::string path) {
        if (imageCache.find(path) != imageCache.end()) {
            delete imageCache[path];
            imageCache.erase(path);
        }
    }
    
    static void ClearCache() {
        for (auto& pair : imageCache) {
            delete pair.second;
        }
        imageCache.clear();
    }
};

std::map<std::string, Image*> Asset::imageCache;

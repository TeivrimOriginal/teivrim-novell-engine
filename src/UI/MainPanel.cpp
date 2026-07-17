#pragma once
#include <string>
#include "EngineCore.hpp"
#include "Interface.cpp"

class MainPanel {
public:
    std::string title;
    std::vector<std::string> options;
    Interface ui;
    
    MainPanel(std::string winTitle) {
        title = winTitle;
        options = {"Start Game", "Load Game", "Options", "Exit"};
    }
    
    void Draw() {
        ui.DrawText(title, 10, 250);
        
        float y = 200;
        for (size_t i = 0; i < options.size(); i++) {
            ui.DrawButton(options[i], 10, y, 200, 30);
            y -= 40;
        }
    }
    
    int GetSelectedOption() {
        // For testing, return 0
        return 0;
    }
};

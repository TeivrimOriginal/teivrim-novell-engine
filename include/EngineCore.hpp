#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

class Character {
public:
    string name;
    string spritePath;
    string voicePath;
    
    Character(string n, string sprite, string voice) {
        name = n;
        spritePath = sprite;
        voicePath = voice;
    }
    
    void Speak(string text) {
        cout << name << ": " << text << endl;
    }
};

class Scene {
public:
    string name;
    string backgroundPath;
    vector<Character> characters;
    string scriptPath;
    
    Scene() {}
    Scene(string n, string background) {
        name = n;
        backgroundPath = background;
    }
    
    void AddCharacter(Character chara) {
        characters.push_back(chara);
    }
    
    void LoadScript() {
        ifstream file(scriptPath);
        if (file.is_open()) {
            string line;
            cout << "--- Scene: " << name << " ---" << endl;
            while (getline(file, line)) {
                cout << line << endl;
            }
            cout << "--- End ---" << endl;
        } else {
            cout << "No script file found for scene: " << name << endl;
        }
    }
};

class VisualNovelEngine {
public:
    string currentScene;
    map<string, Scene> scenes;
    bool isRunning;
    
    VisualNovelEngine() {
        isRunning = false;
    }
    
    void Initialize(string scriptPath) {
        isRunning = true;
        cout << "=== Visual Novel Engine ===" << endl;
        cout << "Script: " << scriptPath << endl;
        cout << "Engine initialized!" << endl;
    }
    
    void LoadScene(string sceneName) {
        if (scenes.find(sceneName) != scenes.end()) {
            currentScene = sceneName;
            scenes[sceneName].LoadScript();
        }
    }
    
    void AddScene(Scene scene) {
        scenes[scene.name] = scene;
    }
    
    void RunLoop() {
        while (isRunning) {
            cout << "Press 'q' to quit or any key to continue..." << endl;
            char input;
            cin >> input;
            if (input == 'q' || input == 'Q') {
                isRunning = false;
            }
        }
    }
};

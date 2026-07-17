#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

class Data {
public:
    static std::vector<std::string> ReadFileLines(std::string path) {
        std::vector<std::string> lines;
        std::ifstream file(path);
        
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        }
        
        return lines;
    }
    
    static std::string ReadFile(std::string path) {
        std::ifstream file(path);
        std::stringstream buffer;
        
        if (file.is_open()) {
            buffer << file.rdbuf();
            file.close();
        }
        
        return buffer.str();
    }
};

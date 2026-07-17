#include "ProjectManager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

ProjectManager::ProjectManager() {}

bool ProjectManager::Load(const std::string& fp) {
    filePath = fp;
    projects.clear();

    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        Project p;
        std::getline(ss, p.name, '|');
        std::getline(ss, p.path, '|');
        std::getline(ss, p.description, '|');
        std::getline(ss, p.createdAt, '|');
        projects.push_back(p);
    }

    file.close();
    return true;
}

bool ProjectManager::Save() {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    for (auto& p : projects) {
        file << p.name << "|" << p.path << "|"
             << p.description << "|" << p.createdAt << "\n";
    }

    file.close();
    return true;
}

std::vector<Project>& ProjectManager::List() { return projects; }
int ProjectManager::GetCount() const { return (int)projects.size(); }

bool ProjectManager::Add(const Project& project) {
    for (auto& p : projects) {
        if (p.name == project.name) return false;
    }
    projects.push_back(project);
    return Save();
}

bool ProjectManager::Delete(const std::string& name) {
    for (size_t i = 0; i < projects.size(); i++) {
        if (projects[i].name == name) {
            projects.erase(projects.begin() + i);
            return Save();
        }
    }
    return false;
}

bool ProjectManager::Rename(const std::string& oldName, const std::string& newName) {
    if (oldName == newName) return true;
    for (auto& p : projects) {
        if (p.name == newName) return false;
    }
    for (auto& p : projects) {
        if (p.name == oldName) {
            p.name = newName;
            return Save();
        }
    }
    return false;
}

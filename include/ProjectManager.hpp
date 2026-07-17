#pragma once
#include <string>
#include <vector>

struct Project {
    std::string name;
    std::string path;
    std::string description;
    std::string createdAt;
};

class ProjectManager {
public:
    ProjectManager();

    bool Load(const std::string& filePath);
    bool Save();

    std::vector<Project>& List();
    int GetCount() const;

    bool Add(const Project& project);
    bool Delete(const std::string& name);
    bool Rename(const std::string& oldName, const std::string& newName);

private:
    std::vector<Project> projects;
    std::string filePath;
};

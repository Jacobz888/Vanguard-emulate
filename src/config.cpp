#include "../include/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    setDefaults();
}

Config::~Config() {
}

void Config::setDefaults() {
    m_settings["pipe_name"] = "933823D3-C77B-4BAE-89D7-A92B567236BC";
    m_settings["buffer_size"] = "104857600";
    m_settings["service_restart_delay"] = "2000";
    m_settings["log_level"] = "INFO";
}

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            m_settings[key] = value;
        }
    }
    
    file.close();
    return true;
}

std::string Config::getPipeName() const {
    auto it = m_settings.find("pipe_name");
    return (it != m_settings.end()) ? it->second : "933823D3-C77B-4BAE-89D7-A92B567236BC";
}

size_t Config::getBufferSize() const {
    auto it = m_settings.find("buffer_size");
    if (it != m_settings.end()) {
        try {
            return std::stoull(it->second);
        } catch (...) {
        }
    }
    return 104857600;
}

int Config::getServiceRestartDelay() const {
    auto it = m_settings.find("service_restart_delay");
    if (it != m_settings.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
        }
    }
    return 2000;
}

std::string Config::getLogLevel() const {
    auto it = m_settings.find("log_level");
    return (it != m_settings.end()) ? it->second : "INFO";
}

std::string Config::get(const std::string& key, const std::string& defaultValue) const {
    auto it = m_settings.find(key);
    return (it != m_settings.end()) ? it->second : defaultValue;
}

std::string Config::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

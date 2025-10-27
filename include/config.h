#pragma once

#include <string>
#include <map>

class Config {
public:
    static Config& getInstance();
    
    bool loadFromFile(const std::string& filename);
    
    std::string getPipeName() const;
    size_t getBufferSize() const;
    int getServiceRestartDelay() const;
    std::string getLogLevel() const;
    
    std::string get(const std::string& key, const std::string& defaultValue = "") const;
    
private:
    Config();
    ~Config();
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    void setDefaults();
    std::string trim(const std::string& str);
    
    std::map<std::string, std::string> m_settings;
};

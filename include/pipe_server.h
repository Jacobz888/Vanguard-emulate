#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

class PipeServer {
public:
    PipeServer(const std::string& pipeName, size_t bufferSize = 100 * 1024 * 1024);
    ~PipeServer();
    
    bool start();
    void stop();
    bool isRunning() const;
    
    bool adjustPrivileges();
    
private:
    void acceptConnections();
    void handleClient(HANDLE pipeHandle);
    HANDLE createNamedPipe();
    
    std::string m_pipeName;
    std::string m_fullPipeName;
    size_t m_bufferSize;
    std::atomic<bool> m_running;
    std::vector<std::thread> m_threads;
    std::thread m_acceptThread;
};

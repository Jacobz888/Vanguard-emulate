#include "../include/service_manager.h"
#include "../include/logger.h"
#include <sstream>
#include <thread>
#include <chrono>

ServiceManager::ServiceManager() : m_scManager(nullptr) {
    m_scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!m_scManager) {
        Logger::getInstance().error("Failed to open Service Control Manager");
    }
}

ServiceManager::~ServiceManager() {
    if (m_scManager) {
        CloseServiceHandle(m_scManager);
    }
}

bool ServiceManager::stopService(const std::string& serviceName) {
    if (!m_scManager) {
        Logger::getInstance().error("Service Control Manager not available");
        return false;
    }
    
    Logger::getInstance().info("Stopping service: " + serviceName);
    
    SC_HANDLE service = OpenService(m_scManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!service) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
            Logger::getInstance().warning("Service does not exist: " + serviceName);
            return true;
        }
        Logger::getInstance().error("Failed to open service: " + serviceName);
        return false;
    }
    
    SERVICE_STATUS status;
    if (!ControlService(service, SERVICE_CONTROL_STOP, &status)) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_NOT_ACTIVE) {
            Logger::getInstance().info("Service already stopped: " + serviceName);
            CloseServiceHandle(service);
            return true;
        }
        Logger::getInstance().error("Failed to stop service: " + serviceName);
        CloseServiceHandle(service);
        return false;
    }
    
    CloseServiceHandle(service);
    
    bool stopped = waitForServiceState(serviceName, SERVICE_STOPPED);
    if (stopped) {
        Logger::getInstance().info("Service stopped successfully: " + serviceName);
    }
    
    return stopped;
}

bool ServiceManager::startService(const std::string& serviceName) {
    if (!m_scManager) {
        Logger::getInstance().error("Service Control Manager not available");
        return false;
    }
    
    Logger::getInstance().info("Starting service: " + serviceName);
    
    SC_HANDLE service = OpenService(m_scManager, serviceName.c_str(), SERVICE_START | SERVICE_QUERY_STATUS);
    if (!service) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
            Logger::getInstance().warning("Service does not exist: " + serviceName);
            return false;
        }
        Logger::getInstance().error("Failed to open service: " + serviceName);
        return false;
    }
    
    if (!StartService(service, 0, nullptr)) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_ALREADY_RUNNING) {
            Logger::getInstance().info("Service already running: " + serviceName);
            CloseServiceHandle(service);
            return true;
        }
        Logger::getInstance().error("Failed to start service: " + serviceName);
        CloseServiceHandle(service);
        return false;
    }
    
    CloseServiceHandle(service);
    
    bool started = waitForServiceState(serviceName, SERVICE_RUNNING);
    if (started) {
        Logger::getInstance().info("Service started successfully: " + serviceName);
    }
    
    return started;
}

bool ServiceManager::restartService(const std::string& serviceName) {
    if (!stopService(serviceName)) {
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return startService(serviceName);
}

bool ServiceManager::isServiceRunning(const std::string& serviceName) {
    SERVICE_STATUS_PROCESS status = queryServiceStatus(serviceName);
    return status.dwCurrentState == SERVICE_RUNNING;
}

bool ServiceManager::stopVanguardServices() {
    Logger::getInstance().info("Stopping Vanguard services");
    
    bool vgkStopped = stopService("vgk");
    bool vgcStopped = stopService("vgc");
    
    return vgkStopped && vgcStopped;
}

bool ServiceManager::startVanguardServices() {
    Logger::getInstance().info("Starting Vanguard services");
    
    bool vgkStarted = startService("vgk");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    bool vgcStarted = startService("vgc");
    
    return vgkStarted && vgcStarted;
}

bool ServiceManager::restartVanguardServices() {
    Logger::getInstance().info("Restarting Vanguard services");
    
    if (!stopVanguardServices()) {
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return startVanguardServices();
}

bool ServiceManager::waitForServiceState(const std::string& serviceName, DWORD targetState, int timeoutSeconds) {
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        SERVICE_STATUS_PROCESS status = queryServiceStatus(serviceName);
        
        if (status.dwCurrentState == targetState) {
            return true;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count();
        
        if (elapsed >= timeoutSeconds) {
            Logger::getInstance().error("Timeout waiting for service state: " + serviceName);
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

SERVICE_STATUS_PROCESS ServiceManager::queryServiceStatus(const std::string& serviceName) {
    SERVICE_STATUS_PROCESS status = { 0 };
    
    if (!m_scManager) {
        return status;
    }
    
    SC_HANDLE service = OpenService(m_scManager, serviceName.c_str(), SERVICE_QUERY_STATUS);
    if (!service) {
        return status;
    }
    
    DWORD bytesNeeded;
    QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, 
                        reinterpret_cast<LPBYTE>(&status), 
                        sizeof(SERVICE_STATUS_PROCESS), 
                        &bytesNeeded);
    
    CloseServiceHandle(service);
    return status;
}

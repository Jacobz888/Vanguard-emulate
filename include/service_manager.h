#pragma once

#include <string>
#include <windows.h>

class ServiceManager {
public:
    ServiceManager();
    ~ServiceManager();
    
    bool stopService(const std::string& serviceName);
    bool startService(const std::string& serviceName);
    bool restartService(const std::string& serviceName);
    bool isServiceRunning(const std::string& serviceName);
    
    bool stopVanguardServices();
    bool startVanguardServices();
    bool restartVanguardServices();
    
private:
    bool executeServiceCommand(const std::string& command);
    bool waitForServiceState(const std::string& serviceName, DWORD targetState, int timeoutSeconds = 30);
    SERVICE_STATUS_PROCESS queryServiceStatus(const std::string& serviceName);
    
    SC_HANDLE m_scManager;
};

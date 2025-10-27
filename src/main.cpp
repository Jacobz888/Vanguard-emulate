#include "../include/logger.h"
#include "../include/config.h"
#include "../include/service_manager.h"
#include "../include/pipe_server.h"
#include <windows.h>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> g_running(true);

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        Logger::getInstance().info("Received shutdown signal");
        g_running = false;
    }
}

bool checkAdminPrivileges() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin == TRUE;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  Vanguard Pipe-Hijacking Emulator" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    Logger& logger = Logger::getInstance();
    logger.setLogFile("vanguard_emulator.log");
    
    Config& config = Config::getInstance();
    if (config.loadFromFile("config.ini")) {
        logger.info("Configuration loaded from config.ini");
    } else {
        logger.warning("Failed to load config.ini, using defaults");
    }
    
    std::string logLevel = config.getLogLevel();
    if (logLevel == "DEBUG") {
        logger.setLogLevel(LogLevel::DEBUG);
    } else if (logLevel == "WARNING") {
        logger.setLogLevel(LogLevel::WARNING);
    } else if (logLevel == "ERROR") {
        logger.setLogLevel(LogLevel::ERROR);
    } else {
        logger.setLogLevel(LogLevel::INFO);
    }
    
    if (!checkAdminPrivileges()) {
        logger.error("This application requires administrator privileges");
        std::cerr << "ERROR: Please run as administrator" << std::endl;
        return 1;
    }
    
    logger.info("Running with administrator privileges");
    
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::string pipeName = config.getPipeName();
    size_t bufferSize = config.getBufferSize();
    int restartDelay = config.getServiceRestartDelay();
    
    logger.info("Pipe name: " + pipeName);
    logger.info("Buffer size: " + std::to_string(bufferSize) + " bytes");
    logger.info("Service restart delay: " + std::to_string(restartDelay) + " ms");
    
    PipeServer pipeServer(pipeName, bufferSize);
    
    if (!pipeServer.adjustPrivileges()) {
        logger.error("Failed to adjust privileges");
        return 1;
    }
    
    ServiceManager serviceManager;
    
    logger.info("===== Phase 1: Stopping Vanguard Services =====");
    if (!serviceManager.stopVanguardServices()) {
        logger.warning("Failed to stop all Vanguard services");
    }
    
    logger.info("Waiting for services to fully stop...");
    std::this_thread::sleep_for(std::chrono::milliseconds(restartDelay));
    
    logger.info("===== Phase 2: Starting Pipe Server =====");
    if (!pipeServer.start()) {
        logger.error("Failed to start pipe server");
        return 1;
    }
    
    logger.info("Pipe server started successfully");
    
    pipeServer.enableProtocolAnalysis(true);
    logger.info("Protocol analysis enabled");
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    logger.info("===== Phase 3: Restarting Vanguard Services =====");
    if (!serviceManager.startVanguardServices()) {
        logger.warning("Failed to start all Vanguard services");
    }
    
    logger.info("===== Emulator Running =====");
    logger.info("Pipe hijacking active. Press Ctrl+C to stop.");
    logger.info("Protocol analysis active - capturing all messages");
    
    while (g_running && pipeServer.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    logger.info("===== Shutting Down =====");
    
    logger.info("Generating protocol analysis report...");
    pipeServer.generateAnalysisReport("logs/protocol_analysis_report.txt");
    
    pipeServer.stop();
    
    logger.info("Emulator stopped");
    logger.info("Analysis artifacts saved in logs/ directory");
    return 0;
}

#include "../include/pipe_server.h"
#include "../include/logger.h"
#include "../include/message_parser.h"
#include <sstream>
#include <chrono>
#include <iomanip>

PipeServer::PipeServer(const std::string& pipeName, size_t bufferSize)
    : m_pipeName(pipeName), m_bufferSize(bufferSize), m_running(false), 
      m_analysisEnabled(false), m_messageParser(nullptr) {
    m_fullPipeName = "\\\\.\\pipe\\" + pipeName;
}

PipeServer::~PipeServer() {
    stop();
}

bool PipeServer::adjustPrivileges() {
    Logger::getInstance().info("Adjusting process privileges");
    
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        Logger::getInstance().error("Failed to open process token");
        return false;
    }
    
    TOKEN_PRIVILEGES tp;
    LUID luid;
    
    if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)) {
        Logger::getInstance().error("Failed to lookup privilege value");
        CloseHandle(hToken);
        return false;
    }
    
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
        Logger::getInstance().error("Failed to adjust token privileges");
        CloseHandle(hToken);
        return false;
    }
    
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        Logger::getInstance().error("Failed to assign all privileges");
        CloseHandle(hToken);
        return false;
    }
    
    CloseHandle(hToken);
    Logger::getInstance().info("SeDebugPrivilege enabled successfully");
    return true;
}

bool PipeServer::start() {
    if (m_running) {
        Logger::getInstance().warning("Pipe server already running");
        return false;
    }
    
    Logger::getInstance().info("Starting pipe server: " + m_fullPipeName);
    
    m_running = true;
    m_acceptThread = std::thread(&PipeServer::acceptConnections, this);
    
    return true;
}

void PipeServer::stop() {
    if (!m_running) {
        return;
    }
    
    Logger::getInstance().info("Stopping pipe server");
    m_running = false;
    
    if (m_acceptThread.joinable()) {
        m_acceptThread.join();
    }
    
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    m_threads.clear();
    Logger::getInstance().info("Pipe server stopped");
}

bool PipeServer::isRunning() const {
    return m_running;
}

void PipeServer::enableProtocolAnalysis(bool enable) {
    m_analysisEnabled = enable;
    
    if (enable && !m_messageParser) {
        m_messageParser = std::make_unique<MessageParser>();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        struct tm timeinfo;
        localtime_s(&timeinfo, &time);
        std::stringstream ss;
        ss << "logs/message_captures/capture_" 
           << std::put_time(&timeinfo, "%Y%m%d_%H%M%S") 
           << ".log";
        
        m_messageParser->setCaptureFile(ss.str());
        Logger::getInstance().info("Protocol analysis enabled. Capturing to: " + ss.str());
    } else if (!enable && m_messageParser) {
        Logger::getInstance().info("Protocol analysis disabled");
    }
}

void PipeServer::generateAnalysisReport(const std::string& filename) {
    if (m_messageParser) {
        m_messageParser->generateReport(filename);
        Logger::getInstance().info("Analysis report generated: " + filename);
    } else {
        Logger::getInstance().warning("Cannot generate report: protocol analysis not enabled");
    }
}

HANDLE PipeServer::createNamedPipe() {
    HANDLE hPipe = CreateNamedPipeA(
        m_fullPipeName.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        static_cast<DWORD>(m_bufferSize),
        static_cast<DWORD>(m_bufferSize),
        0,
        nullptr
    );
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::stringstream ss;
        ss << "Failed to create named pipe. Error: " << error;
        Logger::getInstance().error(ss.str());
        return INVALID_HANDLE_VALUE;
    }
    
    return hPipe;
}

void PipeServer::acceptConnections() {
    Logger::getInstance().info("Pipe server accepting connections");
    
    while (m_running) {
        HANDLE hPipe = createNamedPipe();
        if (hPipe == INVALID_HANDLE_VALUE) {
            Logger::getInstance().error("Failed to create pipe instance");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        
        Logger::getInstance().info("Waiting for client connection...");
        
        BOOL connected = ConnectNamedPipe(hPipe, nullptr);
        if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
            if (m_running) {
                Logger::getInstance().error("Failed to connect to client");
            }
            CloseHandle(hPipe);
            continue;
        }
        
        Logger::getInstance().info("Client connected");
        
        m_threads.push_back(std::thread(&PipeServer::handleClient, this, hPipe));
    }
}

void PipeServer::handleClient(HANDLE pipeHandle) {
    Logger::getInstance().info("Handling client connection");
    
    std::vector<char> buffer(m_bufferSize);
    uint64_t messageCount = 0;
    auto sessionStart = std::chrono::steady_clock::now();
    
    while (m_running) {
        DWORD bytesRead = 0;
        auto readStart = std::chrono::high_resolution_clock::now();
        
        BOOL success = ReadFile(
            pipeHandle,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            &bytesRead,
            nullptr
        );
        
        auto readEnd = std::chrono::high_resolution_clock::now();
        
        if (!success || bytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                Logger::getInstance().info("Client disconnected");
            } else {
                Logger::getInstance().error("Failed to read from pipe");
            }
            break;
        }
        
        messageCount++;
        
        std::stringstream ss;
        ss << "Message #" << messageCount << ": Received " << bytesRead << " bytes from client";
        Logger::getInstance().debug(ss.str());
        
        if (m_analysisEnabled && m_messageParser) {
            m_messageParser->captureMessage(buffer.data(), bytesRead, true);
            
            auto readDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                readEnd - readStart).count();
            
            ss.str("");
            ss << "Read time: " << readDuration << " microseconds";
            Logger::getInstance().debug(ss.str());
            
            ss.str("");
            ss << "Hex preview (first 64 bytes):\n" 
               << m_messageParser->hexDump(buffer.data(), std::min<size_t>(64, bytesRead), true);
            Logger::getInstance().debug(ss.str());
        }
        
        auto writeStart = std::chrono::high_resolution_clock::now();
        
        DWORD bytesWritten = 0;
        success = WriteFile(
            pipeHandle,
            buffer.data(),
            bytesRead,
            &bytesWritten,
            nullptr
        );
        
        auto writeEnd = std::chrono::high_resolution_clock::now();
        
        if (!success) {
            Logger::getInstance().error("Failed to write to pipe");
            break;
        }
        
        if (m_analysisEnabled && m_messageParser) {
            m_messageParser->captureMessage(buffer.data(), bytesWritten, false);
            
            auto writeDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                writeEnd - writeStart).count();
            
            ss.str("");
            ss << "Write time: " << writeDuration << " microseconds";
            Logger::getInstance().debug(ss.str());
        }
        
        ss.str("");
        ss << "Echoed " << bytesWritten << " bytes back to client";
        Logger::getInstance().debug(ss.str());
        
        FlushFileBuffers(pipeHandle);
        
        auto now = std::chrono::steady_clock::now();
        auto sessionDuration = std::chrono::duration_cast<std::chrono::seconds>(
            now - sessionStart).count();
        
        if (m_analysisEnabled && messageCount % 10 == 0) {
            ss.str("");
            ss << "Session stats: " << messageCount << " messages in " 
               << sessionDuration << " seconds";
            Logger::getInstance().info(ss.str());
        }
    }
    
    DisconnectNamedPipe(pipeHandle);
    CloseHandle(pipeHandle);
    
    std::stringstream ss;
    ss << "Client handler terminated. Total messages: " << messageCount;
    Logger::getInstance().info(ss.str());
}

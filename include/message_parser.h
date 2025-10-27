#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <chrono>

struct MessageStats {
    uint64_t messageCount;
    uint64_t totalBytes;
    std::chrono::steady_clock::time_point firstMessage;
    std::chrono::steady_clock::time_point lastMessage;
    std::vector<size_t> messageSizes;
    
    MessageStats();
    void recordMessage(size_t size);
    double getAverageSize() const;
    double getMessageFrequency() const;
    std::string getSummary() const;
};

struct MessagePattern {
    std::vector<uint8_t> bytes;
    size_t offset;
    size_t frequency;
    
    MessagePattern(const std::vector<uint8_t>& b, size_t off);
};

class MessageParser {
public:
    MessageParser();
    ~MessageParser();
    
    void setCaptureFile(const std::string& filename);
    void captureMessage(const void* data, size_t size, bool isIncoming);
    
    std::string hexDump(const void* data, size_t size, bool includeAscii = true) const;
    
    std::string analyzeStructure(const void* data, size_t size) const;
    
    bool detectMagicBytes(const void* data, size_t size, std::vector<uint8_t>& magic) const;
    bool hasLengthField(const void* data, size_t size, size_t& lengthFieldOffset) const;
    
    std::vector<MessagePattern> findPatterns(const std::vector<std::vector<uint8_t>>& messages) const;
    
    MessageStats& getIncomingStats() { return m_incomingStats; }
    MessageStats& getOutgoingStats() { return m_outgoingStats; }
    
    void reset();
    void generateReport(const std::string& filename) const;
    
private:
    std::string formatTimestamp() const;
    std::string detectMessageType(const void* data, size_t size) const;
    bool isTextBased(const void* data, size_t size) const;
    
    std::ofstream m_captureFile;
    MessageStats m_incomingStats;
    MessageStats m_outgoingStats;
    std::vector<std::vector<uint8_t>> m_capturedIncoming;
    std::vector<std::vector<uint8_t>> m_capturedOutgoing;
};

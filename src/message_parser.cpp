#include "../include/message_parser.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <map>

MessageStats::MessageStats() 
    : messageCount(0), totalBytes(0) {
}

void MessageStats::recordMessage(size_t size) {
    auto now = std::chrono::steady_clock::now();
    
    if (messageCount == 0) {
        firstMessage = now;
    }
    lastMessage = now;
    
    messageCount++;
    totalBytes += size;
    messageSizes.push_back(size);
}

double MessageStats::getAverageSize() const {
    if (messageCount == 0) return 0.0;
    return static_cast<double>(totalBytes) / messageCount;
}

double MessageStats::getMessageFrequency() const {
    if (messageCount < 2) return 0.0;
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        lastMessage - firstMessage).count();
    
    if (duration == 0) return 0.0;
    
    return (messageCount - 1) * 1000.0 / duration;
}

std::string MessageStats::getSummary() const {
    std::stringstream ss;
    ss << "Messages: " << messageCount << "\n";
    ss << "Total bytes: " << totalBytes << "\n";
    ss << "Average size: " << std::fixed << std::setprecision(2) << getAverageSize() << " bytes\n";
    ss << "Frequency: " << std::fixed << std::setprecision(2) << getMessageFrequency() << " msg/sec";
    return ss.str();
}

MessagePattern::MessagePattern(const std::vector<uint8_t>& b, size_t off)
    : bytes(b), offset(off), frequency(1) {
}

MessageParser::MessageParser() {
}

MessageParser::~MessageParser() {
    if (m_captureFile.is_open()) {
        m_captureFile.close();
    }
}

void MessageParser::setCaptureFile(const std::string& filename) {
    if (m_captureFile.is_open()) {
        m_captureFile.close();
    }
    m_captureFile.open(filename, std::ios::app | std::ios::binary);
    
    if (m_captureFile.is_open()) {
        m_captureFile << "\n=== Message Capture Session Started: " 
                      << formatTimestamp() << " ===\n\n";
    }
}

void MessageParser::captureMessage(const void* data, size_t size, bool isIncoming) {
    if (size == 0) return;
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> message(bytes, bytes + size);
    
    if (isIncoming) {
        m_incomingStats.recordMessage(size);
        m_capturedIncoming.push_back(message);
    } else {
        m_outgoingStats.recordMessage(size);
        m_capturedOutgoing.push_back(message);
    }
    
    if (m_captureFile.is_open()) {
        m_captureFile << "[" << formatTimestamp() << "] "
                      << (isIncoming ? "INCOMING" : "OUTGOING")
                      << " (" << size << " bytes)\n";
        
        m_captureFile << analyzeStructure(data, size);
        m_captureFile << "\nHex Dump:\n";
        m_captureFile << hexDump(data, size, true);
        m_captureFile << "\n" << std::string(80, '-') << "\n\n";
        m_captureFile.flush();
    }
}

std::string MessageParser::hexDump(const void* data, size_t size, bool includeAscii) const {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    std::stringstream ss;
    
    for (size_t i = 0; i < size; i += 16) {
        ss << std::setfill('0') << std::setw(8) << std::hex << i << "  ";
        
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                ss << std::setfill('0') << std::setw(2) << std::hex 
                   << static_cast<unsigned int>(bytes[i + j]) << " ";
            } else {
                ss << "   ";
            }
            
            if (j == 7) ss << " ";
        }
        
        if (includeAscii) {
            ss << " |";
            for (size_t j = 0; j < 16 && i + j < size; j++) {
                uint8_t c = bytes[i + j];
                ss << (c >= 32 && c <= 126 ? static_cast<char>(c) : '.');
            }
            ss << "|";
        }
        
        ss << "\n";
    }
    
    return ss.str();
}

std::string MessageParser::analyzeStructure(const void* data, size_t size) const {
    std::stringstream ss;
    
    ss << "Structure Analysis:\n";
    ss << "  Size: " << size << " bytes (0x" << std::hex << size << std::dec << ")\n";
    ss << "  Type: " << detectMessageType(data, size) << "\n";
    
    if (size >= 4) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        
        ss << "  First 4 bytes (potential header):\n";
        ss << "    Hex: ";
        for (size_t i = 0; i < std::min(size_t(4), size); i++) {
            ss << std::setfill('0') << std::setw(2) << std::hex 
               << static_cast<unsigned int>(bytes[i]) << " ";
        }
        ss << std::dec << "\n";
        
        uint32_t asUint32 = 0;
        std::memcpy(&asUint32, bytes, 4);
        ss << "    As uint32 (LE): " << asUint32 << " (0x" << std::hex << asUint32 << std::dec << ")\n";
        
        uint32_t asUint32BE = 0;
        for (int i = 0; i < 4; i++) {
            asUint32BE = (asUint32BE << 8) | bytes[i];
        }
        ss << "    As uint32 (BE): " << asUint32BE << " (0x" << std::hex << asUint32BE << std::dec << ")\n";
        
        if (asUint32 == size - 4 || asUint32BE == size - 4) {
            ss << "    ** Possible length field (excluding header) **\n";
        } else if (asUint32 == size || asUint32BE == size) {
            ss << "    ** Possible length field (including header) **\n";
        }
    }
    
    std::vector<uint8_t> magic;
    if (detectMagicBytes(data, size, magic)) {
        ss << "  Magic bytes detected: ";
        for (uint8_t b : magic) {
            ss << std::setfill('0') << std::setw(2) << std::hex 
               << static_cast<unsigned int>(b) << " ";
        }
        ss << std::dec << "\n";
    }
    
    return ss.str();
}

bool MessageParser::detectMagicBytes(const void* data, size_t size, std::vector<uint8_t>& magic) const {
    if (size < 2) return false;
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    
    const std::vector<std::vector<uint8_t>> knownMagic = {
        {0x50, 0x4B},
        {0x4D, 0x5A},
        {0xFF, 0xD8, 0xFF},
        {0x89, 0x50, 0x4E, 0x47},
        {0x1F, 0x8B},
        {0x56, 0x47},
        {0xDE, 0xAD, 0xBE, 0xEF},
        {0xCA, 0xFE, 0xBA, 0xBE}
    };
    
    for (const auto& pattern : knownMagic) {
        if (size >= pattern.size()) {
            bool match = true;
            for (size_t i = 0; i < pattern.size(); i++) {
                if (bytes[i] != pattern[i]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                magic = pattern;
                return true;
            }
        }
    }
    
    return false;
}

bool MessageParser::hasLengthField(const void* data, size_t size, size_t& lengthFieldOffset) const {
    if (size < 4) return false;
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    
    for (size_t offset = 0; offset < std::min(size_t(8), size - 3); offset++) {
        uint32_t value = 0;
        std::memcpy(&value, bytes + offset, 4);
        
        if (value == size || value == size - 4 || value == size - offset - 4) {
            lengthFieldOffset = offset;
            return true;
        }
    }
    
    return false;
}

std::vector<MessagePattern> MessageParser::findPatterns(const std::vector<std::vector<uint8_t>>& messages) const {
    std::vector<MessagePattern> patterns;
    
    if (messages.empty()) return patterns;
    
    std::map<std::vector<uint8_t>, size_t> headerCounts;
    
    for (const auto& msg : messages) {
        if (msg.size() >= 4) {
            std::vector<uint8_t> header(msg.begin(), msg.begin() + 4);
            headerCounts[header]++;
        }
    }
    
    for (const auto& pair : headerCounts) {
        if (pair.second > 1) {
            MessagePattern pattern(pair.first, 0);
            pattern.frequency = pair.second;
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

void MessageParser::reset() {
    m_incomingStats = MessageStats();
    m_outgoingStats = MessageStats();
    m_capturedIncoming.clear();
    m_capturedOutgoing.clear();
}

void MessageParser::generateReport(const std::string& filename) const {
    std::ofstream report(filename);
    if (!report.is_open()) return;
    
    report << "==============================================\n";
    report << "       PIPE PROTOCOL ANALYSIS REPORT\n";
    report << "==============================================\n\n";
    
    report << "Generated: " << formatTimestamp() << "\n\n";
    
    report << "INCOMING MESSAGES (Client -> Server)\n";
    report << "-------------------------------------\n";
    report << m_incomingStats.getSummary() << "\n\n";
    
    report << "OUTGOING MESSAGES (Server -> Client)\n";
    report << "-------------------------------------\n";
    report << m_outgoingStats.getSummary() << "\n\n";
    
    report << "PATTERN ANALYSIS\n";
    report << "----------------\n";
    
    auto incomingPatterns = findPatterns(m_capturedIncoming);
    if (!incomingPatterns.empty()) {
        report << "Incoming message patterns (4-byte headers):\n";
        for (const auto& pattern : incomingPatterns) {
            report << "  Pattern: ";
            for (uint8_t b : pattern.bytes) {
                report << std::setfill('0') << std::setw(2) << std::hex 
                       << static_cast<unsigned int>(b) << " ";
            }
            report << std::dec << " - Frequency: " << pattern.frequency << "\n";
        }
        report << "\n";
    }
    
    auto outgoingPatterns = findPatterns(m_capturedOutgoing);
    if (!outgoingPatterns.empty()) {
        report << "Outgoing message patterns (4-byte headers):\n";
        for (const auto& pattern : outgoingPatterns) {
            report << "  Pattern: ";
            for (uint8_t b : pattern.bytes) {
                report << std::setfill('0') << std::setw(2) << std::hex 
                       << static_cast<unsigned int>(b) << " ";
            }
            report << std::dec << " - Frequency: " << pattern.frequency << "\n";
        }
        report << "\n";
    }
    
    report << "MESSAGE SIZE DISTRIBUTION\n";
    report << "-------------------------\n";
    
    std::map<size_t, size_t> sizeCounts;
    for (size_t size : m_incomingStats.messageSizes) {
        sizeCounts[size]++;
    }
    
    report << "Incoming:\n";
    for (const auto& pair : sizeCounts) {
        report << "  " << pair.first << " bytes: " << pair.second << " messages\n";
    }
    report << "\n";
    
    sizeCounts.clear();
    for (size_t size : m_outgoingStats.messageSizes) {
        sizeCounts[size]++;
    }
    
    report << "Outgoing:\n";
    for (const auto& pair : sizeCounts) {
        report << "  " << pair.first << " bytes: " << pair.second << " messages\n";
    }
    report << "\n";
    
    report << "==============================================\n";
    report.close();
}

std::string MessageParser::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    struct tm timeinfo;
    localtime_s(&timeinfo, &time);
    
    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string MessageParser::detectMessageType(const void* data, size_t size) const {
    if (isTextBased(data, size)) {
        return "Text-based / ASCII";
    }
    
    return "Binary";
}

bool MessageParser::isTextBased(const void* data, size_t size) const {
    if (size == 0) return false;
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    size_t printableCount = 0;
    
    for (size_t i = 0; i < size; i++) {
        uint8_t c = bytes[i];
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') {
            printableCount++;
        }
    }
    
    return (printableCount * 100 / size) > 80;
}

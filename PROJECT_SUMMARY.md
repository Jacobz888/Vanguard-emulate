# Vanguard Pipe-Hijacking Emulator - Project Summary

## Overview

This project implements a Windows-based emulator for the Vanguard anti-cheat system using a **pipe-hijacking strategy**.

## Architecture Decision: Pipe-Hijacking vs Full Emulation

### Traditional Approach (Not Used)
- Completely replace vgk/vgc services
- Implement full driver functionality
- Handle all initialization and communication
- **Challenges**: Complex kernel drivers, signature requirements, constant updates

### Pipe-Hijacking Approach (Implemented)
- Let real services handle initialization
- Intercept the communication pipe before services restart
- Echo messages back to satisfy client checks
- **Advantages**: Simpler, maintainable, no driver reverse engineering

## Project Structure

```
VanguardEmulator/
├── src/                          # Source files
│   ├── main.cpp                  # Entry point and orchestration
│   ├── logger.cpp                # Thread-safe logging
│   ├── config.cpp                # Configuration management
│   ├── service_manager.cpp       # Windows service control
│   └── pipe_server.cpp           # Named pipe server
├── include/                      # Header files
│   ├── logger.h
│   ├── config.h
│   ├── service_manager.h
│   └── pipe_server.h
├── config.ini                    # Runtime configuration
├── VanguardEmulator.sln          # Visual Studio solution
├── VanguardEmulator.vcxproj      # Visual Studio project
├── CMakeLists.txt                # CMake build file
├── build.bat                     # Build script
├── README.md                     # Main documentation
├── BUILDING.md                   # Build instructions
├── CONTRIBUTING.md               # Contribution guidelines
├── LICENSE                       # MIT License
└── .gitignore                    # Git ignore rules
```

## Technical Implementation

### Core Components

#### 1. Logger (logger.h/cpp)
- **Purpose**: Thread-safe logging system
- **Features**:
  - Multiple log levels (DEBUG, INFO, WARNING, ERROR)
  - Console and file output
  - Timestamp precision to milliseconds
  - Singleton pattern
- **Thread Safety**: std::mutex protection

#### 2. Config (config.h/cpp)
- **Purpose**: INI file configuration parser
- **Features**:
  - Key-value pair storage
  - Default values
  - Type conversion helpers
  - Comments support (# and ;)
- **Configurables**:
  - Pipe name (GUID)
  - Buffer size
  - Service restart delay
  - Log level

#### 3. ServiceManager (service_manager.h/cpp)
- **Purpose**: Windows service control
- **Features**:
  - Start/stop services via SC Manager
  - Query service status
  - Wait for state transitions
  - Vanguard-specific service management
- **Services Controlled**:
  - vgk (kernel driver)
  - vgc (client service)

#### 4. PipeServer (pipe_server.h/cpp)
- **Purpose**: Named pipe server implementation
- **Features**:
  - Message-type pipe (PIPE_TYPE_MESSAGE)
  - Unlimited instances for multiple connections
  - Thread-per-connection model
  - Echo strategy for message handling
- **Pipe Configuration**:
  - Name: `\\.\pipe\933823D3-C77B-4BAE-89D7-A92B567236BC`
  - Buffer: 100MB (configurable)
  - Access: Bidirectional (PIPE_ACCESS_DUPLEX)

#### 5. Main (main.cpp)
- **Purpose**: Application entry point and orchestration
- **Features**:
  - Administrator privilege check
  - SeDebugPrivilege elevation
  - Signal handling (Ctrl+C)
  - Phased execution workflow
- **Execution Phases**:
  1. Configuration load and privilege check
  2. Stop Vanguard services
  3. Start pipe server
  4. Restart Vanguard services
  5. Run until shutdown signal

## Execution Flow

```
[Start]
   |
   v
[Check Admin Privileges]
   |
   v
[Load Configuration]
   |
   v
[Elevate to SeDebugPrivilege]
   |
   v
[Phase 1: Stop vgk/vgc Services]
   |
   v
[Wait for Services to Stop]
   |
   v
[Phase 2: Create Named Pipe]
   |
   v
[Start Pipe Server (Listening)]
   |
   v
[Phase 3: Restart vgk/vgc Services]
   |
   v
[Run: Accept Connections & Echo Messages]
   |
   v
[Shutdown Signal Received]
   |
   v
[Stop Pipe Server]
   |
   v
[Cleanup & Exit]
```

## Technical Details

### Named Pipe Configuration

```cpp
HANDLE hPipe = CreateNamedPipeA(
    "\\\\.\\pipe\\933823D3-C77B-4BAE-89D7-A92B567236BC",
    PIPE_ACCESS_DUPLEX,                    // Bidirectional
    PIPE_TYPE_MESSAGE | PIPE_WAIT,         // Message mode
    PIPE_UNLIMITED_INSTANCES,              // Multiple connections
    104857600,                             // 100MB out buffer
    104857600,                             // 100MB in buffer
    0,                                     // Default timeout
    nullptr                                // Default security
);
```

### Privilege Elevation

```cpp
// Acquire SeDebugPrivilege
TOKEN_PRIVILEGES tp;
tp.PrivilegeCount = 1;
tp.Privileges[0].Luid = /* SE_DEBUG_NAME LUID */;
tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
AdjustTokenPrivileges(hToken, FALSE, &tp, ...);
```

### Message Echo Strategy

```cpp
// Read from client
ReadFile(hPipe, buffer, bufferSize, &bytesRead, nullptr);

// Echo back to client
WriteFile(hPipe, buffer, bytesRead, &bytesWritten, nullptr);

// Ensure delivery
FlushFileBuffers(hPipe);
```

## Build Configuration

### Requirements
- **OS**: Windows 10/11 (64-bit)
- **Compiler**: MSVC (Visual Studio 2019+)
- **Standard**: C++17
- **SDK**: Windows SDK 10.0+

### Libraries Linked
```
psapi.lib      - Process Status API
kernel32.lib   - Windows Kernel
user32.lib     - User Interface
advapi32.lib   - Advanced Services
```

### Build Configurations
- **Debug**: Full symbols, no optimizations
- **Release**: Optimized, minimal symbols

## Configuration File (config.ini)

```ini
# Vanguard Pipe-Hijacking Emulator Configuration

# Named pipe GUID (Vanguard communication pipe)
pipe_name=933823D3-C77B-4BAE-89D7-A92B567236BC

# Buffer size for pipe communication (100MB default)
buffer_size=104857600

# Delay in milliseconds between stopping and restarting services
service_restart_delay=2000

# Logging level: DEBUG, INFO, WARNING, ERROR
log_level=INFO
```

## Success Criteria

✅ **Project compiles without errors**
- C++17 features work correctly
- All Windows API calls resolve
- Libraries link successfully

✅ **Administrator privileges checked**
- Application detects privilege level
- Exits gracefully if not admin

✅ **SeDebugPrivilege acquired**
- Token privileges adjusted
- Debug privilege enabled

✅ **Services controlled correctly**
- vgk/vgc services stop
- vgk/vgc services restart
- Status queries work

✅ **Pipe created and listening**
- Named pipe created successfully
- Server accepts connections
- Multiple connections supported

✅ **Messages echoed back**
- Read operations succeed
- Write operations succeed
- Buffers flush correctly

## Limitations & Future Work

### Current Limitations
1. **Echo Strategy**: Naive - just echoes back any message
2. **No Protocol Understanding**: Doesn't parse Vanguard messages
3. **No Encryption**: Messages handled as-is
4. **Basic Error Handling**: Could be more robust
5. **No Anti-Detection**: Services may detect emulation

### Future Enhancements
1. **Protocol Analysis**: Reverse engineer message formats
2. **Smart Responses**: Generate proper responses based on message type
3. **Encryption Support**: Handle encrypted communications
4. **State Management**: Track connection states
5. **Anti-Detection**: Mimic real service behavior more closely
6. **Message Logging**: Capture and analyze traffic
7. **GUI Interface**: Optional graphical interface
8. **Configuration UI**: Runtime configuration changes

## Security & Ethical Considerations

### Purpose
This project is for **educational and research purposes**:
- Understanding anti-cheat systems
- Learning Windows internals
- Studying inter-process communication
- Security research

### Not Intended For
- Circumventing anti-cheat in online games
- Enabling cheating in competitive games
- Bypassing security systems
- Any malicious or unethical use

### Administrator Requirements
This application requires admin privileges because it:
- Controls Windows services
- Acquires debug privileges
- Creates system-level named pipes
- Interacts with kernel-mode services

## Development Notes

### Code Style
- **Classes**: PascalCase (e.g., `ServiceManager`)
- **Methods**: camelCase (e.g., `startService`)
- **Members**: m_ prefix (e.g., `m_logLevel`)
- **Indentation**: 4 spaces
- **Line Length**: ~120 characters

### Threading Model
- **Accept Thread**: Continuously accepts connections
- **Handler Threads**: One per client connection
- **Cleanup**: All threads join on shutdown

### Error Handling
- Windows API errors logged with error codes
- Graceful degradation where possible
- Clear error messages for troubleshooting

## Testing

### Build Testing
```cmd
# Clean build
build.bat clean

# Debug build
build.bat Debug

# Release build
build.bat Release
```

### Runtime Testing
1. Run as Administrator
2. Verify services stop
3. Check pipe creation
4. Verify services restart
5. Test client connections
6. Verify message echo
7. Test graceful shutdown

## Performance Characteristics

### Resource Usage
- **Memory**: ~10-50 MB base + 100MB per connection buffer
- **CPU**: Low (event-driven, blocking I/O)
- **Threads**: 1 accept + N client handlers
- **Disk**: Log file grows with activity

### Scalability
- **Connections**: Unlimited (PIPE_UNLIMITED_INSTANCES)
- **Throughput**: Limited by 100MB buffers
- **Latency**: Low (synchronous echo)

## Troubleshooting

### Build Issues
- Verify Windows SDK installed
- Check C++17 compiler support
- Ensure psapi.lib is linked

### Runtime Issues
- Run as Administrator
- Check services exist (vgk/vgc)
- Verify pipe name not in use
- Review logs for details

## Documentation Files

- **README.md**: Main user documentation
- **BUILDING.md**: Detailed build instructions
- **CONTRIBUTING.md**: Contribution guidelines
- **PROJECT_SUMMARY.md**: This file - technical overview
- **LICENSE**: MIT License with disclaimer

## Version History

### v1.0.0 - Initial Foundation
- Basic pipe-hijacking implementation
- Service management
- Named pipe server
- Echo strategy
- Configuration system
- Logging system

## Credits

This is an educational research project demonstrating:
- Windows service control
- Named pipe programming
- Privilege management
- Multi-threaded server design
- C++17 best practices

## License

MIT License - See LICENSE file for details.

**Educational and research use only. Not for production or malicious use.**

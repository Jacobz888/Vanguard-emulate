# Vanguard Pipe-Hijacking Emulator

A Windows-based emulator that uses a pipe-hijacking strategy to intercept and emulate Vanguard anti-cheat communication.

## Overview

This emulator implements a **pipe-hijacking strategy** rather than full service replacement:

1. Stop Vanguard services (vgk/vgc)
2. Create and hijack the named pipe communication channel
3. Restart Vanguard services (they handle their own initialization)
4. Echo back client messages to satisfy integrity checks

## Architecture

### Pipe-Hijacking Strategy

Instead of replacing the Vanguard services entirely, this emulator:
- Allows the real services to handle complex initialization
- Intercepts the communication pipe before services restart
- Echoes messages back to satisfy client-side checks
- Avoids reimplementing complex kernel-mode drivers

### Project Structure

```
/src/
  main.cpp              - Entry point and orchestration
  pipe_server.cpp       - Named pipe server implementation
  service_manager.cpp   - Service control (start/stop vgk/vgc)
  logger.cpp            - Logging utilities
  config.cpp            - Configuration management
/include/
  *.h                   - Header files
config.ini              - Configuration file
VanguardEmulator.sln    - Visual Studio solution
VanguardEmulator.vcxproj - Visual Studio project
```

## Requirements

- **Operating System**: Windows 10/11 (64-bit)
- **Compiler**: Visual Studio 2019 or later with C++17 support
- **Windows SDK**: 10.0 or later
- **Privileges**: Administrator rights required

## Building

### Visual Studio

1. Open `VanguardEmulator.sln` in Visual Studio
2. Select configuration (Debug or Release)
3. Build Solution (Ctrl+Shift+B)
4. Executable will be in `bin/Debug/` or `bin/Release/`

### Configuration

The project links against:
- `psapi.lib` - Process Status API
- `kernel32.lib` - Windows kernel functions
- `user32.lib` - User interface functions
- `advapi32.lib` - Advanced Windows services

C++ Standard: C++17 or later

## Configuration

Edit `config.ini` to customize behavior:

```ini
# Named pipe GUID (Vanguard communication pipe)
pipe_name=933823D3-C77B-4BAE-89D7-A92B567236BC

# Buffer size for pipe communication (100MB default)
buffer_size=104857600

# Delay between stopping and restarting services (milliseconds)
service_restart_delay=2000

# Logging level: DEBUG, INFO, WARNING, ERROR
log_level=INFO
```

## Usage

1. **Run as Administrator**:
   ```
   Right-click VanguardEmulator.exe -> Run as administrator
   ```

2. The emulator will:
   - Stop Vanguard services (vgk, vgc)
   - Create the named pipe: `\\.\pipe\933823D3-C77B-4BAE-89D7-A92B567236BC`
   - Restart Vanguard services
   - Listen for and echo pipe messages

3. Press `Ctrl+C` to stop the emulator

## Technical Details

### Named Pipe Configuration

- **Pipe Name**: `933823D3-C77B-4BAE-89D7-A92B567236BC`
- **Pipe Type**: `PIPE_TYPE_MESSAGE | PIPE_WAIT`
- **Access Mode**: `PIPE_ACCESS_DUPLEX` (bidirectional)
- **Instances**: `PIPE_UNLIMITED_INSTANCES`
- **Buffer Size**: 100MB (configurable)

### Privilege Elevation

The emulator acquires `SeDebugPrivilege` to:
- Access protected system processes
- Interact with kernel-mode services
- Control system services

### Service Management

Controls the following services:
- **vgk**: Vanguard kernel-mode driver
- **vgc**: Vanguard user-mode client

Operations:
- Stop services before hijacking
- Restart services after pipe creation
- Query service status

### Message Handling

<img width="662" height="287" alt="image" src="https://github.com/user-attachments/assets/3b391761-f2c8-4a7d-8faf-0009c00e34ce" />


The pipe server uses an **echo strategy**:
1. Receive message from client
2. Log message size and content (debug mode)
3. Echo message back verbatim
4. Flush buffers to ensure delivery

This satisfies basic integrity checks without deep protocol knowledge.

### Threading Model

- **Thread-per-connection**: Each client connection gets a dedicated handler thread
- **Accept thread**: Main thread continuously accepts new connections
- **Graceful shutdown**: All threads join on shutdown signal

## Components

### Logger (`logger.h/cpp`)

Provides thread-safe logging with:
- Multiple log levels (DEBUG, INFO, WARNING, ERROR)
- File and console output
- Timestamp precision to milliseconds

### Config (`config.h/cpp`)

Manages configuration:
- INI file parsing
- Default values
- Type conversion helpers

### ServiceManager (`service_manager.h/cpp`)

Controls Windows services:
- Start/stop individual services
- Restart with delays
- Query service status
- Wait for state transitions

### PipeServer (`pipe_server.h/cpp`)

Implements named pipe server:
- Create message-type pipes
- Accept multiple connections
- Echo message handling
- Thread-safe operations

## Logging

Logs are written to:
- **Console**: Real-time output
- **File**: `vanguard_emulator.log`

Log format:
```
[2024-01-15 14:30:45.123] [INFO] Message text
```

## Troubleshooting

### "Failed to open Service Control Manager"
- Ensure running as Administrator
- Check Windows services are accessible

### "Failed to create named pipe"
- Verify pipe name is not already in use
- Check administrator privileges

### "Failed to adjust privileges"
- Run as Administrator
- Check Windows security policy settings

### Services fail to start/stop
- Verify vgk/vgc services exist on system
- Check service permissions
- Review Windows Event Viewer

## Security Notice

This tool is for **educational and research purposes only**. It:
- Requires administrator privileges
- Modifies system service states
- Intercepts inter-process communication
- May interfere with anti-cheat systems

**Use responsibly and only on systems you own or have permission to test.**

## License

Educational/Research use only.

## Technical Notes

### Why Pipe-Hijacking?

Traditional approaches to anti-cheat emulation require:
- Reimplementing complex kernel drivers
- Understanding proprietary protocols
- Maintaining driver signatures

Pipe-hijacking offers advantages:
- **Simplicity**: Let real services handle initialization
- **Maintenance**: No driver reverse engineering needed
- **Compatibility**: Services update themselves
- **Testing**: Easy to validate communication patterns

### Limitations

This is a **foundation** implementation:
- Echo strategy is naive (real protocol may be complex)
- No cryptographic verification
- No anti-tampering detection
- Basic message handling only

Future enhancements would require:
- Protocol reverse engineering
- Message validation
- Cryptographic implementations
- Anti-detection measures

## Development

### Protocol Analysis Features

The emulator now includes **comprehensive protocol analysis tools** for understanding Vanguard pipe communication:

#### Features
- **Message Capture**: All messages logged with hex dumps
- **Timing Analysis**: Microsecond-precision timing for each message
- **Pattern Detection**: Automatic identification of headers and magic bytes
- **Statistical Analysis**: Message counts, sizes, frequencies
- **Structure Analysis**: Length field and encoding detection

#### Usage

1. **Enable Analysis** (automatic in current build):
   ```cpp
   pipeServer.enableProtocolAnalysis(true);
   ```

2. **Set DEBUG Logging** in `config.ini`:
   ```ini
   log_level=DEBUG
   ```

3. **Run and Capture**:
   - Start emulator
   - Launch game
   - Let run for 5-10 minutes
   - Stop with Ctrl+C

4. **Review Results**:
   - **Capture Logs**: `/logs/message_captures/capture_*.log`
   - **Analysis Report**: `/logs/protocol_analysis_report.txt`
   - **Documentation**: `/docs/protocol_analysis.md`

#### Documentation

- **[Protocol Analysis Guide](docs/analysis_guide.md)** - Complete usage instructions
- **[Protocol Analysis Document](docs/protocol_analysis.md)** - Findings and methodology
- **[Logs README](logs/README.md)** - Log file format reference

### Adding Features

1. **Protocol Analysis**: ✅ Implemented - Capture and analyze pipe traffic
2. **Message Parsing**: ✅ Implemented - Decode message structures
3. **Response Logic**: 🔄 To be determined based on analysis
4. **State Management**: 🔄 To be implemented if needed

### Debugging

Enable DEBUG logging in `config.ini`:
```ini
log_level=DEBUG
```

This logs:
- Detailed message sizes and counts
- Hex previews (first 64 bytes)
- Read/write timing (microseconds)
- Session statistics every 10 messages
- Service state transitions
- Pipe connection events
- Thread lifecycle events

## Contributing

This is a research project. Contributions should focus on:
- Educational value
- Code quality
- Documentation
- Security awareness

## Disclaimer

This software is provided for educational and research purposes. The authors are not responsible for misuse or damage caused by this software. Anti-cheat systems are designed to protect game integrity - respect their purpose and use this knowledge responsibly.

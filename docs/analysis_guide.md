# Protocol Analysis Guide

## Quick Start

This guide explains how to use the protocol analysis features to capture and analyze Vanguard pipe communication.

## Prerequisites

- Emulator built and running with administrator privileges
- Vanguard services available on system (vgk, vgc)
- Sufficient disk space for logs (~50-100MB per hour)
- DEBUG log level enabled for detailed output

## Configuration

### Enable Debug Logging

Edit `config.ini`:
```ini
log_level=DEBUG
```

This enables detailed message logging including:
- Individual message sizes
- Timing information
- Hex previews in console
- Service state transitions

### Protocol Analysis Settings

Protocol analysis is **automatically enabled** in the current build.

The emulator will:
- Capture all messages to timestamped log files
- Track statistics for incoming/outgoing messages
- Generate analysis reports on shutdown

## Running Analysis

### Step 1: Start Emulator

```bash
# Windows Command Prompt (as Administrator)
cd path\to\VanguardEmulator
VanguardEmulator.exe
```

Or from Visual Studio:
- Open `VanguardEmulator.sln`
- Build in Debug mode
- Run as Administrator (Project Properties → Debugging → Run as Administrator)

### Step 2: Launch Riot Client/Game

1. Start Riot Client
2. Launch a Vanguard-protected game (League of Legends, VALORANT, etc.)
3. Let the game run for several minutes
4. Perform various in-game actions

### Step 3: Monitor Output

Watch the console for:
```
[YYYY-MM-DD HH:MM:SS.mmm] [INFO] Protocol analysis enabled. Capturing to: logs/message_captures/capture_YYYYMMDD_HHMMSS.log
[YYYY-MM-DD HH:MM:SS.mmm] [DEBUG] Message #1: Received 16 bytes from client
[YYYY-MM-DD HH:MM:SS.mmm] [DEBUG] Read time: 245 microseconds
[YYYY-MM-DD HH:MM:SS.mmm] [DEBUG] Hex preview (first 64 bytes):
00000000  56 47 48 53 00 00 00 0c  01 02 03 04 00 00 00 00  |VGHS............|
```

### Step 4: Stop Emulator

Press `Ctrl+C` to gracefully shutdown.

The emulator will:
- Generate analysis report
- Close all connections
- Save final statistics

## Analyzing Captured Data

### Location of Files

- **Raw Captures**: `/logs/message_captures/capture_YYYYMMDD_HHMMSS.log`
- **Analysis Report**: `/logs/protocol_analysis_report.txt`
- **Main Log**: `vanguard_emulator.log`

### Reading Capture Logs

Open a capture log file:

```
[2024-01-15 14:30:45.123] INCOMING (16 bytes)
Structure Analysis:
  Size: 16 bytes (0x10)
  Type: Binary
  First 4 bytes (potential header):
    Hex: 56 47 48 53
    As uint32 (LE): 1398297686 (0x5347 4756)
    As uint32 (BE): 1447580499 (0x5647 4853)
  Magic bytes detected: 56 47

Hex Dump:
00000000  56 47 48 53 00 00 00 0c  01 02 03 04 00 00 00 00  |VGHS............|
```

#### Key Information

1. **Timestamp**: When message was received/sent
2. **Direction**: INCOMING (client→server) or OUTGOING (server→client)
3. **Size**: Message length in bytes
4. **Structure Analysis**:
   - Type detection (binary vs text)
   - Header interpretation
   - Length field detection
   - Magic byte identification
5. **Hex Dump**: Complete message in hex with ASCII

### Reading Analysis Reports

Open `protocol_analysis_report.txt`:

```
==============================================
       PIPE PROTOCOL ANALYSIS REPORT
==============================================

Generated: 2024-01-15 15:30:45.123

INCOMING MESSAGES (Client -> Server)
-------------------------------------
Messages: 1247
Total bytes: 45823
Average size: 36.74 bytes
Frequency: 4.16 msg/sec

OUTGOING MESSAGES (Server -> Client)
-------------------------------------
Messages: 1247
Total bytes: 45823
Average size: 36.74 bytes
Frequency: 4.16 msg/sec

PATTERN ANALYSIS
----------------
Incoming message patterns (4-byte headers):
  Pattern: 56 47 48 53 - Frequency: 1
  Pattern: 50 49 4e 47 - Frequency: 1234
  Pattern: 53 54 41 54 - Frequency: 12

MESSAGE SIZE DISTRIBUTION
-------------------------
Incoming:
  4 bytes: 1234 messages
  16 bytes: 1 messages
  128 bytes: 12 messages
```

#### Key Metrics

1. **Message Counts**: Total messages exchanged
2. **Average Size**: Typical message size
3. **Frequency**: Messages per second
4. **Patterns**: Recurring headers (potential opcodes)
5. **Size Distribution**: Fixed vs variable size messages

## Analysis Tasks

### 1. Identify Message Categories

Look for patterns:

**Handshake/Initialization**
- First few messages in session
- Larger size (>100 bytes)
- Sent once or rarely

**Status Queries**
- Small, fixed size (4-16 bytes)
- Sent occasionally (every few seconds)
- May have specific header (e.g., "STAT")

**Heartbeats**
- Very small (4-8 bytes)
- High frequency (1-10 per second)
- Often just a constant pattern (e.g., "PING")

**Data Messages**
- Variable size
- Lower frequency
- May contain structured data

### 2. Analyze Message Structure

For each unique pattern:

1. **Extract Header**
   - First 4 bytes usually header/opcode
   - Look for ASCII identifiers (e.g., "VGHS", "PING", "STAT")

2. **Identify Length Field**
   - Check if any field = message size
   - Common at offset 0, 4, or 8
   - Usually uint32 (4 bytes)

3. **Detect Payload**
   - Data after header
   - May be structured (nested fields)
   - May be opaque binary data

4. **Check for Checksums**
   - Last 4-8 bytes often checksum/CRC
   - Compare with computed values

### 3. Test Echo-Back Strategy

For each message type:

1. **Verify Game Proceeds**
   - Does game launch successfully?
   - Any errors in logs?
   - Any connection timeouts?

2. **Monitor Behavior**
   - Does game play normally?
   - Any periodic disconnects?
   - Any degraded functionality?

3. **Check Logs**
   - Riot Client logs
   - Game logs
   - Windows Event Viewer

### 4. Measure Timing

Review timing data:

```
[DEBUG] Read time: 245 microseconds
[DEBUG] Write time: 187 microseconds
```

**Observations:**
- Typical read/write times?
- Any outliers (very slow messages)?
- Response time requirements?

### 5. Compare with Real vgc.exe

If possible, capture traffic with real Vanguard:

1. Use Process Monitor to log named pipe operations
2. Compare message patterns
3. Identify differences in responses
4. Note any validation/signature checks

## Decision Framework

### When Echo-Back Is Sufficient

✅ All message types accept verbatim echo
✅ Game functions normally for extended periods
✅ No errors in logs
✅ No special validation detected

**Action**: Keep current echo strategy

### When Smart Responses Are Needed

❌ Game fails to launch or crashes
❌ Connection timeouts occur
❌ Error messages about anti-cheat
❌ Checksums or signatures detected

**Action**: Implement protocol parsing and smart responses

### Hybrid Approach

Some messages echo, others need smart responses:

1. **Identify Critical Messages**
   - Messages that cause failures if echoed
   - Status queries needing specific responses

2. **Implement Minimal Logic**
   - Parse only critical message types
   - Generate proper responses for those
   - Echo all others

## Common Patterns

### Pattern 1: Simple Heartbeat

```
Incoming:  50 49 4e 47                          |PING|
Response:  50 4f 4e 47                          |PONG|
```

**Strategy**: Replace "PING" with "PONG" in response

### Pattern 2: Status Query

```
Incoming:  53 54 41 54 XX XX XX XX             |STAT....|
Response:  4f 4b 00 00 00 00 00 00             |OK......|
```

**Strategy**: Ignore payload, respond with "OK" status

### Pattern 3: Length-Prefixed Message

```
Incoming:  [4-byte length][payload]
Response:  [4-byte length][payload]
```

**Strategy**: Echo works if no validation

### Pattern 4: Checksummed Message

```
Incoming:  [header][payload][4-byte checksum]
Response:  [header][payload][computed checksum]
```

**Strategy**: Must compute checksum over new payload

## Troubleshooting

### No Messages Captured

**Problem**: Capture file is empty or not created

**Solutions**:
1. Check if client connected:
   ```
   [INFO] Client connected
   ```
2. Verify Vanguard services are running
3. Check if game is launching
4. Review errors in main log

### Analysis Report Is Empty

**Problem**: Report shows 0 messages

**Solutions**:
1. Ensure protocol analysis was enabled
2. Check if client sent any data
3. Verify emulator ran long enough
4. Review capture logs for data

### Large Capture Files

**Problem**: Logs are hundreds of MB

**Causes**:
- Very high message frequency
- Long session duration
- Large individual messages

**Solutions**:
- Capture shorter sessions
- Compress old logs
- Archive to separate drive

### Cannot Parse Hex Dumps

**Problem**: Hex dumps are hard to read

**Tools**:
- Use hex editor (HxD, 010 Editor)
- Copy hex to online decoder
- Use custom parsing scripts

## Advanced Analysis

### Writing Custom Parsers

Create Python scripts to analyze captures:

```python
import re
from collections import Counter

def parse_hex_dump(hex_str):
    """Extract bytes from hex dump format"""
    bytes_list = []
    for line in hex_str.split('\n'):
        # Parse line: "00000000  XX XX XX XX ..."
        match = re.findall(r'([0-9a-f]{2})', line)
        bytes_list.extend([int(b, 16) for b in match])
    return bytes_list

def analyze_patterns(capture_file):
    """Find common 4-byte headers"""
    headers = []
    with open(capture_file, 'r') as f:
        # Parse INCOMING messages
        # Extract first 4 bytes
        # Add to headers list
        pass
    return Counter(headers)

# Usage
patterns = analyze_patterns('logs/message_captures/capture_20240115_143045.log')
print("Most common patterns:")
for pattern, count in patterns.most_common(10):
    print(f"  {pattern}: {count} times")
```

### Statistical Analysis

Calculate metrics:
- Message size variance
- Inter-arrival time distribution
- Burst patterns
- Peak message rates

### Visualization

Generate graphs:
- Message frequency over time
- Size distribution histogram
- Timeline of message types
- Response time scatter plot

## Next Steps

After completing analysis:

1. **Document Findings**
   - Update `docs/protocol_analysis.md`
   - Fill in all discovered message types
   - Note response requirements

2. **Make Implementation Decision**
   - Keep echo-back?
   - Implement smart responses?
   - Hybrid approach?

3. **Update Code**
   - Implement message parser
   - Add response logic
   - Handle edge cases

4. **Test Thoroughly**
   - Extended runtime testing
   - Multiple game sessions
   - Various scenarios

5. **Iterate**
   - Capture new data after changes
   - Verify improvements
   - Refine responses

## Resources

### Documentation
- `/docs/protocol_analysis.md` - Main analysis document
- `/logs/README.md` - Log file information
- Main README - Emulator overview

### Tools
- HxD - Hex editor (Windows)
- 010 Editor - Advanced hex editor with templates
- Process Monitor - System monitoring (Sysinternals)
- Wireshark - Network protocol analyzer (for reference)

### References
- Windows Named Pipes Documentation
- IPC Protocol Analysis
- Anti-Cheat Research Papers

---

**Questions or Issues?**

If you encounter problems or need clarification:
1. Check emulator logs for errors
2. Review this guide thoroughly
3. Consult protocol analysis documentation
4. Search for similar anti-cheat research

---

*Happy Analyzing!*

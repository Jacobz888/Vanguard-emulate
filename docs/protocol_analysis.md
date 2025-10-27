# VGC Pipe Protocol Analysis

## Overview

This document contains findings from analyzing the actual message traffic flowing through the Vanguard anti-cheat pipe (`933823D3-C77B-4BAE-89D7-A92B567236BC`).

The analysis helps determine whether simple echo-back responses are sufficient or if the emulator needs smarter protocol handling.

## Analysis Infrastructure

### Message Capture System

The emulator includes comprehensive protocol analysis tools:

- **Hex Dump Logging**: All messages are logged in hexadecimal format with ASCII representation
- **Timing Analysis**: Message read/write times tracked in microseconds
- **Statistical Collection**: Message counts, sizes, and frequencies recorded
- **Pattern Detection**: Automatic identification of repeated headers and magic bytes
- **Structure Analysis**: Detection of length fields, message types, and encoding

### Capture Files

Message captures are saved to: `/logs/message_captures/capture_YYYYMMDD_HHMMSS.log`

Each capture includes:
- Timestamp (millisecond precision)
- Direction (INCOMING/OUTGOING)
- Message size
- Structure analysis
- Full hex dump with ASCII representation

### Analysis Reports

Summary reports are generated to: `/logs/protocol_analysis_report.txt`

Reports include:
- Message statistics (count, total bytes, average size, frequency)
- Pattern analysis (recurring headers)
- Size distribution
- Timing characteristics

## Message Structure

### General Format

```
[Timestamp] DIRECTION (SIZE bytes)
Structure Analysis:
  Size: X bytes (0xY)
  Type: Binary/Text
  First 4 bytes: [hex values]
    As uint32 (LE): value
    As uint32 (BE): value
    ** Length field detection **
  Magic bytes: [if detected]

Hex Dump:
00000000  XX XX XX XX XX XX XX XX  XX XX XX XX XX XX XX XX  |................|
...
```

### Length Field Detection

The analyzer automatically checks if the first 4-8 bytes contain a length field by:
- Comparing values to total message size
- Checking for message size minus header (4 bytes)
- Testing both little-endian and big-endian interpretations

### Message Type Detection

Messages are classified as:
- **Text-based/ASCII**: >80% printable characters
- **Binary**: Non-printable binary protocol

## Message Categories

### 1. Initialization/Handshake

**Characteristics:**
- Sent at connection establishment
- May contain version information
- One-time messages

**Analysis Method:**
- Compare first messages across multiple sessions
- Look for fixed headers or magic bytes
- Check for version numbers or client identifiers

### 2. Status Queries

**Characteristics:**
- "Is anti-cheat alive?" type messages
- May require specific acknowledgment responses
- Critical for launcher/game to proceed

**Analysis Method:**
- Identify short, repeated messages
- Test if echo-back is accepted
- Determine required response format

### 3. Heartbeat/Keep-Alive

**Characteristics:**
- Periodic messages at fixed intervals
- Often small, fixed-size packets
- High frequency (e.g., every 100ms, 1s, etc.)

**Analysis Method:**
- Measure message frequency over time
- Look for patterns in timing
- Identify fixed-size repeating messages

### 4. Configuration Requests

**Characteristics:**
- Game/launcher requesting AC settings
- May contain policy information
- Moderate size (config data)

**Analysis Method:**
- Look for messages with structured data
- Check for key-value pairs or nested structures
- Test if responses need modification

### 5. Game State Updates

**Characteristics:**
- Runtime information from game
- May include process info, memory checksums
- Variable size depending on data

**Analysis Method:**
- Correlate with game events (launch, level load, etc.)
- Look for process IDs, memory addresses
- Identify optional vs. required responses

### 6. Exit/Cleanup

**Characteristics:**
- Final messages before disconnection
- Session termination signals
- May require acknowledgment

**Analysis Method:**
- Capture messages immediately before disconnect
- Check if lack of response causes issues
- Determine if graceful shutdown is needed

## Response Requirements

### Echo-Back Strategy

**Current Implementation:**
- All incoming messages echoed verbatim
- No modification to content
- Immediate response (no delays)

**Testing:**
- Monitor for errors in launcher/game logs
- Check if game proceeds normally
- Identify timeouts or failed connections

### Smart Response Scenarios

Messages that may need modification:

1. **Status Code Responses**
   - Message requires "OK", "RUNNING", or error codes
   - Echo-back may not convey proper state
   - Solution: Parse message, generate appropriate response

2. **Timestamp/Counter Fields**
   - Message expects current timestamp
   - Message expects incrementing sequence numbers
   - Solution: Inject real-time values

3. **Computed Values**
   - Checksums over message content
   - Cryptographic signatures
   - HMACs or hash values
   - Solution: Implement proper computation

4. **State-Dependent Responses**
   - Response varies based on emulator state
   - Session-specific information required
   - Solution: Maintain state machine

## Timing Analysis

### Message Frequency

Measured in messages per second (msg/sec):
- **High-frequency**: >10 msg/sec (likely heartbeats)
- **Medium-frequency**: 1-10 msg/sec (status updates)
- **Low-frequency**: <1 msg/sec (configuration, state changes)

### Response Time Expectations

Tracked in microseconds (μs):
- **Read time**: Time to receive message from pipe
- **Write time**: Time to send response to pipe
- **Round-trip**: Total time for request-response cycle

### Timeout Behavior

Monitor for:
- Connection drops after delayed responses
- Retry attempts from client
- Error messages in logs

### Order Dependencies

Check if:
- Messages must be processed in specific order
- Some messages are prerequisites for others
- Out-of-order responses cause failures

## Pattern Detection

### Header Patterns

The analyzer identifies recurring 4-byte headers:
- Tracks frequency of each pattern
- Correlates patterns with message types
- Identifies opcodes or message type identifiers

### Magic Bytes

Known signatures checked:
- `50 4B` - ZIP/PKZip
- `4D 5A` - Windows executable
- `56 47` - Custom "VG" marker (Vanguard?)
- `DE AD BE EF` - Debug marker
- `CA FE BA BE` - Java class file

### Size Patterns

Distribution analysis:
- Fixed-size messages (likely control messages)
- Variable-size messages (likely data payloads)
- Correlation between size and function

## Data Collection Methodology

### Capture Scenarios

1. **Launcher Startup**
   - Start emulator
   - Launch Riot Client
   - Observe initial handshake
   - Capture first 30 seconds

2. **Game Launch**
   - Start emulator and launcher
   - Launch League of Legends/VALORANT
   - Capture through loading screens
   - Record first minute of gameplay

3. **During Gameplay**
   - Let game run for 5-10 minutes
   - Perform various actions
   - Monitor message frequency
   - Check for new message types

4. **On Exit**
   - Close game normally
   - Capture final messages
   - Check for cleanup sequences
   - Verify graceful shutdown

### Comparison with Real vgc.exe

**Methodology:**
1. Capture traffic with real Vanguard running (if possible)
2. Compare message patterns with emulator captures
3. Identify differences in responses
4. Determine which differences cause failures

**Tools:**
- Named pipe monitoring tools
- Process Monitor (Sysinternals)
- API hooking frameworks

## Findings

### Session Date: [To be filled during analysis]

#### Message Statistics

**Incoming Messages:**
- Total messages: [TBD]
- Total bytes: [TBD]
- Average size: [TBD] bytes
- Frequency: [TBD] msg/sec

**Outgoing Messages:**
- Total messages: [TBD]
- Total bytes: [TBD]
- Average size: [TBD] bytes
- Frequency: [TBD] msg/sec

#### Detected Patterns

**Header Pattern 1:**
- Bytes: `XX XX XX XX`
- Frequency: [TBD] messages
- Suspected type: [TBD]

**Header Pattern 2:**
- Bytes: `XX XX XX XX`
- Frequency: [TBD] messages
- Suspected type: [TBD]

#### Message Types Identified

1. **Type 1: [Name]**
   - Size: [fixed/variable]
   - Frequency: [rate]
   - Purpose: [description]
   - Echo-back sufficient: [YES/NO]
   - Notes: [observations]

2. **Type 2: [Name]**
   - Size: [fixed/variable]
   - Frequency: [rate]
   - Purpose: [description]
   - Echo-back sufficient: [YES/NO]
   - Notes: [observations]

#### Critical Messages

Messages that **MUST** have proper responses:
1. [Message type] - [Reason]
2. [Message type] - [Reason]

Messages that can be ignored:
1. [Message type] - [Reason]
2. [Message type] - [Reason]

#### Timing Characteristics

- Average read time: [TBD] μs
- Average write time: [TBD] μs
- Message interval: [TBD] ms
- Timeout threshold: [TBD] ms (if discovered)

## Decision Matrix

### Echo-Back Sufficiency

| Message Type | Echo Works? | Smart Response Needed? | Priority |
|--------------|-------------|------------------------|----------|
| Handshake    | [TBD]       | [TBD]                  | [H/M/L]  |
| Status Query | [TBD]       | [TBD]                  | [H/M/L]  |
| Heartbeat    | [TBD]       | [TBD]                  | [H/M/L]  |
| Config       | [TBD]       | [TBD]                  | [H/M/L]  |
| State Update | [TBD]       | [TBD]                  | [H/M/L]  |
| Exit         | [TBD]       | [TBD]                  | [H/M/L]  |

### Implementation Recommendation

**Option A: Keep Echo-Back**
- **When**: All messages accept echo responses
- **Pros**: Simple, no protocol knowledge needed
- **Cons**: May not work long-term

**Option B: Implement Smart Responses**
- **When**: Some messages require specific responses
- **Pros**: Robust, matches real protocol
- **Cons**: Requires reverse engineering

**Option C: Hybrid Approach**
- **When**: Most messages echo, few need smart responses
- **Pros**: Minimal complexity, handles critical cases
- **Cons**: Still requires some protocol knowledge

**Current Recommendation: [To be determined after analysis]**

## Next Steps

### Immediate Actions

1. **Run Analysis**
   - Start emulator with analysis enabled
   - Launch Riot Client and game
   - Let run for 10+ minutes
   - Capture all scenarios (startup, gameplay, exit)

2. **Review Captures**
   - Examine `/logs/message_captures/*.log`
   - Look for patterns in hex dumps
   - Identify message categories

3. **Generate Report**
   - Review `/logs/protocol_analysis_report.txt`
   - Fill in statistics in this document
   - Document all message types found

### Further Investigation

1. **Protocol Reverse Engineering**
   - If smart responses needed
   - Disassemble vgc.exe message handlers
   - Document protocol specification

2. **Cryptographic Analysis**
   - If signatures/HMACs detected
   - Identify algorithms used
   - Determine if bypass possible

3. **State Machine Design**
   - If state-dependent responses needed
   - Map all states and transitions
   - Implement proper state tracking

### Testing Validation

1. **Stability Testing**
   - Run for extended periods (hours)
   - Monitor for crashes or disconnects
   - Check game performance

2. **Compatibility Testing**
   - Test with multiple games (LoL, VALORANT, etc.)
   - Test with different Vanguard versions
   - Test with updates

3. **Edge Case Testing**
   - Rapid connect/disconnect
   - Message flooding
   - Malformed messages
   - Delayed responses

## Tools and Utilities

### Hex Dump Reader

Use the built-in `MessageParser::hexDump()` function:
```cpp
std::string dump = parser.hexDump(data, size, true);
```

### Pattern Finder

Use the `findPatterns()` function to identify recurring headers:
```cpp
auto patterns = parser.findPatterns(capturedMessages);
```

### Report Generator

Generate analysis reports:
```cpp
parser.generateReport("logs/analysis_report.txt");
```

## References

- Windows Named Pipes Documentation
- IPC Protocol Analysis Techniques
- Anti-Cheat System Research Papers
- Vanguard System Architecture (public information)

## Appendix A: Message Format Examples

### Example 1: Hypothetical Handshake
```
[2024-01-15 14:30:45.123] INCOMING (16 bytes)
Structure Analysis:
  Size: 16 bytes (0x10)
  Type: Binary
  First 4 bytes: 56 47 48 53
    As uint32 (LE): 1398297686
    As uint32 (BE): 1447580499
    ** Possible magic bytes **

Hex Dump:
00000000  56 47 48 53 00 00 00 0c  01 02 03 04 00 00 00 00  |VGHS............|
```

### Example 2: Hypothetical Status Query
```
[2024-01-15 14:30:45.234] INCOMING (8 bytes)
Structure Analysis:
  Size: 8 bytes (0x8)
  Type: Binary
  First 4 bytes: 53 54 41 54
    As uint32 (LE): 1414745683
    As uint32 (BE): 1398031697

Hex Dump:
00000000  53 54 41 54 00 00 00 00                           |STAT....|
```

### Example 3: Hypothetical Heartbeat
```
[2024-01-15 14:30:46.123] INCOMING (4 bytes)
Structure Analysis:
  Size: 4 bytes (0x4)
  Type: Binary
  First 4 bytes: 50 49 4e 47
    As uint32 (LE): 1196312912
    As uint32 (BE): 1347310663

Hex Dump:
00000000  50 49 4e 47                                       |PING|
```

## Appendix B: Analysis Checklist

- [ ] Capture launcher startup messages
- [ ] Capture game launch messages
- [ ] Capture gameplay messages (5-10 min)
- [ ] Capture exit messages
- [ ] Review all capture logs
- [ ] Identify all message patterns
- [ ] Classify messages by type
- [ ] Test echo-back for each type
- [ ] Document required smart responses
- [ ] Measure timing characteristics
- [ ] Generate final analysis report
- [ ] Make implementation decision
- [ ] Update emulator if needed
- [ ] Validate changes with testing

## Conclusion

**To Be Completed After Analysis**

Based on the captured data and analysis, the recommended approach is:

[To be filled after completing analysis]

Key findings:
1. [Finding 1]
2. [Finding 2]
3. [Finding 3]

Implementation requirements:
1. [Requirement 1]
2. [Requirement 2]
3. [Requirement 3]

---

*Last Updated: [Timestamp]*
*Analysis Version: 1.0*
*Emulator Version: 1.0*

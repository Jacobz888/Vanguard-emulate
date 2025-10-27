# Quick Start Guide

Get the Vanguard Pipe-Hijacking Emulator running in 5 minutes.

## Prerequisites

✅ Windows 10/11 (64-bit)  
✅ Visual Studio 2019 or later  
✅ Administrator privileges

## Build & Run (3 Steps)

### Step 1: Build

Open Command Prompt and run:

```cmd
cd path\to\VanguardEmulator
build.bat Release
```

Or open `VanguardEmulator.sln` in Visual Studio and press F7.

### Step 2: Configure (Optional)

Edit `config.ini` if needed:

```ini
pipe_name=933823D3-C77B-4BAE-89D7-A92B567236BC
buffer_size=104857600
service_restart_delay=2000
log_level=INFO
```

### Step 3: Run

Right-click `bin\Release\VanguardEmulator.exe` → **Run as administrator**

## What It Does

```
1. Stops vgk/vgc services
2. Creates named pipe: \\.\pipe\933823D3-C77B-4BAE-89D7-A92B567236BC
3. Restarts vgk/vgc services
4. Listens for connections and echoes messages
```

## Expected Output

```
========================================
  Vanguard Pipe-Hijacking Emulator
========================================

[2024-01-15 14:30:45.123] [INFO] Configuration loaded from config.ini
[2024-01-15 14:30:45.234] [INFO] Running with administrator privileges
[2024-01-15 14:30:45.345] [INFO] SeDebugPrivilege enabled successfully
[2024-01-15 14:30:45.456] [INFO] ===== Phase 1: Stopping Vanguard Services =====
[2024-01-15 14:30:45.567] [INFO] Stopping service: vgk
[2024-01-15 14:30:46.678] [INFO] Service stopped successfully: vgk
[2024-01-15 14:30:46.789] [INFO] Stopping service: vgc
[2024-01-15 14:30:47.890] [INFO] Service stopped successfully: vgc
[2024-01-15 14:30:49.901] [INFO] ===== Phase 2: Starting Pipe Server =====
[2024-01-15 14:30:49.912] [INFO] Starting pipe server: \\.\pipe\933823D3-C77B-4BAE-89D7-A92B567236BC
[2024-01-15 14:30:50.023] [INFO] Pipe server started successfully
[2024-01-15 14:30:51.134] [INFO] ===== Phase 3: Restarting Vanguard Services =====
[2024-01-15 14:30:51.245] [INFO] Starting Vanguard services
[2024-01-15 14:30:51.356] [INFO] Starting service: vgk
[2024-01-15 14:30:52.467] [INFO] Service started successfully: vgk
[2024-01-15 14:30:52.578] [INFO] Starting service: vgc
[2024-01-15 14:30:53.689] [INFO] Service started successfully: vgc
[2024-01-15 14:30:53.790] [INFO] ===== Emulator Running =====
[2024-01-15 14:30:53.891] [INFO] Pipe hijacking active. Press Ctrl+C to stop.
```

## Stop the Emulator

Press `Ctrl+C` in the console window.

## Troubleshooting

### "Please run as administrator"
→ Right-click executable → Run as administrator

### "Failed to open Service Control Manager"
→ Ensure you have admin privileges and services are accessible

### "Failed to create named pipe"
→ Check if pipe name is already in use by another process

### Services don't exist
→ This emulator requires vgk/vgc services to be installed on your system

## Log Files

Check `vanguard_emulator.log` in the same directory for detailed logs.

## Next Steps

📖 Read `README.md` for detailed documentation  
🔧 Read `BUILDING.md` for build options  
💻 Read `PROJECT_SUMMARY.md` for technical details  
🤝 Read `CONTRIBUTING.md` to contribute

## Support

Having issues? Check:
1. This quick start guide
2. README.md troubleshooting section
3. BUILDING.md for build issues
4. Log file for error details

---

**⚠️ Educational Use Only**: This tool is for research and learning purposes. Use responsibly.

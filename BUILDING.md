# Building the Vanguard Emulator

This document provides detailed instructions for building the Vanguard Pipe-Hijacking Emulator.

## Prerequisites

### Required Software

1. **Windows 10/11 (64-bit)**
   - This is a Windows-specific application

2. **Visual Studio 2019 or later**
   - Download from: https://visualstudio.microsoft.com/
   - Required workload: "Desktop development with C++"
   - Includes: MSVC compiler, Windows SDK

3. **Windows SDK 10.0 or later**
   - Usually included with Visual Studio
   - Required for Windows API headers

### Optional Tools

- **CMake 3.15+**: For alternative build system
- **Git**: For version control

## Building with Visual Studio

### Method 1: Using Visual Studio IDE

1. Open Visual Studio

2. Open the solution:
   - File → Open → Project/Solution
   - Navigate to `VanguardEmulator.sln`
   - Click Open

3. Select build configuration:
   - Toolbar dropdown: `Debug` or `Release`
   - Platform: `x64`

4. Build the project:
   - Menu: Build → Build Solution
   - Or press: `Ctrl+Shift+B`
   - Or press: `F7`

5. Output location:
   - Debug build: `bin/Debug/VanguardEmulator.exe`
   - Release build: `bin/Release/VanguardEmulator.exe`

### Method 2: Using Command Line

1. Open Visual Studio Developer Command Prompt:
   - Start Menu → Visual Studio 2022 → Developer Command Prompt

2. Navigate to project directory:
   ```cmd
   cd C:\path\to\VanguardEmulator
   ```

3. Build with MSBuild:
   ```cmd
   msbuild VanguardEmulator.sln /p:Configuration=Release /p:Platform=x64 /m
   ```

### Method 3: Using Build Script

1. Open Command Prompt as Administrator

2. Navigate to project directory:
   ```cmd
   cd C:\path\to\VanguardEmulator
   ```

3. Run build script:
   ```cmd
   build.bat Release
   ```

   Or for Debug build:
   ```cmd
   build.bat Debug
   ```

   To clean build artifacts:
   ```cmd
   build.bat clean
   ```

## Building with CMake

### Using CMake GUI

1. Open CMake GUI

2. Set source directory: `C:/path/to/VanguardEmulator`

3. Set build directory: `C:/path/to/VanguardEmulator/build`

4. Click "Configure"
   - Select generator: "Visual Studio 16 2019" or later
   - Platform: x64
   - Click Finish

5. Click "Generate"

6. Click "Open Project" to open in Visual Studio

7. Build in Visual Studio as normal

### Using CMake Command Line

1. Open Command Prompt

2. Navigate to project directory:
   ```cmd
   cd C:\path\to\VanguardEmulator
   ```

3. Create build directory:
   ```cmd
   mkdir build
   cd build
   ```

4. Configure with CMake:
   ```cmd
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

5. Build:
   ```cmd
   cmake --build . --config Release
   ```

6. Output location:
   ```
   build/bin/VanguardEmulator.exe
   ```

## Build Configurations

### Debug Configuration

- **Purpose**: Development and debugging
- **Optimizations**: Disabled
- **Debug symbols**: Full
- **Runtime checks**: Enabled
- **Output**: `bin/Debug/`

**When to use:**
- Development
- Troubleshooting issues
- Adding new features

### Release Configuration

- **Purpose**: Production use
- **Optimizations**: Full (`/O2`)
- **Debug symbols**: Limited
- **Runtime checks**: Disabled
- **Output**: `bin/Release/`

**When to use:**
- Final testing
- Deployment
- Performance measurement

## Compiler Settings

### Language Standard

- **C++ Standard**: C++17 (ISO/IEC 14882:2017)
- **Conformance Mode**: Enabled
- **Character Set**: Unicode

### Include Directories

```
$(ProjectDir)include
```

### Preprocessor Definitions

**Debug:**
```
_DEBUG
_CONSOLE
```

**Release:**
```
NDEBUG
_CONSOLE
```

### Linked Libraries

```
psapi.lib      - Process Status API
kernel32.lib   - Windows Kernel functions
user32.lib     - User interface functions
advapi32.lib   - Advanced Windows services
```

## Troubleshooting Build Issues

### "Cannot open include file"

**Problem**: Missing Windows SDK or incorrect include paths

**Solution:**
1. Verify Windows SDK is installed
2. Check Visual Studio → Tools → Get Tools and Features
3. Install "Windows 10 SDK" or later
4. Restart Visual Studio

### "LNK2019: unresolved external symbol"

**Problem**: Missing library dependencies

**Solution:**
1. Verify `psapi.lib` is linked
2. Check project properties:
   - Configuration Properties → Linker → Input
   - Additional Dependencies should include: `psapi.lib;kernel32.lib;user32.lib;advapi32.lib`

### "error MSB8036: The Windows SDK version was not found"

**Problem**: Specified Windows SDK version not installed

**Solution:**
1. Open `VanguardEmulator.vcxproj` in text editor
2. Find `<WindowsTargetPlatformVersion>` tag
3. Change to installed version (e.g., `10.0`)
4. Or install the required SDK version

### Build succeeds but executable doesn't run

**Problem**: Missing runtime dependencies or insufficient privileges

**Solution:**
1. Ensure running on Windows (not Wine/Linux)
2. Run as Administrator
3. Check Visual C++ Redistributables are installed

### "error C2039: 'thread': is not a member of 'std'"

**Problem**: C++ standard version too old

**Solution:**
1. Verify C++ Standard is set to C++17 or later
2. Project Properties → C/C++ → Language
3. Language Standard → ISO C++17 Standard

## Verifying the Build

After building, verify the executable:

1. **Check file exists:**
   ```cmd
   dir bin\Release\VanguardEmulator.exe
   ```

2. **Check file size:**
   - Should be between 50KB - 500KB (depending on configuration)

3. **Test execution (as Administrator):**
   ```cmd
   cd bin\Release
   VanguardEmulator.exe
   ```

   Should output:
   ```
   ========================================
     Vanguard Pipe-Hijacking Emulator
   ========================================
   ```

## Clean Build

To perform a clean build:

### Visual Studio
1. Build → Clean Solution
2. Build → Rebuild Solution

### Command Line
```cmd
msbuild VanguardEmulator.sln /t:Clean /p:Configuration=Release /p:Platform=x64
msbuild VanguardEmulator.sln /t:Build /p:Configuration=Release /p:Platform=x64
```

### Build Script
```cmd
build.bat clean
build.bat Release
```

## Build Artifacts

After successful build, you'll find:

```
bin/
  Release/
    VanguardEmulator.exe    - Main executable
obj/
  Release/
    *.obj                   - Object files
    *.pdb                   - Debug symbols
```

## Deployment

To deploy the application:

1. **Copy required files:**
   ```
   VanguardEmulator.exe
   config.ini
   ```

2. **No additional DLLs required** (statically linked)

3. **System requirements:**
   - Windows 10/11 64-bit
   - Administrator privileges
   - Visual C++ Redistributable (usually pre-installed)

## Next Steps

After building successfully:

1. Read `README.md` for usage instructions
2. Configure `config.ini` for your needs
3. Run as Administrator
4. Review logs in `vanguard_emulator.log`

## Support

For build issues:
1. Check this document thoroughly
2. Verify all prerequisites are installed
3. Try a clean build
4. Check Visual Studio output window for detailed errors

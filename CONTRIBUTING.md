# Contributing to Vanguard Emulator

Thank you for your interest in contributing to this educational research project!

## Code of Conduct

This project is for **educational and research purposes only**. All contributions must:
- Be legal and ethical
- Focus on learning and understanding
- Include proper documentation
- Not promote malicious use

## How to Contribute

### Reporting Issues

If you find a bug or have a suggestion:
1. Check if the issue already exists
2. Provide detailed information:
   - Operating system and version
   - Visual Studio version
   - Steps to reproduce
   - Expected vs actual behavior
   - Relevant log output

### Submitting Code

1. **Fork the repository**

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow existing code style
   - Add comments for complex logic
   - Update documentation as needed

4. **Test your changes**
   - Build in both Debug and Release
   - Test on clean Windows installation
   - Verify no regressions

5. **Commit your changes**
   ```bash
   git commit -am "Add feature: description"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Describe your changes clearly
   - Reference any related issues
   - Explain testing performed

## Code Style Guidelines

### C++ Style

- **Standard**: C++17
- **Naming**:
  - Classes: `PascalCase`
  - Methods: `camelCase`
  - Variables: `camelCase`
  - Member variables: `m_camelCase`
  - Constants: `UPPER_CASE`
  
- **Formatting**:
  - Indentation: 4 spaces (no tabs)
  - Braces: Opening brace on same line
  - Line length: ~120 characters max

- **Headers**:
  - Use `#pragma once` for header guards
  - Include order: Project headers, system headers
  - Forward declarations when possible

### Example Code Style

```cpp
class ExampleClass {
public:
    ExampleClass();
    ~ExampleClass();
    
    bool performAction(const std::string& input);
    
private:
    void helperMethod();
    
    std::string m_data;
    int m_counter;
};

bool ExampleClass::performAction(const std::string& input) {
    if (input.empty()) {
        return false;
    }
    
    m_data = input;
    helperMethod();
    
    return true;
}
```

## Areas for Contribution

### High Priority

- **Protocol Analysis**: Understanding Vanguard pipe messages
- **Error Handling**: Improved error messages and recovery
- **Documentation**: Usage examples and troubleshooting
- **Testing**: Compatibility across Windows versions

### Medium Priority

- **Configuration**: Additional configuration options
- **Logging**: Enhanced logging capabilities
- **Performance**: Optimization opportunities
- **Security**: Privilege management improvements

### Low Priority

- **UI**: Optional GUI interface
- **Monitoring**: Real-time statistics
- **Automation**: Scripting support

## Development Setup

1. Install prerequisites (see BUILDING.md)
2. Clone repository
3. Open in Visual Studio
4. Build and test

## Testing Guidelines

Before submitting:

1. **Build Tests**
   - Debug configuration compiles
   - Release configuration compiles
   - No compiler warnings

2. **Functional Tests**
   - Application starts successfully
   - Administrator check works
   - Services stop/start correctly
   - Pipe server creates successfully
   - Clean shutdown works

3. **Code Quality**
   - No memory leaks
   - Proper resource cleanup
   - Thread-safe operations
   - Error handling present

## Documentation

When adding features:

1. Update README.md if user-facing
2. Update BUILDING.md if build process changes
3. Add code comments for complex logic
4. Update config.ini.example if adding settings

## Commit Message Guidelines

Format:
```
<type>: <short description>

<optional longer description>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `test`: Adding tests
- `chore`: Maintenance tasks

Examples:
```
feat: Add message encryption support

Implement AES-256 encryption for pipe messages to enhance
security and better emulate production behavior.

fix: Correct service restart timing

Services were restarting too quickly causing race condition.
Added configurable delay between stop and start.
```

## Review Process

All contributions will be reviewed for:
1. Code quality and style
2. Functionality and correctness
3. Documentation completeness
4. Testing coverage
5. Security implications
6. Educational value

## Questions?

If you have questions about contributing:
- Check existing documentation
- Review closed issues for similar questions
- Open a new issue with "Question:" prefix

## License

By contributing, you agree that your contributions will be licensed under the same MIT License that covers the project.

## Acknowledgments

We appreciate all contributions, big and small. Contributors will be acknowledged in the project documentation.

Thank you for helping improve this educational project!

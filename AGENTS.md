# Loong Interpreter Agent Guide

## Build/Test Commands
- **Build**: `build.cmd` (Win64), `build32.cmd` (Win32), `build.sh` (Linux), or `g++ *.cpp -Iinclude -std=c++11 -o loong`
- **Run single test**: Compile test file (e.g., `g++ test_lexer.cpp Lexer.cpp Token.cpp -Iinclude -o test_lexer.exe`) then execute
- **Run interpreter test**: `./loong test_sample.loong` (Linux/Mac) or `loong.exe test_sample.loong` (Windows)
- **No automated test framework** - run manual tests via compiled executables

## Code Style Guidelines

### Naming Conventions
- **Classes**: PascalCase with 'C' prefix (e.g., `CLexer`, `CParser`)
- **Member variables**: 'm_' prefix + camelCase (e.g., `m_strText`, `m_nPos`)
- **Functions**: camelCase (e.g., `advance()`, `error()`)
- **Constants**: UPPER_CASE (e.g., `LICENSE_SIZE`)

### Formatting & Structure
- **Indentation**: Tabs
- **Braces**: Opening brace on same line as control statement
- **Includes**: Local headers first (quotes), then system headers (angle brackets)
- **Comments**: Prefer Chinese comments, English acceptable
- **Headers**: Use `#pragma once`, place in `include/` directory
- **File organization**: One header/cpp pair per class

### Language Features
- **C++ Standard**: C++11
- **Namespaces**: Allow `using namespace std;` in implementation files
- **Error handling**: Use `printf()` for errors, avoid exceptions
- **Platform code**: Use `#ifdef _WIN32` for Windows-specific code
- **Dependencies**: Minimal external dependencies, self-contained interpreter
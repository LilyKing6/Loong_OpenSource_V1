# Loong

Loong is an interpreted programming language with support for both English and Chinese keywords, built with modern C++23.

## Features

- Dual-language keywords (English and Chinese)
- Dynamic typing with `int`, `float`, `string`, `array`, `dict`, `class`
- Classes with constructors and member methods
- `include` (source import) and `import` (library loading)
- Built-in functions: `print`, `typeof`, `sprintf`, `atoi`, `atof`, etc.
- File I/O, DLL/shared library loading
- Regular expression and math functions
- C++23 with CMake build system

## Build

Requirements:
- CMake 3.20+
- GCC 14+ / MSVC 2022+ / Clang 18+ (with C++23 support)

```bash
cmake -B build
cmake --build build
```

## Usage

```bash
./build/loong program.lo
```

## Project Structure

```
├── CMakeLists.txt
├── include/loong/       # Public headers
│   ├── ast.hpp
│   ├── call_stack.hpp
│   ├── debug.hpp
│   ├── global_data.hpp
│   ├── interpreter.hpp
│   ├── lexer.hpp
│   ├── library.hpp
│   ├── parser.hpp
│   ├── token.hpp
│   ├── variable.hpp
│   └── version.hpp
├── src/                  # Implementation
├── tests/                # Test files
└── samples/              # Example .lo programs
```

## License

MIT License. Copyright (c) 2024 Lily King

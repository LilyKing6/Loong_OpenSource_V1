# Loong

Loong 是一门解释型编程语言，同时支持中英文关键字，基于现代 C++23 构建。

## 特性

- 中英文双语关键字
- 动态类型，支持 `int`、`float`、`string`、`array`、`dict`、`class`
- 类支持构造函数与成员方法
- `include`（源码导入）与 `import`（库加载）
- 内置函数：`print`、`typeof`、`sprintf`、`atoi`、`atof` 等
- 文件 I/O、动态库加载
- 正则表达式与数学函数
- C++23 + CMake 构建系统

## 构建

依赖：
- CMake 3.20+
- GCC 14+ / MSVC 2022+ / Clang 18+（需支持 C++23）

```bash
cmake -B build
cmake --build build
```

## 用法

```bash
./build/loong program.lo
```

## 项目结构

```
├── CMakeLists.txt
├── include/loong/       # 公开头文件
│   ├── ast.hpp
│   ├── call_stack.hpp
│   ├── global_data.hpp
│   ├── interpreter.hpp
│   ├── lexer.hpp
│   ├── library.hpp
│   ├── parser.hpp
│   ├── token.hpp
│   ├── variable.hpp
│   └── version.hpp
├── src/                  # 实现文件
├── tests/                # 测试
└── samples/              # 示例 .lo 程序
```

## 许可证

MIT License. Copyright (c) 2023-2026 Lily King

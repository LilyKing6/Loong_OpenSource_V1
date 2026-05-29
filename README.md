# Loong

Loong 是一门解释型编程语言，同时支持中英文关键字，基于现代 C++23 构建。

## 特性

- 中英文双语关键字
- 动态类型，支持 `int`、`float`、`string`、`array`、`dict`、`class`、`handle`
- 类支持构造函数、成员方法、私有成员（`__` 前缀）、`self` 引用
- 函数支持默认参数
- `#include`（源码导入）与 `#import`（库加载），自动去重
- `static` 声明支持按文件命名空间隔离
- C 风格格式化字符串（`sprintf` / `printf`）
- 文件 I/O、时间函数、系统调用
- 字符串/数组/字典内置方法库
- 引用计数自动内存管理
- 正则表达式与数学函数（预留接口）
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
./build/loong program.lo [args...]
```

## 数据类型

| 类型 | 关键字 | 说明 |
|---|---|---|
| 整数 | `int` | 64 位有符号整数 |
| 浮点数 | `float` | 双精度浮点数 |
| 字符串 | `string` | UTF-8 字符串 |
| 数组 | `array` | 动态数组，引用计数 |
| 字典 | `dict` | 有序键值映射，引用计数 |
| 类 | `class` | 类实例（内部为 Dict），引用计数 |
| 句柄 | `handle` | 原始指针（文件句柄、迭代器等） |
| 空 | `null` | 空值 |
| 未定义 | — | 未初始化变量 |

**真值规则**：`0`、`0.0`、空字符串、空数组、空字典、`null` 为假，其余为真。

## 关键字

### 控制流
`if` / `else` / `while` / `for` / `break` / `continue` / `return`

### 声明
`func` / `class` / `static` / `global`

### 程序入口
`program` — 入口点声明为 `program main() { ... }`

### 字面量
`true` / `false` / `null`

## 运算符

### 算术
`+` `-` `*` `/` `%`

### 比较
`==` `!=` `>` `<` `>=` `<=`

### 逻辑（短路求值）
`&&` `||` `!`

### 位运算
`&` `|` `^` `~` `<<` `>>`

### 赋值
`=` `+=` `-=` `*=` `/=` `%=` `&=` `|=` `^=` `~=` `<<=` `>>=`

### 自增/自减
`++` `--`（仅后缀）

### 其他
`.` 成员访问  `[]` 下标  `()` 调用/分组

### 运算符优先级（由低到高）

1. `||`
2. `&&`
3. `|`
4. `^`
5. `&`
6. `==` `!=`
7. `>` `<` `>=` `<=`
8. `>>`
9. `<<`
10. `+` `-`
11. `*` `/` `%`
12. 一元 `!` `~` `-`
13. `.` `[]`
14. 字面量、变量、函数调用、括号表达式

## 语法

### 程序结构

```
// 顶层声明：函数、类、全局变量、static、#include、#import
func add(a, b) { return a + b; }

program main() {
    // 程序体
    print(add(1, 2));
}
```

### 变量与赋值

```
x = 10;
name = "hello";
arr = [1, 2, 3];
dict = {"key1": "val1", "key2": "val2"};
```

### 条件语句

```
if (x > 0) {
    print("正数");
} else if (x == 0) {
    print("零");
} else {
    print("负数");
}
```

### 循环

```
// while
while (x < 10) {
    x++;
}

// for（C 风格）
for (i = 0; i < 5; i++) {
    print(i);
}
```

### 函数

```
func greet(name, prefix = "Hello") {
    print(prefix + ", " + name + "!");
}

greet("Loong");           // Hello, Loong!
greet("Loong", "Hi");     // Hi, Loong!
```

### 类

```
class Point(x, y) {
    func self.show() {
        print("(" + _str(self.x) + ", " + _str(self.y) + ")");
    }
}

p = Point(3, 4);
p.show();  // (3, 4)
```

- `self` 引用当前实例
- `__` 前缀的成员为私有，外部不可访问
- 方法声明为 `func self.methodName(...) { }`

### global 声明

```
counter = 0;

func increment() {
    global counter;
    counter++;
}
```

### static 声明

```
static helper = 42;
static func internalFunc() { ... }
static class InternalClass() { ... }
```

`static` 声明的变量/函数/类按文件名命名空间隔离，防止跨文件冲突。

### 注释

```
// 单行注释
#! shebang 风格注释
/* 块注释 */
```

### 数字字面量

```
42          // 十进制
3.14        // 浮点
0xFF        // 十六进制
0b1010      // 二进制
077         // 八进制
```

### 字符串转义

`\\` `\r` `\n` `\t` `\a` `\b` `\v` `\f` `\?` `\'` `\"` `\0` `\xHH`

## 包含与导入

```
#include "utils.lo"     // 从当前目录读取源码并解析
#import "stdlib.lo"     // 从解释器 Include/ 目录读取
```

- 自动去重，同一文件只包含一次
- 可在顶层或函数体内使用

## 内置函数

### 基本内置

| 函数 | 说明 |
|---|---|
| `print(...)` | 打印所有参数并换行，支持数组/字典美化输出 |
| `sprintf(fmt, ...)` | C 风格格式化，返回格式化字符串 |
| `printf(fmt, ...)` | C 风格格式化，输出到标准输出 |
| `_len(x)` | 返回字符串/数组/字典的长度 |
| `_str(x)` | 转换为字符串 |
| `_int(x)` | 转换为整数 |
| `_float(x)` | 转换为浮点数 |
| `_type(x)` | 返回类型名：`"INT"` `"FLOAT"` `"STRING"` `"ARRAY"` `"DICT"` `"CLASS"` `"HANDLE"` `"nullptr"` `"UNDEFINED"` |
| `_input()` | 从标准输入读取一行 |
| `_copy(x)` | 深拷贝数组/字典/类实例 |
| `_getargv()` | 获取命令行参数数组 |

### 格式化占位符

`%s` 字符串、`%c` 字符、`%d` 整数（64位自动提升）、`%f` 浮点、`%x` 十六进制小写、`%X` 十六进制大写、`%o` 八进制

### 扩展函数（通过 `_fun` 调用）

**文件操作：**

```
h = _fun("fopen", "data.bin", "rb");
_fun("fseek", h, 0);
data = _fun("fread", h, 100);
_fun("fclose", h);
_fun("fremove", "old.txt");
_fun("frename", "old.txt", "new.txt");
sz = _fun("fsize", h);
_fun("fwrite", h, "content");
```

**时间函数：**

```
_fun("time_clock")              // 时钟滴答数
info = _fun("time_now")         // 返回字典：time, time_str, year, mon, day, hour, min, sec
_fun("time_rand")               // 随机整数
_fun("time_sleep", 1000)        // 休眠 1000 毫秒
```

**系统函数：**

```
_fun("system", "ls")            // 执行 shell 命令
_fun("os_platform")             // 返回 "WIN x64" / "LINUX x64" / "MAC x64"
_fun("curdir")                  // 当前目录
_fun("getenv", "PATH")          // 获取环境变量
_fun("getstdin", 1024)          // 从 stdin 读取二进制数据
_fun("putstdin", data)          // 向 stdout 写入二进制数据
```

### 字符串方法

| 方法 | 说明 |
|---|---|
| `str.substr(pos)` | 从 pos 到末尾的子串 |
| `str.substr(pos, len)` | 从 pos 起 len 个字符 |
| `str.find(key)` | 查找子串，返回索引或 -1 |
| `str.find(key, start)` | 从 start 位置查找 |
| `str.rfind(key)` | 从右查找子串 |
| `str.replace(old, new)` | 替换所有匹配 |
| `str.replace(pos, len, new)` | 替换指定位置子串 |
| `str.split(delim)` | 按分隔符拆分为数组 |
| `str.size()` | 字符串长度 |
| `str.insert(pos, s)` | 在 pos 处插入字符串 |
| `str.erase(pos, len)` | 删除指定长度子串 |
| `str.trim()` | 去除两端空白 |
| `str.ltrim()` | 去除左端空白 |
| `str.rtrim()` | 去除右端空白 |
| `str.lower()` | 转小写 |
| `str.upper()` | 转大写 |

### 数组方法

| 方法 | 说明 |
|---|---|
| `arr.append(val)` | 末尾添加元素 |
| `arr.size()` | 元素数量 |
| `arr.resize(n)` | 调整数组大小 |
| `arr.resize(n, fill)` | 调整大小并用 fill 填充 |
| `arr.clear()` | 清空数组 |
| `arr.erase(i)` | 删除索引 i 处元素 |
| `arr.erase(start, end)` | 删除 start 到 end 范围元素 |
| `arr.insert(i, val)` | 在索引 i 处插入元素 |
| `arr.sort()` | 升序排序 |
| `arr.swap(other)` | 与另一数组交换内容 |
| `arr.create2d(d1, d2)` | 创建二维数组 |
| `arr.create3d(d1, d2, d3)` | 创建三维数组 |

### 字典方法

| 方法 | 说明 |
|---|---|
| `dict.find(key)` | 按键查找值 |
| `dict.size()` | 条目数量 |
| `dict.insert(key, val)` | 插入键值对，返回 1(新) / 0(已存在) |
| `dict.erase(key)` | 删除条目 |
| `dict.clear()` | 清空字典 |
| `dict.begin()` | 获取正向迭代器 |
| `dict.end(iter)` | 迭代器是否到达末尾 |
| `dict.rbegin()` | 获取反向迭代器 |
| `dict.rend(iter)` | 反向迭代器是否到达开头 |
| `dict.next(iter)` | 推进迭代器 |
| `dict.get(iter)` | 获取迭代器位置的 [key, value] |
| `dict.get(key)` | 获取指定键的 [key, value] |

### 字典迭代示例

```
d = {"a": 1, "b": 2, "c": 3};
it = d.begin();
while (!d.end(it)) {
    kv = d.get(it);
    print(kv[0] + " = " + _str(kv[1]));
    d.next(it);
}
```

## 项目结构

```
├── CMakeLists.txt
├── include/loong/       # 公开头文件
│   ├── ast.hpp          # 抽象语法树节点
│   ├── call_stack.hpp   # 调用栈
│   ├── debug.hpp        # 调试工具
│   ├── global_data.hpp  # 全局数据
│   ├── interpreter.hpp  # 解释器
│   ├── lexer.hpp        # 词法分析器
│   ├── library.hpp      # 内置库（字符串/数组/字典/文件/函数）
│   ├── parser.hpp       # 语法分析器
│   ├── token.hpp        # 词法单元
│   ├── variable.hpp     # 变量类型系统
│   └── version.hpp      # 版本信息
├── src/                  # 实现文件
├── tests/                # 测试
└── samples/              # 示例 .lo 程序
```

## 许可证

MIT License. Copyright (c) 2023-2026 Lily King

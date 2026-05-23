import pathlib

# Fix 1: interpreter.cpp - formattedPrint unused int n + unused results
f = pathlib.Path(r'D:\Projects\Loong_Opensource_Reload\loong_reload\src\interpreter.cpp')
data = f.read_bytes()

# Fix 1a: Remove unused int n in formattedPrint
old1 = b'va_list args;\r\n\tint n;\r\n\tva_start(args, format);\r\n\tn = vsnprintf(buffer, 1024, format, args);'
data = data.replace(old1, b'va_list args;\r\n\tva_start(args, format);\r\n\tvsnprintf(buffer, 1024, format, args);', 1)
print('Fix 1: removed unused int n in formattedPrint')

# Fix 2: Remove unused vector<Variable> results in builtin handlers
# In _len: vector results followed by vector<AstNode*>& exprs
data = data.replace(b'vector<Variable> results;\r\n\t\t\tvector<AstNode*>& exprs = node->exprs();\r\n\t\t\tif (exprs.size()!=1)',
                    b'vector<AstNode*>& exprs = node->exprs();\r\n\t\t\tif (exprs.size()!=1)', 1)
print('Fix 2: removed unused results in _len')

# _str
data = data.replace(b'vector<Variable> results;\r\n\t\t\tvector<AstNode*>& exprs = node->exprs();\r\n\t\t\tif (exprs.size() != 1)',
                    b'vector<AstNode*>& exprs = node->exprs();\r\n\t\t\tif (exprs.size() != 1)', 1)
print('Fix 3: removed unused results in _str')

# _int + _float + _type: they have \t\t\t indentation
data = data.replace(b'\t\t\tvector<Variable> results;\r\n\t\t\tvector<AstNode*>& exprs = node->exprs();\r\n\t\t\tif (exprs.size() != 1)\r\n\t\t\t\terror(',
                    b'\t\t\tvector<AstNode*>& exprs = node->exprs();\r\n\t\t\tif (exprs.size() != 1)\r\n\t\t\t\terror(', 3)
print('Fix 4: removed unused results in _int / _float / _type')

f.write_bytes(data)
print('interpreter.cpp done')

# Fix 5: parser.cpp - formattedPrint unused int n
f2 = pathlib.Path(r'D:\Projects\Loong_Opensource_Reload\loong_reload\src\parser.cpp')
data2 = f2.read_bytes()

# parser.cpp uses LF, not CRLF
old_parser = b'va_list args;\n\tint n;\n\tva_start(args, format);\n\tn = vsnprintf(buffer, 1024, format, args);'
data2 = data2.replace(old_parser, b'va_list args;\n\tva_start(args, format);\n\tvsnprintf(buffer, 1024, format, args);', 1)
print('Fix 5: removed unused int n in parser.cpp formattedPrint')

f2.write_bytes(data2)
print('parser.cpp done')

# Fix 6: library.cpp - const_cast + double semicolons + File::write signature
f3 = pathlib.Path(r'D:\Projects\Loong_Opensource_Reload\loong_reload\src\library.cpp')
data3 = f3.read_bytes()

# 6a: const_cast in callFunc
data3 = data3.replace(b'(Variable&)a[2]', b'a[2]', 1)
print('Fix 6a: removed const_cast')

# 6b: File::write signature
data3 = data3.replace(b'bool File::write(void* handle, Variable& content)',
                       b'bool File::write(void* handle, const Variable& content)', 1)
print('Fix 6b: File::write signature updated')

# 6c: double semicolons
data3 = data3.replace(b'FILE* fp = (FILE*)handle;;', b'FILE* fp = (FILE*)handle;')
print('Fix 6c: removed double semicolons')

f3.write_bytes(data3)
print('library.cpp done')

# Fix 7: library.hpp - File::write signature
f4 = pathlib.Path(r'D:\Projects\Loong_Opensource_Reload\loong_reload\include\loong\library.hpp')
data4 = f4.read_bytes()
data4 = data4.replace(b'bool write(void* handle, Variable& content);',
                       b'bool write(void* handle, const Variable& content);', 1)
f4.write_bytes(data4)
print('Fix 7: library.hpp File::write signature updated')

print('\nAll fixes applied.')

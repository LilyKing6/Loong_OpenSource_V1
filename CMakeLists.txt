cmake_minimum_required(VERSION 3.20)
project(loong VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp
    src/token.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/interpreter.cpp
    src/variable.cpp
    src/call_stack.cpp
    src/library.cpp
    src/global_data.cpp
)

add_executable(loong ${SOURCES})

target_include_directories(loong
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(loong PRIVATE kernel32 ws2_32)
else()
    target_link_libraries(loong PRIVATE dl pthread)
endif()

add_executable(test_lexer tests/test_lexer.cpp src/token.cpp src/lexer.cpp)
target_include_directories(test_lexer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

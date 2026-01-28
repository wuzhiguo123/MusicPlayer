# ==============================================
# 1. 基本配置
# ==============================================
cmake_minimum_required(VERSION 3.10)

# 指定交叉编译器（无 toolchain 版本）
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)

project(qianwen C)

# C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# 编译警告
add_compile_options(-Wall -Wextra)

# 导出 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(JSON-C_LIBRARIES /root/json-arm-install/lib/libjson-c.so.5.4.0)
set(JSON-C_INCLUDE_DIRS /root/json-arm-install/include)



# ==============================================
# 5. MusicPlayer 可执行文件
# ==============================================
add_executable(${PROJECT_NAME} qianwen.c)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JSON-C_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _POSIX_C_SOURCE=200809L
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${JSON-C_LIBRARIES}

)





 
# ==============================================
# 7. 调试输出
# ==============================================
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
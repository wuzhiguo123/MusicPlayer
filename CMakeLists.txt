# ==============================================
# 1. 基本配置部分
# ==============================================

# 告诉CMake需要的最低版本（3.10是较新的稳定版本）
cmake_minimum_required(VERSION 3.10)

# 设置项目名称（会创建一些有用的变量，如PROJECT_NAME）
# 第二个参数指定语言：C表示C语言项目
project(MusicPlayer C)

# ==============================================
# 2. 编译器设置部分
# ==============================================

# 设置C标准版本（C11是当前常用的标准）
set(CMAKE_C_STANDARD 11)          # 使用C11标准
set(CMAKE_C_STANDARD_REQUIRED ON) # 必须使用指定标准
set(CMAKE_C_EXTENSIONS OFF)       # 关闭编译器扩展，提高可移植性

# 设置编译警告（帮助发现潜在问题）
# -Wall: 启用大部分警告
# -Wextra: 启用额外警告
# -Werror: 将警告视为错误（可选，开发时建议使用）
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# ==============================================
# 3. 变量定义部分
# ==============================================

# 定义源文件列表（方便管理）
set(PLAYER_SOURCES player/player.c)
set(MUSIC_SOURCES musiclink/link.c)
set(SELECT_SOURCES select/select.c)
set(NET_SOURCES net/net.c)
set(MAIN_SOURCES main.c)

# 将所有源文件合并到一个列表中
set(ALL_SOURCES 
    ${PLAYER_SOURCES}
    ${MUSIC_SOURCES}
    ${SELECT_SOURCES}
    ${MAIN_SOURCES}
    ${NET_SOURCES}
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON-C REQUIRED json-c)
message(STATUS "JSON-C_LIBRARIES: ${JSON-C_LIBRARIES}")
message(STATUS "JSON-C_INCLUDE_DIRS: ${JSON-C_INCLUDE_DIRS}")
find_package(Threads REQUIRED) 
find_package(ALSA REQUIRED)


# 定义包含目录（头文件路径）
set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/player
    ${CMAKE_CURRENT_SOURCE_DIR}/musiclink
    ${CMAKE_CURRENT_SOURCE_DIR}/select
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ==============================================
# 4. 可执行文件配置部分
# ==============================================



# 创建可执行文件
# 语法：add_executable(目标名 源文件列表)
add_executable(${PROJECT_NAME}  # 使用项目名称作为可执行文件名
    ${ALL_SOURCES}              # 包含所有源文件
)

# 添加头文件搜索路径
# 语法：target_include_directories(目标 PUBLIC/PRIVATE/INTERFACE 路径列表)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIRECTORIES}
    ${JSON-C_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
)


# ==============================================
# 5. 库链接部分（如果需要链接外部库）
# ==============================================

# 如果您的代码使用了共享内存函数，可能需要链接rt库
# 语法：target_link_libraries(目标 库1 库2 ...)

target_link_libraries(${PROJECT_NAME}
    ${JSON-C_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}  # 或使用 ${CMAKE_THREAD_LIBS_INIT}
)
# 示例：如果shm/chm.c使用了共享内存函数，添加这些库
# target_link_libraries(${PROJECT_NAME}
#     rt      # 实时扩展库（包含shm_open, shm_unlink等）
#     pthread # POSIX线程库（如果使用了多线程）
# )

# ==============================================
# 6. 安装配置（可选）
# ==============================================

# 安装可执行文件到系统路径
# install(TARGETS ${PROJECT_NAME}
#     DESTINATION bin          # 安装到/usr/local/bin
# )

# ==============================================
# 7. 额外配置（可选，用于调试）
# ==============================================

# 启用编译命令数据库（供IDE和工具使用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 打印配置信息（调试用）
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "包含目录: ${INCLUDE_DIRECTORIES}")
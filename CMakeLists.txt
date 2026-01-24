# ==============================================
# 1. 基本配置
# ==============================================
cmake_minimum_required(VERSION 3.10)

# 指定交叉编译器（无 toolchain 版本）
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)

project(MusicPlayer C)

# C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 编译警告
add_compile_options(-Wall -Wextra)

# 导出 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================
# 2. 线程库（关键点）
# ==============================================
find_package(Threads REQUIRED)

# ==============================================
# 3. 第三方库（你现有的）
# ==============================================
set(JSON-C_LIBRARIES /root/json-arm-install/lib/libjson-c.so.5.4.0)
set(JSON-C_INCLUDE_DIRS /root/json-arm-install/include)

set(ALSA_LIBRARIES /root/alsa-arm-install/lib/libasound.so.2.0.0)
set(ALSA_INCLUDE_DIRS /root/alsa-arm-install/include)

# ==============================================
# 4. 源文件
# ==============================================
set(PLAYER_SOURCES  player/player.c)
set(MUSIC_SOURCES   musiclink/link.c)
set(SELECT_SOURCES  select/select.c)
set(NET_SOURCES     net/net.c)
set(DEVICE_SOURCES  device/device.c)
set(MAIN_SOURCES    main.c)

set(ALL_SOURCES
    ${PLAYER_SOURCES}
    ${MUSIC_SOURCES}
    ${SELECT_SOURCES}
    ${NET_SOURCES}
    ${DEVICE_SOURCES}
    ${MAIN_SOURCES}
)

# ==============================================
# 5. MusicPlayer 可执行文件
# ==============================================
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    player
    musiclink
    select
    net
    device
    ${JSON-C_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _POSIX_C_SOURCE=200809L
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${JSON-C_LIBRARIES}
    ${ALSA_LIBRARIES}
    Threads::Threads
)

# ==============================================
# 6. server 可执行文件
# ==============================================
add_executable(server
    net/server.c
)

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    net
    ${JSON-C_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
)

target_link_libraries(server PRIVATE
    ${JSON-C_LIBRARIES}
    ${ALSA_LIBRARIES}
    Threads::Threads
)

# ==============================================
# 7. 调试输出
# ==============================================
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
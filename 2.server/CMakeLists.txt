# 设置最低 CMake 版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(server CXX)

# 设置一个变量来包含所有的源文件
set(ALL_SOURCES
    main.cpp
    server.cpp
    database.cpp
    player.cpp
)

# 添加可执行文件
add_executable(server ${ALL_SOURCES})

# --- 关键修改部分 ---

# --- 使用 PkgConfig 查找 Libevent ---
find_package(PkgConfig REQUIRED)
pkg_search_module(LIBEVENT REQUIRED libevent)
pkg_search_module(MYSQL REQUIRED mysqlclient)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
# find_package(MySQL REQUIRED)

message(" MySQL_INCLUDE_DIRS:" ${MySQL_INCLUDE_DIRS})
message(" MYSQL_LIBRARIES:" ${MySQL_LIBRARIES})

# 将头文件目录添加到目标
target_include_directories(server PRIVATE 
                                    ${LIBEVENT_INCLUDE_DIRS}
                                    ${MYSQL_INCLUDE_DIRS}
                                    ${JSONCPP_INCLUDE_DIRS})
# 链接库到目标
target_link_libraries(server PRIVATE ${LIBEVENT_LIBRARIES}
                                      ${MYSQL_LIBRARIES}
                                      ${JSONCPP_LIBRARIES})

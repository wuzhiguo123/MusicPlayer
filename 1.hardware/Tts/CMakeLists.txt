cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER  aarch64-linux-gnu-gcc)

project(tts C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)


set(ALSA_LIBRARIES /root/alsa-arm-install/lib/libasound.so.2.0.0)
set(ALSA_INCLUDE_DIRS /root/alsa-arm-install/include)

set(ONNXRUNTIME_LIBRARIES /root/model/lib/libonnxruntime.so)
set(SHERPA_LIBRARY /root/model/lib/libsherpa-onnx-c-api.so)

set(SHERPA_INCLUDE_DIR /root/model/include)

set(ALL_SOURCES 
    main.c
    alsa.c
    tts.c)
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SHERPA_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ALSA_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${ONNXRUNTIME_LIBRARIES}
    ${SHERPA_LIBRARY}
    ${ALSA_LIBRARIES}
)

cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SkeletonViewer LANGUAGES CXX)

# ------------------------------------------------------------------------------
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

add_executable(${PROJECT_NAME} ${SOURCES})

# --------------------- ImGUI --------------------------------------------------
add_library(imgui
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui
    ${CMAKE_SOURCE_DIR}/include/imgui/backends
)

# ------------------------------------------------------------------------------
# 头文件路径（GLFW + GLEW 在项目 include）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_BINARY_DIR}
)

# ------------------------------------------------------------------------------
if (MSVC)
    message(STATUS "Detected compiler: MSVC")
    set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/msvc-2022)
elseif (MINGW)
    message(STATUS "Detected compiler: MinGW")
    set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/mingw64)
else()
    message(WARNING "Unknown compiler, defaulting to MSVC libs.")
    set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/msvc-2022)
endif()

target_link_directories(${PROJECT_NAME} PRIVATE ${LIB_DIR})

if (MSVC)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        opengl32
        glfw3dll
        glew32.dll
        assimp-vc143-mtd.dll
        imgui
    )
elseif (MINGW)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        opengl32
        glfw3dll                                                                                                                                                                  
        glew32.dll
        assimp.dll
        imgui
    )
endif()

# 自动复制 DLL
file(GLOB DLL_FILES "${LIB_DIR}/*.dll")

foreach(DLL ${DLL_FILES})
    add_custom_command(TARGET SkeletonViewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            "$<TARGET_FILE_DIR:SkeletonViewer>"
    )
endforeach()
cmake_minimum_required(VERSION 3.20)
project(SPH LANGUAGES CXX C)

# ---- C++ Standard ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Compiler Flags ----
if(MSVC)
    add_compile_options(/W4 /std:c++latest)
else()
    add_compile_options(-O2 -Wall -Wextra -Wpedantic)
endif()

# ---- Find Packages ----
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# ---- Include Directories ----
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/imgui
        ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# ---- Sources ----
set(SOURCES
        src/main.cpp
        src/UI/Window.cpp
        src/UI/Renderer.cpp
        src/UI/Mesh.cpp
        src/UI/Camera.cpp
        src/UI/glad.c
        include/Particle.h
        include/Math/Vec.h
        src/Particle.cpp
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        include/UI/ImGuiManager.h
        src/UI/ImGuiManager.cpp
        include/Rules.h
        include/Math/SPH.h
        src/Math/SPH.cpp
)

# ---- Executable ----
add_executable(NES ${SOURCES})

# ---- Link Libraries ----
target_link_libraries(NES PRIVATE glfw OpenGL::GL)

# ---- Platform Specific ----
if(APPLE)
    target_link_libraries(NES PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo")
elseif(WIN32)
    target_link_libraries(NES PRIVATE opengl32)
elseif(UNIX)
    target_link_libraries(NES PRIVATE dl pthread X11 Xrandr Xi Xxf86vm Xinerama Xcursor)
endif()

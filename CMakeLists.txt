cmake_minimum_required(VERSION 3.19)
project(NoiseSynth)
set(CMAKE_CXX_STANDARD 17)

include_directories(./external ./base ./src ./src/utils)
include_directories(./external/glad/include)

find_package(OpenCV REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)

link_libraries(${GLFW_LINK})
link_directories(./lib)

IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    link_libraries(libassimp.dylib)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    link_libraries(glfw3.lib assimp-vc142-mt.lib)
ENDIF()

# Add source to this project's executable.
set(baseSOURCES external/glad/src/glad.c base/shader.cpp base/camera.cpp base/object3d.cpp base/application.cpp base/model.cpp base/skybox.cpp base/texture.cpp external/tiny_obj_loader/tiny_obj_loader.cc)
set(imguiSources external/imgui/imgui_widgets.cpp external/imgui/imgui_impl_glfw.cpp external/imgui/imgui_impl_opengl3.cpp external/imgui/imgui.cpp external/imgui/imgui_draw.cpp external/imgui/imgui_tables.cpp)
set(stbSources external/stb/stb_image.cpp external/stb/stb_vorbis.c)
file(GLOB SOURCES "src/*" "src/utils/*")
add_executable(NoiseSynth ${SOURCES} ${baseSOURCES} ${imguiSources} ${stbSources})

target_link_libraries(NoiseSynth 
PRIVATE
    OpenGL::GL
    glfw
    ${OpenCV_LIBS}
)



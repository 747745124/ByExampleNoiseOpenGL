cmake_minimum_required(VERSION 3.19)
project(Test)
set(CMAKE_CXX_STANDARD 17)

# set(GLAD_H external/glad/include)
# set(GLFW_H external/glfw/include)
# set(IMGUI_H /imgui)
# set(STB_H /stb)
# set(dependencies /base)

# include_directories(${GLAD_H} ${GLFW_H} ${dependencies}${IMGUI_H}${STB_H})
include_directories(./external ./base ./src ./src/utils)
include_directories(./external/glad/include)
include_directories(/opt/homebrew/include)
include_directories(/opt/homebrew/include/opencv)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(GLFW_LINK /opt/homebrew/Cellar/glfw/3.3.8/lib/libglfw.3.dylib)
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
add_executable(sceneRoam ${SOURCES} ${baseSOURCES} ${imguiSources} ${stbSources})

# Linking System OpenGL Framework
if(APPLE)
    target_link_libraries(sceneRoam "-framework OpenGL")
    target_link_libraries(sceneRoam ${OpenCV_LIBS})
endif()


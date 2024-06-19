############################################################################################################
## On Windows, please download VulkanSDK on https://vulkan.lunarg.com/sdk/home#windows
## Select all extensions to be sure to have all features of the project
############################################################################################################

############################################################################################################
## On linux, please download VulkanSDK on https://vulkan.lunarg.com/sdk/home#linux
## Launch . ./setup-env.sh inside the downloaded archive to put VULKAN_SDK variable
## in the environment variables but don't change your terminal session when you'll launch run.sh script !  
############################################################################################################

cmake_minimum_required(VERSION 3.7)
project(raytracer)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wfatal-errors -Werror -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

##################
## GLFW ##########
##################

set(GLFW_DIR ${PROJECT_SOURCE_DIR}/libs/glfw)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)


##################
## dearImGui #####
##################
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/libs/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)


##################
## libraries #####
##################

find_package(Vulkan REQUIRED)
set(LIBRARIES "glfw;Vulkan::Vulkan")

# Use vulkan headers from glfw:
include_directories(${GLFW_DIR}/deps)

add_executable(${PROJECT_NAME} ${sources} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PUBLIC -DImTextureID=ImU64)
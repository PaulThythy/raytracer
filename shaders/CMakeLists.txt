find_package(Vulkan REQUIRED)

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SHADERS_OUTPUT_DIR ${SHADERS_DIR}/build)

file(GLOB_RECURSE GLSL_FILES "*.glsl")

foreach(GLSL ${GLSL_FILES})
    get_filename_component(FILENAME ${GLSL} NAME_WE)
    set(SPIRV "${SHADERS_OUTPUT_DIR}/${FILENAME}.spv")
    message(STATUS ${GLSL})

    if(${FILENAME} MATCHES frag$)

        message(STATUS "Building fragment shader " ${FILENAME})
        add_custom_command(OUTPUT ${SPIRV}
			COMMAND ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=fragment ${GLSL} -o ${SPIRV}
			DEPENDS ${GLSL}
		)

    elseif(${FILENAME} MATCHES vert$)
		message(STATUS "Building vertex shader " ${FILENAME})
		add_custom_command(OUTPUT ${SPIRV}
			COMMAND ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=vertex ${GLSL} -o ${SPIRV}
			DEPENDS ${GLSL}
		)
    
    endif()

    list(APPEND SPIRV_BINARY_FILES ${SPIRV})

endforeach(GLSL)


add_custom_target(
    shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(raytracer shaders)
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ComputerGraphics CXX)

# Slightly modified versions of the files from the cpp-starter-project by Jason Turner:
# https://github.com/lefticus/cpp_starter_project/
include("cmake/CompilerWarnings.cmake") # Enable almost all compiler warnings and CMake option to enable -Werror.
include("cmake/Sanitizers.cmake") # CMake options to enable address, memory, UB and thread sanitizers.
include("cmake/StaticAnalyzers.cmake") # CMake options to enable clang-tidy or cpp-check.

# Use pmm to download and install (to temp directory) the vcpkg package manager.
# Then invoke vcpkg (through pmm) to install (download & compile) freeglut.
include("cmake/pmm.cmake")
pmm(VERBOSE VCPKG
	REVISION acff7d4aa6e18355585ba52c1eba789c02ed3fef
	REQUIRES glfw3 glew glm ms-gsl fmt stb assimp)

set(OpenGL_GL_PREFERENCE GLVND) # Prevent CMake warning about legacy fallback on Linux.
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
# NOTE: stb does not support find_package. Just gotta hope that CMake or the compiler picks up the header file.
find_package(assimp CONFIG REQUIRED)

set(PLATFORM_SPECIFIC_PACKAGES "")
if (UNIX)
	# Some older versions of libstdc++ (like g++8) require explicit linking against std::filesystem.
	# The custom Findfilesystem.cmake file located in the cmake folder was copied from:
	# https://github.com/inviwo/inviwo/blob/master/cmake/modules/FindFilesystem.cmake
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
	find_package(filesystem REQUIRED)

	# Issue with assimps CMake files (assimps fault, not vcpkg) not linking to all the required libraries.
	# Seems to only affect Linux users...
	find_library(ASSIMP_ZLIB_LIBRARY z)
	find_library(ASSIMP_IRRXML_LIBRARY IrrXML)

	set(PLATFORM_SPECIFIC_PACKAGES std::filesystem ${ASSIMP_ZLIB_LIBRARY} ${ASSIMP_IRRXML_LIBRARY})
endif()

add_executable(FinalProject
    "src/application.cpp"
    "src/mesh.cpp"
    "src/texture.cpp"
    "src/shader.cpp"
    "src/texture.cpp"
    "src/window.cpp"
    "src/stb_image.cpp"
    "src/tiny_obj.cpp"
    "src/camera.cpp"
)
target_compile_features(FinalProject PRIVATE cxx_std_17)
target_link_libraries(FinalProject PRIVATE
	OpenGL::GL GLEW::GLEW
	glfw
	glm
	Microsoft.GSL::GSL
	assimp::assimp
	fmt::fmt
	${PLATFORM_SPECIFIC_PACKAGES})
enable_sanitizers(FinalProject)
set_project_warnings(FinalProject)

# Copy all files in the resources folder to the build directory after every successful build.
add_custom_command(TARGET FinalProject POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_LIST_DIR}/resources/" "$<TARGET_FILE_DIR:FinalProject>/resources/")

# We would like to copy the files when they changed. Even if no *.cpp files were modified (and
# thus no build is triggered). We tell CMake that the executable depends on the shader files in
# the build directory. We also tell it how to generate those files (by copying them from the
# shaders folder in this directory). The gather all glsl files in the shaders folder when CMake
# is configured. So if you were to add a shader file then you need to configure CMake again.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

set(shader_copies "")
file(GLOB shader_sources "${CMAKE_CURRENT_LIST_DIR}/shaders/*")
foreach (shader_file IN LISTS shader_sources)
	get_filename_component(file_name ${shader_file} NAME)
	message("shader_file: ${file_name}")
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/shaders/${file_name}"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_LIST_DIR}/shaders/${file_name}"
		"${CMAKE_BINARY_DIR}/shaders/${file_name}"
		DEPENDS "${CMAKE_CURRENT_LIST_DIR}/shaders/${file_name}"
		)
	LIST(APPEND shader_copies "${CMAKE_BINARY_DIR}/shaders/${file_name}")
endforeach()
add_custom_target(copy_shaders DEPENDS ${shader_copies})
add_dependencies(FinalProject copy_shaders)


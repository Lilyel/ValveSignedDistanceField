# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)


project (ValveSignedDistanceField)


# Show warnings.
if(MSVC)
	add_compile_options(/W3 /WX)
else(MSVC)
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif(MSVC)


# Look for SFML.
find_package(SFML 2.5 COMPONENTS graphics system window REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

# Add ImGUI (and ImGUI-SFML) to the project.
include_directories(${CMAKE_SOURCE_DIR}/ImGUI)
file(GLOB_RECURSE IMGUI_SRC_FILES ${CMAKE_SOURCE_DIR}/ImGUI/*.cpp)

# Look for OpenGL.
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})


# Retrieve files of the project.
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/Sources/*.cpp)


# Add sources to this project's executable.
add_executable(ValveSignedDistanceField ${SRC_FILES} ${IMGUI_SRC_FILES})


# Link dependencies libs.
#set_target_properties(target_link_libraries(SFMLTest sfml-graphics sfml-audio) PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(ValveSignedDistanceField sfml-graphics sfml-system sfml-window ${OPENGL_LIBRARIES})

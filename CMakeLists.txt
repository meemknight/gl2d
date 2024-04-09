#https://github.com/meemknight/gl2d

cmake_minimum_required(VERSION 3.16)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

project(gl2dDemo)
project(gl2dPostProcessDemo)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw-3.3.2)		#window oppener
add_subdirectory(thirdparty/glad)			#opengl loader
add_subdirectory(thirdparty/stb_image)		#loading immaged
add_subdirectory(thirdparty/stb_truetype)	#loading ttf files
add_subdirectory(thirdparty/glm)			#math

add_subdirectory(gl2d)						#2D rendering library


add_executable(gl2dDemo)
set_property(TARGET gl2dDemo PROPERTY CXX_STANDARD 17)
target_compile_definitions(gl2dDemo PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions(gl2dDemo PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the game
#add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

target_sources(gl2dDemo PRIVATE "src/main.cpp" )
if(MSVC) # If using the VS compiler...
	target_compile_definitions(gl2dDemo PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
target_link_libraries(gl2dDemo PRIVATE glm glfw 
	glad stb_image stb_truetype gl2d)


	
add_executable(gl2dPostProcessDemo)
set_property(TARGET gl2dPostProcessDemo PROPERTY CXX_STANDARD 17)
target_compile_definitions(gl2dPostProcessDemo PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions(gl2dPostProcessDemo PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the game
#add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

target_sources(gl2dPostProcessDemo PRIVATE "src/mainPostProcess.cpp" )
if(MSVC) # If using the VS compiler...
	target_compile_definitions(gl2dPostProcessDemo PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
target_link_libraries(gl2dPostProcessDemo PRIVATE glm glfw 
	glad stb_image stb_truetype gl2d)


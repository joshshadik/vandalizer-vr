cmake_minimum_required (VERSION 3.6)
project (vandalizer-vr)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake/modules ${CMAKE_MODULE_PATH})

set(ROOT_PATH "${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("${ROOT_PATH}/jkps-gl/" "${CMAKE_CURRENT_BINARY_DIR}/jkps-gl/")

option(WASM_BUILD OFF)

file(GLOB SRC_FILES
    "${ROOT_PATH}/src/*.cpp"
    
)

file(GLOB HEADER_FILES
   "${ROOT_PATH}/src/*.h"
)

add_executable(vandalizer-vr
    ${SRC_FILES}
    ${HEADER_FILES}
)

source_group(TREE "${ROOT_PATH}/src/" PREFIX "source" FILES ${SRC_FILES})
source_group(TREE "${ROOT_PATH}/src/" PREFIX "headers" FILES ${HEADER_FILES})

target_include_directories(vandalizer-vr PUBLIC 
"${ROOT_PATH}/jkps-gl/include/"
)

if( WASM_BUILD )
    target_link_libraries(vandalizer-vr
        jkps-gl
    )

    set(CMAKE_CXX_FLAGS "--preload-file resources/ -O0 -s USE_WEBGL2=1 -s USE_GLFW=3  -s ALLOW_MEMORY_GROWTH=1 -s WASM=1  -std=c++11 ${CMAKE_CXX_FLAGS} ")
    add_definitions(-Wall -g -std=c++11)
    target_compile_definitions(vandalizer-vr PUBLIC USE_WASM)

    file(COPY "${ROOT_PATH}/wasm/" DESTINATION "${CMAKE_BINARY_DIR}/")
else()

    find_package(glew REQUIRED)
    find_package(glfw3 REQUIRED)

    target_link_libraries(vandalizer-vr
        jkps-gl
        glfw
    )

endif()

file(COPY "${ROOT_PATH}/resources/" DESTINATION "${CMAKE_BINARY_DIR}/resources/")
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vandalizer-vr)

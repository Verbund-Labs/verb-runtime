cmake_minimum_required(VERSION 3.16)
project(verb-runtime VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include)

set(RUNTIME_SOURCES
    src/value.cpp
    src/verbrt.cpp
)

add_library(verbrt_static STATIC ${RUNTIME_SOURCES})
set_target_properties(verbrt_static PROPERTIES OUTPUT_NAME "verbrt")

add_library(verbrt_shared SHARED ${RUNTIME_SOURCES})
set_target_properties(verbrt_shared PROPERTIES OUTPUT_NAME "verbrt")

target_link_libraries(verbrt_static PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(verbrt_shared PUBLIC ${CMAKE_DL_LIBS})

# Test binary
add_executable(test_runtime tests/test_runtime.cpp)
target_link_libraries(test_runtime PRIVATE verbrt_static)

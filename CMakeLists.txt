cmake_minimum_required (VERSION 3.10)

project (async-sync-fsm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

file(GLOB BASE_SRC CONFIGURE_DEPENDS
    "src/*.hpp"
    "src/*.cpp"
)

# file(GLOB FSM_SRC CONFIGURE_DEPENDS
#     "src/fsm/*.hpp"
#     "src/fsm/*.cpp"
# )
# add_library(FSM ${FSM_SRC})

add_executable(${PROJECT_NAME} ${BASE_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE src/fsm)
# target_link_libraries(${PROJECT_NAME} PRIVATE FSM)

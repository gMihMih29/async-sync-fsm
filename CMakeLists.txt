cmake_minimum_required (VERSION 3.10)

project (async-sync-fsm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# file(GLOB ARTICLE_FSM_SRC CONFIGURE_DEPENDS
#     "src/article-fsm.cpp"
# )

add_executable(article-fsm "src/article-fsm.cpp")
target_include_directories(article-fsm PRIVATE src/fsm)

add_executable(reachability-graph "src/reachability-graph.cpp")
target_include_directories(reachability-graph PRIVATE src/fsm)

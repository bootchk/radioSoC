
# test of cmake, another cmake proj using this library

add_library(clock INTERFACE)

target_include_directories(clock INTERFACE "${CMAKE_CURRENT_LIST_DIR}" )




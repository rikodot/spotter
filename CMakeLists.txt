cmake_minimum_required (VERSION 3.6)
project(Spotter)

add_executable(Spotter main.cpp "utils/database.h" "utils/personal.h" "utils/utils.h" "utils/commands.h" "utils/api.h" "utils/spotter.h" "utils/api.cpp" "utils/check.cpp" "utils/commands.cpp" "utils/database.cpp" "utils/spotter.cpp" "utils/utils.cpp" "utils/personal.cpp" "config.h")

add_subdirectory(sleepy-discord)
target_link_libraries(Spotter sleepy-discord)

set_property(TARGET Spotter PROPERTY CXX_STANDARD 20)
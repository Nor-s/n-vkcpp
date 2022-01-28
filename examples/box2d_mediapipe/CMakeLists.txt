find_package(box2d REQUIRED FATAL_ERROR)

add_subdirectory(box)
add_subdirectory(hand)
add_subdirectory(class)

add_executable(hand-window ./main.cpp)

target_compile_options(hand-window PRIVATE -Wall -Werror -O3 -std=c++17)

# include
target_include_directories(hand-window PUBLIC ./ ../)
target_link_libraries(hand-window handbox::app)

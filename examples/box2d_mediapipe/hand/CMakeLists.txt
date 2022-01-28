add_library(hand STATIC ./hand_input.h ./hand_input.cpp ./hand_object.h
                        ./hand_object.cpp)

add_library(vkcpp::hand ALIAS hand)

target_compile_options(vkcpp PRIVATE -Wall -Werror -O3 -std=c++17)

# include
target_include_directories(hand PUBLIC ./)
target_link_libraries(hand PRIVATE vkcpp::vkcpp vkcpp::box)

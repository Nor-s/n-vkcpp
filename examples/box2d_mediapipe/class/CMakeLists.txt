add_library(handbox_app STATIC ./application.h ./application.cpp)

add_library(handbox::app ALIAS handbox_app)

target_compile_options(vkcpp PRIVATE -Wall -Werror -O3 -std=c++17)

# include
target_include_directories(handbox_app PUBLIC ./)
target_link_libraries(handbox_app vkcpp::vkcpp vkcpp::hand vkcpp::box)

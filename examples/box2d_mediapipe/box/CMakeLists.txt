add_library(box_object STATIC ./box_object.h ./box_object.cpp box_manager.h
                              box_manager.cpp)

add_library(vkcpp::box ALIAS box_object)

target_compile_options(vkcpp PRIVATE -Wall -Werror -O3 -std=c++17)

# include
target_include_directories(box_object PUBLIC ./)
target_link_libraries(box_object box2d::box2d vkcpp::vkcpp)

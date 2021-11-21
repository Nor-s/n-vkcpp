add_executable(test_3DObject main.cpp)
target_compile_options(test_3DObject PUBLIC -Wall -O3 -std=c++17)
target_include_directories(test_3DObject PUBLIC ./)
target_link_libraries(test_3DObject vkcpp::vkcpp)

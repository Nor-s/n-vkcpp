set(PAINTING_SRC_FILES main.cpp ./class/application.cpp ./class/brush.cpp
                       ./class/picture.cpp ./class/population.cpp)

add_executable(painting ${PAINTING_SRC_FILES})
# 컴파일 옵션 설정
target_compile_options(painting PUBLIC -Wall -O3 -std=c++17)

if(APPLE)
  # include
  target_include_directories(painting PUBLIC ./)

  target_link_libraries(painting vkcpp::vkcpp)

endif(APPLE)

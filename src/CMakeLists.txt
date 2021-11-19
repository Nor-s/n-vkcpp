include(FetchContent)

find_package(Vulkan REQUIRED FATAL_ERROR)

find_package(glfw3 REQUIRED FATAL_ERROR)

find_package(glm REQUIRED FATAL_ERROR)
#[[
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 0.9.9.9)

FetchContent_MakeAvailable(glm)
#]]

add_subdirectory(vkcpp)

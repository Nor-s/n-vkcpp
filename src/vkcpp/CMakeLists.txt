set(VKCPP_SRC_FILES
    ./std_header.h
    ./vulkan_header.h
    ./object/shader_attribute.hpp
    # vkcpp window
    ./window/window.hpp
    ./window/main_window.h
    ./window/main_window.cpp
    # vkcpp device
    ./device/device.h
    ./device/device.cpp
    ./device/instance.h
    ./device/instance.cpp
    ./device/physical_device.h
    ./device/physical_device.cpp
    ./device/queue.h
    ./device/queue.cpp
    ./device/surface.h
    ./device/surface.cpp
    # vkcpp object/camera
    ./object/camera/camera.h
    ./object/camera/camera.cpp
    ./object/camera/main_camera.h
    ./object/camera/main_camera.cpp
    ./object/camera/sub_camera.h
    ./object/camera/sub_camera.cpp
    # vkcpp object
    ./object/model.h
    ./object/model.cpp
    ./object/object2d.h
    ./object/object2d.cpp
    # vkcpp pattern
    ./pattern/singleton.hpp
    # vkcpp buffer
    ./render/buffer/buffer.hpp
    ./render/buffer/buffer.tpp
    ./render/buffer/uniform_buffers.hpp
    ./render/buffer/uniform_buffers.tpp
    ./render/buffer/descriptor_sets.h
    ./render/buffer/descriptor_sets.cpp
    # render
    ./render/command/command_buffers.h
    ./render/command/command_buffers.cpp
    ./render/command/command_pool.h
    ./render/command/command_pool.cpp
    # vkcpp image
    ./render/image/image_depth.h
    ./render/image/image_depth.cpp
    ./render/image/image.h
    ./render/image/image.cpp
    ./render/image/image2d.h
    ./render/image/image2d.cpp
    ./render/image/offscreen.h
    ./render/image/offscreen.cpp
    # vkcpp pipeline
    ./render/pipeline/pipeline.hpp
    ./render/pipeline/graphics_pipeline.h
    ./render/pipeline/graphics_pipeline.cpp
    # vkcpp swapchain
    ./render/swapchain/framebuffers.h
    ./render/swapchain/framebuffers.cpp
    ./render/swapchain/offscreens.h
    ./render/swapchain/offscreens.cpp
    ./render/swapchain/render_pass.h
    ./render/swapchain/render_pass.cpp
    ./render/swapchain/swapchain.h
    ./render/swapchain/swapchain.cpp
    ./render/render_stage.h
    ./render/render_stage.cpp
    # vkcpp utility
    ./utility/create.h
    ./utility/create.cpp
    ./utility/utility.h
    ./utility/utility.cpp)
add_library(vkcpp STATIC ${VKCPP_SRC_FILES})

add_library(vkcpp::vkcpp ALIAS vkcpp)
# 컴파일 옵션 설정
target_compile_options(vkcpp PRIVATE -Wall -O3 -std=c++17)

if(WIN32)
  # include
  target_include_directories(vkcpp PUBLIC ./../../external ./../vkcpp
                                          C:/VulkanSDK/1.2.189.2/Include)
  # lib
  target_link_directories(vkcpp PUBLIC ./lib C:/VulkanSDK/1.2.189.2/Lib)
  # l
  target_link_libraries(vkcpp -lpthread -lglfw3 -lvulkan-1 -lgdi32 -static)
endif(WIN32)

if(APPLE)
  # include
  target_include_directories(vkcpp PUBLIC ./ ./../../external)
  target_link_libraries(vkcpp PUBLIC glfw glm::glm Vulkan::Vulkan -ldl
                                     -lpthread)

endif(APPLE)

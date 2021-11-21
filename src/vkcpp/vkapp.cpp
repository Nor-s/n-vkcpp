#include "vkapp.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device/surface.h"
#include "device/instance.h"
#include "device/device.h"
#include "device/queue.h"
#include "render/swapchain/swapchain.h"
#include "render/render_stage.h"
#include "render/command/command_pool.h"
#include "render/command/command_buffers.h"
#include "object/object2d.h"
#include "window/main_window.h"
#include "object/camera/main_camera.h"

namespace vkcpp
{
	Vkapp::~Vkapp() {}
	void Vkapp::run(uint32_t width, uint32_t height, std::string title)
	{
		init_window(width, height, title);
		init_device();
		command_pool_ = std::make_unique<vkcpp::CommandPool>(device_.get(), device_->get_graphics_queue(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		init_render();
		init_synobj();
		main_loop();
		cleanup();
	}
	void Vkapp::init_window(uint32_t width, uint32_t height, std::string title)
	{

		vkcpp::MainWindow::getInstance()->set_window(width, height, title);
		vkcpp::MainWindow::getInstance()->set_user_pointer(this);
		vkcpp::MainWindow::getInstance()->set_framebuffer_size_callback(framebufferResizeCallback);
		vkcpp::MainWindow::getInstance()->set_scroll_callback(scrollCallback);

		instance_ = std::make_unique<vkcpp::Instance>();
		surface_ = std::make_unique<vkcpp::Surface>(instance_.get(), vkcpp::MainWindow::getInstance()->create_surface(*instance_));
	}

	void Vkapp::init_render()
	{
		swapchain_ = std::make_unique<vkcpp::Swapchain>(device_.get(), surface_.get());
		render_stage_ = std::make_unique<vkcpp::RenderStage>(device_.get(), swapchain_.get());
		command_buffers_ = std::make_unique<vkcpp::CommandBuffers>(device_.get(), command_pool_.get(), swapchain_->get_image_views().size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		is_command_buffer_updated_.resize(swapchain_->get_image_views().size(), false);
	}

	void Vkapp::init_synobj()
	{
		image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		render_finished_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);
		images_in_flight_.resize(swapchain_->get_image_views().size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(*device_, &semaphore_info, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
				vkCreateSemaphore(*device_, &semaphore_info, nullptr, &render_finished_semaphores_[i]) != VK_SUCCESS ||
				vkCreateFence(*device_, &fence_info, nullptr, &in_flight_fences_[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void Vkapp::init_device()
	{
		instance_->query_gpus(surface_.get());
		vkcpp::PhysicalDevice* gpu = instance_->get_suitable_gpu(device_extensions_);
		device_ = std::make_unique<vkcpp::Device>(gpu);
	}

	void Vkapp::record_command_buffers()
	{
		uint32_t size = command_buffers_->size();
		for (uint32_t i = 0; i < size; i++)
		{
			record_command_buffer(i);
		}
	}

	void Vkapp::record_command_buffer(int idx)
	{
		command_buffers_->begin_command_buffer(idx, 0);

		command_buffers_->begin_render_pass(idx, render_stage_.get());

		command_buffers_->end_render_pass(idx, render_stage_.get());

		command_buffers_->end_command_buffer(idx);
		is_command_buffer_updated_[idx] = true;
	}

	void Vkapp::draw_frame()
	{
		vkWaitForFences(*device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

		uint32_t image_index;
		VkResult result = vkAcquireNextImageKHR(*device_, *swapchain_, UINT64_MAX, image_available_semaphores_[current_frame_], VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		// if object update -> re record command buffer
		if (!is_command_buffer_updated_[image_index])
		{
			record_command_buffer(image_index);
		}

		if (images_in_flight_[image_index] != VK_NULL_HANDLE)
		{
			vkWaitForFences(*device_, 1, &images_in_flight_[image_index], VK_TRUE, UINT64_MAX);
		}
		images_in_flight_[image_index] = in_flight_fences_[current_frame_];

		// Prepare submit command buffer to graphics queue
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { image_available_semaphores_[current_frame_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(*command_buffers_)[image_index];

		VkSemaphore signalSemaphores[] = { render_finished_semaphores_[current_frame_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Submit to the graphics queue
		vkResetFences(*device_, 1, &in_flight_fences_[current_frame_]);
		device_->graphics_queue_submit(&submitInfo, 1, in_flight_fences_[current_frame_], "failed submit draw cmd buffer in application class!");

		// Prepare present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { *swapchain_ };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &image_index;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(*(device_->get_present_queue()), &presentInfo);

		// Detect : resize window
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized_)
		{
			framebuffer_resized_ = false;
			recreate_swapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Vkapp::main_loop()
	{
#ifndef NDEBUG
			auto current_time = std::chrono::high_resolution_clock::now();
#endif
			while (!vkcpp::MainWindow::getInstance()->should_close())
			{
				vkcpp::MainWindow::getInstance()->process_events();
#ifndef NDEBUG
				auto new_time = std::chrono::high_resolution_clock::now();
				float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
				std::cout << "time: " << frame_time << "\n";
				current_time = new_time;
#endif

				draw_frame();
			}
	}

	void Vkapp::cleanup()
	{
		vkDeviceWaitIdle(*device_);

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(*device_, render_finished_semaphores_[i], nullptr);
			vkDestroySemaphore(*device_, image_available_semaphores_[i], nullptr);
			vkDestroyFence(*device_, in_flight_fences_[i], nullptr);
		}

		cleanup_swapchain();

		command_pool_.reset();
		device_.reset();
		surface_.reset();
		instance_.reset();

		vkcpp::MainWindow::getInstance()->destroy_window();
	}

	void Vkapp::cleanup_swapchain()
	{
		command_buffers_.reset();
		render_stage_.reset();
		swapchain_.reset();
	}

	void Vkapp::recreate_swapchain()
	{
		auto size = vkcpp::MainWindow::getInstance()->get_framebuffer_size();

		while (size.first == 0 || size.second == 0)
		{
			size = vkcpp::MainWindow::getInstance()->get_framebuffer_size();
			vkcpp::MainWindow::getInstance()->wait_events();
		}

		vkDeviceWaitIdle(*device_);

		cleanup_swapchain();
		init_render();
		// TODO check renderpass compatibility
		// If the number of new swapchain images is the same as the old image,
		// there is no need to change the number of buffers.(commandbuffer and etc.)
		record_command_buffers();

		images_in_flight_.resize(swapchain_->get_image_views().size(), VK_NULL_HANDLE);
		vkcpp::MainCamera::getInstance()->update();
	}

	void Vkapp::reset_command_buffers_update_flag()
	{
		is_command_buffer_updated_ = std::vector<bool>(is_command_buffer_updated_.size(), false);
	}

} // namespace vkcpp

/**
 * call back
 */
namespace vkcpp
{
	void Vkapp::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (yoffset > 0)
		{
			// scrolled up
			vkcpp::MainCamera::getInstance()->forward_look_at(-2.0f);
		}
		else if (yoffset < 0)
		{
			vkcpp::MainCamera::getInstance()->forward_look_at(2.0f);
		}
	}
	void Vkapp::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Vkapp*>(glfwGetWindowUserPointer(window));
		app->framebuffer_resized_ = true;
	}
} // namespace vkcpp

namespace vkcpp
{

} // namespace vkcpp
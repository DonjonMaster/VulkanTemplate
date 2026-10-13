#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>

#include "Application.h"
#include "Renderer.h"

class Surface
{
public:
    vk::raii::SurfaceKHR& get_surface();
    void create_surface();
    void create_swap_chain();
    void create_image_views();
    
private:
    // Forward declaration
    Renderer renderer_class_;
    Application application_class_;
    
    vk::raii::SurfaceKHR surface_ = nullptr;
    
    // Swap chain
    vk::raii::SwapchainKHR swap_chain_ = nullptr;
    std::vector<vk::Image> swap_chain_images_;
    vk::SurfaceFormatKHR swap_chain_surface_format;
    vk::Extent2D swap_chain_extent_;
    
    // Image views
    std::vector<vk::raii::ImageView> swap_chain_image_views_;
    
    vk::SurfaceFormatKHR choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const &available_formats);
    vk::PresentModeKHR choose_swap_present_mode(std::vector<vk::PresentModeKHR> const & available_present_modes);
    vk::Extent2D choose_swap_extent(vk::SurfaceCapabilitiesKHR const &capabilities);
    
    uint32_t choose_swap_min_image_count(vk::SurfaceCapabilitiesKHR const &capabilities);
    ;
};

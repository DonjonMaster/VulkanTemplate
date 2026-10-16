#pragma once

#include "Dependencies.h"

// Forward declaration
class Application;

class Surface
{
public:
    Surface(Application*);
    ~Surface();
    
    vk::raii::SurfaceKHR& get_surface();
    vk::Extent2D& get_swap_chain_extent();
    vk::SurfaceFormatKHR& get_swap_chain_surface_format();
    std::vector<vk::Image>& get_swap_chain_images_();
    std::vector<vk::raii::ImageView>& get_swap_chain_image_views();
    
    void create_surface();
    void create_swap_chain();
    void create_image_views();
    
private:
    // Forward declaration
    Application* application_ = nullptr;
    
    
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
    
};

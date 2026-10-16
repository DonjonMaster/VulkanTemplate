#include "Surface.h"

#include <utility>
#include <algorithm>

#include "Application.h"
#include "Renderer.h"

vk::raii::SurfaceKHR& Surface::get_surface()
{
    return surface_;
}

vk::Extent2D& Surface::get_swap_chain_extent()
{
    return swap_chain_extent_;
}

vk::SurfaceFormatKHR& Surface::get_swap_chain_surface_format()
{
    return swap_chain_surface_format;
}

void Surface::create_surface()
{
    VkSurfaceKHR _surface;
    vk::raii::Instance& instance = renderer_class_->get_instance();
    GLFWwindow* window = application_class_->window;
    
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    surface_ = vk::raii::SurfaceKHR(instance, _surface);
}

void Surface::create_swap_chain()
{
    vk::SurfaceCapabilitiesKHR surface_capabilities = renderer_class_->get_physical_device().getSurfaceCapabilitiesKHR(*surface_);
    swap_chain_extent_ = choose_swap_extent(surface_capabilities);
    uint32_t min_image_count = choose_swap_min_image_count(surface_capabilities);
    
    std::vector<vk::SurfaceFormatKHR> available_formats = renderer_class_->get_physical_device().getSurfaceFormatsKHR(*surface_);
    swap_chain_surface_format = choose_swap_surface_format(available_formats);
    
    // Define swap chain info
    vk::SwapchainCreateInfoKHR swap_chain_create_info_{
        .surface = *surface_,
        .minImageCount = min_image_count,
        .imageFormat = swap_chain_surface_format.format,
        .imageColorSpace = swap_chain_surface_format.colorSpace,
        .imageExtent = swap_chain_extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = choose_swap_present_mode(renderer_class_->get_physical_device().getSurfacePresentModesKHR(*surface_)),
        .clipped = true
    };
    
    
}

void Surface::create_image_views()
{
    // At creation, the image swap chain must be empty
    assert(swap_chain_image_views_.empty());
    
    // Define image information
    vk::ImageViewCreateInfo image_view_create_info{
        .viewType = vk::ImageViewType::e2D,
        .format = swap_chain_surface_format,
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
    };
    
    // Map the color channels
    image_view_create_info.components = {
    vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, 
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
    
    // Define the image's purpose and which part should be accessed
    image_view_create_info.subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor, 
        .levelCount = 1u, 
        .layerCount = 1u
    };
    
    // Add the image to the image swap chain
    for (auto &image : swap_chain_images_)
    {
        image_view_create_info.image = image;
        vk::raii::Device& device = renderer_class_->get_device();
        swap_chain_image_views_.emplace_back(device, image_view_create_info);
    }
}


vk::SurfaceFormatKHR Surface::choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const& available_formats)
{
    // abort if empty list
    assert(!available_formats.empty());
    
    // Search for the preferred combination
    const auto format_it = std::ranges::find_if(
        available_formats,
        [](const auto& format)
        {
            return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });
    
    // If at the end of the list, return the first item of the list
    return format_it != available_formats.end() ? *format_it : available_formats[0];
}

vk::PresentModeKHR Surface::choose_swap_present_mode(std::vector<vk::PresentModeKHR> const& available_present_modes)
{
    auto is_present_mode_available = [](auto present_mode)
    {
        return present_mode == vk::PresentModeKHR::eFifo; 
    };
    
    auto is_better_option_available = [](const vk::PresentModeKHR value)
    {
        return vk::PresentModeKHR::eMailbox == value;
    };
    
    // Check if eFifo is available, if not, abort
    assert(std::ranges::any_of(available_present_modes, is_present_mode_available));
    
    // Check if eMailbox is available, if is, change to eMailBox, else, change to eFifo
    return std::ranges::any_of(available_present_modes, is_better_option_available) ? 
    vk::PresentModeKHR::eMailbox :
    vk::PresentModeKHR::eFifo;
}

vk::Extent2D Surface::choose_swap_extent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
    // Check if extent is defined manually
    if (std::cmp_not_equal(capabilities.currentExtent.width, ~0))
    {
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(application_class_->window, &width, &height);
    // Set size to actual window size
    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

uint32_t Surface::choose_swap_min_image_count(vk::SurfaceCapabilitiesKHR const& surface_capabilities)
{
    auto min_image_count = std::max<auto>(3u, surface_capabilities.minImageCount);
    if ((0 < surface_capabilities.maxImageCount) && (surface_capabilities.maxImageCount < min_image_count))
    {
        min_image_count = surface_capabilities.maxImageCount;
    }
    return min_image_count;
}
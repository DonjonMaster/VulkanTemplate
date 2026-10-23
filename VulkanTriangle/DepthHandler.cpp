#include "DepthHandler.h"

#include "Application.h"

DepthHandler::DepthHandler(Application* application) : application_(application)
{
}

void DepthHandler::create_depth_ressources()
{
    depth_format = find_depth_format();
    
    std::tie(depth_image_, depth_image_memory_) = application_->texture_mapping.create_image(
        application_->surface.get_swap_chain_extent().width, 
        application_->surface.get_swap_chain_extent().height,
        depth_format, 
        vk::ImageTiling::eOptimal, 
        vk::ImageUsageFlagBits::eDepthStencilAttachment, 
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    
    depth_image_view_ = application_->texture_mapping.create_image_view(
        depth_image_, 
        depth_format, 
        vk::ImageAspectFlagBits::eDepth
        );
    
}

vk::raii::ImageView& DepthHandler::get_depth_image_view()
{
    return depth_image_view_;
}

vk::raii::Image& DepthHandler::get_depth_image()
{
    return depth_image_;
}

vk::Format& DepthHandler::get_depth_format()
{
    return depth_format;
}

vk::Format DepthHandler::find_supported_format_(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                                vk::FormatFeatureFlags features)
{
    for (const auto format : candidates)
    {
        vk::FormatProperties props = application_->renderer.get_physical_device().getFormatProperties(format);
        if (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures& features) == features)) ||
            ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures& features) == features)))
            {
            return format;
            }
    }
    
    throw std::runtime_error("failed to find supported format!");    
}

vk::Format DepthHandler::find_depth_format()
{
    return find_supported_format_(
{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

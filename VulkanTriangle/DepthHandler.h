#pragma once

#include "Dependencies.h"

class Application;

class DepthHandler
{
public:
    DepthHandler(Application*);
    
    void create_depth_ressources();
    
    vk::raii::ImageView& get_depth_image_view();
    vk::raii::Image& get_depth_image();
    vk::Format& get_depth_format();
    vk::Format find_depth_format();
    
private:
    
    vk::Format find_supported_format_(
        const std::vector<vk::Format>& candidates,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features
        );
    
    
    
    Application* application_;
    
    vk::raii::Image depth_image_ = nullptr;
    vk::raii::DeviceMemory depth_image_memory_ = nullptr;
    vk::raii::ImageView depth_image_view_ = nullptr;
    vk::Format depth_format;
    
};

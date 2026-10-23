#pragma once

#include "Dependencies.h"

class Application;

class TextureMapping
{
public:
    TextureMapping(Application*);
    
    void create_texture_image();
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> create_image(
        uint32_t width, uint32_t height, vk::Format format, 
        vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
    
    
    vk::raii::ImageView create_image_view(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspect_flags);
    void create_texture_image_view();
    
    void create_texture_sampler();
    
    vk::raii::ImageView& get_texture_image_view();
    vk::raii::Sampler& get_texture_sampler();
    
private:
    
    vk::raii::Image texture_image = nullptr;
    vk::raii::DeviceMemory texture_image_memory = nullptr;
    
    vk::raii::ImageView texture_image_view_ = nullptr;
    vk::raii::Sampler texture_sampler = nullptr;

    Application* application_;
};

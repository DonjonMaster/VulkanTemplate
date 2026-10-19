#pragma once

#include "Dependencies.h"

class Application;

class Buffers
{
public:
    Buffers(Application*);
    ~Buffers();
    
    void create_command_pool();
    void create_command_buffer();
    
    vk::raii::CommandBuffer& get_command_buffer();    
    
private:
    // VARIABLES
    
    // CommandPools manage the memory that is used to store buffers
    vk::raii::CommandPool command_pool_ = nullptr;
    // CommandBuffer are automatically freed when their command pool is destroyed
    vk::raii::CommandBuffer command_buffer_ = nullptr;
    
    // METHODS
    
    void record_command_buffer(uint32_t image_index);
    // Transform an image layout to become suitable for rendering on a flat screen
    void transition_image_layout(
        uint32_t image_index,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask
    );
    
    Application* application_;
};

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "Dependencies.h"

class Application;

class Buffers
{
public:
    Buffers(Application*);
    ~Buffers();
    
    void create_command_pool();
    void create_command_buffer();
    void create_vertex_buffer();
    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    void create_index_buffer();
    void create_uniform_buffers();
    
    void update_uniform_buffers(uint32_t current_image);
    
    
    void copy_buffer(vk::raii::Buffer & src_buffer, vk::raii::Buffer & dst_buffer, vk::DeviceSize size);
    
    void record_command_buffer(uint32_t image_index);
    
    std::vector<vk::raii::CommandBuffer>& get_command_buffer();

    static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
    
    vk::raii::CommandBuffer begin_single_time_commands();
    void end_single_time_commands(vk::raii::CommandBuffer&& command_buffer);
    
    void transition_image_layout (vk::raii::CommandBuffer& command_buffer, const vk::raii::Image &image, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
    
    void copy_buffer_to_image(vk::raii::CommandBuffer& command_buffer, const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);
    
    
    bool frame_buffer_resized = false;
    
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    
    
    std::vector<vk::raii::Buffer>& get_uniform_buffers();

private:
    // VARIABLES
    
    // CommandPools manage the memory that is used to store buffers
    vk::raii::CommandPool command_pool_ = nullptr;
    // CommandBuffer are automatically freed when their command pool is destroyed
    std::vector<vk::raii::CommandBuffer> command_buffer_;
    // Buffer for vertices
    vk::raii::Buffer vertex_buffer_ = nullptr;
    vk::raii::DeviceMemory vertex_buffer_memory = nullptr;
    // Buffer for index
    vk::raii::Buffer index_buffer_ = nullptr;
    vk::raii::DeviceMemory index_buffer_memory = nullptr;
    // Uniform Buffers
    vk::raii::Buffer index_buffer = nullptr;
    vk::raii::DeviceMemory index_buffer_Memory = nullptr;
    
    std::vector<vk::raii::Buffer> uniform_buffers_;
    std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
    std::vector<void *> uniform_buffers_mapped;
    
    // METHODS
    
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

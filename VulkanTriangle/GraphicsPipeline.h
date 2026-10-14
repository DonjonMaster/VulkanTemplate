#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <vulkan/vulkan_raii.hpp>

#define VK_USE_PLATFORM_WIN32_KHR

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Renderer.h"

class GraphicsPipeline
{
public:
    void create_graphics_pipeline();
    
    Renderer* renderer = nullptr;
    
private:
    static std::vector<char> read_file(const std::string& filename);
    [[nodsicard]] vk::raii::ShaderModule create_shader_module(const std::vector<char>& code) const;
    
    vk::raii::PipelineLayout pipeline_layout_ = nullptr;
    
    Surface* surface_ = nullptr;
};

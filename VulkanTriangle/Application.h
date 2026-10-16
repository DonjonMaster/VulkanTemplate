#pragma once

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GraphicsPipeline.h"
#include "Renderer.h"

class Application
{
public:
    int width = 800;
    int height = 600;
    
    
    GLFWwindow* window = nullptr;
    Renderer* renderer = nullptr;
    Surface* surface = nullptr;
    GraphicsPipeline* graphics_pipeline = nullptr;
    
    void init();
    void run();
    void destroy();
    
private:
    void init_window();
    void init_vulkan();
    
    void destroy_window();
    void destroy_vulkan();
};

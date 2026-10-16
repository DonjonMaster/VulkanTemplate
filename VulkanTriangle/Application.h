#pragma once

#include "Buffers.h"
#include "GraphicsPipeline.h"
#include "Renderer.h"
#include "Surface.h"

class Application
{
public:
    int width = 800;
    int height = 600;
    
    explicit Application();
    
    
    GLFWwindow* window;
    Renderer renderer;
    Surface surface;
    GraphicsPipeline graphics_pipeline;
    Buffers buffers;
    
    void init();
    void run();
    void destroy();
    
private:
    void init_window();
    void init_vulkan();
    
    void destroy_window();
    void destroy_vulkan();
};

#include "Application.h"

void Application::init()
{
    init_window();
    init_vulkan();
}

void Application::run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Application::destroy()
{
    destroy_vulkan();
    destroy_window();
}


void Application::init_window()
{
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(800, 600, "VulkanTriangle", nullptr, nullptr);
    
}

void Application::init_vulkan()
{
    renderer->create_instance();
    renderer->setup_debug_callback();
    surface->create_surface();
    renderer->pick_physical_device();
    renderer->create_logical_device();
    surface->create_swap_chain();
    surface->create_image_views();
    graphics_pipeline->create_graphics_pipeline();
}

void Application::destroy_window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::destroy_vulkan()
{
}



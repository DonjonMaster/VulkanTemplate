#include "Application.h"


Application::Application():renderer(this),surface(this), graphics_pipeline(this), buffers(this), drawer(this), texture_mapping(this), depth_handler(this)
{
    
}

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
        drawer.draw_frame();
    }
    
    renderer.get_device().waitIdle();
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(800, 600, "VulkanTriangle", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, Buffers::framebuffer_resize_callback);
    
}

void Application::init_vulkan()
{
    
    renderer.create_instance();
    renderer.setup_debug_callback();
    surface.create_surface();
    renderer.pick_physical_device();
    renderer.create_logical_device();
    
    surface.create_swap_chain();
    surface.create_image_views();
    
    graphics_pipeline.create_descriptor_set_layout();
    graphics_pipeline.create_graphics_pipeline();
    
    buffers.create_command_pool();
    
    depth_handler.create_depth_ressources();
    
    texture_mapping.create_texture_image();
    texture_mapping.create_texture_image_view();
    texture_mapping.create_texture_sampler();
    
    buffers.create_vertex_buffer();
    buffers.create_index_buffer();
    buffers.create_uniform_buffers();
    
    graphics_pipeline.create_descriptor_pool();
    graphics_pipeline.create_descriptor_sets();
    
    buffers.create_command_buffer();
    
    drawer.create_sync_objects();
}

void Application::destroy_window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::destroy_vulkan()
{
}



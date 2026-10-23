#pragma once

#include "Dependencies.h"

class Surface;
class Application;

class Renderer
{
public:
    Renderer(Application*);
    
    void create_instance();
    // Setup logger
    void setup_debug_callback();
    // Setup devices
    void pick_physical_device();
    void create_logical_device();
    
    uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);
    
    vk::raii::Instance& get_instance();
    vk::raii::PhysicalDevice& get_physical_device();
    vk::raii::Device& get_device();
    
    uint32_t get_final_queue_index();
    
    vk::raii::Queue get_graphics_queue();

private:
    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    // Logger
    vk::raii::DebugUtilsMessengerEXT debug_messenger_ = nullptr;
    // Devices
    vk::raii::PhysicalDevice physical_device_ = nullptr;
    vk::raii::Device device_ = nullptr;
    // Queue handles
    vk::raii::Queue graphics_queue_ = nullptr;

    // Create a chain of feature structures
    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
    feature_chain_ = {
        {.features = {.samplerAnisotropy = true}}, // vk::PhysicalDeviceFeatures2
        {.shaderDrawParameters = true}, // Enable shader draw parameters from Vulkan 1.1
        {.synchronization2 = true, .dynamicRendering = true}, // Enable dynamic rendering from Vulkan 1.3
        {.extendedDynamicState = true} // Enable extended dynamic state from the extension
    };

    // Get required extensions
    std::vector<const char*> get_required_instance_extensions();
    // Get required layers
    std::vector<const char*> get_required_layers();
    // Get required device extensions
    std::vector<const char*> get_required_device_extensions();
    
    // Get and Check layers requisites
    std::vector<const char*> get_check_layers_requisites();
    // Get and Check extensions requisites
    std::vector<const char*> get_check_extensions_requisites();
    
    uint32_t get_queue_index();

    // Logger
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_call_back(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data
    );

    bool is_device_suitable(const vk::PhysicalDevice& physical_device);
    
    uint32_t queue_index;
    
    Application* application_;
};

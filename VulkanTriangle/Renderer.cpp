#include "Renderer.h"

#include "Application.h"
#include "Debug.h"

#include <iostream>
#include <map>


Renderer::Renderer(Application* application) : application_(application)
{
}

void Renderer::create_instance()
{
    // Define applications info
    constexpr vk::ApplicationInfo application_info
    {
        .pApplicationName = "Vulkan Triangle",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "I<3Triangles",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vk::ApiVersion14
    };

    //
    //*** GLFW EXTENSIONS
    //

    std::vector<const char*> required_extension = get_check_extensions_requisites();

    //
    //*** LAYERS
    //
    
    std::vector<const char*> required_layers = get_check_layers_requisites();

    // Create instance info
    vk::InstanceCreateInfo create_info
    {
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
        .ppEnabledLayerNames = required_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(required_extension.size()),
        .ppEnabledExtensionNames = required_extension.data()
    };
    
    instance_ = vk::raii::Instance(context_, create_info);

    setup_debug_callback();
}

void Renderer::setup_debug_callback()
{
    // Setup severity of the message
    vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    // Setup type of the message
    vk::DebugUtilsMessageTypeFlagsEXT message_type_flags(vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    // Create message with debug_call_back
    vk::DebugUtilsMessengerCreateInfoEXT debug_info{
        .messageSeverity = severity_flags,
        .messageType = message_type_flags,
        .pfnUserCallback = &debug_call_back
    };

    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(debug_info);
}

void Renderer::pick_physical_device()
{
    auto check_if_suitable_device = [&] (auto const & physical_device)
    {
        return is_device_suitable(physical_device);
    };
    
    // For every physical_device
   std::vector<vk::raii::PhysicalDevice> physical_devices = instance_.enumeratePhysicalDevices();
    // Check if it is suitable
   auto const dev_iter = std::ranges::find_if(physical_devices, check_if_suitable_device);
    
    // If at the end of the list, no suitable devices were found --> error
    if (dev_iter == physical_devices.end())
        throw std::runtime_error("Failed to find suitable GPU");
    
    // Declare physical_device
    physical_device_ = *dev_iter;
}

void Renderer::create_logical_device()
{
    queue_index = get_queue_index();
    
    float queue_priority = 0.5f;
    // Create Device queue info
    vk::DeviceQueueCreateInfo device_queue_create_info {.queueFamilyIndex = queue_index, .queueCount = 1, .pQueuePriorities = &queue_priority};
    
    auto required_device_extension = get_required_device_extensions();
    
    auto surface_capabilities = physical_device_.getSurfaceCapabilitiesKHR(*application_->surface.get_surface());
    std::vector<vk::SurfaceFormatKHR> available_formats = physical_device_.getSurfaceFormatsKHR(*application_->surface.get_surface());
    std::vector<vk::PresentModeKHR> available_present_modes = physical_device_.getSurfacePresentModesKHR(*application_->surface.get_surface());
    
    // Define the logical device info
    vk::DeviceCreateInfo device_create_info = {
        .pNext = &feature_chain_.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(required_device_extension.size()),
        .ppEnabledExtensionNames = required_device_extension.data(),
    };
    
    device_ = vk::raii::Device(physical_device_, device_create_info);
    graphics_queue_ = vk::raii::Queue(device_, queue_index, 0);
}

vk::Bool32 Renderer::debug_call_back(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                     vk::DebugUtilsMessageTypeFlagsEXT type,
                                     const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
                                     void* p_user_data)
{
    if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        ||
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        std::cout << "Validation Layer : Type [" << to_string(type) << "] - Message: " << p_callback_data->pMessage << '\n';
    return vk::False;
}

bool Renderer::is_device_suitable(const vk::PhysicalDevice& physical_device)
{
    auto required_device_extensions = get_required_device_extensions();

    // Check if it supports Vulkan API
    bool supports_vulkan1_4 = physical_device.getProperties().apiVersion >= vk::ApiVersion14;

    // Check if it can do queues and graphic operations
    auto queue_families = physical_device.getQueueFamilyProperties();

    auto are_queues_and_graphics_supported = [](const auto& qfp)
    {
        // "!!" converts to bool by applying NOT operator twice
        return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    };
    
    bool supports_graphics = std::ranges::any_of(queue_families, are_queues_and_graphics_supported);

    auto available_device_extensions = physical_device.enumerateDeviceExtensionProperties();
    
    // Check if all required_device_extensions are in available_device_extensions
    auto are_device_extensions_supported = [&available_device_extensions](const auto& required_device_extension)
    {
        return std::ranges::any_of(available_device_extensions,
                                   [required_device_extension](const auto& available_device_extension)
                                   {
                                       return strcmp(available_device_extension.extensionName,
                                                     required_device_extension) == 0;
                                   });
    };

    bool supports_all_required_extensions = std::ranges::all_of(required_device_extensions,
                                                                are_device_extensions_supported);

    // Check if the physicalDevice supports the required features
    auto features = physical_device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                                 vk::PhysicalDeviceVulkan11Features,
                                                 vk::PhysicalDeviceVulkan13Features,
                                                 vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool supports_required_features = features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering;
    features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
    
    // Return true if physicalDevice meets every criteria
    return supports_all_required_extensions && supports_graphics && supports_vulkan1_4 && supports_required_features;
}

std::vector<const char*> Renderer::get_check_extensions_requisites()
{
    auto required_extension = get_required_instance_extensions();

    auto extension_properties = context_.enumerateInstanceExtensionProperties();

    for (auto& i : required_extension)
    {
        // lambda function, compares two strings (name of extensions) to see if two matches
        auto compare_extension = [extension = i](const auto& extension_property)
        {
            return strcmp(extension_property.extensionName, extension) == 0;
        };
        // if extension not found, or supported, return error
        if (std::ranges::none_of(extension_properties, compare_extension))
        {
            throw std::runtime_error("Required GLFW extension not supported : " + std::string(i));
        }
    }
    return required_extension;
}

std::vector<const char*> Renderer::get_check_layers_requisites()
{
    std::vector<const char*> required_layers = get_required_layers();

    // Check required layers

    // Get layers properties from context
    auto layer_properties = context_.enumerateInstanceLayerProperties();

    auto check_layer_properties = [&layer_properties](const auto& required_layer)
    {
        return std::ranges::none_of(layer_properties, [required_layer](const auto& layer_property)
        {
            return strcmp(layer_property.layerName, required_layer) == 0;
        });
    };

    auto unsupported_layer_it = std::ranges::find_if(required_layers, check_layer_properties);

    // In case the layer does not correspond to the end of the list, it means it stopped somewhere, and there is a problem
    if (unsupported_layer_it != required_layers.end())
    {
        throw std::runtime_error("Required layer not supported : " + std::string(*unsupported_layer_it));
    }
    return required_layers;
}

uint32_t Renderer::get_queue_index()
{
    std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device_.getQueueFamilyProperties();
    
    // Get the first index
    uint32_t queue_index = ~0;
    
    // Find the one who supports both graphics and SurfaceKHR
    for (uint32_t qfp_index = 0; qfp_index < queue_family_properties.size(); qfp_index++)
    {
        if ((queue_family_properties[qfp_index].queueFlags & vk::QueueFlagBits::eGraphics) 
            && physical_device_.getSurfaceSupportKHR(qfp_index, application_->surface.get_surface()))
        {
            // Found, so register, and quit loop
            queue_index = qfp_index;
            break;
        }
    }
    // if everything is still inverted, then error
    if (queue_index == ~0)
    {
        throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }
    return queue_index;
}

uint32_t Renderer::find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memory_properties = physical_device_.getMemoryProperties();
    
    // Find suitable memory type
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    
    throw std::runtime_error("failed to find suitable memory type!");
}


std::vector<const char*> Renderer::get_required_instance_extensions()
{
    uint32_t glfw_extension_count = 0;

    // Add to list glfw required instance extensions
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    std::vector extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    // Add our own required extensions to list
    std::vector<const char*> required_extensions = {vk::EXTDebugUtilsExtensionName};
    extensions.append_range(required_extensions);

    return extensions;
}

std::vector<const char*> Renderer::get_required_layers()
{
    const std::vector validationLayers = {"VK_LAYER_KHRONOS_validation"};

    // Add our own required layers to the list
    std::vector<const char*> required_layers = {};

    // Merge necessary layers to required list
    if (Debug::enable_debug)
        required_layers.append_range(validationLayers);

    return required_layers;
}

std::vector<const char*> Renderer::get_required_device_extensions()
{
    std::vector<const char*> device_extensions = {vk::KHRSwapchainExtensionName};

    // Add our own required device extensions to the list
    std::vector<const char*> required_device_extension = {};

    // Merge to required list
    required_device_extension.append_range(device_extensions);

    return required_device_extension;
}

vk::raii::Instance& Renderer::get_instance()
{
    return instance_;
}

vk::raii::PhysicalDevice& Renderer::get_physical_device()
{
    return physical_device_;
}

vk::raii::Device& Renderer::get_device()
{
    return device_;
}

uint32_t Renderer::get_final_queue_index()
{
    return queue_index;
}

vk::raii::Queue Renderer::get_graphics_queue()
{
    return graphics_queue_;
}

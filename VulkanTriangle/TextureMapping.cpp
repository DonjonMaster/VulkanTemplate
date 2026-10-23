#include "TextureMapping.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>
#include <stb/stb_image.h>

#include "Application.h"

TextureMapping::TextureMapping(Application* application) : application_(application)
{
}

void TextureMapping::create_texture_image()
{
    int tex_width, tex_height, tex_channels;
    
    stbi_uc *pixels = stbi_load(
        "textures/texture_3.jpg", 
        &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    
    vk::DeviceSize imageSize = tex_width * tex_height * 4;
    
    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }
    
    // Create a HostVisble buffer for image transfer
    auto [stagingBuffer, stagingBufferMemory] =
        application_->buffers.create_buffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // Copy the pixel values from the image to the buffer
    void* data = stagingBufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();
    
    // Clean up the original pixel array
    stbi_image_free(pixels);
    
    std::tie(texture_image, texture_image_memory) = create_image(tex_width,
        tex_height,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    
    // Transition the texture image to something optimal for transfer
    vk::raii::CommandBuffer command_buffer = application_->buffers.begin_single_time_commands();
    application_->buffers.transition_image_layout(command_buffer, texture_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    application_->buffers.copy_buffer_to_image(command_buffer, stagingBuffer, texture_image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    
    // Execute the buffer to image copy operation
    application_->buffers.transition_image_layout(command_buffer, texture_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    application_->buffers.end_single_time_commands(std::move(command_buffer));
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> TextureMapping::create_image(uint32_t width, uint32_t height,
    vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::ImageCreateInfo image_info{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode =vk::SharingMode::eExclusive
    };
    
    vk::raii::Image image = vk::raii::Image(application_->renderer.get_device(), image_info);
    
    vk::MemoryRequirements memory_requirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocate_info{
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = application_->renderer.find_memory_type(memory_requirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory image_memory = vk::raii::DeviceMemory(application_->renderer.get_device(), allocate_info);
    image.bindMemory(image_memory, 0);
    
    return {std::move(image), std::move(image_memory)};
}

vk::raii::ImageView TextureMapping::create_image_view(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspect_flags)
{
    vk::ImageViewCreateInfo image_view_create_info{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    return vk::raii::ImageView(application_->renderer.get_device(), image_view_create_info);
}

void TextureMapping::create_texture_image_view()
{
    texture_image_view_ = create_image_view(*texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

void TextureMapping::create_texture_sampler()
{
    vk::PhysicalDeviceProperties properties = application_->renderer.get_physical_device().getProperties();
    vk::SamplerCreateInfo sampler_info{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .borderColor = vk::BorderColor::eIntTransparentBlack,
        .unnormalizedCoordinates = vk::False
    };
    
    texture_sampler = vk::raii::Sampler(application_->renderer.get_device(), sampler_info);
}

vk::raii::ImageView& TextureMapping::get_texture_image_view()
{
    return texture_image_view_;
}

vk::raii::Sampler& TextureMapping::get_texture_sampler()
{
    return texture_sampler;
}

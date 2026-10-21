#include "Buffers.h"

#include "Application.h"
#include "Vertex.h"

Buffers::Buffers(Application* application) : application_(application)
{
}

Buffers::~Buffers()
{
}

void Buffers::create_command_pool()
{
    vk::CommandPoolCreateInfo pool_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = application_->renderer.get_final_queue_index()
    };
    
    command_pool_ = vk::raii::CommandPool(application_->renderer.get_device(), pool_info);
}

void Buffers::create_command_buffer()
{
    
    vk::CommandBufferAllocateInfo allocate_info{
        .commandPool = command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(application_->drawer.get_max_frames_in_flights())
    };

    command_buffer_ = vk::raii::CommandBuffers(application_->renderer.get_device(), allocate_info);
}

void Buffers::create_vertex_buffer()
{
    vk::DeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    // Create host visible temporary buffer with allocated memory
    auto [staging_buffer, staging_buffer_memory] = create_buffer(
        buffer_size,
        vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
    
    // Get vertices data to buffers
    void *data_staging = staging_buffer_memory.mapMemory(0, buffer_size);
    memcpy(data_staging, vertices.data(), buffer_size);
    
    staging_buffer_memory.unmapMemory();
    
    // Create local device buffer as actual vertex buffer
    std::tie(vertex_buffer_, vertex_buffer_memory) = create_buffer(
        buffer_size, 
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
        );
    
    // Copy temporary buffer's data to device buffer
    copy_buffer(staging_buffer, vertex_buffer_, buffer_size);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Buffers::create_buffer(vk::DeviceSize size,
    vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    // Assign Buffer
    vk::BufferCreateInfo buffer_info{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    vk::raii::Buffer buffer = vk::raii::Buffer(application_->renderer.get_device(), buffer_info);
    
    // Get suitable memory
    vk::MemoryRequirements memory_requirements = buffer.getMemoryRequirements();
    
    vk::MemoryAllocateInfo allocate_info{
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = application_->renderer.find_memory_type(memory_requirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory buffer_memory = vk::raii::DeviceMemory(application_->renderer.get_device(), allocate_info);
    
    // Allocate memory to buffer
    buffer.bindMemory(*buffer_memory, 0);
    return {std::move(buffer), std::move(buffer_memory)};
}

void Buffers::create_index_buffer()
{
    vk::DeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
    // Assign Buffer
    auto [staging_buffer, staging_buffer_memory] = create_buffer(
        buffer_size, 
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    
    void *data = staging_buffer_memory.mapMemory(0, buffer_size);
    memcpy(data, indices.data(), (size_t) buffer_size);
    staging_buffer_memory.unmapMemory();
    
    std::tie(index_buffer_, index_buffer_memory) =
        create_buffer(
            buffer_size, 
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
            vk::MemoryPropertyFlagBits::eDeviceLocal
            );
    
    copy_buffer(staging_buffer, index_buffer_, buffer_size);
}

void Buffers::create_uniform_buffers()
{
    for (size_t i = 0; i < application_->drawer.get_max_frames_in_flights(); i++)
    {
        vk::DeviceSize buffer_size = sizeof(UniformBufferObject);
        auto [buffer, buffer_mem] = create_buffer(
            buffer_size, 
            vk::BufferUsageFlagBits::eUniformBuffer, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );
        
        uniform_buffers_.emplace_back(std::move(buffer));
        uniform_buffers_memory.emplace_back(std::move(buffer_mem));
        // We map memory now to get a pointer to which we can write the data later on
        uniform_buffers_mapped.emplace_back(uniform_buffers_memory.back().mapMemory(0, buffer_size));
    }
}

void Buffers::update_uniform_buffers(uint32_t current_image)
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    
    auto current_time = std::chrono::high_resolution_clock::now();
    float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    
    UniformBufferObject ubo{};
    // Rotate 90-degree every time
    ubo.model = rotate(glm::mat4(1.0f), delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // Place camera at a 45-degree angle
    ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    ubo.proj = glm::perspective(
        glm::radians(45.0f), 
        static_cast<float>(application_->surface.get_swap_chain_extent().width) 
        / 
        static_cast<float>(application_->surface.get_swap_chain_extent().height), 
        0.1f,
        10.0f);
    
    memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

std::vector<vk::raii::Buffer>& Buffers::get_uniform_buffers()
{
    return uniform_buffers_;
}

void Buffers::copy_buffer(vk::raii::Buffer& src_buffer, vk::raii::Buffer& dst_buffer, vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocate_info{
        .commandPool = command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer command_copy_buffer = std::move(
        application_->renderer.get_device().allocateCommandBuffers(allocate_info).front()
        );
    
    command_copy_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    command_copy_buffer.copyBuffer(*src_buffer, *dst_buffer, vk::BufferCopy(0, 0, size));
    command_copy_buffer.end();
    
    application_->renderer.get_graphics_queue().submit(vk::SubmitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_copy_buffer,
    }, nullptr);
    
    application_->renderer.get_graphics_queue().waitIdle();
}

std::vector<vk::raii::CommandBuffer>& Buffers::get_command_buffer()
{
    return command_buffer_;
}

void Buffers::framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->buffers.frame_buffer_resized = true;
}

void Buffers::record_command_buffer(uint32_t image_index)
{
    auto swap_chain_extent = application_->surface.get_swap_chain_extent();
    
    // Details about the usage of a type of buffer
    auto &command_buffer = command_buffer_[application_->drawer.get_frame_index()];
    
    command_buffer.begin({});
    
    // Before rendering (transition from swapchain image to a colored image)
    transition_image_layout(
        image_index,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );
    
    // Setup color attachment
    vk::ClearValue clear_color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = application_->surface.get_swap_chain_image_views()[image_index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clear_color
    };
    
    vk::RenderingInfo rendering_info = {
        .renderArea = {.offset = {0, 0}, .extent = swap_chain_extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };
    
    // Begin rendering
    command_buffer.beginRendering(rendering_info);
    
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *application_->graphics_pipeline.get_graphics_pipeline_());
    // Bind the vertex buffer
    command_buffer.bindVertexBuffers(0, *vertex_buffer_, {0});
    command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);
    
    command_buffer.setViewport(0, 
        vk::Viewport(
        0.0f, 
        static_cast<float>(swap_chain_extent.height), 
        static_cast<float>(swap_chain_extent.width),
        -static_cast<float>(swap_chain_extent.height),
        0.0f,
        1.0f));
    
    command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
    
    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, 
        application_->graphics_pipeline.get_pipeline_layout(),
        0,
        *application_->graphics_pipeline.get_descriptor_sets()[application_->drawer.get_frame_index()],
        nullptr
        );
    command_buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    
    command_buffer.endRendering();
    
    // After rendering (transition from swapchain image to a presentable image for flat screens)
    transition_image_layout(
        image_index,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
        );
    
    command_buffer.end();
}

void Buffers::transition_image_layout(uint32_t image_index, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask, vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = application_->surface.get_swap_chain_images_()[image_index],
        .subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1}
    };
    
    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    
    command_buffer_[application_->drawer.get_frame_index()].pipelineBarrier2(dependency_info);
}

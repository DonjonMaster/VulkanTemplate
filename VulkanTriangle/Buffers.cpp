#include "Buffers.h"

#include "Application.h"

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

std::vector<vk::raii::CommandBuffer>& Buffers::get_command_buffer()
{
    return command_buffer_;
}

bool& Buffers::get_frame_buffer_resized()
{
    return frame_buffer_resized;
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
    
    command_buffer.setViewport(0, vk::Viewport(
        0.0f, 
        0.0, 
        static_cast<float>(swap_chain_extent.width), 
        static_cast<float>(swap_chain_extent.height),
        0.0f,
        1.0f));
    
    command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
    
    command_buffer.draw(3, 1, 0, 0);
    
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

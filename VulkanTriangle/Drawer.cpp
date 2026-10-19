#include "Drawer.h"

#include "Application.h"

Drawer::Drawer(Application* application) : application_(application), device(application_->renderer.get_device())
{
}

Drawer::~Drawer()
{
}

void Drawer::draw_frame()
{
    auto fence_result = device.waitForFences(*draw_fence, vk::True, UINT64_MAX);
    if (fence_result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence !");
    }
    device.resetFences(*draw_fence);
    
    auto [result, image_index] = application_->surface.get_swap_chain().acquireNextImage(UINT64_MAX, *present_complete_semaphore, nullptr);
    
    vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submit_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphore,
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*application_->buffers.get_command_buffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphore
    };
    
    application_->renderer.get_graphics_queue().submit(submit_info, *draw_fence);
    
    const vk::PresentInfoKHR present_info_khr{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &*application_->surface.get_swap_chain(),
        .pImageIndices = &image_index
    };
    
    result = application_->renderer.get_graphics_queue().presentKHR(present_info_khr);
}

void Drawer::create_sync_objects()
{
    present_complete_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    render_finished_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    draw_fence = vk::raii::Fence(device, {.flags = vk::FenceCreateFlagBits::eSignaled});
}
    

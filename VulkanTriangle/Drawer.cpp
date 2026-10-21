#include "Drawer.h"

#include <iostream>

#include "Application.h"

Drawer::Drawer(Application* application) : application_(application), device(application_->renderer.get_device())
{
}

Drawer::~Drawer()
{
}

void Drawer::draw_frame()
{    
    auto fence_result = device.waitForFences(*in_flight_fences_[frame_index_], vk::True, UINT64_MAX);
    if (fence_result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence !");
    }
    
    auto [result, image_index] = application_->surface.get_swap_chain().acquireNextImage(UINT64_MAX, *present_complete_semaphores_[frame_index_], nullptr);
    
    // If the image is not a success, and is not suboptimal, then there is an error with the swap chain image
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    // Only reset the fence if we are submitting work
    device.resetFences(*in_flight_fences_[frame_index_]);
    
    // New command buffers for new image
    application_->buffers.get_command_buffer()[frame_index_].reset();
    application_->buffers.record_command_buffer(image_index);
    
    vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    
    application_->buffers.update_uniform_buffers(frame_index_);
    
    const vk::SubmitInfo submit_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphores_[frame_index_],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*application_->buffers.get_command_buffer()[frame_index_],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphore_[image_index]
    };
    
    // Render the image
    application_->renderer.get_graphics_queue().submit(submit_info, *in_flight_fences_[frame_index_]);
    
    // Advance to the next frame
    frame_index_ = (frame_index_ + 1) % MAX_FRAMES_IN_FLIGHTS;
    
    const vk::PresentInfoKHR present_info_khr{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphore_[image_index],
        .swapchainCount = 1,
        .pSwapchains = &*application_->surface.get_swap_chain(),
        .pImageIndices = &image_index
    };
    
    // Present the image for flat screen
    result = application_->renderer.get_graphics_queue().presentKHR(present_info_khr);
    
    // If the window was resized OR the swap chain is out of date, we should recreate the swap chain
    if (application_->buffers.frame_buffer_resized || result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
    {
        application_->buffers.frame_buffer_resized = false;
        application_->surface.recreate_swap_chain();
    }
    else
    {
        // Since there are no other success codes than eSuccess, presentKHR already threw an exception; So there is no need.
        assert(result == vk::Result::eSuccess);
    }
}

void Drawer::create_sync_objects()
{
    assert(present_complete_semaphores_.empty() && render_finished_semaphore_.empty() && in_flight_fences_.empty());
    
    // The render_finished_semaphore vector is sized by the size of the swap chain
    for (size_t i = 0; i < application_->surface.get_swap_chain_images_().size(); i++)
    {
        render_finished_semaphore_.emplace_back(device, vk::SemaphoreCreateInfo());
    }
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHTS; i++)
    {
        present_complete_semaphores_.emplace_back(device, vk::SemaphoreCreateInfo());
        in_flight_fences_.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

const int& Drawer::get_max_frames_in_flights()
{
    return MAX_FRAMES_IN_FLIGHTS;
}

uint32_t& Drawer::get_frame_index()
{
    return frame_index_;
}
    

#pragma once

#include "Dependencies.h"

class Application;

class Drawer
{
public:
    Drawer(Application*);
    ~Drawer();
    
    void draw_frame();
    void create_sync_objects();
    
    const int& get_max_frames_in_flights();
    uint32_t& get_frame_index();
    
private:
    const int MAX_FRAMES_IN_FLIGHTS = 2;
    uint32_t frame_index_ = 0;
    
    std::vector<vk::raii::Semaphore> present_complete_semaphores_;
    std::vector<vk::raii::Semaphore> render_finished_semaphore_;
    std::vector<vk::raii::Fence> in_flight_fences_;
    
    Application* application_;
    vk::raii::Device& device;
};

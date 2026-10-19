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
    
private:
    
    vk::raii::Semaphore present_complete_semaphore = nullptr;
    vk::raii::Semaphore render_finished_semaphore = nullptr;
    vk::raii::Fence draw_fence = nullptr;
    
    Application* application_;
    vk::raii::Device& device;
};

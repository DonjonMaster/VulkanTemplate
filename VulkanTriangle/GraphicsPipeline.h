#pragma once
#include <string>
#include <vector>
#include <fstream>

#include "Dependencies.h"

class Application;

class GraphicsPipeline
{
public:
    GraphicsPipeline(Application*);
    
    void create_graphics_pipeline();
    
    vk::raii::Pipeline& get_graphics_pipeline_();

private:
    static std::vector<char> read_file(const std::string& filename);
    [[nodsicard]] vk::raii::ShaderModule create_shader_module(const std::vector<char>& code) const;
    
    vk::raii::PipelineLayout pipeline_layout_ = nullptr;
    vk::raii::Pipeline graphics_pipeline_ = nullptr;
    
    Application* application_;
};

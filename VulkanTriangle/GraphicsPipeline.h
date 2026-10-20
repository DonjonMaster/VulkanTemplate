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
    
    void create_descriptor_pool();
    void create_descriptor_set_layout();
    void create_descriptor_sets();
    void create_graphics_pipeline();
    
    vk::raii::Pipeline& get_graphics_pipeline_();
    vk::raii::PipelineLayout& get_pipeline_layout();
    
    std::vector<vk::raii::DescriptorSet>& get_descriptor_sets();

private:
    static std::vector<char> read_file(const std::string& filename);
    [[nodsicard]] vk::raii::ShaderModule create_shader_module(const std::vector<char>& code) const;
    
    vk::raii::DescriptorSetLayout descriptor_set_layout_ = nullptr;
    vk::raii::PipelineLayout pipeline_layout_ = nullptr;
    vk::raii::Pipeline graphics_pipeline_ = nullptr;
    
    vk::raii::DescriptorPool descriptor_pool_ = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptor_sets_;
    
    Application* application_;
};

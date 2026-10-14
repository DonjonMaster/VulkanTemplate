#include "GraphicsPipeline.h"

void GraphicsPipeline::create_graphics_pipeline()
{
   vk::raii::ShaderModule shader_module = create_shader_module(read_file("shaders/slang.spv"));
    
    // Vertex shader
    vk::PipelineShaderStageCreateInfo vert_shader_stage_info{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shader_module,
        .pName = "vertMain"
    };
    
    // Fragment shader
    vk::PipelineShaderStageCreateInfo frag_shader_stage_info{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shader_module,
        .pName = "fragMain",
    };
    
    vk::PipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
    
    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    
    vk::PipelineDynamicStateCreateInfo dynamic_state{
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()
    };
    
    vk::PipelineVertexInputStateCreateInfo vertex_input_info;
    
    vk::PipelineInputAssemblyStateCreateInfo input_assembly{
        .topology = vk::PrimitiveTopology::eTriangleList
    };
    
    vk::Extent2D swap_chain_extent = surface_->get_swap_chain_extent();
    
    vk::Viewport viewport{
        0.0f, 
        0.0f,
        static_cast<float>(swap_chain_extent.width), 
        static_cast<float>(swap_chain_extent.height),
        0.0f,
        1.0f
    };
    
    vk::Rect2D scissor{vk::Offset2D { 0, 0}, swap_chain_extent};
    
    vk::PipelineViewportStateCreateInfo viewport_state{
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };
    
    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f
    };
    
    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };
    
    vk::PipelineColorBlendAttachmentState color_blend_attachment{
        .blendEnable = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrc1Alpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    
    vk::PipelineColorBlendStateCreateInfo color_blending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment
    };
    
    vk::PipelineLayoutCreateInfo pipeline_layout_info{
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };
    
    pipeline_layout_ = vk::raii::PipelineLayout(renderer->get_device(), pipeline_layout_info);
}

std::vector<char> GraphicsPipeline::read_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }
}

vk::raii::ShaderModule GraphicsPipeline::create_shader_module(const std::vector<char>& code) const
{
    vk::ShaderModuleCreateInfo create_info{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()) 
    };
    
    vk::raii::ShaderModule shader_module{renderer->get_device(), create_info};
    
    return shader_module;
}

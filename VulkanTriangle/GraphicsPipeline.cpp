#include "GraphicsPipeline.h"

#include "Application.h"
#include "Vertex.h"

GraphicsPipeline::GraphicsPipeline(Application* application) : application_(application)
{
}

void GraphicsPipeline::create_graphics_pipeline()
{
    auto shader_code = read_file("shaders/slang.spv");
    auto shader_module = create_shader_module(shader_code);
    
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
    
    std::vector dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    
    vk::PipelineDynamicStateCreateInfo dynamic_state{
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()
    };
    
    auto binding_description = Vertex::get_binding_description();
    auto attribute_descriptions = Vertex::get_attribute_descriptions();
    
    vk::PipelineVertexInputStateCreateInfo vertex_input_info{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
        .pVertexAttributeDescriptions = attribute_descriptions.data()
    };
    
    vk::PipelineInputAssemblyStateCreateInfo input_assembly{
        .topology = vk::PrimitiveTopology::eTriangleList
    };
    
    vk::Extent2D swap_chain_extent = application_->surface.get_swap_chain_extent();
    
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
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
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
    
    pipeline_layout_ = vk::raii::PipelineLayout(application_->renderer.get_device(), pipeline_layout_info);
    
    vk::PipelineRenderingCreateInfo pipeline_rendering_create_info{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &application_->surface.get_swap_chain_surface_format().format
    };
    
    vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info = 
        {
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = pipeline_layout_,
        .renderPass = nullptr
    };
    
    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info_chain = {
        graphics_pipeline_create_info, 
        pipeline_rendering_create_info
    };
    
    graphics_pipeline_ = vk::raii::Pipeline(
        application_->renderer.get_device(), 
        nullptr, 
        pipeline_create_info_chain.
        get<vk::GraphicsPipelineCreateInfo>()
        );
}

vk::raii::Pipeline& GraphicsPipeline::get_graphics_pipeline_()
{
    return graphics_pipeline_;
}

std::vector<char> GraphicsPipeline::read_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }
    
    std::vector<char> buffer(file.tellg());
    
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    
    return buffer;
}

vk::raii::ShaderModule GraphicsPipeline::create_shader_module(const std::vector<char>& code) const
{
    vk::ShaderModuleCreateInfo create_info{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()) 
    };
    
    vk::raii::ShaderModule shader_module(application_->renderer.get_device(), create_info);
    
    return shader_module;
}

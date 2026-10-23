#include "GraphicsPipeline.h"

#include "Application.h"
#include "Vertex.h"

GraphicsPipeline::GraphicsPipeline(Application* application) : application_(application)
{
}

void GraphicsPipeline::create_descriptor_pool()
{
    std::array<vk::DescriptorPoolSize, 2> pool_sizes{
        {
            {.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = static_cast<uint32_t>(application_->drawer.get_max_frames_in_flights())},
            {.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = static_cast<uint32_t>(application_->drawer.get_max_frames_in_flights())}}};
    
    vk::DescriptorPoolCreateInfo pool_info{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(application_->drawer.get_max_frames_in_flights()),
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data()
    };
    
    descriptor_pool_ = vk::raii::DescriptorPool(application_->renderer.get_device(), pool_info);
}

void GraphicsPipeline::create_descriptor_set_layout()
{
    
    std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
        {
            {
                .binding = 0, 
                .descriptorType = vk::DescriptorType::eUniformBuffer, 
                .descriptorCount = 1, 
                .stageFlags = vk::ShaderStageFlagBits::eVertex
            },
            {
                .binding = 1, 
                .descriptorType = vk::DescriptorType::eCombinedImageSampler, 
                .descriptorCount = 1, 
                .stageFlags = vk::ShaderStageFlagBits::eFragment
            }
        }
    };
    
    vk::DescriptorSetLayoutCreateInfo layout_info{.bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data()};
    
    descriptor_set_layout_ = vk::raii::DescriptorSetLayout(application_->renderer.get_device(), layout_info);
}

void GraphicsPipeline::create_descriptor_sets()
{
    std::vector<vk::DescriptorSetLayout> layouts(application_->drawer.get_max_frames_in_flights(), *descriptor_set_layout_);
    
    vk::DescriptorSetAllocateInfo allocate_info{
        .descriptorPool = descriptor_pool_,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };
    
    descriptor_sets_ = application_->renderer.get_device().allocateDescriptorSets(allocate_info);
    
    for (size_t i = 0; i < application_->drawer.get_max_frames_in_flights(); i++)
    {
        vk::DescriptorBufferInfo buffer_info{
            .buffer = application_->buffers.get_uniform_buffers()[i],
            .offset = 0,
            .range = sizeof(Buffers::UniformBufferObject)
        };
        
        vk::DescriptorImageInfo image_info{
            .sampler = application_->texture_mapping.get_texture_sampler(),
            .imageView = application_->texture_mapping.get_texture_image_view(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        
        std::array<vk::WriteDescriptorSet, 2> descriptor_writes{
            {
                {
                    .dstSet = descriptor_sets_[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &buffer_info
                },
                {
                    .dstSet = descriptor_sets_[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &image_info}
                }
        };
        
        application_->renderer.get_device().updateDescriptorSets(descriptor_writes, {});
    }
    
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
        .cullMode = vk::CullModeFlagBits::eFront,
        .frontFace = vk::FrontFace::eCounterClockwise,
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
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptor_set_layout_,
        .pushConstantRangeCount = 0,
    };
    
    pipeline_layout_ = vk::raii::PipelineLayout(application_->renderer.get_device(), pipeline_layout_info);
    
    vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info = 
        {
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil_,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = pipeline_layout_,
        .renderPass = nullptr
    };
    
    vk::PipelineRenderingCreateInfo pipeline_rendering_create_info{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &application_->surface.get_swap_chain_surface_format().format,
        .depthAttachmentFormat = application_->depth_handler.find_depth_format()
    };
    
    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info_chain = {
        graphics_pipeline_create_info, 
        pipeline_rendering_create_info
    };
    
    graphics_pipeline_ = vk::raii::Pipeline(
        application_->renderer.get_device(), 
        nullptr, 
        pipeline_create_info_chain.get<vk::GraphicsPipelineCreateInfo>()
        );
}

vk::raii::Pipeline& GraphicsPipeline::get_graphics_pipeline_()
{
    return graphics_pipeline_;
}

vk::raii::PipelineLayout& GraphicsPipeline::get_pipeline_layout()
{
    return pipeline_layout_;
}

std::vector<vk::raii::DescriptorSet>& GraphicsPipeline::get_descriptor_sets()
{
    return descriptor_sets_;
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

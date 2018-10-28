///////////////////////////////////////////////////////////////////////////////////////////////////
///
///     VkHello
///
///     Copyright 2018, Brandon Light
///     All rights reserved.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "VkBuilder.h"

VkPipelineBuilder::VkPipelineBuilder(
    VkDevice vkDevice,
    const VkAllocationCallbacks* pAllocator)
    :
    m_device(vkDevice),
    m_pAllocator(pAllocator)
{

}

VkPipelineBuilder::~VkPipelineBuilder()
{

}

VkPipelineBuilder* VkPipelineBuilder::Create(
    VkDevice vkDevice,
    const VkAllocationCallbacks* pAllocator)
{
    VkPipelineBuilder* pBuilder = new VkPipelineBuilder(vkDevice, pAllocator);

    if (pBuilder->Init() == false)
    {
        delete pBuilder;
        pBuilder = nullptr;
    }

    return pBuilder;
}

void VkPipelineBuilder::Destroy()
{
    delete this;
}

bool VkPipelineBuilder::Init()
{
    m_vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    SetVertexState();  // use defaults

    m_inputAssemblyState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_inputAssemblyState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    m_viewport.x        = 0;
    m_viewport.y        = 0;
    m_viewport.width    = 0; // Updated in SetViewportState()
    m_viewport.height   = 0; // Updated in SetViewportState()
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_scissorRect.extent.width = 0;  // Updated in SetViewportState()
    m_scissorRect.extent.height = 0; // Updated in SetViewportState()
    m_scissorRect.offset.x      = 0;
    m_scissorRect.offset.y      = 0;

    m_viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewportState.viewportCount = 1;
    m_viewportState.pViewports    = &m_viewport;
    m_viewportState.scissorCount  = 1;
    m_viewportState.pScissors     = &m_scissorRect;

    m_rasterizationState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_rasterizationState.depthClampEnable        = VK_FALSE;
    m_rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    m_rasterizationState.polygonMode             = VK_POLYGON_MODE_FILL;
    m_rasterizationState.cullMode                = VK_CULL_MODE_NONE;
    m_rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_rasterizationState.depthBiasEnable         = VK_FALSE;
    m_rasterizationState.lineWidth               = 1.0f;

    m_multisampleState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_multisampleState.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    m_multisampleState.sampleShadingEnable   = VK_FALSE;
    m_multisampleState.alphaToOneEnable      = VK_FALSE;
    m_multisampleState.alphaToCoverageEnable = VK_FALSE;


    m_blendAttachment = {};
    m_blendAttachment.blendEnable = VK_FALSE;
    m_blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                       VK_COLOR_COMPONENT_G_BIT |
                                       VK_COLOR_COMPONENT_B_BIT |
                                       VK_COLOR_COMPONENT_A_BIT;

    m_colorBlendState.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_colorBlendState.logicOpEnable   = VK_FALSE;
    m_colorBlendState.attachmentCount = 1;
    m_colorBlendState.pAttachments    = &m_blendAttachment;


    m_pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_pipelineCreateInfo.flags = 0;

    m_depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    ///@todo Pipeline cache?  
    m_pipelineCreateInfo.basePipelineHandle = 0;
    m_pipelineCreateInfo.basePipelineIndex = 0;


    m_pipelineCreateInfo.pVertexInputState   = &m_vertexInputState;
    m_pipelineCreateInfo.pInputAssemblyState = &m_inputAssemblyState;

    m_pipelineCreateInfo.pViewportState      = &m_viewportState;
    m_pipelineCreateInfo.pRasterizationState = &m_rasterizationState;
    m_pipelineCreateInfo.pMultisampleState   = &m_multisampleState;
    m_pipelineCreateInfo.pColorBlendState    = &m_colorBlendState;


    // Not supported yet
    m_pipelineCreateInfo.pDepthStencilState  = NULL;
    m_pipelineCreateInfo.pDynamicState       = NULL;
    m_pipelineCreateInfo.pTessellationState  = NULL;


    m_pipelineCreateInfo.pStages    = &m_stages[0];

    return true;
}

void VkPipelineBuilder::SetVertexState()
{
    m_vertexAttributeDescription.binding  = 0;
    m_vertexAttributeDescription.format   = VK_FORMAT_R32G32_SFLOAT;
    m_vertexAttributeDescription.location = 0;
    m_vertexAttributeDescription.offset   = 0;

    m_vertexBindingDescription.binding   = 0;
    m_vertexBindingDescription.stride    = 8;
    m_vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_vertexInputState.vertexBindingDescriptionCount   = 1;
    m_vertexInputState.pVertexBindingDescriptions      = &m_vertexBindingDescription;
    m_vertexInputState.vertexAttributeDescriptionCount = 1;
    m_vertexInputState.pVertexAttributeDescriptions    = &m_vertexAttributeDescription;

}

void VkPipelineBuilder::SetViewportState(
    uint32_t width,
    uint32_t height)
{
    // Don't support scissor yet -- its fully open
    m_viewport.width            = static_cast<float>(width);
    m_viewport.height           = static_cast<float>(height);
    m_scissorRect.extent.width  = width;
    m_scissorRect.extent.height = height;
}

void VkPipelineBuilder::SetShaderState(
    VkShaderModule vertexShader,
    VkShaderModule fragmentShader)
{
    m_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stages[0].flags = 0;
    m_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    m_stages[0].module = vertexShader;
    m_stages[0].pName = "main";
    m_stages[0].pSpecializationInfo = NULL;

    m_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stages[1].flags = 0;
    m_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    m_stages[1].module = fragmentShader;
    m_stages[1].pName = "main";
    m_stages[1].pSpecializationInfo = NULL;

    m_numStages = 2;
}

VkPipeline VkPipelineBuilder::GetPipeline(
    VkPipelineLayout layout,
    VkRenderPass renderPass,
    uint32_t subpassIndex)
{
    // caller supplied data
    m_pipelineCreateInfo.layout     = layout;
    m_pipelineCreateInfo.renderPass = renderPass;
    m_pipelineCreateInfo.subpass    = subpassIndex;

    // Uncached pipeline create state
    m_pipelineCreateInfo.stageCount = m_numStages;


    VkPipeline pipeline;
    VkResult vkResult = vkCreateGraphicsPipelines(m_device, 0, 1, &m_pipelineCreateInfo, m_pAllocator, &pipeline);

    return pipeline;
}


ResourceBuilder::ResourceBuilder(
    VkDevice device,
    const VkAllocationCallbacks* pAllocator)
    :
    m_device(device),
    m_pAllocator(pAllocator)
{

}

ResourceBuilder::~ResourceBuilder()
{

}

ResourceBuilder* ResourceBuilder::Create(
    VkDevice device,
    const VkAllocationCallbacks* pAllocator)
{
    ResourceBuilder* pBuilder = new ResourceBuilder(device, pAllocator);

    if (pBuilder->Init() == false)
    {
        delete pBuilder;
        pBuilder = nullptr;
    }

    return pBuilder;
}

void ResourceBuilder::Destroy()
{
    delete this;
}

bool ResourceBuilder::Init()
{
    m_imageCreateInfo.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    // Only support 2D
    m_imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;

    // Default format is R8G8B8A8, nothing else yet supported
    m_imageCreateInfo.format    = VK_FORMAT_R8G8B8A8_UNORM;

    // Mips, arrays, multisample not yet supported
    m_imageCreateInfo.mipLevels     = 1;
    m_imageCreateInfo.arrayLayers   = 1;
    m_imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;


    // Buffer Init
    m_bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    return true;
}

void ResourceBuilder::SetImageDimensions(
    uint32_t width,
    uint32_t height,
    uint32_t depth)
{
    m_imageCreateInfo.extent.width  = width;
    m_imageCreateInfo.extent.height = height;
    m_imageCreateInfo.extent.depth  = depth;
}

void ResourceBuilder::SetImageStaging(
    bool isStaging)
{
    if (isStaging == true)
    {
        m_imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        m_imageCreateInfo.tiling        = VK_IMAGE_TILING_LINEAR;
    }
    else
    {
        m_imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    }

    ///@todo refactor usage to be externally customizable (and optimal..)
    if (isStaging == true)
    {
        m_imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    else
    {
        ///@todo To be optimal, we need to mark the usage according to expected usage...
        m_imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                  VK_IMAGE_USAGE_SAMPLED_BIT;
    }
}

Resource* ResourceBuilder::GetImageResource()
{
    Resource* pResource = new Resource();

    VkResult result = vkCreateImage(m_device, &m_imageCreateInfo, m_pAllocator, &pResource->image);

    if (result != VK_SUCCESS)
    {
        delete pResource;
        pResource = NULL;
    }
    else
    {
        vkGetImageMemoryRequirements(m_device, pResource->image, &pResource->memReqs);
    }

    return pResource;
}

void ResourceBuilder::SetBufferSize(
    uint32_t size)
{
    m_bufferCreateInfo.size = size;
}

void ResourceBuilder::SetBufferUsage(
    bool vertexBuffer)
{
    ///@todo How to handle other buffer types?
    //  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    //  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    //  VK_BUFFER_USAGE_INDEX_BUFFER_BIT

    if (vertexBuffer == true)
    {
        m_bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    else
    {
        m_bufferCreateInfo.usage = 0;
    }
}


Resource* ResourceBuilder::GetBufferResource()
{
    Resource* pResource = new Resource();

    VkResult result = vkCreateBuffer(m_device, &m_bufferCreateInfo, m_pAllocator, &pResource->buffer);

    if (result != VK_SUCCESS)
    {
        delete pResource;
        pResource = NULL;
    }
    else
    {
        vkGetBufferMemoryRequirements(m_device, pResource->buffer, &pResource->memReqs);
    }

    return pResource;
}
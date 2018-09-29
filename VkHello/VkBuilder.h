///////////////////////////////////////////////////////////////////////////////////////////////////
///
///     VkHello
///
///     Copyright 2018, Brandon Light
///     All rights reserved.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "vulkan\vulkan.h"

class VkPipelineBuilder
{
public:
    static VkPipelineBuilder* Create(VkDevice);
    virtual void Destroy();

    VkPipeline GetPipeline(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex);


    void SetViewportState(uint32_t width, uint32_t height);

    void SetShaderState(VkShaderModule vertexShader, VkShaderModule fragmentShader);

private:
    VkPipelineBuilder(const VkPipelineBuilder&); // disallow copy constructor

    VkPipelineBuilder(VkDevice device);

    virtual ~VkPipelineBuilder();


    // Add to public interface when defaults are not good enough
    void SetVertexState();

    bool Init();

private:
    VkDevice m_device;
    VkGraphicsPipelineCreateInfo              m_pipelineCreateInfo;


    uint32_t                                  m_numStages;
    VkPipelineShaderStageCreateInfo           m_stages[5];

    // Primary pipeline create info structures
    VkPipelineVertexInputStateCreateInfo      m_vertexInputState;
    VkPipelineInputAssemblyStateCreateInfo    m_inputAssemblyState;
    VkPipelineViewportStateCreateInfo         m_viewportState;
    VkPipelineRasterizationStateCreateInfo    m_rasterizationState;
    VkPipelineColorBlendStateCreateInfo       m_colorBlendState;

    // Defaults only
    VkPipelineMultisampleStateCreateInfo      m_multisampleState;
    // Unsupported for now
    VkPipelineTessellationStateCreateInfo     m_tessellationState;
    VkPipelineDepthStencilStateCreateInfo     m_depthStencilState;
    VkPipelineDynamicStateCreateInfo          m_dynamicState;


    // Internal structures for primary create info structures
    // Vertex
    VkVertexInputBindingDescription     m_vertexBindingDescription;
    VkVertexInputAttributeDescription   m_vertexAttributeDescription;
    // Viewport
    VkViewport                          m_viewport;
    VkRect2D                            m_scissorRect;
    // Color Blend State
    VkPipelineColorBlendAttachmentState m_blendAttachment;
};
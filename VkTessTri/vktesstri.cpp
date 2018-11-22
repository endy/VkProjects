///////////////////////////////////////////////////////////////////////////////////////////////////
///
///     VkTessTri
///
///     Copyright 2018, Brandon Light
///     All rights reserved.
///
///     Basic tessellated triangle
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "VkUtils.h"

#include "IvyWindow.h"
#include "SimpleAllocator.h"

#include "VkBuilder.h"


///@todo Remove once the shader module setup is refactored out
#include <fstream>
#include <vector>

const char* RequiredInstanceExtensionNames[] =
{
    "VK_EXT_debug_report",
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
};
const uint32_t RequiredInstanceExtensionCount = sizeof(RequiredInstanceExtensionNames) / sizeof(RequiredInstanceExtensionNames[0]);

const char* RequiredDeviceExtensionNames[] =
{
    "VK_KHR_swapchain",
};
const uint32_t RequiredDeviceExtensionCount = sizeof(RequiredDeviceExtensionNames) / sizeof(RequiredDeviceExtensionNames[0]);

const char* RequiredLayerNames[] =
{
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_object_tracker",
    "VK_LAYER_LUNARG_standard_validation",
};
const uint32_t RequiredLayerCount = sizeof(RequiredLayerNames) / sizeof(RequiredLayerNames[0]);


const AppInfo appInfo =
{
    "VkTessTri",
    "NullEngine",
    VK_API_VERSION_1_1,

    RequiredInstanceExtensionNames,
    RequiredInstanceExtensionCount,
    RequiredLayerNames,
    RequiredLayerCount,

    RequiredDeviceExtensionNames,
    RequiredDeviceExtensionCount,

    GetSimpleAllocator()
};



///@todo decouple descriptor layout from push constant layout
///      build one descriptor set up front, use it for everything
///      push constants tied to programs, so pipeline layout + program is a uint
void createPipelineLayout(
    VkDevice vkDevice,
    VkDescriptorSetLayoutBinding* pBindings,
    uint32_t                      bindingCount,
    VkPushConstantRange*          pPushConstantRanges,
    uint32_t                      pushConstantRangeCount,
    VkDescriptorSetLayout*        pDescriptorSetLayout,
    VkPipelineLayout*             pPipelineLayout)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pBindings    = pBindings;
    descriptorSetLayoutCreateInfo.bindingCount = bindingCount;

    VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &descriptorSetLayoutCreateInfo, GetSimpleAllocator(), pDescriptorSetLayout));

    VkPipelineLayoutCreateInfo layoutCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        pDescriptorSetLayout,
        0,
        NULL
    };

    layoutCreateInfo.pPushConstantRanges    = pPushConstantRanges;
    layoutCreateInfo.pushConstantRangeCount = pushConstantRangeCount;

    VK_CHECK(vkCreatePipelineLayout(vkDevice, &layoutCreateInfo, GetSimpleAllocator(), pPipelineLayout));
}

///@todo create shader to wrap SPV loading & module creation, create program to group them?
void createGraphicsPipeline(
    VkDevice device,
    VkPipelineLayout layout,
    VkRenderPass renderPass,
    uint32_t     subpassIndex,
    VkPipeline* pPipeline)
{
    std::ifstream vsFile = std::ifstream("vert.spv", std::ios::ate | std::ios::binary);

    size_t shader_size = static_cast<size_t>(vsFile.tellg());
    std::vector<char> vert_shader(shader_size);
    vsFile.seekg(0);
    vsFile.read(vert_shader.data(), shader_size);
    vsFile.close();


    std::ifstream tescFile = std::ifstream("tesc.spv", std::ios::ate | std::ios::binary);

    shader_size = static_cast<size_t>(tescFile.tellg());
    std::vector<char> tesc_shader(shader_size);
    tescFile.seekg(0);
    tescFile.read(tesc_shader.data(), shader_size);
    tescFile.close();

    std::ifstream teseFile = std::ifstream("tese.spv", std::ios::ate | std::ios::binary);

    shader_size = static_cast<size_t>(teseFile.tellg());
    std::vector<char> tese_shader(shader_size);
    teseFile.seekg(0);
    teseFile.read(tese_shader.data(), shader_size);
    teseFile.close();


    std::ifstream fsFile = std::ifstream("frag.spv", std::ios::ate | std::ios::binary);

    shader_size = static_cast<size_t>(fsFile.tellg());
    std::vector<char> frag_shader(shader_size);
    fsFile.seekg(0);
    fsFile.read(frag_shader.data(), shader_size);
    fsFile.close();

    VkShaderModule vsModule  = 0;
    VkShaderModule tcsModule = 0;
    VkShaderModule tesModule = 0;
    VkShaderModule fsModule  = 0;

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};

    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    shaderModuleCreateInfo.codeSize = vert_shader.size();
    shaderModuleCreateInfo.pCode = (uint32_t*)vert_shader.data();
    VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, GetSimpleAllocator(), &vsModule));

    shaderModuleCreateInfo.codeSize = tesc_shader.size();
    shaderModuleCreateInfo.pCode = (uint32_t*)tesc_shader.data();
    VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, GetSimpleAllocator(), &tcsModule));

    shaderModuleCreateInfo.codeSize = tese_shader.size();
    shaderModuleCreateInfo.pCode = (uint32_t*)tese_shader.data();
    VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, GetSimpleAllocator(), &tesModule));

    shaderModuleCreateInfo.codeSize = frag_shader.size();
    shaderModuleCreateInfo.pCode = (uint32_t*)frag_shader.data();
    VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, GetSimpleAllocator(), &fsModule));


    VkPipelineBuilder* pBuilder = VkPipelineBuilder::Create(device, GetSimpleAllocator());

    pBuilder->SetViewportState(256, 256);
    pBuilder->SetShaderState(vsModule, tcsModule, tesModule, fsModule);
    pBuilder->SetVertexState(false);

    pBuilder->SetTessellationState(3);

    *pPipeline = pBuilder->GetPipeline(layout, renderPass, subpassIndex);

    pBuilder->Destroy();


    // Free shader modules no longer needed
    vkDestroyShaderModule(device, vsModule,  GetSimpleAllocator());
    vkDestroyShaderModule(device, tcsModule, GetSimpleAllocator());
    vkDestroyShaderModule(device, tesModule, GetSimpleAllocator());
    vkDestroyShaderModule(device, fsModule,  GetSimpleAllocator());
}



int main()
{
    Ivy::IvyWindow* pWindow = Ivy::IvyWindow::Create(256, 256);
    pWindow->SetWindowName(appInfo.pAppName);

    // Init Vulkan

    VkCoreInfo vulkanInfo = {};
    if (createVulkan(&appInfo, &vulkanInfo) == false)
    {
        return -1;
    }

    VkSwapchainInfo swapchainInfo = {};
    createSwapchain(&vulkanInfo, GetModuleHandle(NULL), pWindow->GetHwnd(), GetSimpleAllocator(), &swapchainInfo);


    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool        = vulkanInfo.vkCommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer commandBuffer = {};
    vkAllocateCommandBuffers(vulkanInfo.vkDevice, &commandBufferAllocateInfo, &commandBuffer);


    ///@todo Create Renderpass / Subpass Generator
    // Create Render Pass
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format         = VK_FORMAT_B8G8R8A8_UNORM;
    colorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments    = &colorAttachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount        = 1;
    renderPassCreateInfo.pAttachments           = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount           = 1;
    renderPassCreateInfo.pSubpasses             = &subpassDescription;

    VkRenderPass renderPass;
    VK_CHECK(vkCreateRenderPass(vulkanInfo.vkDevice, &renderPassCreateInfo, GetSimpleAllocator(), &renderPass));

    // Create Framebuffers
    VkFramebuffer* framebuffers = new VkFramebuffer[swapchainInfo.swapchainImageCount];
    for (uint32_t i = 0; i < swapchainInfo.swapchainImageCount; ++i)
    {
        VkFramebufferCreateInfo framebufferCreateInfo = {};

        framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.width           = 256;
        framebufferCreateInfo.height          = 256;
        framebufferCreateInfo.layers          = 1;
        framebufferCreateInfo.renderPass      = renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments    = &swapchainInfo.pSwapchainImageViews[i];

        VK_CHECK(vkCreateFramebuffer(vulkanInfo.vkDevice, &framebufferCreateInfo, GetSimpleAllocator(), &framebuffers[i]));
    }

    // Create Pipeline

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout      pipelineLayout;

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.size   = 16;
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    createPipelineLayout(vulkanInfo.vkDevice, NULL, 0, &pushConstantRange, 1, &descriptorSetLayout, &pipelineLayout);

    VkPipeline graphicsPipeline;
    createGraphicsPipeline(vulkanInfo.vkDevice, pipelineLayout, renderPass, 0, &graphicsPipeline);


    pWindow->Show();

    BOOL quit = FALSE;
    while (quit == FALSE)
    {
        VkSemaphore presentCompleteSemaphore;
        VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
        presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        presentCompleteSemaphoreCreateInfo.pNext = NULL;
        presentCompleteSemaphoreCreateInfo.flags = 0;

        VK_CHECK(vkCreateSemaphore(vulkanInfo.vkDevice, &presentCompleteSemaphoreCreateInfo, GetSimpleAllocator(), &presentCompleteSemaphore));

        uint32_t imageIndex = 0;
        VK_CHECK(vkAcquireNextImageKHR(vulkanInfo.vkDevice, swapchainInfo.swapchain, UINT64_MAX, presentCompleteSemaphore, 0, &imageIndex));


        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // HW: regenerate per frame vs cache per frame

        VK_CHECK(vkResetCommandPool(vulkanInfo.vkDevice, vulkanInfo.vkCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.framebuffer = framebuffers[imageIndex];
        renderPassBeginInfo.renderPass = renderPass;

        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = 256;
        renderPassBeginInfo.renderArea.extent.height = 256;

        static const VkClearValue FramebufferClearValue = { 1.0f, 1.0f, 0.0, 1.0f };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &FramebufferClearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        ///@todo Abstract out pipeline barriers
        VkImageMemoryBarrier prePresentBarrier = {};
        prePresentBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.pNext               = NULL;
        prePresentBarrier.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
        prePresentBarrier.oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        prePresentBarrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        prePresentBarrier.image               = swapchainInfo.pSwapchainImages[imageIndex];

        VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0,
                             0,
                             NULL,
                             0,
                             NULL,
                             1,
                             pmemory_barrier);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo       = {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;
        submitInfo.pWaitSemaphores    = &presentCompleteSemaphore;
        submitInfo.waitSemaphoreCount = 1;

        VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        submitInfo.pWaitDstStageMask          = &pipe_stage_flags;

        VK_CHECK(vkQueueSubmit(vulkanInfo.vkQueue, 1, &submitInfo, VK_NULL_HANDLE));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains      = &swapchainInfo.swapchain;
        presentInfo.swapchainCount   = 1;
        presentInfo.pImageIndices    = &imageIndex;

        VK_CHECK(vkQueuePresentKHR(vulkanInfo.vkQueue, &presentInfo));

        VK_CHECK(vkQueueWaitIdle(vulkanInfo.vkQueue));

        vkDestroySemaphore(vulkanInfo.vkDevice, presentCompleteSemaphore, GetSimpleAllocator());

        pWindow->ProcessMsg(&quit);
    }

    if (framebuffers != nullptr)
    {
        vkDestroyFramebuffer(vulkanInfo.vkDevice, framebuffers[0], GetSimpleAllocator());
        vkDestroyFramebuffer(vulkanInfo.vkDevice, framebuffers[1], GetSimpleAllocator());

        delete[] framebuffers;
    }


    destroySwapchain(&vulkanInfo, &swapchainInfo, GetSimpleAllocator());
    destroyVulkan(&vulkanInfo, GetSimpleAllocator());

    return 0;
}
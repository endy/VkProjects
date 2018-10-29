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


int main()
{
    Ivy::IvyWindow* pWindow = Ivy::IvyWindow::Create(256, 256);

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

        vkQueueWaitIdle(vulkanInfo.vkQueue);

        vkDestroySemaphore(vulkanInfo.vkDevice, presentCompleteSemaphore, GetSimpleAllocator());

        pWindow->ProcessMsg(&quit);
    }

    destroySwapchain(&vulkanInfo, &swapchainInfo, GetSimpleAllocator());
    destroyVulkan(&vulkanInfo, GetSimpleAllocator());

    return 0;
}
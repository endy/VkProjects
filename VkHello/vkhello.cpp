// Next Steps:
// #1 Refactor resource memory mgmt into a class (resource class owns it but its externally allocated to refactor later?)

// #2
//   Refactor all vulkan config & setup into a separate class to clean up basic setup

// #3
//   Refactor descriptor set creation to a single utility function (creates layout + descriptor set)
//      Thoughts -- maybe generate a list of resources used as input to create the descriptor?
//      Problems -- how to map descriptors & programs?  Maybe need to group descriptor w/ program, program takes precidence


#include <iostream>
#include <fstream>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR 1

#include "vulkan\vulkan.h"

#include "VkBuilder.h"

#include "IvyWindow.h"

// Scene Globals
static const char* VkHelloApplicationName = "VkHello";
static const char* VkHelloEngineName      = "IvyEngine";

static const uint32_t VkHelloImageWidth         = 256;
static const uint32_t VkHelloImageHeight        = 256;
static const VkClearValue FramebufferClearValue = { 1.0f, 1.0f, 0.0, 1.0f };

// VkHello only uses 1 queue
static const uint32_t VkHelloQueueCount       = 1;
static const uint32_t VkHelloDeviceQueueIndex = 0;

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
const uint32_t RequiredLayerCount = 0; // sizeof(RequiredLayerNames) / sizeof(RequiredLayerNames[0]);

struct VkHelloVulkanDeviceInfo
{
    VkInstance                       vkInstance;
    VkPhysicalDevice                 vkPhysicalDevice;
    VkPhysicalDeviceMemoryProperties vkDeviceMemoryProperties;
    VkDevice                         vkDevice;
    VkQueue                          vkQueue;
    VkCommandPool                    vkCommandPool;
};

struct VkHelloSwapchainInfo
{
    VkSurfaceKHR   surface;
    VkSwapchainKHR swapchain;
    VkImage        images[2];
};

void printDeviceMemoryProperties(
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties)
{
    int32_t memory_type_device_local_index = -1;

    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryHeapCount; i++)
    {
        std::cout << "Heap " << i << " flags: " << physicalDeviceMemoryProperties.memoryHeaps[i].flags << std::endl;

        if (physicalDeviceMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            std::cout << "                               " << "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT" << std::endl;
        }

        std::cout << "Heap " << i << " size:  " << physicalDeviceMemoryProperties.memoryHeaps[i].size << std::endl;
    }
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        std::cout << "Memory Type " << i << " Heap Index: " << physicalDeviceMemoryProperties.memoryTypes[i].heapIndex << std::endl;
        std::cout << "Memory Type " << i << " Property Flags:  " << physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags << std::endl;

        if ((memory_type_device_local_index < 0) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            memory_type_device_local_index = i;
        }

        if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            std::cout << "                               " << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << std::endl;
        }
        if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            std::cout << "                               " << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << std::endl;
        }
        if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        {
            std::cout << "                               " << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << std::endl;
        }
        if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
        {
            std::cout << "                               " << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << std::endl;
        }
        if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        {
            std::cout << "                               " << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << std::endl;
        }
    }
}

int32_t memoryTypeIndexWithGivenProperties(
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
    uint32_t                           memTypeBits,
    VkMemoryPropertyFlags            requiredFlags)
{
    // printDeviceMemoryProperties(deviceMemoryProperties);

    int32_t memoryType = -1;
    static const uint32_t MemTypeBitsCount = sizeof(VkMemoryRequirements::memoryTypeBits) * 8;

    for (uint32_t i = 0; i < MemTypeBitsCount; i++)
    {
        if (((memTypeBits >> i) & 0x1) &&
            ((deviceMemoryProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags))
        {
            memoryType = i;
            break;
        }        
    }

    return memoryType;
}


void createFramebuffer(
    VkDevice device,
    VkRenderPass renderPass,
    VkImage image,
    VkImageView* pImageView,
    VkFramebuffer* pFramebuffer)
{
    VkImageViewCreateInfo imageViewCreateInfo = {};

    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;

    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;

    VkResult err = vkCreateImageView(device, &imageViewCreateInfo, NULL, pImageView);

    VkFramebufferCreateInfo framebufferCreateInfo = {};

    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.width = VkHelloImageWidth;
    framebufferCreateInfo.height = VkHelloImageHeight;
    framebufferCreateInfo.layers = 1;

    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = pImageView;

    err = vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, pFramebuffer);

}

void initRenderPassBeginInfo(
    VkRenderPass renderPass,
    VkFramebuffer framebuffer,
    VkRenderPassBeginInfo* renderPassBeginInfo)
{
    renderPassBeginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo->framebuffer = framebuffer;
    renderPassBeginInfo->renderPass = renderPass;

    renderPassBeginInfo->renderArea.offset.x = 0;
    renderPassBeginInfo->renderArea.offset.y = 0;
    renderPassBeginInfo->renderArea.extent.width  = VkHelloImageWidth;
    renderPassBeginInfo->renderArea.extent.height = VkHelloImageHeight;
}


///@todo decouple descriptor layout from push constant layout
///      build one descriptor set up front, use it for everything
///      push constants tied to programs, so pipeline layout + program is a uint
void createPipelineLayout(
    VkDevice device,
    VkDescriptorSetLayout* pDescriptorSetLayout,
    VkPipelineLayout*      pPipelineLayout)
{
    VkResult result = VK_SUCCESS;

    VkDescriptorSetLayoutBinding layoutBinding[2] =
    {
        {
            0,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            NULL
        },
        {
            1,
            VK_DESCRIPTOR_TYPE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            NULL
        }
    };


    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 2;
    descriptorSetLayoutCreateInfo.pBindings = &layoutBinding[0];

    result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, pDescriptorSetLayout);
    
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

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.size   = 16;
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    layoutCreateInfo.pushConstantRangeCount = 1;
    layoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    result = vkCreatePipelineLayout(device, &layoutCreateInfo, NULL, pPipelineLayout);
}

///@todo create shader to wrap SPV loading & module creation, create program to group them?
VkResult createGraphicsPipeline(
    VkDevice device,
    VkPipelineLayout layout,
    VkRenderPass renderPass,
    uint32_t     subpassIndex,
    VkPipeline* pPipeline)
{

    VkResult vkResult = VK_SUCCESS;

    std::ifstream vsFile = std::ifstream("vert.spv", std::ios::ate | std::ios::binary);

    size_t shader_size = static_cast<size_t>(vsFile.tellg());
    std::vector<char> vert_shader(shader_size);
    vsFile.seekg(0);
    vsFile.read(vert_shader.data(), shader_size);
    vsFile.close();

    std::ifstream fsFile = std::ifstream("frag.spv", std::ios::ate | std::ios::binary);

    shader_size = static_cast<size_t>(fsFile.tellg());
    std::vector<char> frag_shader(shader_size);
    fsFile.seekg(0);
    fsFile.read(frag_shader.data(), shader_size);
    fsFile.close();

    VkShaderModule vsModule = 0;
    VkShaderModule fsModule = 0;

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};

    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    shaderModuleCreateInfo.codeSize = vert_shader.size();
    shaderModuleCreateInfo.pCode = (uint32_t*) vert_shader.data();
    vkResult = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &vsModule);

    shaderModuleCreateInfo.codeSize = frag_shader.size();
    shaderModuleCreateInfo.pCode = (uint32_t*)frag_shader.data();
    vkResult = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &fsModule);
 
    VkPipelineBuilder* pBuilder = VkPipelineBuilder::Create(device);

    pBuilder->SetViewportState(VkHelloImageWidth, VkHelloImageHeight);
    pBuilder->SetShaderState(vsModule, fsModule);

    *pPipeline = pBuilder->GetPipeline(layout, renderPass, subpassIndex);
    
    pBuilder->Destroy();


    // Free shader modules no longer needed
    vkDestroyShaderModule(device, vsModule, NULL);
    vkDestroyShaderModule(device, fsModule, NULL);

    return vkResult;
}


Resource* createTexture(
    VkHelloVulkanDeviceInfo* pVulkanInfo,
    VkCommandBuffer cmdBuf)
{
    // create texture

    VkResult err;
    Resource* pTexture = NULL;

    ResourceBuilder* pResBuilder = ResourceBuilder::Create(pVulkanInfo->vkDevice);


    pResBuilder->SetImageDimensions(VkHelloImageWidth, VkHelloImageHeight, 1);
    pResBuilder->SetImageStaging(false);
    pTexture = pResBuilder->GetImageResource();
    
    
    VkMemoryRequirements memoryRequirements = {};
    vkGetImageMemoryRequirements(pVulkanInfo->vkDevice, pTexture->image, &memoryRequirements);
    
    // Assign image to a device
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    
    allocateInfo.memoryTypeIndex = memoryTypeIndexWithGivenProperties(pVulkanInfo->vkDeviceMemoryProperties, 
                                        memoryRequirements.memoryTypeBits,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkDeviceMemory textureMemory;
    err = vkAllocateMemory(pVulkanInfo->vkDevice, &allocateInfo, NULL, &textureMemory);
    err = vkBindImageMemory(pVulkanInfo->vkDevice, pTexture->image, textureMemory, 0);
    
    
    pResBuilder->SetImageStaging(true);
    Resource* pStagingRes = pResBuilder->GetImageResource();
    
    
    vkGetImageMemoryRequirements(pVulkanInfo->vkDevice, pStagingRes->image, &memoryRequirements);
    
    allocateInfo.memoryTypeIndex = memoryTypeIndexWithGivenProperties(pVulkanInfo->vkDeviceMemoryProperties,
        memoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    VkDeviceMemory stagingMemory;
    err = vkAllocateMemory(pVulkanInfo->vkDevice, &allocateInfo, NULL, &stagingMemory);
    err = vkBindImageMemory(pVulkanInfo->vkDevice, pStagingRes->image, stagingMemory, 0);
    
    uint32_t* pTexelData = NULL;
    err = vkMapMemory(pVulkanInfo->vkDevice, stagingMemory, 0, VK_WHOLE_SIZE, 0, reinterpret_cast<void**>(&pTexelData));
    
    for (uint32_t h = 0; h < VkHelloImageHeight / 2; h++)
    { 
        for (uint32_t w = 0; w < VkHelloImageWidth / 2; w++)
        {
            uint32_t ti = (h * VkHelloImageWidth) + w;
            pTexelData[ti] = ((uint32_t) ((w*255.0) / (float)VkHelloImageWidth) << 0)  |
                             ((uint32_t) ((h*255.0) / (float)VkHelloImageHeight)  << 8)  |
                             (0x0 << 16) |
                             (0xFF << 24);
        }
    }
    
    vkUnmapMemory(pVulkanInfo->vkDevice, stagingMemory);
    
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.image = pStagingRes->image;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;

    VkImageCopy region = { 0 };
    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    
    region.extent.depth = 1;
    region.extent.width = VkHelloImageWidth;
    region.extent.height = VkHelloImageHeight;
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    err = vkBeginCommandBuffer(cmdBuf, &beginInfo);
    
    vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
    
    imageBarrier.image = pTexture->image;
    imageBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; ///@todo this probably isn't correct...
    vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
    
    
    vkCmdCopyImage(cmdBuf,
                   pStagingRes->image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   pTexture->image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,
                   &region);
    err = vkEndCommandBuffer(cmdBuf);
    
    VkSubmitInfo submitInfo = {};
    
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    submitInfo.pWaitSemaphores = NULL; // &presentCompleteSemaphore;
    submitInfo.waitSemaphoreCount = 0;
    
    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    submitInfo.pWaitDstStageMask = &pipe_stage_flags;
    
    err = vkQueueSubmit(pVulkanInfo->vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(pVulkanInfo->vkQueue);

    
    
    vkFreeMemory(pVulkanInfo->vkDevice, stagingMemory, NULL);
    vkDestroyImage(pVulkanInfo->vkDevice, pStagingRes->image, NULL);
    delete pStagingRes;
    pStagingRes = NULL;
    
    ///@todo make generic creation paths & copy paths

    return pTexture;
}


void createSwapchain(
    VkHelloVulkanDeviceInfo* pVulkanInfo,
    HINSTANCE hInstance,
    HWND hWnd,
    VkHelloSwapchainInfo* pSwapchainInfo)
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKHR = {};

    surfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfoKHR.hinstance = hInstance;
    surfaceCreateInfoKHR.hwnd = hWnd;

    VkResult result = vkCreateWin32SurfaceKHR(pVulkanInfo->vkInstance, &surfaceCreateInfoKHR, NULL, &pSwapchainInfo->surface);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pVulkanInfo->vkPhysicalDevice, pSwapchainInfo->surface, &surfaceCapabilities);

    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pVulkanInfo->vkPhysicalDevice, pSwapchainInfo->surface, &surfaceFormatCount, NULL);
    VkSurfaceFormatKHR* surfaceFormats = new VkSurfaceFormatKHR[surfaceFormatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(pVulkanInfo->vkPhysicalDevice, pSwapchainInfo->surface, &surfaceFormatCount, &surfaceFormats[0]);

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pVulkanInfo->vkPhysicalDevice, pSwapchainInfo->surface, &presentModeCount, NULL);
    VkPresentModeKHR* presentModes = new VkPresentModeKHR[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(pVulkanInfo->vkPhysicalDevice, pSwapchainInfo->surface, &presentModeCount, &presentModes[0]);

    VkBool32 supported = FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(pVulkanInfo->vkPhysicalDevice, 0, pSwapchainInfo->surface, &supported);

    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.minImageCount = surfaceCapabilities.minImageCount;
    swapchainCreateInfoKHR.surface = pSwapchainInfo->surface;
    swapchainCreateInfoKHR.imageExtent.width = surfaceCapabilities.currentExtent.width;
    swapchainCreateInfoKHR.imageExtent.height = surfaceCapabilities.currentExtent.width;
    swapchainCreateInfoKHR.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    result = vkCreateSwapchainKHR(pVulkanInfo->vkDevice, &swapchainCreateInfoKHR, NULL, &pSwapchainInfo->swapchain);

    delete[] surfaceFormats;
    delete[] presentModes;
}

// Checks if all app-required extensions are available, returns false on first one found not to be present, else true
bool checkRequiredInstanceExtensionsAvailable()
{
    bool success = true;

    uint32_t instanceExtensionCount = 0;
    VkResult err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    VkExtensionProperties* pInstanceExtensions = new VkExtensionProperties[instanceExtensionCount];
    err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, pInstanceExtensions);

    for (uint32_t ri = 0; ri < RequiredInstanceExtensionCount; ++ri)
    {
        uint32_t requiredNameLength = strlen(RequiredInstanceExtensionNames[ri]);
        bool foundExtension = false;
        for (uint32_t qi = 0; qi < instanceExtensionCount; ++qi)
        {
            if ((strlen(pInstanceExtensions[qi].extensionName) == requiredNameLength) &&
                (strncmp(pInstanceExtensions[qi].extensionName, RequiredInstanceExtensionNames[ri], requiredNameLength) == 0))
            {
                foundExtension = true;
                break;
            }
        }

        if (foundExtension == false)
        {
            std::cout << "Required extension " << RequiredInstanceExtensionNames[ri] << " is not available." << std::endl;
            success = false;
            break;
        }
    }

    delete[] pInstanceExtensions;

    return success;
}

bool checkRequiredInstanceLayersAvailable()
{
    bool success = true;

    uint32_t layerPropertyCount = 0;
    vkEnumerateInstanceLayerProperties(&layerPropertyCount, NULL);
    VkLayerProperties* pLayerProperties = new VkLayerProperties[layerPropertyCount];
    vkEnumerateInstanceLayerProperties(&layerPropertyCount, &pLayerProperties[0]);

    for (uint32_t ri = 0; ri < RequiredLayerCount; ++ri)
    {
        uint32_t requiredNameLength = strlen(RequiredLayerNames[ri]);
        bool foundLayer = false;
        for (uint32_t qi = 0; qi < layerPropertyCount; ++qi)
        {
            if ((strlen(pLayerProperties[qi].layerName) == requiredNameLength) &&
                (strncmp(pLayerProperties[qi].layerName, RequiredLayerNames[ri], requiredNameLength) == 0))
            {
                foundLayer = true;
                break;
            }
        }

        if (foundLayer == false)
        {
            std::cout << "Required layer " << RequiredLayerNames[ri] << " is not available." << std::endl;
            success = false;
            break;
        }
    }

    delete[] pLayerProperties;

    return success;
}

bool checkRequiredDeviceExtensionsAvailable(
    VkPhysicalDevice physicalDevice)
{
    bool success = true;

    uint32_t deviceExtensionPropertyCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionPropertyCount, NULL);
    VkExtensionProperties* pDeviceExtensionProperties = new VkExtensionProperties[deviceExtensionPropertyCount];
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionPropertyCount, &pDeviceExtensionProperties[0]);

    for (uint32_t ri = 0; ri < RequiredDeviceExtensionCount; ++ri)
    {
        uint32_t requiredNameLength = strlen(RequiredDeviceExtensionNames[ri]);
        bool foundLayer = false;
        for (uint32_t qi = 0; qi < deviceExtensionPropertyCount; ++qi)
        {
            if ((strlen(pDeviceExtensionProperties[qi].extensionName) == requiredNameLength) &&
                (strncmp(pDeviceExtensionProperties[qi].extensionName, RequiredDeviceExtensionNames[ri], requiredNameLength) == 0))
            {
                foundLayer = true;
                break;
            }
        }

        if (foundLayer == false)
        {
            std::cout << "Required device extension " << RequiredDeviceExtensionNames[ri] << " is not available." << std::endl;
            success = false;
            break;
        }
    }

    delete[] pDeviceExtensionProperties;

    return success;
}



// Heavy loading for Vulkan Initialization:
//  (0. Required extension support checking)
//   1. Create instance and query physical device
//   2. Create device, queue, and command pool
bool initVulkan(VkHelloVulkanDeviceInfo* pVulkanInfo)
{
    if ((checkRequiredInstanceExtensionsAvailable() == false) ||
        (checkRequiredInstanceLayersAvailable() == false))
    {
        return false;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = VkHelloApplicationName;
    appInfo.pEngineName = VkHelloEngineName;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.ppEnabledLayerNames = &RequiredLayerNames[0];
    instanceInfo.enabledLayerCount = RequiredLayerCount;
    instanceInfo.ppEnabledExtensionNames = &RequiredInstanceExtensionNames[0];
    instanceInfo.enabledExtensionCount = RequiredInstanceExtensionCount;

    VkResult result = vkCreateInstance(&instanceInfo, NULL, &pVulkanInfo->vkInstance);

    // Currently assume 1 device
    uint32_t physicalDeviceCount = 1;
    result = vkEnumeratePhysicalDevices(pVulkanInfo->vkInstance, &physicalDeviceCount, &pVulkanInfo->vkPhysicalDevice);

    // 
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(pVulkanInfo->vkPhysicalDevice, &physicalDeviceProperties);

    if (checkRequiredDeviceExtensionsAvailable(pVulkanInfo->vkPhysicalDevice) == false)
    {
        vkDestroyInstance(pVulkanInfo->vkInstance, NULL);
        return false;
    }

    vkGetPhysicalDeviceMemoryProperties(pVulkanInfo->vkPhysicalDevice, &pVulkanInfo->vkDeviceMemoryProperties);

    uint32_t queueFamilyPropertiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(pVulkanInfo->vkPhysicalDevice, &queueFamilyPropertiesCount, NULL);
    VkQueueFamilyProperties* pQueueFamilyProperties = new VkQueueFamilyProperties[queueFamilyPropertiesCount];

    vkGetPhysicalDeviceQueueFamilyProperties(pVulkanInfo->vkPhysicalDevice, &queueFamilyPropertiesCount, &pQueueFamilyProperties[0]);

    int32_t graphicsComputeQueueFamilyIndex = -1;

    // get graphics queue that supports presents
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; ++i)
    {
        if (((pQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) ||
            ((pQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
        {
            continue;
        }

        if (vkGetPhysicalDeviceWin32PresentationSupportKHR(pVulkanInfo->vkPhysicalDevice, i) == VK_FALSE)
        {
            continue;
        }

        graphicsComputeQueueFamilyIndex = i;
    }

    // Create VkDevice and queue for that device
    float queuePriorities[] = { 1.0 };

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsComputeQueueFamilyIndex;
    queueCreateInfo.queueCount       = VkHelloQueueCount;
    queueCreateInfo.pQueuePriorities = &queuePriorities[0];

    VkDeviceCreateInfo deviceCreateInfo      = {};
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount    = VkHelloQueueCount;
    deviceCreateInfo.ppEnabledExtensionNames = &RequiredDeviceExtensionNames[0];
    deviceCreateInfo.enabledExtensionCount   = sizeof(RequiredDeviceExtensionNames) / sizeof(char*);

    vkCreateDevice(pVulkanInfo->vkPhysicalDevice, &deviceCreateInfo, NULL, &pVulkanInfo->vkDevice);
    vkGetDeviceQueue(pVulkanInfo->vkDevice, graphicsComputeQueueFamilyIndex, VkHelloDeviceQueueIndex, &pVulkanInfo->vkQueue);

    // Create Command Pool
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = graphicsComputeQueueFamilyIndex;
    commandPoolCreateInfo.flags = 0;  // flags=0 Choosing to reset buffers in bulk, not per command buffer

    result = vkCreateCommandPool(pVulkanInfo->vkDevice, &commandPoolCreateInfo, NULL, &pVulkanInfo->vkCommandPool);

    return true;
}

int main()
{
    VkResult err;
    Ivy::IvyWindow* pWindow = Ivy::IvyWindow::Create(VkHelloImageWidth, VkHelloImageHeight);

    // Init Vulkan
    VkHelloVulkanDeviceInfo vulkanInfo = {};
    if (initVulkan(&vulkanInfo) == false)
    {
        return -1;
    }

    // Init Swapchain
    VkHelloSwapchainInfo swapchainInfo = {};
    createSwapchain(&vulkanInfo, GetModuleHandle(NULL), pWindow->GetHwnd(), &swapchainInfo);

    uint32_t imageCount = 0;
    err = vkGetSwapchainImagesKHR(vulkanInfo.vkDevice, swapchainInfo.swapchain, &imageCount, NULL);

    VkImage* swapchainImages = new VkImage[imageCount];
    err = vkGetSwapchainImagesKHR(vulkanInfo.vkDevice, swapchainInfo.swapchain, &imageCount, &swapchainImages[0]);

    ///@todo aggregate swapchain images/framebuffers somehow



    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool        = vulkanInfo.vkCommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer commandBuffer = {};
    vkAllocateCommandBuffers(vulkanInfo.vkDevice, &commandBufferAllocateInfo, &commandBuffer);

    // Create Render Pass
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format = VK_FORMAT_B8G8R8A8_UNORM;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments    = &colorAttachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    VkRenderPass renderPass;
    err = vkCreateRenderPass(vulkanInfo.vkDevice, &renderPassCreateInfo, NULL, &renderPass);

    // Create Framebuffers
    VkImageView* imageViews = new VkImageView[imageCount];
    VkFramebuffer* framebuffers = new VkFramebuffer[imageCount];
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        createFramebuffer(vulkanInfo.vkDevice, renderPass, swapchainImages[i], &imageViews[i], &framebuffers[i]);
    }


    Resource* pTexture = createTexture(&vulkanInfo, commandBuffer);

    VkImageView textureImageView;
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.image = pTexture->image;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    err = vkCreateImageView(vulkanInfo.vkDevice, &imageViewCreateInfo, NULL, &textureImageView);


    VkSampler   textureSampler;
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    err = vkCreateSampler(vulkanInfo.vkDevice, &samplerCreateInfo, NULL, &textureSampler);


    // Create Pipeline

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout      pipelineLayout;
    createPipelineLayout(vulkanInfo.vkDevice, &descriptorSetLayout, &pipelineLayout);

    VkPipeline graphicsPipeline;
    createGraphicsPipeline(vulkanInfo.vkDevice, pipelineLayout, renderPass, 0, &graphicsPipeline);


    // Allocate Descriptor Set

    VkDescriptorPoolSize vkPoolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
        { VK_DESCRIPTOR_TYPE_SAMPLER,       1 }
    };

    VkDescriptorPoolCreateInfo descPoolCreateInfo = {};
    descPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolCreateInfo.maxSets = 1;
    descPoolCreateInfo.poolSizeCount = (sizeof(vkPoolSizes) / sizeof(VkDescriptorPoolSize));
    descPoolCreateInfo.pPoolSizes    = &vkPoolSizes[0];

    VkDescriptorPool descPool;
    err = vkCreateDescriptorPool(vulkanInfo.vkDevice, &descPoolCreateInfo, NULL, &descPool);

    VkDescriptorSetAllocateInfo descSetAllocInfo = {};
    descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descSetAllocInfo.descriptorSetCount = 1;
    descSetAllocInfo.pSetLayouts = &descriptorSetLayout;
    descSetAllocInfo.descriptorPool = descPool;

    VkDescriptorSet descSet;
    err = vkAllocateDescriptorSets(vulkanInfo.vkDevice, &descSetAllocInfo, &descSet);

    // Update Descriptor Set
    VkDescriptorImageInfo descriptorImageInfo[2];
    descriptorImageInfo[0].imageView   = textureImageView;
    descriptorImageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    descriptorImageInfo[1].sampler   = textureSampler;

    VkWriteDescriptorSet descriptorWrites[2] =
    {
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            descSet,
            0,
            0,
            1,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            &descriptorImageInfo[0],
            NULL,
            NULL
        },
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            descSet,
            1,
            0,
            1,
            VK_DESCRIPTOR_TYPE_SAMPLER,
            &descriptorImageInfo[1],
            NULL,
            NULL
        }
    };

    vkUpdateDescriptorSets(vulkanInfo.vkDevice, 2, descriptorWrites, 0, NULL);

    ResourceBuilder* pResBuilder = ResourceBuilder::Create(vulkanInfo.vkDevice);

    uint32_t vbSize = 16 * 96; // 96 verts / 3=32 tris
    pResBuilder->SetBufferSize(16 * 96);

    Resource* pVertexBuffer = pResBuilder->GetBufferResource();

    pResBuilder->Destroy();
    pResBuilder = NULL;

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(vulkanInfo.vkDevice, pVertexBuffer->buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;

    VkDeviceMemory vertexBufferMemory;
    allocateInfo.memoryTypeIndex = memoryTypeIndexWithGivenProperties(vulkanInfo.vkDeviceMemoryProperties,
                                                                      memoryRequirements.memoryTypeBits,
                                                                      (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    err = vkAllocateMemory(vulkanInfo.vkDevice, &allocateInfo, NULL, &vertexBufferMemory);
    err = vkBindBufferMemory(vulkanInfo.vkDevice, pVertexBuffer->buffer, vertexBufferMemory, 0);

    float vbData[] = { -0.9f, -0.9f,
                        0.9f, -0.9f,
                       -0.9f, 0.9f,
                       0.9f, -0.9f,
                       0.9f,  0.9f,
                       -0.9f,  0.9f,};

    float* pData;
    vkMapMemory(vulkanInfo.vkDevice, vertexBufferMemory, 0, memoryRequirements.size, 0, (void**)&pData);
    memcpy(pData, &vbData[0], sizeof(vbData));
    vkUnmapMemory(vulkanInfo.vkDevice, vertexBufferMemory);





    // Scene Loop Setup
    bool flip = true;
    float pushConsts[] = { 0.1f, 0.2f, 2.0f, 1.0f,
                           0.0f, 0.0f, 0.0f, 0.0f  };
    pWindow->Show();

    BOOL quit = FALSE;

    while (!quit)
    {
        VkSemaphore presentCompleteSemaphore;
        VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
        presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        presentCompleteSemaphoreCreateInfo.pNext = NULL;
        presentCompleteSemaphoreCreateInfo.flags = 0;

        err = vkCreateSemaphore(vulkanInfo.vkDevice, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);

        uint32_t imageIndex = 0;
        err = vkAcquireNextImageKHR(vulkanInfo.vkDevice, swapchainInfo.swapchain, UINT64_MAX, presentCompleteSemaphore, 0, &imageIndex);

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        initRenderPassBeginInfo(renderPass, framebuffers[imageIndex], &renderPassBeginInfo);


        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &FramebufferClearValue;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // HW: regenerate per frame vs cache per frame

        err = vkResetCommandPool(vulkanInfo.vkDevice, vulkanInfo.vkCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        err = vkBeginCommandBuffer(commandBuffer, &beginInfo);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSet, 0, NULL);

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        uint32_t pushConstIndex = (flip) ? 0 : 4;
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16, &pushConsts[pushConstIndex]);
        //flip = !flip;

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &pVertexBuffer->buffer, &offset);


        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        ///@todo Abstract out pipeline barriers
        VkImageMemoryBarrier prePresentBarrier = {};
        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.pNext = NULL;
        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        prePresentBarrier.image = swapchainImages[imageIndex];
        VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
            NULL, 1, pmemory_barrier);


        err = vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};

        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
        submitInfo.waitSemaphoreCount = 1;

        VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        submitInfo.pWaitDstStageMask = &pipe_stage_flags;

        err = vkQueueSubmit(vulkanInfo.vkQueue, 1, &submitInfo, VK_NULL_HANDLE);

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType          = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains    = &swapchainInfo.swapchain;
        presentInfo.swapchainCount = 1;
        
        presentInfo.pImageIndices = &imageIndex;

        err = vkQueuePresentKHR(vulkanInfo.vkQueue, &presentInfo);

        vkQueueWaitIdle(vulkanInfo.vkQueue);

        vkDestroySemaphore(vulkanInfo.vkDevice, presentCompleteSemaphore, NULL);


        pWindow->ProcessMsg(&quit);
    }

    // Free Resources

    // Wait, full stop, before proceeding to free/destroy objects
    err = vkDeviceWaitIdle(vulkanInfo.vkDevice);

    vkFreeMemory(vulkanInfo.vkDevice, vertexBufferMemory, NULL);
    vkDestroyBuffer(vulkanInfo.vkDevice, pVertexBuffer->buffer, NULL);

    vkDestroyFramebuffer(vulkanInfo.vkDevice, framebuffers[0], NULL);
    vkDestroyFramebuffer(vulkanInfo.vkDevice, framebuffers[1], NULL);

    vkDestroyImageView(vulkanInfo.vkDevice, imageViews[0], NULL);
    vkDestroyImageView(vulkanInfo.vkDevice, imageViews[1], NULL);

    vkDestroyPipeline(vulkanInfo.vkDevice, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(vulkanInfo.vkDevice, pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(vulkanInfo.vkDevice, descriptorSetLayout, NULL);

    vkDestroyRenderPass(vulkanInfo.vkDevice, renderPass, NULL);

    // Q: If calling reset with release flag on the pool, do you need to free the individual command buffs?
    // A: Reseting the buffer is not the same as freeing it.
    vkResetCommandPool(vulkanInfo.vkDevice, vulkanInfo.vkCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    vkFreeCommandBuffers(vulkanInfo.vkDevice, vulkanInfo.vkCommandPool, 1, &commandBuffer); 

    vkDestroyCommandPool(vulkanInfo.vkDevice, vulkanInfo.vkCommandPool, NULL);
    vkDestroySwapchainKHR(vulkanInfo.vkDevice, swapchainInfo.swapchain, NULL);
    vkDestroySurfaceKHR(vulkanInfo.vkInstance, swapchainInfo.surface, NULL);
    vkDestroyDevice(vulkanInfo.vkDevice, NULL);
    vkDestroyInstance(vulkanInfo.vkInstance, NULL);

    std::cout << "Bye Bye" << std::endl;
    return 0;
}
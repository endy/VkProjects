///////////////////////////////////////////////////////////////////////////////////////////////////
///
///     VkHello
///
///     Copyright 2018, Brandon Light
///     All rights reserved.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

///@todo Configure this for portability
#define VK_USE_PLATFORM_WIN32_KHR 1

#include "vulkan\vulkan.h"


#include <iostream>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////////////////////////
///   Common Structures
///////////////////////////////////////////////////////////////////////////////////////////////////
struct AppInfo
{
    const char*     pAppName;
    const char*     pEngineName;

    uint32_t        apiVersionRequired;

    const char**    ppRequiredInstanceExtensionList;
    const uint32_t  requiredInstanceExtensionCount;
    const char**    ppRequiredInstanceLayerList;
    const uint32_t  requiredInstanceLayerCount;
    const char**    ppRequiredDeviceExtensionList;
    const uint32_t  requiredDeviceExtensionCount;

    const VkAllocationCallbacks* pAllocator;
};

struct VkCoreInfo
{
    VkInstance                       vkInstance;
    VkPhysicalDevice                 vkPhysicalDevice;
    VkPhysicalDeviceMemoryProperties vkDeviceMemoryProperties;
    VkDevice                         vkDevice;
    VkQueue                          vkQueue;
    VkCommandPool                    vkCommandPool;
};

struct VkSwapchainInfo
{
    VkSurfaceKHR   surface;
    VkSwapchainKHR swapchain;

    uint32_t       swapchainImageCount;
    VkImage*       pSwapchainImages;
    VkImageView*   pSwapchainImageViews;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///   Useful Macros
///////////////////////////////////////////////////////////////////////////////////////////////////
#define VK_CHECK(x)  (assert((x) == VK_SUCCESS))


///////////////////////////////////////////////////////////////////////////////////////////////////
///   Utility Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
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
    uint32_t                         memTypeBits,
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


// Checks if all app-required extensions are available, returns false on first one found not to be present, else true
bool checkRequiredInstanceExtensionsAvailable(
    const char* requiredInstanceExtensionList[],
    uint32_t requiredInstanceExtensionCount)
{
    bool success = true;

    uint32_t instanceExtensionCount = 0;
    VkResult err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    VkExtensionProperties* pInstanceExtensions = new VkExtensionProperties[instanceExtensionCount];
    err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, pInstanceExtensions);

    for (uint32_t ri = 0; ri < requiredInstanceExtensionCount; ++ri)
    {
        uint32_t requiredNameLength = strlen(requiredInstanceExtensionList[ri]);
        bool foundExtension = false;
        for (uint32_t qi = 0; qi < instanceExtensionCount; ++qi)
        {
            if ((strlen(pInstanceExtensions[qi].extensionName) == requiredNameLength) &&
                (strncmp(pInstanceExtensions[qi].extensionName, requiredInstanceExtensionList[ri], requiredNameLength) == 0))
            {
                foundExtension = true;
                break;
            }
        }

        if (foundExtension == false)
        {
            std::cout << "Required extension " << requiredInstanceExtensionList[ri] << " is not available." << std::endl;
            success = false;
            break;
        }
    }

    delete[] pInstanceExtensions;

    return success;
}

bool checkRequiredInstanceLayersAvailable(
    const char* requiredInstanceLayers[],
    uint32_t requiredInstanceLayerCount)
{
    bool success = true;

    uint32_t layerPropertyCount = 0;
    vkEnumerateInstanceLayerProperties(&layerPropertyCount, NULL);
    VkLayerProperties* pLayerProperties = new VkLayerProperties[layerPropertyCount];
    vkEnumerateInstanceLayerProperties(&layerPropertyCount, &pLayerProperties[0]);

    for (uint32_t ri = 0; ri < requiredInstanceLayerCount; ++ri)
    {
        uint32_t requiredNameLength = strlen(requiredInstanceLayers[ri]);
        bool foundLayer = false;
        for (uint32_t qi = 0; qi < layerPropertyCount; ++qi)
        {
            if ((strlen(pLayerProperties[qi].layerName) == requiredNameLength) &&
                (strncmp(pLayerProperties[qi].layerName, requiredInstanceLayers[ri], requiredNameLength) == 0))
            {
                foundLayer = true;
                break;
            }
        }

        if (foundLayer == false)
        {
            std::cout << "Required layer " << requiredInstanceLayers[ri] << " is not available." << std::endl;
            success = false;
            break;
        }
    }

    delete[] pLayerProperties;

    return success;
}

bool checkRequiredDeviceExtensionsAvailable(
    VkPhysicalDevice physicalDevice,
    const char* requiredDeviceExtensionList[],
    uint32_t requiredDeviceExtensionCount)
{
    bool success = true;

    uint32_t deviceExtensionPropertyCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionPropertyCount, NULL);
    VkExtensionProperties* pDeviceExtensionProperties = new VkExtensionProperties[deviceExtensionPropertyCount];
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionPropertyCount, &pDeviceExtensionProperties[0]);

    for (uint32_t ri = 0; ri < requiredDeviceExtensionCount; ++ri)
    {
        uint32_t requiredNameLength = strlen(requiredDeviceExtensionList[ri]);
        bool foundLayer = false;
        for (uint32_t qi = 0; qi < deviceExtensionPropertyCount; ++qi)
        {
            if ((strlen(pDeviceExtensionProperties[qi].extensionName) == requiredNameLength) &&
                (strncmp(pDeviceExtensionProperties[qi].extensionName, requiredDeviceExtensionList[ri], requiredNameLength) == 0))
            {
                foundLayer = true;
                break;
            }
        }

        if (foundLayer == false)
        {
            std::cout << "Required device extension " << requiredDeviceExtensionList[ri] << " is not available." << std::endl;
            success = false;
            break;
        }
    }

    delete[] pDeviceExtensionProperties;

    return success;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///   Core Setup Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

// Heavy loading for Vulkan Initialization:
//  (0. Required extension support checking)
//   1. Create instance and query physical device
//   2. Create device, queue, and command pool
bool createVulkan(
    const AppInfo* pAppInfo,
    VkCoreInfo* pVulkanInfo
    )
{
    if ((checkRequiredInstanceExtensionsAvailable(pAppInfo->ppRequiredInstanceExtensionList,
                                                  pAppInfo->requiredInstanceExtensionCount) == false) ||
        (checkRequiredInstanceLayersAvailable(pAppInfo->ppRequiredInstanceLayerList,
                                              pAppInfo->requiredInstanceLayerCount) == false))
    {
        return false;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName  = pAppInfo->pAppName;
    appInfo.pEngineName       = pAppInfo->pEngineName;
    appInfo.apiVersion        = pAppInfo->apiVersionRequired;

    VkInstanceCreateInfo instanceInfo    = {};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.flags                   = 0;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.ppEnabledLayerNames     = pAppInfo->ppRequiredInstanceLayerList;
    instanceInfo.enabledLayerCount       = pAppInfo->requiredInstanceLayerCount;
    instanceInfo.ppEnabledExtensionNames = pAppInfo->ppRequiredInstanceExtensionList;
    instanceInfo.enabledExtensionCount   = pAppInfo->requiredInstanceExtensionCount;

    VkResult result = vkCreateInstance(&instanceInfo, pAppInfo->pAllocator, &pVulkanInfo->vkInstance);

    // Currently assume 1 device
    uint32_t physicalDeviceCount = 1;
    result = vkEnumeratePhysicalDevices(pVulkanInfo->vkInstance, &physicalDeviceCount, &pVulkanInfo->vkPhysicalDevice);

    // 
    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(pVulkanInfo->vkPhysicalDevice, &physicalDeviceProperties);

    if (checkRequiredDeviceExtensionsAvailable(pVulkanInfo->vkPhysicalDevice,
                                               pAppInfo->ppRequiredDeviceExtensionList,
                                               pAppInfo->requiredDeviceExtensionCount) == false)
    {
        vkDestroyInstance(pVulkanInfo->vkInstance, pAppInfo->pAllocator);
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

    ///@todo Allow for configurable queue priorities
    float queuePriorities[] = { 1.0 };

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsComputeQueueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriorities[0];

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount    = 1;
    deviceCreateInfo.ppEnabledExtensionNames = pAppInfo->ppRequiredDeviceExtensionList;
    deviceCreateInfo.enabledExtensionCount   = pAppInfo->requiredDeviceExtensionCount;

    vkCreateDevice(pVulkanInfo->vkPhysicalDevice, &deviceCreateInfo, pAppInfo->pAllocator, &pVulkanInfo->vkDevice);

    ///@todo Query the queue with the properties we need to select the index
    const uint32_t DeviceQueueIndex = 0;
    vkGetDeviceQueue(pVulkanInfo->vkDevice, graphicsComputeQueueFamilyIndex, DeviceQueueIndex, &pVulkanInfo->vkQueue);

    // Create Command Pool
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = graphicsComputeQueueFamilyIndex;
    commandPoolCreateInfo.flags            = 0;  // flags=0 Choosing to reset buffers in bulk, not per command buffer

    result = vkCreateCommandPool(pVulkanInfo->vkDevice, &commandPoolCreateInfo, pAppInfo->pAllocator, &pVulkanInfo->vkCommandPool);

    return true;
}

void destroyVulkan(
    VkCoreInfo* pVulkanInfo,
    const VkAllocationCallbacks* pAllocator)
{
    vkDestroyCommandPool(pVulkanInfo->vkDevice, pVulkanInfo->vkCommandPool, pAllocator);
    vkDestroyDevice(pVulkanInfo->vkDevice, pAllocator);
    vkDestroyInstance(pVulkanInfo->vkInstance, pAllocator);
}


void createSwapchain(
    VkCoreInfo*                  pVulkanInfo,
    HINSTANCE                    hInstance,
    HWND                         hWnd,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainInfo*             pSwapchainInfo)
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKHR = {};

    surfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfoKHR.hinstance = hInstance;
    surfaceCreateInfoKHR.hwnd = hWnd;

    VkResult result = vkCreateWin32SurfaceKHR(pVulkanInfo->vkInstance, &surfaceCreateInfoKHR, pAllocator, &pSwapchainInfo->surface);

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
    swapchainCreateInfoKHR.sType                = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.minImageCount        = surfaceCapabilities.minImageCount;
    swapchainCreateInfoKHR.surface              = pSwapchainInfo->surface;
    swapchainCreateInfoKHR.imageExtent.width    = surfaceCapabilities.currentExtent.width;
    swapchainCreateInfoKHR.imageExtent.height   = surfaceCapabilities.currentExtent.height;
    swapchainCreateInfoKHR.imageFormat          = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfoKHR.imageUsage           = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.imageColorSpace      = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfoKHR.imageArrayLayers     = 1;
    swapchainCreateInfoKHR.compositeAlpha       = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.preTransform         = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfoKHR.presentMode          = VK_PRESENT_MODE_FIFO_KHR;

    VK_CHECK(vkCreateSwapchainKHR(pVulkanInfo->vkDevice, &swapchainCreateInfoKHR, NULL, &pSwapchainInfo->swapchain));

    delete[] surfaceFormats;
    delete[] presentModes;


    VK_CHECK(vkGetSwapchainImagesKHR(pVulkanInfo->vkDevice, pSwapchainInfo->swapchain, &pSwapchainInfo->swapchainImageCount, NULL));

    pSwapchainInfo->pSwapchainImages = new VkImage[pSwapchainInfo->swapchainImageCount];
    VK_CHECK(vkGetSwapchainImagesKHR(pVulkanInfo->vkDevice, pSwapchainInfo->swapchain, &pSwapchainInfo->swapchainImageCount, &pSwapchainInfo->pSwapchainImages[0]));


    pSwapchainInfo->pSwapchainImageViews = new VkImageView[pSwapchainInfo->swapchainImageCount];

    for (uint32_t i = 0; i < pSwapchainInfo->swapchainImageCount; i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};

        imageViewCreateInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image    = pSwapchainInfo->pSwapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format   = VK_FORMAT_B8G8R8A8_UNORM;

        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;

        VK_CHECK(vkCreateImageView(pVulkanInfo->vkDevice, &imageViewCreateInfo, pAllocator, &pSwapchainInfo->pSwapchainImageViews[i]));
    }

}


void destroySwapchain(
    const VkCoreInfo* pVulkanInfo,
    VkSwapchainInfo*  pSwapchainInfo,
    const VkAllocationCallbacks* pAllocator)
{
    if (pSwapchainInfo->pSwapchainImageViews != nullptr)
    {
        for (uint32_t i = 0; i < pSwapchainInfo->swapchainImageCount; i++)
        {
            vkDestroyImageView(pVulkanInfo->vkDevice, pSwapchainInfo->pSwapchainImageViews[i], pAllocator);
        }

        delete [] pSwapchainInfo->pSwapchainImageViews;
        pSwapchainInfo->pSwapchainImageViews = nullptr;
    }

    if (pSwapchainInfo->pSwapchainImages != nullptr)
    {
        delete [] pSwapchainInfo->pSwapchainImages;
    }

    vkDestroySwapchainKHR(pVulkanInfo->vkDevice, pSwapchainInfo->swapchain, pAllocator);
    vkDestroySurfaceKHR(pVulkanInfo->vkInstance, pSwapchainInfo->surface, pAllocator);

}


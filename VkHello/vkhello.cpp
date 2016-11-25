

#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR 1

#include "vulkan\vulkan.h"

#include "IvyWindow.h"

static const uint32_t VkHelloImageWidth = 256;
static const uint32_t VkHelloImageHeight = 256;

void printDeviceMemoryProperties(
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties)
{
	int32_t memory_type_device_local_index = -1;

	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryHeapCount; i++)
	{
		std::cout << "Heap " << i << " flags: " << physicalDeviceMemoryProperties.memoryHeaps[i].flags << std::endl;
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

void createFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	VkImage image,
	VkImageView* pImageView,
	VkFramebuffer* pFramebuffer)
{
	VkImageViewCreateInfo imageViewCreateInfo;
	memset(&imageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
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

	VkFramebufferCreateInfo framebufferCreateInfo;
	memset(&framebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.width = VkHelloImageWidth;
	framebufferCreateInfo.height = VkHelloImageHeight;
	framebufferCreateInfo.layers = 1;

	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.pAttachments = pImageView;
	
	memset(pFramebuffer, 0, sizeof(VkFramebuffer));

	err = vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, pFramebuffer);

}

void initRenderPassBeginInfo(
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	VkRenderPassBeginInfo* renderPassBeginInfo)
{
	memset(renderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
	renderPassBeginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo->framebuffer = framebuffer;
	renderPassBeginInfo->renderPass = renderPass;

	renderPassBeginInfo->renderArea.offset.x = 0;
	renderPassBeginInfo->renderArea.offset.y = 0;
	renderPassBeginInfo->renderArea.extent.width = VkHelloImageWidth;
	renderPassBeginInfo->renderArea.extent.height = VkHelloImageHeight;	
}

int main()
{
	// Allocate some additional resources


	Ivy::IvyWindow* pWindow = Ivy::IvyWindow::Create(VkHelloImageWidth, VkHelloImageHeight);
	pWindow->Show();


	uint32_t instance_extension_count = 0;

	VkResult err = vkEnumerateInstanceExtensionProperties(
		NULL, &instance_extension_count, NULL);


	VkExtensionProperties* instance_extensions = new VkExtensionProperties[instance_extension_count];
	memset(&instance_extensions[0], 0, sizeof(instance_extensions)*instance_extension_count);
	err = vkEnumerateInstanceExtensionProperties(
		NULL, &instance_extension_count, instance_extensions);

	for (uint32_t i = 0; i < instance_extension_count; ++i)
	{
		std::cout << instance_extensions[i].extensionName << std::endl;
	}
	delete[] instance_extensions;

	uint32_t layerPropertyCount = 0;
	err = vkEnumerateInstanceLayerProperties(&layerPropertyCount, NULL);

	const char* instanceExtensionNames[] =
	{
		"VK_EXT_debug_report",
		"VK_KHR_surface",
		"VK_KHR_win32_surface"
	};

	const char* deviceExtensionNames[] =
	{
		"VK_KHR_swapchain",
		"VK_NV_glsl_shader"
	};

	VkLayerProperties* layerProperties = new VkLayerProperties[layerPropertyCount];
    memset(&layerProperties[0], 0, sizeof(VkLayerProperties)*layerPropertyCount);
	
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, &layerProperties[0]);

    for (uint32_t i = 0; i < layerPropertyCount; ++i)
    {
		std::cout << layerProperties[i].layerName << " " << layerProperties[i].description << std::endl;
    }

	delete[] layerProperties;

	const char* layerNames[] =
	{
	//	"VK_LAYER_LUNARG_api_dump",
		"VK_LAYER_LUNARG_core_validation",
		//"VK_LAYER_LUNARG_device_limits",
		"VK_LAYER_LUNARG_image",
		//"VK_LAYER_LUNARG_mem_tracker",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_standard_validation",
	};


    VkInstanceCreateInfo instanceInfo;
    memset(&instanceInfo, 0, sizeof(VkInstanceCreateInfo));

    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(VkApplicationInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VkHello";
    appInfo.pEngineName = "IVY";
    appInfo.apiVersion = 1 << 22;

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.ppEnabledLayerNames = &layerNames[0];
	instanceInfo.enabledLayerCount = sizeof(layerNames) / sizeof(char*);
	instanceInfo.ppEnabledExtensionNames = &instanceExtensionNames[0];
	instanceInfo.enabledExtensionCount = sizeof(instanceExtensionNames) / sizeof(char*);

	VkInstance instance;
	memset(&instance, 0, sizeof(VkInstance));
	
	VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);


	static const uint32_t MaxDevices = 4;
	uint32_t physicalDeviceCount = MaxDevices;
	VkPhysicalDevice physicalDevices[MaxDevices];

	err = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &physicalDevices[0]);

	VkPhysicalDeviceProperties physicalDeviceProperties;
	memset(&physicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
	vkGetPhysicalDeviceProperties(physicalDevices[0], &physicalDeviceProperties);

	uint32_t propertyCount = 0;
	err = vkEnumerateDeviceExtensionProperties(physicalDevices[0], NULL, &propertyCount, NULL);

	VkExtensionProperties* extensionProperties = new VkExtensionProperties[propertyCount];
	err = vkEnumerateDeviceExtensionProperties(physicalDevices[0], NULL, &propertyCount, &extensionProperties[0]);

	for (uint32_t i = 0; i < propertyCount; i++)
	{
		std::cout << extensionProperties[i].extensionName << std::endl;
	}


	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	memset(&physicalDeviceMemoryProperties, 0, sizeof(physicalDeviceMemoryProperties));
	vkGetPhysicalDeviceMemoryProperties(physicalDevices[0], &physicalDeviceMemoryProperties);

	///@todo Add Verbosity Level
	// printDeviceMemoryProperties(physicalDeviceMemoryProperties);


	static const uint32_t MaxQueues = 10;
	uint32_t queueFamilyPropertiesCount = MaxQueues;
	VkQueueFamilyProperties queueFamilyProperties[MaxQueues];
	memset(&queueFamilyProperties[0], 0, sizeof(VkQueueFamilyProperties) * MaxQueues);

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamilyPropertiesCount, &queueFamilyProperties[0]);

	uint32_t vkHelloQueueFamilyIndex = 0;

	VkBool32 presentSupported = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevices[0], vkHelloQueueFamilyIndex);


	// Setup Create Queue Info
	static const uint32_t RequestedQueues = 1;
	float queuePriorities[] = { 1.0 };
	uint32_t vkHelloQueueIndex = 0;

	VkDeviceQueueCreateInfo queueCreateInfo;
	memset(&queueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	queueCreateInfo.sType		     = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = vkHelloQueueFamilyIndex;
	queueCreateInfo.queueCount       = RequestedQueues;
	queueCreateInfo.pQueuePriorities = &queuePriorities[0];

	// Create VkDevice
	VkDeviceCreateInfo deviceCreateInfo;
	memset(&deviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

	deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos    = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensionNames[0];
	deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensionNames) / sizeof(char*);

	VkDevice device;
	memset(&device, 0, sizeof(VkDevice));
	err = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, NULL, &device);


	// Have device, now setup swapchain
	
	
	
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKHR;
	memset(&surfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
	surfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfoKHR.hinstance = GetModuleHandle(NULL);
	surfaceCreateInfoKHR.hwnd = pWindow->GetHwnd();

	VkSurfaceKHR surfaceKHR = 0;

	result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfoKHR, NULL, &surfaceKHR);

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[0], surfaceKHR, &surfaceCapabilities);


	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[0], surfaceKHR, &surfaceFormatCount,  NULL);
	VkSurfaceFormatKHR* surfaceFormats = new VkSurfaceFormatKHR[surfaceFormatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[0], surfaceKHR, &surfaceFormatCount, &surfaceFormats[0]);


	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[0], surfaceKHR, &presentModeCount, NULL);
	VkPresentModeKHR* presentModes = new VkPresentModeKHR[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[0], surfaceKHR, &presentModeCount, &presentModes[0]);

	VkBool32 supported = FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surfaceKHR, &supported);

	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR;
	memset(&swapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
	swapchainCreateInfoKHR.sType			  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.minImageCount	  = surfaceCapabilities.minImageCount;
	swapchainCreateInfoKHR.surface			  = surfaceKHR;
	swapchainCreateInfoKHR.imageExtent.width  = surfaceCapabilities.currentExtent.width;
	swapchainCreateInfoKHR.imageExtent.height = surfaceCapabilities.currentExtent.width;
	swapchainCreateInfoKHR.imageFormat		  = VK_FORMAT_B8G8R8A8_UNORM;
	swapchainCreateInfoKHR.imageUsage		  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageColorSpace	  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfoKHR.imageArrayLayers   = 1;
	swapchainCreateInfoKHR.compositeAlpha	  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfoKHR.preTransform		  = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfoKHR.presentMode        = VK_PRESENT_MODE_FIFO_KHR;

	///@TODO finish setup!!!

	VkSwapchainKHR swapchainKHR = 0;
	result = vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, NULL, &swapchainKHR);

	delete[] surfaceFormats;
	delete[] presentModes;

	VkQueue queue;
	memset(&queue, 0, sizeof(VkQueue));
	vkGetDeviceQueue(device, vkHelloQueueFamilyIndex, vkHelloQueueIndex, &queue);

	// Create Command Pool
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	memset(&commandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
	commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = vkHelloQueueFamilyIndex;
	commandPoolCreateInfo.flags            = 0;  // flags=0 Choosing to reset buffers in bulk, not per command buffer


	VkCommandPool commandPool;
	memset(&commandPool, 0, sizeof(VkCommandPool));
	err = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	memset(&commandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	commandBufferAllocateInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool		 = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer commandBuffer;
    memset(&commandBuffer, 0, sizeof(VkCommandBuffer));
    vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

    ///@todo finish creating image view, framebuffer, render pass 


    VkAttachmentDescription colorAttachmentDescription;
    memset(&colorAttachmentDescription, 0, sizeof(VkAttachmentDescription));
    colorAttachmentDescription.format        = VK_FORMAT_B8G8R8A8_UNORM;
    colorAttachmentDescription.samples       = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentDescription.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription;
    memset(&subpassDescription, 0, sizeof(VkSubpassDescription));
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Setup color Attachment
    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo;
    memset(&renderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments    = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount    = 1;
    renderPassCreateInfo.pSubpasses      = &subpassDescription;

    VkRenderPass renderPass;
    memset(&renderPass, 0, sizeof(VkRenderPass));

    err = vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass);


	/*
    VkImageCreateInfo imageCreateInfo;
    memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent.width  = VkHelloImageWidth;
    imageCreateInfo.extent.height = VkHelloImageHeight;
    imageCreateInfo.extent.depth  = 1;
    imageCreateInfo.mipLevels     = 1;
    imageCreateInfo.arrayLayers   = 1;
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL; // todo: LINEAR fails
    imageCreateInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


    VkImage image = 0;
    err = vkCreateImage(device, &imageCreateInfo, NULL, &image);

	VkMemoryRequirements memoryRequirements;
	memset(&memoryRequirements, 0, sizeof(VkMemoryRequirements));
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	// Assign image to a device
	VkMemoryAllocateInfo allocateInfo;
	memset(&allocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;

	// memoryRequirements.memoryTypeBits;
	allocateInfo.memoryTypeIndex = 8;

	VkDeviceMemory memory;
	err = vkAllocateMemory(device, &allocateInfo, NULL, &memory);

	err = vkBindImageMemory(device, image, memory, 0);

	*/

	uint32_t imageCount = 0;
	err = vkGetSwapchainImagesKHR(device, swapchainKHR, &imageCount, NULL);

	VkImage* swapchainImages = new VkImage[imageCount];
	err = vkGetSwapchainImagesKHR(device, swapchainKHR, &imageCount, &swapchainImages[0]);
	///@todo delete swapchain images

	VkImageView* imageViews = new VkImageView[imageCount];
	VkFramebuffer* framebuffers = new VkFramebuffer[imageCount];
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		createFramebuffer(device, renderPass, swapchainImages[i], &imageViews[i], &framebuffers[i]);
	}

	// Do Something Interesting Here
	while (1)
    {
		VkSemaphore presentCompleteSemaphore;
		VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
		presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		presentCompleteSemaphoreCreateInfo.pNext = NULL;
		presentCompleteSemaphoreCreateInfo.flags = 0;

		err = vkCreateSemaphore(device, &presentCompleteSemaphoreCreateInfo, NULL, &presentCompleteSemaphore);

		uint32_t imageIndex = 0;
		err = vkAcquireNextImageKHR(device, swapchainKHR, UINT64_MAX, presentCompleteSemaphore, 0, &imageIndex);

		VkRenderPassBeginInfo renderPassBeginInfo;

		initRenderPassBeginInfo(renderPass, framebuffers[imageIndex], &renderPassBeginInfo);


		VkClearValue clearValue;
		clearValue.color.float32[0] = 1.0f;
		clearValue.color.float32[1] = 1.0f;
		clearValue.color.float32[2] = 0.0f;
		clearValue.color.float32[3] = 1.0f;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

        VkCommandBufferBeginInfo beginInfo;
        memset(&beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // HW: regenerate per frame vs cache per frame

		err = vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        err = vkBeginCommandBuffer(commandBuffer, &beginInfo);

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Do cool vulkan stuff here...

        vkCmdEndRenderPass(commandBuffer);


		VkImageMemoryBarrier prePresentBarrier;
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

        VkSubmitInfo submitInfo;
        memset(&submitInfo, 0, sizeof(VkSubmitInfo));
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
		submitInfo.waitSemaphoreCount = 1;

		VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		submitInfo.pWaitDstStageMask = &pipe_stage_flags;

        err = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		VkPresentInfoKHR presentInfo;
		memset(&presentInfo, 0, sizeof(VkPresentInfoKHR));
		presentInfo.sType          = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pSwapchains    = &swapchainKHR;
		presentInfo.swapchainCount = 1;
		
		presentInfo.pImageIndices = &imageIndex;

		err = vkQueuePresentKHR(queue, &presentInfo);

		vkQueueWaitIdle(queue);

		vkDestroySemaphore(device, presentCompleteSemaphore, NULL);

		pWindow->ProcessMsg(false);
    }


    // Free Resources Here
    {
        err = vkDeviceWaitIdle(device);  // Implications of not calling this ahead of freeing data?



    //    vkDestroyFramebuffer(device, framebuffer, NULL);
        vkDestroyRenderPass(device, renderPass, NULL);
    //    vkDestroyImageView(device, imageView, NULL);
      //  vkDestroyImage(device, image, NULL);

        // Q: If calling reset with release flag on the pool, do you need to free the individual command buffs?
        // A: Reseting the buffer is not the same as freeing it.
        vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

        vkDestroyCommandPool(device, commandPool, NULL);
    }

    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

    std::cout << "Bye Bye" << std::endl;
    return 0;
}
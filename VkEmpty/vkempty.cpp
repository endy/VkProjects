///////////////////////////////////////////////////////////////////////////////////////////////////
///
///     VkEmpty
///
///     Copyright 2018, Brandon Light
///     All rights reserved.
///
///     Stub project for any simple vulkan app.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "VkUtils.h"

#include "IvyWindow.h"

const char* RequiredInstanceExtensionNames[] =
{
    "VK_EXT_debug_report",
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
};
const uint32_t RequiredInstanceExtensionCount = sizeof(RequiredInstanceExtensionNames) / sizeof(RequiredInstanceExtensionNames[0]);

const AppInfo appInfo =
{
    "VkEmpty",
    "NullEngine",
    VK_API_VERSION_1_1,

    RequiredInstanceExtensionNames,
    RequiredInstanceExtensionCount,
    NULL,
    0,

    NULL,
    0,

    NULL
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


    pWindow->Show();

    BOOL quit = FALSE;
    while (quit == FALSE)
    {
        pWindow->ProcessMsg(&quit);
    }

    destroyVulkan(&vulkanInfo, NULL);

    return 0;
}
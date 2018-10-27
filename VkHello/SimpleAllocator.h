#pragma once


#include "vulkan\vulkan.h"

#include <cstdio>

#define SIMPLE_ENABLE_PRINTS 1

#if SIMPLE_ENABLE_PRINTS
#define SIMPLE_PRINT(x, ...) printf(x, __VA_ARGS__)
#else
#define SIMPLE_PRINT(x, ...) void(0)
#endif

struct SimpleAllocation
{
    uint32_t size;
    void*    allocation;
};

void* VKAPI_PTR SimpleAlloc(
    void*                                       pUserData,
    size_t                                      size,
    size_t                                      alignment,
    VkSystemAllocationScope                     allocationScope)
{
    SIMPLE_PRINT("UserData=%s Alloc - Scope=%d Size=%d\n", static_cast<char*>(pUserData), allocationScope, size);

    char* pAllocation = static_cast<char*>(std::malloc(size + sizeof(SimpleAllocation)));

    reinterpret_cast<SimpleAllocation*>(pAllocation)->size       = size;
    reinterpret_cast<SimpleAllocation*>(pAllocation)->allocation = reinterpret_cast<void*>(pAllocation + sizeof(SimpleAllocation));


    return reinterpret_cast<SimpleAllocation*>(pAllocation)->allocation;
}

void* VKAPI_PTR SimpleRealloc(
    void*                                       pUserData,
    void*                                       pOriginal,
    size_t                                      size,
    size_t                                      alignment,
    VkSystemAllocationScope                     allocationScope)
{
    SIMPLE_PRINT("UserData=%s Re-Alloc - Scope=%d Size=%d\n", static_cast<char*>(pUserData), allocationScope, size);

    SimpleAllocation* pReturnAlloc = NULL;
    SimpleAllocation* pOriginalAlloc = reinterpret_cast<SimpleAllocation*>(reinterpret_cast<char*>(pOriginal) - sizeof(SimpleAllocation));

    if (pOriginalAlloc->size < size)
    {
        // Alloc a larger chunk
        pReturnAlloc = static_cast<SimpleAllocation*>(std::malloc(size + sizeof(SimpleAllocation)));

        pReturnAlloc->size       = size;
        pReturnAlloc->allocation = reinterpret_cast<void*>(reinterpret_cast<char*>(pReturnAlloc) + sizeof(SimpleAllocation));

        // Copy over data
        std::memcpy(pReturnAlloc->allocation, pOriginalAlloc->allocation, pOriginalAlloc->size);

        // Free original allocation
        std::free(pOriginalAlloc);
    }
    else
    {
        pReturnAlloc = pOriginalAlloc;
    }

    return reinterpret_cast<SimpleAllocation*>(pReturnAlloc)->allocation;
}

void VKAPI_PTR SimpleFree(
    void*                                       pUserData,
    void*                                       pMemory)
{
    if (pMemory != nullptr)
    {
        SimpleAllocation* pAllocation = reinterpret_cast<SimpleAllocation*>(reinterpret_cast<char*>(pMemory) - sizeof(SimpleAllocation));

        SIMPLE_PRINT("Free:  Size=%d\n", pAllocation->size);

        std::free(pAllocation);
    }
}

void VKAPI_PTR SimpleInternalAllocNotify(
    void*                                       pUserData,
    size_t                                      size,
    VkInternalAllocationType                    allocationType,
    VkSystemAllocationScope                     allocationScope)
{
    printf("internal alloc notify\n");
}

void VKAPI_PTR  SimpleInternalFreeNotify(
    void*                                       pUserData,
    size_t                                      size,
    VkInternalAllocationType                    allocationType,
    VkSystemAllocationScope                     allocationScope)
{
    printf("internal free notify");
}

static char* UserData = "Muh user datah\n";

static const VkAllocationCallbacks GlobalSimpleAllocator = 
{
    &UserData[0],
    SimpleAlloc,
    SimpleRealloc,
    SimpleFree,
    SimpleInternalAllocNotify,
    SimpleInternalFreeNotify
};
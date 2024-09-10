#pragma once

#include  "vulkantypes.h"

// vulkan buffer needs to bound to be used
VkResult  vulkanBufferCreate      (VulkanContext*         context,
                                   u64                    size,
                                   VkBufferUsageFlagBits  usage,
                                   u32                    memoryPropertyFlags,
                                   bool                   bindOnCreate,
                                   VulkanBuffer*          outBuffer);

VkResult  vulkanBufferResize      (VulkanContext*         context,
                                   u64                    newSize,
                                   VulkanBuffer*          buffer,
                                   VkQueue                queue,
                                   VkCommandPool          commandPool);

void      vulkanBufferDestroy     (VulkanContext*         context,
                                   VulkanBuffer*          buffer);

// you cannot use buffer htat is not bound
VkResult  vulkanBufferBind        (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset);

// this is goona sourround the loading of buffer into the memmory
void*     vulkanBufferLockMemory  (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset,
                                   u64                    size,
                                   u32                    flags);

VkResult  vulkanBufferUnlockMemory(VulkanContext*         context,
                                   VulkanBuffer*          buffer);

//lock -> load -> unlock
VkResult  vulkanBufferLoadData    (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset,
                                   u64                    size,
                                   u32                    flags,
                                   const void*            data);

// copy data from one buffer to another
VkResult  vulkanBufferCopyTo      (VulkanContext*         context,
                                   VkCommandPool          pool,
                                   VkFence                fence,
                                   VkQueue                queue,
                                   VkBuffer               source,
                                   u64                    sourceOffset,
                                   VkBuffer               dest,
                                   u64                    destOffset,
                                   u64                    size);

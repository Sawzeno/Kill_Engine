#pragma once

#include  "vulkantypes.h"

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

VkResult  vulkanBufferDestroy     (VulkanContext*         context,
                                   VulkanBuffer*          buffer);

VkResult  vulkanBufferBind        (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset);

void*     vulkanBufferLockMemory  (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset,
                                   u64                    size,
                                   u32                    flags);

VkResult  vulkanBufferUnlockMemory(VulkanContext*         context,
                                   VulkanBuffer*          buffer);

VkResult  vulkanBufferLoadData    (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset,
                                   u64                    size,
                                   u32                    flags,
                                   const void*            data);

VkResult  vulkanBufferCopyTo      (VulkanContext*         context,
                                   VkCommandPool          pool,
                                   VkFence                fence,
                                   VkQueue                queue,
                                   VkBuffer               source,
                                   u64                    sourceOffset,
                                   VkBuffer               dest,
                                   u64                    destOffset,
                                   u64                    size);

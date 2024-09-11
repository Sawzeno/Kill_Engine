#pragma once

#include  "vulkantypes.h"

VkResult  vulkanFenceCreate(VulkanContext* context,
                            b32 createSignaled,
                            VulkanFence* outFence);

VkResult  vulkanFenceDestroy(VulkanContext* context, 
                             VulkanFence* fence);

b32        vulkanFenceWait   (VulkanContext* context,
                             VulkanFence* fence, u64 timeoutNS);

VkResult  vulkanFenceReset  (VulkanContext* context,
                             VulkanFence* fence);

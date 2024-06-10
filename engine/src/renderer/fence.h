#pragma once

#include  "vulkantypes.h"

VkResult  vulkanFenceCreate(VulkanContext* context,
                            u8 createSignaled,
                            VulkanFence* outFence);

VkResult  vulkanFenceDestroy(VulkanContext* context, 
                             VulkanFence* fence);

u8        vulkanFenceWait   (VulkanContext* context,
                             VulkanFence* fence, u64 timeoutNS);

VkResult  vulkanFenceReset  (VulkanContext* context,
                             VulkanFence* fence);

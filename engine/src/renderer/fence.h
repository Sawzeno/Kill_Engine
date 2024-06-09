#pragma once

#include  "vulkantypes.h"

VkResult  vulkanFenceCreate(vulkanContext* context,
                            u8 createSignaled,
                            vulkanFence* outFence);

VkResult  vulkanFenceDestroy(vulkanContext* context, 
                             vulkanFence* fence);

u8  vulkanFenceWait(vulkanContext* context, vulkanFence* fence, u64 timeoutNS);

VkResult  vulkanFenceReset(vulkanContext* context, vulkanFence* fence);

#pragma once
#include  "vulkantypes.h"

VkResult  vulkanCommandBufferAllocate         (VulkanContext* context,
                                               VkCommandPool pool,//pool we want to allocate our buffer from
                                               u8 isPrimary,
                                               VulkanCommandBuffer* outCommandBuffer);

VkResult  vulkanCommandBufferFree             (VulkanContext* context,
                                               VkCommandPool pool, 
                                               VulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferBegin            (VulkanCommandBuffer* commandBuffer,
                                               u8 isSingleUse,
                                               u8 isRenderPassContinue,
                                               u8 isSimultaneousUse);

VkResult  vulkanCommandBufferEnd              (VulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferUpdateSubmitted  (VulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferReset            (VulkanCommandBuffer* commandBuffer);

// allocates and begins recording out to command buffer
VkResult  vulkanCommandBufferStartSingleUse   (VulkanContext* context,
                                               VkCommandPool pool,
                                               VulkanCommandBuffer* outCommandBuffer);

//ends recording, submits and waits for queue operation ande frees the provided command buffer
VkResult  vulkanCommandBufferStopSingleUse    (VulkanContext* context,
                                               VkCommandPool pool,
                                               VulkanCommandBuffer* commandBuffer,
                                               VkQueue queue);

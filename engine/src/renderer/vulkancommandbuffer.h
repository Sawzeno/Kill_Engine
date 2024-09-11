#pragma once
#include  "vulkantypes.h"

VkResult  vulkanCommandBufferAllocate         (VulkanContext* context,
                                               VkCommandPool pool,//pool we want to allocate our buffer from
                                               b32 isPrimary,
                                               VulkanCommandBuffer* outCommandBuffer);

VkResult  vulkanCommandBufferFree             (VulkanContext* context,
                                               VkCommandPool pool, 
                                               VulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferBegin            (VulkanCommandBuffer* commandBuffer,
                                               b32 isSingleUse,
                                               b32 isRenderPassContinue,
                                               b32 isSimultaneousUse);

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

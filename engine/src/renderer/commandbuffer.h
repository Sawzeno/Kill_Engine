#pragma once
#include  "vulkantypes.h"

VkResult  vulkanCommandBufferAllocate(vulkanContext* context,
                                     VkCommandPool pool,//pool we want to allocate our buffer from
                                     u8 isPrimary,
                                     vulkanCommandBuffer* outCommandBuffer);

VkResult  vulkanCommandBufferFree   (vulkanContext* context,
                                     VkCommandPool pool, 
                                     vulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferBegin  (vulkanCommandBuffer* commandBuffer,
                                     u8 isSingleUse,
                                     u8 isRenderPassContinue,
                                     u8 isSimultaneousUse);

VkResult  vulkanCommandBufferEnd        (vulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferUpdateSubmitted(vulkanCommandBuffer* commandBuffer);

VkResult  vulkanCommandBufferReset      (vulkanCommandBuffer* commandBuffer);

// allocates and begins recording out to command buffer
VkResult  vulkanCommandBufferStartSingleUse  (vulkanContext* context,
                                              VkCommandPool pool,
                                              vulkanCommandBuffer* outCommandBuffer);

//ends recording, submits and waits for queue operation ande frees the provided command buffer
VkResult  vulkanCommandBufferStopSingleUse    (vulkanContext* context,
                                              VkCommandPool pool,
                                              vulkanCommandBuffer* commandBuffer,
                                              VkQueue queue);

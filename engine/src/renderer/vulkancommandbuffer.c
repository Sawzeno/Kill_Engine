#include  "vulkancommandbuffer.h"
#include  "rendererutils.h"

#include  "core/logger.h"
#include  "core/kmemory.h"


VkResult  vulkanCommandBufferAllocate(VulkanContext* context,
                                      VkCommandPool pool ,//pool we want to allocate our buffer from
                                      u8 isPrimary,
                                      VulkanCommandBuffer* outCommandBuffer){
TRACEFUNCTION;
  VkResult result   = {0};
  kzeroMemory(outCommandBuffer, sizeof(VulkanCommandBuffer));

  VkCommandBufferAllocateInfo allocateInfo  = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandPool        = pool ;
  allocateInfo.level              = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocateInfo.commandBufferCount = 1;
  allocateInfo.pNext              = NULL;

  outCommandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  result  = vkAllocateCommandBuffers(context->device.logicalDevice,
                                     &allocateInfo,
                                     &outCommandBuffer->handle);
  VK_CHECK_VERBOSE(result, "vkAllocateCommandBuffers failed");
  outCommandBuffer->state = COMMAND_BUFFER_STATE_READY;
  KINFO("SUCCESFULLY ALLOCATED A COMANND BUFFER");
  return result;
}

VkResult  vulkanCommandBufferFree   (VulkanContext* context,
                                     VkCommandPool pool, 
                                     VulkanCommandBuffer* commandBuffer){
TRACEFUNCTION;
  VkResult result = {0};
  KDEBUG("vkFreeCommandBuffers called");
  vkFreeCommandBuffers(context->device.logicalDevice, pool, 1, &commandBuffer->handle);
  commandBuffer->handle = 0;
  commandBuffer->state  = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  KINFO("COMMAND BUFFER HAS BEEN FREED !");
  return VK_SUCCESS;
}

VkResult  vulkanCommandBufferBegin  (VulkanCommandBuffer* commandBuffer,
                                     u8 isSingleUse,
                                     u8 isRenderPassContinue,
                                     u8 isSimultaneousUse){
  TRACEFUNCTION;
  VkResult result = {0};
  VkCommandBufferBeginInfo beginInfo  = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  beginInfo.flags = 0;
  if(isSingleUse){//each recording of the command buffer will only be submitted once
    //and command buffer will be reset and recorded again bw each submission
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }
  if(isRenderPassContinue){//secondaray command buffer is considered to be entirely
    //inside a render pass, if this is a primary then this bit is ignored
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  }
  if(isSingleUse){//the command buffer can be resubmitted to a queue
    //while it is in pending state and recorded into multiple command buffers
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  }

  result = vkBeginCommandBuffer(commandBuffer->handle, &beginInfo);
  VK_CHECK_VERBOSE(result, "vkBeginCommandBuffer failed");
  commandBuffer->state  = COMMAND_BUFFER_STATE_RECORDING;
  KDEBUG("SUCCESFULLY BEGIN COMMAND BUFFER");
  return result;
}

VkResult  vulkanCommandBufferEnd        (VulkanCommandBuffer* commandBuffer){
  TRACEFUNCTION;
  VkResult  result  = vkEndCommandBuffer(commandBuffer->handle);
  VK_CHECK_VERBOSE(result, "vkEndCommandBuffer failed");
  commandBuffer->state  = COMMAND_BUFFER_STATE_RECORDING;
  KDEBUG("SUCCESFULLY END COMMAND BUFFER");
  return result;
}

VkResult  vulkanCommandBufferUpdateSubmitted(VulkanCommandBuffer* commandBuffer){
  TRACEFUNCTION;
  commandBuffer->state  = COMMAND_BUFFER_STATE_SUBMITTED;
  return VK_SUCCESS;
}

VkResult  vulkanCommandBufferReset      (VulkanCommandBuffer* commandBuffer){
  TRACEFUNCTION;
  commandBuffer->state  = COMMAND_BUFFER_STATE_READY;
  return VK_SUCCESS;
}

// allocates and begins recording out to command buffer
VkResult  vulkanCommandBufferStartSingleUse  (VulkanContext* context,
                                              VkCommandPool pool,
                                              VulkanCommandBuffer* outCommandBuffer){
  TRACEFUNCTION;
  VkResult result = {0};
  result  = vulkanCommandBufferAllocate(context, pool, true, outCommandBuffer);
  VK_CHECK_RESULT(result, "failed to allocate single use command buffer");
  result  = vulkanCommandBufferBegin(outCommandBuffer, true, false, false);
  VK_CHECK_RESULT(result, "failed to start single use command buffer");
  KDEBUG("SUCCESFULLY STARTED SINGLE USE BUFFER");
  return VK_SUCCESS;
}

//ends recording, submits and waits for queue operation ande frees the provided command buffer
VkResult  vulkanCommandBufferStopSingleUse    (VulkanContext* context,
                                              VkCommandPool pool,
                                              VulkanCommandBuffer* commandBuffer,
                                              VkQueue queue){
  TRACEFUNCTION;
  VkResult  result  = {0};
  //first end the command buffer
  result  = vulkanCommandBufferEnd(commandBuffer);
  VK_CHECK_RESULT(result, "failed to stop single use commmand buffer");

  //submit the queue
  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer->handle;
  result  = vkQueueSubmit(queue, 1, &submitInfo, 0);
  VK_CHECK_VERBOSE(result, "vkQueueSubmit failed");

  //wait for it to finish
  result  = vkQueueWaitIdle(queue);
  VK_CHECK_VERBOSE(result , "vkQueueWaitIdle failed");

  //free the command buffer
  vulkanCommandBufferFree(context, pool, commandBuffer);
  KDEBUG("SUCCESFULLY ENDED SINGLE USE BUFFER");
  return result;
}


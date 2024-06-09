#include  "commandbuffer.h"
#include  "utils.h"
#include  "vulkantypes.h"
#include  "core/kmemory.h"
#include <vulkan/vulkan_core.h>


VkResult  vulkanCommandBufferAllocate(vulkanContext* context,
                                      VkCommandPool pool ,//pool we want to allocate our buffer from
                                      u8 isPrimary,
                                      vulkanCommandBuffer* outCommandBuffer){

  VkResult result   = {0};
  kzeroMemory(outCommandBuffer, sizeof(vulkanCommandBuffer));

  VkCommandBufferAllocateInfo allocateInfo  = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandPool        = pool ;
  allocateInfo.level              = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocateInfo.commandBufferCount = 1;
  allocateInfo.pNext              = NULL;

  outCommandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  result  = vkAllocateCommandBuffers(context->device.logicalDevice,
                                     &allocateInfo,
                                     &outCommandBuffer->handle);
  VK_CHECK2(result, "FAILED TO ALLOCATE A COMMAND BUFFER");
  outCommandBuffer->state = COMMAND_BUFFER_STATE_READY;
  UINFO("SUCCESFULLY ALLOCATED A COMANND BUFFER");
  return result;
}

VkResult  vulkanCommandBufferFree   (vulkanContext* context,
                                     VkCommandPool pool, 
                                     vulkanCommandBuffer* commandBuffer){
  VkResult result = {0};
  vkFreeCommandBuffers(context->device.logicalDevice, pool, 1, &commandBuffer->handle);
  commandBuffer->handle = 0;
  commandBuffer->state  = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  UINFO("COMMAND BUFFER HAS BEEN FREED !");
  return VK_SUCCESS;
}

VkResult  vulkanCommandBufferBegin  (vulkanCommandBuffer* commandBuffer,
                                     u8 isSingleUse,
                                     u8 isRenderPassContinue,
                                     u8 isSimultaneousUse){
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
  VK_CHECK2(result, "failed to begin command buffer");
  commandBuffer->state  = COMMAND_BUFFER_STATE_RECORDING;
  UINFO("SUCCESFULLY BEGIN COMMAND BUFFER");
  return result;
}

VkResult  vulkanCommandBufferEnd        (vulkanCommandBuffer* commandBuffer){
  VkResult  result  = vkEndCommandBuffer(commandBuffer->handle);
  VK_CHECK2(result, "failed to end command buffer");
  commandBuffer->state  = COMMAND_BUFFER_STATE_RECORDING;
  UINFO("SUCCESFULLY END COMMAND BUFFER");
  return result;
}

VkResult  vulkanCommandBufferUpdateSubmitted(vulkanCommandBuffer* commandBuffer){
  commandBuffer->state  = COMMAND_BUFFER_STATE_SUBMITTED;
  return VK_SUCCESS;
}

VkResult  vulkanCommandBufferReset      (vulkanCommandBuffer* commandBuffer){
  commandBuffer->state  = COMMAND_BUFFER_STATE_READY;
  return VK_SUCCESS;
}

// allocates and begins recording out to command buffer
VkResult  vulkanCommandBufferStartSingleUse  (vulkanContext* context,
                                              VkCommandPool pool,
                                              vulkanCommandBuffer* outCommandBuffer){
  VkResult result = {0};
  result  = vulkanCommandBufferAllocate(context, pool, true, outCommandBuffer);
  VK_CHECK2(result, __FUNCTION__);
  result  = vulkanCommandBufferBegin(outCommandBuffer, true, false, false);
  VK_CHECK2(result, __FUNCTION__);
  UINFO("SUCCESFULLY STARTED SINGLE USE BUFFER");
  return VK_SUCCESS;
}

//ends recording, submits and waits for queue operation ande frees the provided command buffer
VkResult  vulkanCommandBufferStopSingleUse    (vulkanContext* context,
                                              VkCommandPool pool,
                                              vulkanCommandBuffer* commandBuffer,
                                              VkQueue queue){
  VkResult  result  = {0};
  //first end the command buffer
  result  = vulkanCommandBufferEnd(commandBuffer);
  VK_CHECK2(result, __FUNCTION__);
  //submit the queue
  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer->handle;
  result  = vkQueueSubmit(queue, 1, &submitInfo, 0);
  VK_CHECK2(result, "failed to submit queue");
  //wait for it to finish
  result  = vkQueueWaitIdle(queue);
  VK_CHECK2(result, "command buffer finished with an error");
  //free the command buffer
  vulkanCommandBufferFree(context, pool, commandBuffer);
  UINFO("SUCCESFULLY ENDED SINGLE USE BUFFER");
  return result;
}


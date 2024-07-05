#include  "vulkanframebuffer.h"
#include  "rendererutils.h"

#include  "core/kmemory.h"
#include  "core/logger.h"


VkResult  vulkanFrameBufferCreate(VulkanContext* context,
                                 VulkanRenderPass* renderPass, 
                                 u32 width, u32 height, 
                                 u32 attachmentCount,
                                 VkImageView* attachments, 
                                 VulkanFrameBuffer* outFrameBuffer){
  TRACEFUNCTION;
  VkResult  result  = {0};
  // take a copy of attachments , renderpass , attachment count
  outFrameBuffer->attachments = kallocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);

  for(u32 i =0; i <attachmentCount; ++i){
    outFrameBuffer->attachments[i]  = attachments[i];
  }
  outFrameBuffer->renderPass  = renderPass;
  outFrameBuffer->attachmentCount = attachmentCount;

  u32 swapchainExtentWidth = context->device.swapchainSupport.capabilities.currentExtent.width;
  u32 swapchainExtentHeight = context->device.swapchainSupport.capabilities.currentExtent.height;

  VkFramebufferCreateInfo createInfo  = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  createInfo.renderPass  = renderPass->handle;
  createInfo.attachmentCount  = attachmentCount;
  createInfo.pAttachments  = attachments;
  createInfo.width  = swapchainExtentWidth >= width ? width : swapchainExtentWidth;
  createInfo.height  = swapchainExtentHeight >= height ? height : swapchainExtentHeight;
  createInfo.layers  = 1;

  result  = vkCreateFramebuffer(context->device.logicalDevice, &createInfo, context->allocator, &outFrameBuffer->handle);
  VK_CHECK_VERBOSE(result, "vkCreateFramebuffer failed");
  KINFO("SUCCESFULLY CREATED FRAMEBUFFER");
  return result;
}

VkResult  vulkanFrameBufferDestroy(VulkanContext* context, VulkanFrameBuffer* frameBuffer){
  TRACEFUNCTION;
  vkDestroyFramebuffer(context->device.logicalDevice, frameBuffer->handle, context->allocator);
   if(frameBuffer->attachments){
    kfree(frameBuffer->attachments, sizeof(VkImageView)* frameBuffer->attachmentCount , MEMORY_TAG_RENDERER);
    frameBuffer->attachments    = 0;
  }
  frameBuffer->attachmentCount  = 0;
  frameBuffer->renderPass       = 0;
  frameBuffer->handle           = 0;
  KINFO("SUCCESFULLY DESTROYED FRAMEBUFFER");
  return VK_SUCCESS;
}

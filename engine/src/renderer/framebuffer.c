#include  "framebuffer.h"

#include  "core/kmemory.h"
#include <vulkan/vulkan_core.h>
#include  "utils.h"


VkResult  vulkanFrameBufferCreate(vulkanContext* context,
                                 vulkanRenderPass* renderPass, 
                                 u32 width, u32 height, 
                                 u32 attachmentCount,
                                 VkImageView* attachments, 
                                 vulkanFrameBuffer* outFrameBuffer){
  VkResult  result  = {0};
  // take a copy of attachments , renderpass , attachment count
  outFrameBuffer->attachments = kallocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);
  MEMERR(outFrameBuffer->attachments);
  for(u32 i =0; i <attachmentCount; ++i){
    outFrameBuffer->attachments[i]  = attachments[i];
  }
  outFrameBuffer->renderPass  = renderPass;
  outFrameBuffer->attachmentCount = attachmentCount;
  
  VkFramebufferCreateInfo createInfo  = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  createInfo.renderPass  = renderPass->handle;
  createInfo.attachmentCount  = attachmentCount;
  createInfo.pAttachments  = attachments;
  createInfo.width  = width;
  createInfo.height  = height;
  createInfo.layers  = 1;

  result  = vkCreateFramebuffer(context->device.logicalDevice, &createInfo, context->allocator, &outFrameBuffer->handle);
  VK_CHECK2(result, "FAILED TO CREATE FRAMEBUFFER");
  UINFO("SUCCESFULLY CREATED FRAMEBUFFER");
  return result;
}

VkResult  vulkanFrameBufferDestroy(vulkanContext* context, vulkanFrameBuffer* frameBuffer){
  vkDestroyFramebuffer(context->device.logicalDevice, frameBuffer->handle, context->allocator);
   if(frameBuffer->attachments){
    kfree(frameBuffer->attachments, sizeof(VkImageView)* frameBuffer->attachmentCount , MEMORY_TAG_RENDERER);
    frameBuffer->attachments    = 0;
  }
  frameBuffer->attachmentCount  = 0;
  frameBuffer->renderPass       = 0;
  frameBuffer->handle           = 0;
  UINFO("SUCCESFULLY DESTROYED FRAMEBUFFER");
  return VK_SUCCESS;
}

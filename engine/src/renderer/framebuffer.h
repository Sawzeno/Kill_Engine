#pragma once

#include  "vulkantypes.h"
// framebuffer has the reference to the attachments where the images are rendered to
// renderpass will need to know bout the framebuffer , 
// framebuffers are used to relay data back to swapchain

VkResult  vulkanFrameBufferCreate(vulkanContext* context,
                                 vulkanRenderPass* renderPass, 
                                 u32 width, u32 height, 
                                 u32 attachmentCount,
                                 VkImageView* attachments, 
                                 vulkanFrameBuffer* outFrameBuffer);

VkResult  vulkanFrameBufferDestroy(vulkanContext* context, vulkanFrameBuffer* frameBuffer);

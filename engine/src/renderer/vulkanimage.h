#pragma once

#include  "vulkantypes.h"

VkResult  vulkanImageCreate     (VulkanContext*     context,
                                 VkImageType        type,
                                 u32 width, u32     height,
                                 VkFormat           format,
                                 VkImageTiling      tiling,
                                 VkImageUsageFlags  usage,
                                 VkMemoryPropertyFlags memoryFlags,
                                 u32                createView,
                                 VkImageAspectFlags viewAspectFlags,
                                 VulkanImage*       outImage);

VkResult  vulkanImageViewCreate (VulkanContext*     context,
                                 VkFormat           format,
                                 VulkanImage*       image,
                                 VkImageAspectFlags aspectFlags);

//transitions the provided image from old layout to new layout
VkResult  vulkanImageTransitionLayout (VulkanContext*  context,
                                       VulkanCommandBuffer* commandBuffer,
                                       VulkanImage*    image,
                                       VkFormat        format,
                                       VkImageLayout   oldLayout,
                                       VkImageLayout   newLayout);
//copies data in buffer to provided image
void      vulkanImageCopyFromBuffer   (VulkanContext* context,
                                       VulkanImage*   image,
                                       VkBuffer       buffer,
                                       VulkanCommandBuffer* commandBuffer);

void      vulkanImageDestroy    (VulkanContext*     context,
                                 VulkanImage*       image);

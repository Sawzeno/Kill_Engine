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

void      vulkanImageDestroy    (VulkanContext*     context,
                                 VulkanImage*       image);

void      vulkanImageTransitionLayout(VulkanContext* context,
                                      VulkanCommandBuffer* commandBuffer,
                                      VulkanImage* image,
                                      VkFormat format,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout);

void      vulkanImageCopyFromBuffer(VulkanContext* context,
                                    VulkanImage* image,
                                    VkBuffer buffer,
                                    VulkanCommandBuffer* commandBuffer);


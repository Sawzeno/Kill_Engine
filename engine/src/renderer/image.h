#pragma once

#include  "vulkantypes.h"

VkResult  vulkanImageCreate(vulkanContext* context,
                      VkImageType type,
                      u32 width, u32 height,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags memoryFlags,
                      u32 createView,
                      VkImageAspectFlags viewAspectFlags,
                      vulkanImage* outImage);

VkResult  vulkanImageViewCreate(vulkanContext* context,
                          VkFormat format,
                          vulkanImage* image,
                          VkImageAspectFlags aspectFlags);

void vulkanImageDestroy(vulkanContext* context, vulkanImage* image);

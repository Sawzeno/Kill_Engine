#pragma once

#include  "vulkantypes.h"
#include <vulkan/vulkan_core.h>

VkResult  vulkanDeviceCreate  (vulkanContext* context);
VkResult  vulkanDeviceDestroy (vulkanContext* context);

VkResult vulkanDeviceQuerySwapchainSupport(
  VkPhysicalDevice  physicalDevice,
  VkSurfaceKHR      surface,
  vulkanSwapchainSupportInfo* outSupportInfo);

VkResult  vulkanDeviceDetectDepthFormat(vulkanDevice* device);

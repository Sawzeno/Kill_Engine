#pragma once

#include  "vulkantypes.h"

VkResult  vulkanDeviceCreate  (VulkanContext* context);
VkResult  vulkanDeviceDestroy (VulkanContext* context);

VkResult vulkanDeviceQuerySwapchainSupport(
  VkPhysicalDevice  physicalDevice,
  VkSurfaceKHR      surface,
  VulkanSwapchainSupportInfo* outSupportInfo);

VkResult  vulkanDeviceDetectDepthFormat(VulkanDevice* device);

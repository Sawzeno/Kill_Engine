#pragma once

#include  "vulkantypes.h"
#include <vulkan/vulkan_core.h>

VkResult  vulkanDeviceCreate  (VulkanContext* context);
VkResult  vulkanDeviceDestroy (VulkanContext* context);

VkResult vulkanDeviceQuerySwapchainSupport(
  VkPhysicalDevice  physicalDevice,
  VkSurfaceKHR      surface,
  VulkanSwapchainSupportInfo* outSupportInfo);

VkResult  vulkanDeviceDetectDepthFormat(VulkanDevice* device);

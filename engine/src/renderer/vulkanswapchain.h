#pragma once

#include  "vulkantypes.h"

VkResult vulkanSwapchainCreate      (VulkanContext*   context,
                                     u32 width, u32   height,
                                     VulkanSwapchain* outSwapchain);

VkResult vulkanSwapchainRecreate    (VulkanContext*   context,
                                     u32 width, u32   height,
                                     VulkanSwapchain* outSwapchain);

VkResult vulkanSwapchainDestroy     (VulkanContext*   context, VulkanSwapchain* swapchain);

VkResult  vulkanSwapchainAcquireNextImageIndex (VulkanContext* context,
                                     VulkanSwapchain* swapchain,
                                     u64              timeoutNS, 
                                     VkSemaphore      imageAvailableSemaphore, 
                                     VkFence          fence, 
                                     u32*             outImageIndex);

VkResult vulkanSwapchainPresent     (VulkanContext*   context,
                                     VulkanSwapchain* swapchain, 
                                     VkQueue          graphicsQueue,
                                     VkQueue          presentQueue,
                                     VkSemaphore      renderCompleteSemaphore,
                                     u32              presentImageIndex);

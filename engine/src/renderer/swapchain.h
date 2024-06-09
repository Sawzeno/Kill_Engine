#pragma once

#include  "vulkantypes.h"

VkResult vulkanSwapchainCreate          (vulkanContext* context,
                                     u32 width, u32 height,
                                     vulkanSwapchain* outSwapchain);

VkResult vulkanSwapchainRecreate        (vulkanContext* context,
                                     u32 width, u32 height,
                                     vulkanSwapchain* outSwapchain);

VkResult vulkanSwapchainDestroy         (vulkanContext* context, vulkanSwapchain* swapchain);

VkResult  vulkanSwapchainAcquireNextImageIndex (vulkanContext* context,
                                     vulkanSwapchain* swapchain,
                                     u64 timeoutNS, 
                                     VkSemaphore imageAvailableSemaphore, 
                                     VkFence fence, 
                                     u32* outImageIndex);

VkResult vulkanSwapchainPresent         (vulkanContext* context,
                                     vulkanSwapchain* swapchain, 
                                     VkQueue graphicsQueue,
                                     VkQueue presentQueue,
                                     VkSemaphore renderCompleteSemaphore,
                                     u32 presentImageIndex);

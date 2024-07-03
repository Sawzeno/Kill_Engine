#pragma once

#include  "vulkantypes.h"
#include <vulkan/vulkan_core.h>

#define VK_CHECK_VERBOSE(result, message)                           \
  if(result != VK_SUCCESS){                                         \
  UERROR("%s -> %s: %d, %s ",__FUNCTION__, message,result,vulkanResultToString(result)); \
  return result;                                                    \
}

#define VK_CHECK_RESULT(result, message , ...)                      \
  if(result != VK_SUCCESS){                                         \
  UERROR(message,##__VA_ARGS__);                                    \
  return result;                                                    \
}

#define VK_CHECK_BOOL(result, message, ...)                         \
  if (result != VK_SUCCESS){                                        \
  UERROR(message,##__VA_ARGS__);                                    \
  return false;                                                     \
}

#define VK_CHECK_FENCE(result)                                      \
    switch(result) {                                                \
      case VK_SUCCESS:                                              \
        fence->isSignaled = true;                                   \
        UINFO("FENCE WAS SUCCESSFULLY EXECUTED");                   \
        return true;                                                \
      case VK_TIMEOUT:                                              \
        UWARN("vkFenceWait -> FENCE TIMED OUT");                    \
        break;                                                      \
      case VK_ERROR_DEVICE_LOST:                                    \
        UERROR("vkFenceWait -> DEVICE LOST");                       \
        break;                                                      \
      case VK_ERROR_OUT_OF_HOST_MEMORY:                             \
        UERROR("vkFenceWait -> VK_ERROR_OUT_OF_HOST_MEMORY");       \
        break;                                                      \
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:                           \
        UERROR("vkFenceWait -> VK_ERROR_OUT_OF_DEVICE_MEMORY");     \
        break;                                                      \
      default:                                                      \
        UERROR("vkFenceWait -> UNKNOWN ERROR HAS OCCURRED");        \
        break;                                                      \
    }


bool            vulkanResultIsSuccess         (VkResult result);
const char*     vulkanResultToString          (VkResult result);

void            printAvailableLayers          ();
void            printAvailableExtensions      ();
VkResult        checkAvailableLayers          (const char** requiredLayers);
VkResult        checkAvailableExtensions      (const char** requiredExtensions); 

void            logVulkanContext              (const VulkanContext* context) ;
void            logVulkanDevice               (const VulkanDevice* device) ;
void            logVulkanSwapchainSupportInfo (const VulkanSwapchainSupportInfo* supportInfo) ;
void            logVulkanSwapchain            (const VulkanSwapchain* swapchain) ;
void            logVulkanImage                (const VulkanImage* image) ;
void            logVulkanRenderPass           (const VulkanRenderPass* renderPass) ;
void            logVulkanCommandBuffer        (const VulkanCommandBuffer* commandBuffer) ;
void            logVulkanFramebuffer          (const VulkanFrameBuffer* framebuffer) ;
void            logVulkanFence                (const VulkanFence* fence) ;

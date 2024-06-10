#pragma once
#include  "defines.h"
#include  "vulkantypes.h"

#define VK_CHECK(result, message)                          \
if (result != VK_SUCCESS) {                                      \
  UFATAL("%s: %d, %s", message, result, vkResultToString(result)); \
  return false;                                                 \
}

#define VK_CHECK2(result, message)\
do{                               \
if(result != VK_SUCCESS){         \
  UFATAL("%s: %d, %s", message, result, vkResultToString(result)); \
  return result;                     \
}                                 \
}while(0)                        \


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


void            printAvailableLayers          ();
void            printAvailableExtenesions     ();
VkResult        checkAvailableLayers          (const char** requiredLayers);
VkResult        checkAvailableExtensions      (const char** requiredExtensions); 
const char*     vkResultToString              (VkResult result);
u8              vulkanResultIsSuccess         (VkResult result);
void            logVulkanContext              (const VulkanContext* context) ;
void            logVulkanDevice               (const VulkanDevice* device) ;
void            logVulkanSwapchainSupportInfo (const VulkanSwapchainSupportInfo* supportInfo) ;
void            logVulkanSwapchain            (const VulkanSwapchain* swapchain) ;
void            logVulkanImage                (const VulkanImage* image) ;
void            logVulkanRenderPass           (const VulkanRenderPass* renderPass) ;
void            logVulkanCommandBuffer        (const VulkanCommandBuffer* commandBuffer) ;
void            logVulkanFramebuffer          (const struct VulkanFrameBuffer* framebuffer) ;
void            logVulkanFence                (const VulkanFence* fence) ;

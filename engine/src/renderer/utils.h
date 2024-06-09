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


void      printAvailableLayers      ();
void      printAvailableExtenesions ();
VkResult  checkAvailableLayers      (const char** requiredLayers);
VkResult  checkAvailableExtensions  (const char** requiredExtensions); 
const char* vkResultToString        (VkResult result);
u8  vulkanResultIsSuccess(VkResult result);
void logVulkanContext(const vulkanContext* context) ;
void logVulkanDevice(const vulkanDevice* device) ;
void logVulkanSwapchainSupportInfo(const vulkanSwapchainSupportInfo* supportInfo) ;
void logVulkanSwapchain(const vulkanSwapchain* swapchain) ;
void logVulkanImage(const vulkanImage* image) ;
void logVulkanRenderPass(const vulkanRenderPass* renderPass) ;
void logVulkanCommandBuffer(const vulkanCommandBuffer* commandBuffer) ;
void logVulkanFramebuffer(const struct vulkanFrameBuffer* framebuffer) ;
void logVulkanFence(const vulkanFence* fence) ;

#pragma once

#include  "vulkantypes.h"
#include <vulkan/vulkan_core.h>

#define VK_CHECK_VERBOSE(result, message)                           \
  if(result != VK_SUCCESS){                                         \
  UERROR("%s -> %s: %d, %s ",__FUNCTION__, message,result,vulkanResultToString(result)); \
  KERROR("%s -> %s: %d, %s ",__FUNCTION__, message,result,vulkanResultToString(result)); \
  return result;                                                    \
}

#define VK_CHECK_RESULT(result, message , ...)                      \
  if(result != VK_SUCCESS){                                         \
  UERROR(message,##__VA_ARGS__);                                    \
  KERROR(message,##__VA_ARGS__);                                    \
  return result;                                                    \
}

#define VK_CHECK_BOOL(result, message, ...)                         \
  if (result != VK_SUCCESS){                                        \
  UERROR(message,##__VA_ARGS__);                                    \
  KERROR(message,##__VA_ARGS__);                                    \
  return false;                                                     \
}

bool            vulkanResultIsSuccess         (VkResult result);
const char*     vulkanResultToString          (VkResult result);
VkResult        vulkanDebuggerCreate          (VulkanContext* context);

void            printAvailableLayers          ();
void            printAvailableExtensions      ();
VkResult        checkAvailableLayers          (const char** requiredLayers);
VkResult        checkAvailableExtensions      (const char** requiredExtensions); 

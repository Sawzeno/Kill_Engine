#include  "rendererutils.h"
#include  "containers/darray.h"
#include  "core/logger.h"
#include  "defines.h"

#include  <string.h>
#include  <vulkan/vulkan.h>
#include  <vulkan/vulkan_core.h>

VKAPI_ATTR VkBool32 VKAPI_CALL 
vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                VkDebugUtilsMessageTypeFlagsEXT             message_types,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void*                                       user_data)
{
  switch (message_severity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      UERROR(callback_data->pMessage);
      KERROR(callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      UWARN(callback_data->pMessage);
      KWARN(callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      KINFO(callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      KTRACE(callback_data->pMessage);
      break;
  }
  return VK_FALSE;
}


const char*
vulkanResultToString(VkResult result)
{
  switch (result) {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_EVENT_SET: return "VK_EVENT_SET";
    case VK_EVENT_RESET: return "VK_EVENT_RESET";
    case VK_INCOMPLETE: return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
    default: return "UNKNOWN_ERROR";
  }
}

bool
vulkanResultIsSuccess(VkResult result)
{
  switch (result) {
    case VK_SUCCESS:
    case VK_NOT_READY:
    case VK_TIMEOUT:
    case VK_EVENT_SET:
    case VK_EVENT_RESET:
    case VK_INCOMPLETE:
    case VK_SUBOPTIMAL_KHR:
    case VK_THREAD_IDLE_KHR:
    case VK_THREAD_DONE_KHR:
    case VK_OPERATION_DEFERRED_KHR:
    case VK_OPERATION_NOT_DEFERRED_KHR:
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
      return true; // true (success codes)

    case VK_ERROR_OUT_OF_HOST_MEMORY:
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    case VK_ERROR_INITIALIZATION_FAILED:
    case VK_ERROR_DEVICE_LOST:
    case VK_ERROR_MEMORY_MAP_FAILED:
    case VK_ERROR_LAYER_NOT_PRESENT:
    case VK_ERROR_EXTENSION_NOT_PRESENT:
    case VK_ERROR_FEATURE_NOT_PRESENT:
    case VK_ERROR_INCOMPATIBLE_DRIVER:
    case VK_ERROR_TOO_MANY_OBJECTS:
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_UNKNOWN:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    case VK_ERROR_FRAGMENTATION:
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
    case VK_ERROR_SURFACE_LOST_KHR:
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    case VK_ERROR_VALIDATION_FAILED_EXT:
    case VK_ERROR_INVALID_SHADER_NV:
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
    case VK_ERROR_NOT_PERMITTED_EXT:
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
      return false; // false (error codes)

    default:
      return false; // false (unknown codes treated as errors)
  }
}


void
printAvailableExtenesions()
{
  u32 extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

  KINFO("Available Vulkan extensions:");
  for (u32 i = 0; i < extensionCount; ++i) {
    KINFO("%s", extensions[i].extensionName);
  }
  free(extensions);
}

void
printAvailableLayers()
{
  u32 layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);

  VkLayerProperties* layers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&layerCount, layers);

  KINFO("Available Vulkan instance layers:");
  for (u32 i = 0; i < layerCount; ++i) {
    KINFO("%s", layers[i].layerName);
  }
  free(layers);
}

VkResult
checkAvailableExtensions(const char** requiredExtensions)
{
  VkResult result = {0};
  u32 extensionCount = 0;
  result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  VK_CHECK_VERBOSE(result, "failed to get number of extensionProperties");

  VkExtensionProperties* availableExtensions  = DARRAY_RESERVE(VkExtensionProperties, extensionCount);

  result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, availableExtensions);
  VK_CHECK_VERBOSE(result, "failed to get the names of extension properties");

  for (u32 i = 0; i < DARRAY_LENGTH(requiredExtensions) ; ++i) {
    u8 found = false;
    const char* requiredExtension = requiredExtensions[i];
    for (u32 j = 0; j < extensionCount; ++j) {
      if (strcmp(requiredExtension, availableExtensions[j].extensionName) == 0) {
        KINFO("FOUND EXTENSION: %s", requiredExtension);
        found = true;
        break;
      }
    }
    if (!found) {
      KFATAL("REQUIRED EXTENSION %s IS MISSING", requiredExtension);
      free(availableExtensions);
      return !VK_SUCCESS;
    }
  }

  KINFO("ALL REQUIRED EXTENSIONS ARE PRESENT");
  DARRAY_DESTROY(availableExtensions);
  return VK_SUCCESS;
}

VkResult
checkAvailableLayers(const char** requiredLayers)
{
  u32 layerCount = 0;
  VkResult result = {0};
  result =  vkEnumerateInstanceLayerProperties(&layerCount, NULL);

  VK_CHECK_VERBOSE(result, "failed to get numner of layers");
  VkLayerProperties*  availableLayers = DARRAY_RESERVE(VkLayerProperties, layerCount);

  result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
  VK_CHECK_VERBOSE(result, "failed to names of layers");

  for(u32 i = 0; i < DARRAY_LENGTH(requiredLayers); ++i){
    u8 found = false;
    const char* layer = requiredLayers[i];
    for( u32 j= 0; j < layerCount; ++j ){
      if(strcmp(layer, availableLayers[j].layerName) == 0){
        KINFO("FOUND LAYER  %s",layer);
        found = true;
        break;
      }
    }
    if(!found){
      KFATAL("REQUIRED LAYER %s IS MISSING", layer);
      return !VK_SUCCESS;
    }
  }
  KINFO("ALL REQUIRED LAYERS ARE PRESENT");
  DARRAY_DESTROY(availableLayers);
  return VK_SUCCESS;
}


VkResult
vulkanDebuggerCreate (VulkanContext* context)
{
TRACEFUNCTION;
u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  debugCreateInfo.messageSeverity = logSeverity;
  debugCreateInfo.pfnUserCallback = vkDebugCallback;
  debugCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     | 
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  ;

  PFN_vkCreateDebugUtilsMessengerEXT func =
    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");

  VkResult  result =  func(context->instance, &debugCreateInfo, context->allocator, &context->debugMessenger);
  VK_CHECK_VERBOSE(result, "vkGetInstanceProcAddr failed in vulknaDebuggerCreate");
  KINFO("VULKAN DEBUGGER CREATED");
  return result;
}


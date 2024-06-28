#include  "rendererutils.h"
#include  "containers/darray.h"
#include  "core/logger.h"
#include  "defines.h"

#include  <string.h>
#include  <vulkan/vulkan.h>
#include  <vulkan/vulkan_core.h>

const char* vkResultToString(VkResult result) {
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


u8 vulkanResultIsSuccess(VkResult result) {
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


void printAvailableExtenesions(){
  u32 extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

  KTRACE("Available Vulkan extensions:");
  for (u32 i = 0; i < extensionCount; ++i) {
    KTRACE("%s", extensions[i].extensionName);
  }
}

void printAvailableLayers() {
  u32 layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);

  VkLayerProperties* layers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&layerCount, layers);

  KTRACE("Available Vulkan instance layers:");
  for (u32 i = 0; i < layerCount; ++i) {
    KTRACE("%s", layers[i].layerName);
  }
}

VkResult checkAvailableExtensions(const char** requiredExtensions) {
  VkResult result = {0};
  u32 extensionCount = 0;
  result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  VK_CHECK2(result, "failed to get number of extensionProperties");

  VkExtensionProperties* availableExtensions = _darrayCreate(extensionCount, sizeof(VkExtensionProperties));
  MEMERR(availableExtensions);

  result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, availableExtensions);
  VK_CHECK2(result, "failed to get the names of extension properties");

  for (u32 i = 0; i < DARRAY_LENGTH(requiredExtensions) ; ++i) {
    u8 found = false;
    const char* requiredExtension = requiredExtensions[i];
    for (u32 j = 0; j < extensionCount; ++j) {
      if (strcmp(requiredExtension, availableExtensions[j].extensionName) == 0) {
        KTRACE("FOUND EXTENSION: %s", requiredExtension);
        found = true;
        break;
      }
    }
    if (!found) {
      KFATAL("Required extension is missing: %s", requiredExtension);
      free(availableExtensions);
      return !VK_SUCCESS;
    }
  }

  KINFO("ALL REQUIRED EXTENSIONS ARE PRESENT");
  DARRAY_DESTROY(availableExtensions);
  return VK_SUCCESS;
}

VkResult checkAvailableLayers(const char** requiredLayers){

  u32 layerCount = 0;
  VkResult result = {0};
  result =  vkEnumerateInstanceLayerProperties(&layerCount, NULL);
  VK_CHECK2(result, "failed to get numner of layers");
  VkLayerProperties* availableLayers = _darrayCreate(layerCount, sizeof(VkLayerProperties));
  MEMERR(availableLayers);
  result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
  VK_CHECK2(result, "failed to names of layers");

  for(u32 i = 0; i < DARRAY_LENGTH(requiredLayers); ++i){
    u8 found = false;
    const char* layer = requiredLayers[i];
    for( u32 j= 0; j < layerCount; ++j ){
      if(strcmp(layer, availableLayers[j].layerName) == 0){
        KTRACE("FOUND LAYER : %s",layer);
        found = true;
        break;
      }
    }
    if(!found){
      KFATAL("%s is missing", layer);
      return !VK_SUCCESS;
    }
  }
  KINFO("ALL REQUIRED LAYERS ARE PRESENT");
  DARRAY_DESTROY(availableLayers);
  return VK_SUCCESS;
}

//:%s/\(\w\+\)_\(\w\+\)/\1\2/g
void logVulkanFence(const VulkanFence* fence) {
  UINFO("Vulkan Fence Details:");
  UINFO("  Handle: %p", (void*)fence->handle);
  UINFO("  Is Signaled: %hhu", fence->isSignaled);
}

void logVulkanFrameBuffer(const struct VulkanFrameBuffer* framebuffer) {
  UINFO("Vulkan Framebuffer Details:");
  UINFO("  Handle: %p",      (void*)framebuffer->handle);
  UINFO("  Attachment Count: %u",   framebuffer->attachmentCount);
  UINFO("  Render Pass: %p", (void*)framebuffer->renderPass);
}

void logVulkanCommandBuffer(const VulkanCommandBuffer* commandBuffer) {
  UINFO("Vulkan Command Buffer Details:");
  UINFO("  Handle: %p", (void*)commandBuffer->handle);
  // Print state if needed
  // KINFO("  State: ...");
}

void logVulkanRenderPass(const VulkanRenderPass* renderPass) {
  UINFO("Vulkan Render Pass Details:");
  UINFO("  Handle: %p", (void*)renderPass->handle);
  UINFO("  x: %f, y: %f, w: %f, h: %f", renderPass->x, renderPass->y, renderPass->w, renderPass->h);
  UINFO("  r: %f, g: %f, b: %f, a: %f", renderPass->r, renderPass->g, renderPass->b, renderPass->a);
  UINFO("  Depth: %f", renderPass->depth);
  UINFO("  Stencil: %u", renderPass->stencil);
  // Print state if needed
  // KINFO("  State: ...");
}

void logVulkanImage(const VulkanImage* image) {
  UINFO("Vulkan Image Details:");
  UINFO("  Handle: %p", (void*)image->handle);
  UINFO("  Memory: %p", (void*)image->memory);
  UINFO("  View: %p",   (void*)image->view);
  UINFO("  Width: %u",         image->width);
  UINFO("  Height: %u",        image->height);
}


void logVulkanSwapchain(const VulkanSwapchain* swapchain) {
  UINFO("Vulkan Swapchain Details:");
  UINFO("  Handle: %p", (void*)        swapchain->handle);
  UINFO("  Image Format: %d",          swapchain->imageFormat.format);
  UINFO("  Max Frames In Flight: %hu", swapchain->maxFramesInFlight);
  UINFO("  Image Count: %u",           swapchain->imageCount);
  // Print details of images and views if needed
  for (u32 i = 0; i < swapchain->imageCount; ++i) {
    UINFO("  Image[%u] Handle: %p", i, (void*)swapchain->images[i]);
    UINFO("  Image[%u] View: %p", i,   (void*)swapchain->views[i]);
  }
  UINFO("  Depth Attachment: Handle: %p, Memory: %p, View: %p, Width: %u, Height: %u",
        (void*)swapchain->depthAttachment.handle,
        (void*)swapchain->depthAttachment.memory,
        (void*)swapchain->depthAttachment.view,
               swapchain->depthAttachment.width,
               swapchain->depthAttachment.height);
  UINFO("  Frame Buffers: %p", (void*)swapchain->frameBuffers);
}

void logVulkanSwapchainSupportInfo(const VulkanSwapchainSupportInfo* supportInfo) {
  KINFO("Vulkan Swapchain Support Info Details:");
  KINFO("  Capabilities: ..."); 
  for (u32 i = 0; i < supportInfo->formatCount; ++i) {
    UINFO("  Format[%u]: ...", i); 
  }
  for (u32 i = 0; i < supportInfo->presentModesCount; ++i) {
    UINFO("  Present Mode[%u]: ...", i);
  }
  UINFO("  Format Count: %u", supportInfo->formatCount);
  UINFO("  Present Modes Count: %u", supportInfo->presentModesCount);
}

void logVulkanDevice(const VulkanDevice* device) {
  UINFO("Vulkan Device Details:");
  UINFO("  Physical Device: %p",(void*)device->physicalDevice);
  UINFO("  Logical Device: %p", (void*)device->logicalDevice);
  UINFO("  Graphics Queue Index: %d",  device->graphicsQueueIndex);
  UINFO("  Present Queue Index: %d",   device->presentQueueIndex);
  UINFO("  Transfer Queue Index: %d",  device->transferQueueIndex);
  UINFO("  Graphics Queue: %p", (void*)device->graphicsQueue);
  UINFO("  Present Queue: %p",  (void*)device->presentQueue);
  UINFO("  Transfer Queue: %p", (void*)device->transferQueue);
  UINFO("  Graphics Command Pool: %p", (void*)device->graphicsCommandPool);
  UINFO("  Depth Format: %d",          device->depthFormat);
}


void logVulkanContext(const VulkanContext* context) {
  UINFO("Vulkan Context Details:");
  UINFO("  Framebuffer Width: %u",                  context->frameBufferWidth);
  UINFO("  Framebuffer Height: %u",                 context->frameBufferHeight);
  UINFO("  Framebuffer Size Generation: %llu",      context->frameBufferSizeGeneration);
  UINFO("  Framebuffer Size Last Generation: %llu", context->frameBufferSizeLastGeneration);
  UINFO("  Vulkan Instance: %p",             (void*)context->instance);
  UINFO("  Allocator: %p",                   (void*)context->allocator);
  UINFO("  Surface: %p",                     (void*)context->surface);
  UINFO("  Debug Messenger: %p",             (void*)context->debugMessenger);
  UINFO("  Vulkan Device: %p",              (void*)&context->device);
  UINFO("  Swapchain: %p",                  (void*)&context->swapchain);
  UINFO("  Main Render Pass: %p",           (void*)&context->mainRenderPass);
  UINFO("  Image Index: %u",                        context->imageIndex);
  UINFO("  Current Frame: %u",                      context->currentFrame);
  UINFO("  Recreating Swapchain: %u",               context->recreatingSwapchain);
  UINFO("  Graphics Command Buffers: %p",    (void*)context->graphicsCommandBuffers);
  UINFO("  Image Available Semaphores: %p",  (void*)context->imageAvailableSemaphores);
  UINFO("  Queue Complete Semaphores: %p",   (void*)context->queueCompleteSemaphores);
  UINFO("  In Flight Fence Count: %u",              context->inFlightFenceCount);
  UINFO("  In Flight Fences: %p",            (void*)context->inFlightFences);
  UINFO("  Images In Flight: %p",            (void*)context->imagesInFlight);
  UINFO("  Find Memory Index Function: %p",  (void*)context->findMemoryIndex);
}


#include  "rendererbackend.h"
#include  "rendererutils.h"
#include  "core/logger.h"

#include  "vulkantypes.h"
#include  "../core/application.h"
#include "core/kmemory.h"
#include  "vulkanplatform.h"
#include  "device.h"
#include  "swapchain.h"
#include  "renderpass.h"
#include  "commandbuffer.h"
#include  "framebuffer.h"
#include  "fence.h"

#include  "shaders/vulkanshaderobject.h"

#include  "containers/darray.h"
#include  "defines.h"
#include  "string.h"

static  u32 cachedFrameBufferWidth    = 0;
static  u32 cachedFrameBufferHeight   = 0;
static  char* applicationName;

static VulkanContext context;
static const char* validationLayers = "VK_LAYER_KHRONOS_validation";
u8    recreateSwapchain();

i32 findMemoryIndex (u32 typeFilter, u32 propertyFlags);

VkResult  createCommandBuffers();
VkResult  regenerateFrameBuffers(VulkanSwapchain* swapchain, VulkanRenderPass* renderPass);

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
  VkDebugUtilsMessageTypeFlagsEXT message_types,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* user_data);



u8  rendererBackendInitialize(){
TRACEFUNCTION;
  //function pointers
  context.findMemoryIndex = findMemoryIndex;
  //TODO
  context.allocator = NULL;
  //get applications size
  applicationGetFrameBufferSize (&cachedFrameBufferWidth, &cachedFrameBufferHeight);
  applicationGetName            (applicationName);
  context.frameBufferWidth    = (cachedFrameBufferWidth != 0 ? cachedFrameBufferWidth : 1280);
  context.frameBufferHeight   = (cachedFrameBufferHeight!= 0 ? cachedFrameBufferHeight: 1440);
  //check for each Vulkan function
  VkResult result  = {0};
  //Vulkan INSTANCE
  VkApplicationInfo appInfo   = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.apiVersion          = VK_API_VERSION_1_2;
  appInfo.pApplicationName    = applicationName;
  appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName         = "KILL ENGINE";
  appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);

  KDEBUG("----------------------REQUIRED EXTENSIONS----------------------");
  const char** requiredExtensions   = (const char**)_darrayCreate(1,sizeof(const char*));
  DARRAY_PUSH(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME);
  DARRAY_PUSH(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  DARRAY_PUSH(requiredExtensions, &"VK_KHR_xcb_surface");
  result  =  checkAvailableExtensions(requiredExtensions);
  VK_CHECK(result, "REQUIRED EXTENSIONS NOT AVAILABLE");

  KDEBUG("----------------------REQUIRED LAYERS----------------------");
  const char**  requiredLayers  = (const char**)DARRAY_CREATE(const char*);
  DARRAY_PUSH(requiredLayers, &"VK_LAYER_KHRONOS_validation");
  result  = checkAvailableLayers(requiredLayers);
  VK_CHECK(result, "REQUIRED LAYERS NOT AVAILABLE");

  KDEBUG("----------------------Vulkan INSTANCE----------------------");
  VkInstanceCreateInfo createInfo     = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  createInfo.pApplicationInfo         = &appInfo;
  createInfo.enabledExtensionCount    = DARRAY_LENGTH(requiredExtensions);
  createInfo.ppEnabledExtensionNames  = requiredExtensions;
  createInfo.enabledLayerCount        = DARRAY_LENGTH(requiredLayers);
  createInfo.ppEnabledLayerNames      = requiredLayers;

  context.allocator = 0;
  result  = vkCreateInstance(&createInfo, context.allocator, &(context.instance));
  VK_CHECK(result, "FAILED TO CREATE INSTANCE");
  KINFO("VULKAN ISNTANCE CREATED");
  UINFO("VULKAN ISNTANCE CREATED");

  KDEBUG("----------------------Vulkan DEBUGGER----------------------");
  KDEBUG("CREATING VULKAN DEBUGGER");
  u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT    |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT  |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  debugCreateInfo.messageSeverity = logSeverity;
  debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     | 
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugCreateInfo.pfnUserCallback = vkDebugCallback;

  PFN_vkCreateDebugUtilsMessengerEXT func =
    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");

  result =  func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger);
  VK_CHECK(result, "FAILED TO CREATE DEBUGGER");
  KINFO("Vulkan DEBUGGER CREATED");

  KDEBUG("----------------------Vulkan SURFACE----------------------");
  KINFO("CREATING VULKAN SURFACE");
  result  = createVulkanSurface(&context);
  VK_CHECK(result, "FAILED TO CREATE Vulkan SURFACE");

  KDEBUG("----------------------Vulkan DEVICE----------------------");
  KINFO("CREATING VULKAN DEVICE");
  result =  vulkanDeviceCreate(&context);
  VK_CHECK(result, "FAILED TO CREATE DEVICE !");

  KDEBUG("----------------------SWAPCHAIN----------------------");
  KINFO("CREATING VULKAN SWAPCHAIN");
  result = vulkanSwapchainCreate(&context, context.frameBufferWidth, context.frameBufferHeight, &context.swapchain);
  VK_CHECK(result, "FAILED TO CREATE SWAPCHAIN");

  KDEBUG("----------------------RENDERPASS----------------------");
  KINFO("CREATING VULKAN RENDERPASS");
  result =  vulkanRenderPassCreate(&context,
                                   &context.mainRenderPass,
                                   0, 0, context.frameBufferWidth, context.frameBufferHeight,
                                   0.0f, 0.0f, 0.2f, 1.0f,
                                   1.0f, 0);
  VK_CHECK(result, "FAILED TO CREATE RENDERPASS");

  KDEBUG("----------------------SWAPCHAIN FRAMEBUFFERS----------------------");
  KINFO("CREATING VULKAN FRAMEBUFFERS");
  context.swapchain.frameBuffers  = DARRAY_RESERVE(VulkanFrameBuffer, context.swapchain.imageCount);
  regenerateFrameBuffers(&context.swapchain, &context.mainRenderPass);

  KDEBUG("----------------------COMMAND BUFFERS----------------------");
  KINFO("CREATING COMMAND BUFFERS");
  result = createCommandBuffers();
  VK_CHECK(result, "FAILED TO CREATE COMMAND BUFFERS");

  KDEBUG("----------------------SYNC OBJECTS----------------------");
  KINFO("CREATING SYNC OBJCTS");
  context.imageAvailableSemaphores  = DARRAY_RESERVE(VkSemaphore, context.swapchain.maxFramesInFlight);
  context.queueCompleteSemaphores   = DARRAY_RESERVE(VkSemaphore, context.swapchain.maxFramesInFlight);
  context.inFlightFences            = DARRAY_RESERVE(VulkanFence, context.swapchain.maxFramesInFlight);

  for(u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i){
    VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    result  = vkCreateSemaphore(context.device.logicalDevice,&semaphoreCreateInfo,context.allocator,&context.imageAvailableSemaphores[i]);
    result  = vkCreateSemaphore(context.device.logicalDevice,&semaphoreCreateInfo,context.allocator,&context.queueCompleteSemaphores[i]);

    //Create the fence in a sinfaled state, indicating hat the FIRST FRAME HAS ALREADY BEED RENDERED.
    //this will prevent the application from waiting indefinitely for the first frame to render since i t
    //cannot be renderer until a frame is RENDERED BEFORE it
    result = vulkanFenceCreate(&context, true, &context.inFlightFences[i]);  
    VK_CHECK(result, "FAILED TO CERATE SEMAPHORES");
  }

  // in flight images should not exist at this point, so CLEAR THE LIST , these are sorted in POINTERS
  // because the initial state shiould be zero , and will be 0 when not in use, ACTUAL FENCES ARE NOT 
  // OWNED BY THE LIST

  context.imagesInFlight  = DARRAY_RESERVE(VulkanFence, context.swapchain.imageCount);
  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    context.imagesInFlight[i] = 0;
  }

  KDEBUG("----------------------SHADER OBJECTS----------------------");
  KINFO("CREATING SHADER OBJECTS");
  result  = vulkanObjectShaderCreate(&context, &context.objectShader);
  VK_CHECK(result, "ERROR LOADING BUILTIN BASIC LIGHTING SHADER");
  
  
  KINFO("Vulkan RENDERER BACKEND INITIALIZED");
  UINFO("Vulkan RENDERER BACKEND INITIALIZED");
  return true;
}

//-----------------------------------------------------RENDERER BEGIN FRAME---------------------------------------------------------

u8  rendererBackendBeginFrame(f32 deltaTime){
  KTRACE("----------------------BEGIN FRAME-----------------");
  KDEBUG("deltatime : %f",deltaTime);
  VulkanDevice* device  = &context.device;
    
  // logVulkanContext    (&context);
  // logVulkanDevice     (&context.device) ;
  // logVulkanSwapchain  (&context.swapchain) ;
  // logVulkanImage      (&context.swapchain.depthAttachment) ;
  // logVulkanRenderPass (&context.mainRenderPass) ;
  // logVulkanCommandBuffer(context.graphicsCommandBuffers) ;
  //check if recreating swap chain and boot out 
  if(context.recreatingSwapchain){
    KWARN("MIDST OF RECREATING SWAPCHAIN");
    VkResult result =  vkDeviceWaitIdle(device->logicalDevice);
    if(!vulkanResultIsSuccess(result)){
      UERROR("rendererBackendBeginFrame vkDeviceWaitIdle (1) failed : %s", vkResultToString(result));
      return false;
    }
    KINFO("RECREATED SWAPCHAIN, BOOTED!");
    return false;
  }

  //check if frameBuffer has been resized , if so then swapchain must be recreated 
  if(context.frameBufferSizeGeneration != context.frameBufferSizeLastGeneration){
    KWARN("FRAME BUFFER SIZE CAHANGED");
    VkResult result = vkDeviceWaitIdle(device->logicalDevice);
    //if swapchain recreation failed (ex : if the window was minimized);
    VK_CHECK(result, "SWAPCHAIN RECRETION FAILED");

    if(!recreateSwapchain()){
      
      return false;
    }

    KWARN("RECREATING SWAPCHAIN BOOTING!!");
    return false;
  }

  //wait for execution of the current frame to complete, the fence being free will allow this one to move on
  if(!vulkanFenceWait(&context,
                      &context.inFlightFences[context.currentFrame],
                      UINT64_MAX)){
    KWARN("IN FLIGHT FENCE WAIT FAILURE !");
  }

  //acquire the next image from the swapchain. pass along the semaphore that should be signaled when it completes
  //the same semaphore will be waited on by the queue submission to ensure this image is available.
  if(VK_SUCCESS != vulkanSwapchainAcquireNextImageIndex(&context,
                                           &context.swapchain,
                                           UINT64_MAX,
                                           context.imageAvailableSemaphores[context.currentFrame],
                                           0,
                                           &context.imageIndex)){
    KWARN("FAILED TO ACQUIRE NEXT IMAGE INDEX FRM SWAPCHAIN");
    return false;
  }

  //begin recording commands
  VulkanCommandBuffer* commandBuffer  =  &context.graphicsCommandBuffers[context.imageIndex];
  vulkanCommandBufferReset(commandBuffer);
  vulkanCommandBufferBegin(commandBuffer, false, false, false);

  //dynamic state
  VkViewport viewport = {0};
  viewport.x          = 0.0f;
  viewport.y          = 0.0f;
  viewport.width      = (f32)context.frameBufferWidth;
  viewport.height     = (f32)context.frameBufferHeight;
  viewport.maxDepth   = 0.0f;
  viewport.minDepth   = 1.0f;

  //Scissoring
  VkRect2D scissoring;
  scissoring.offset.x = scissoring.offset.y = 0;
  scissoring.extent.width   = context.frameBufferWidth;
  scissoring.extent.height  = context.frameBufferHeight;

  vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissoring);

  context.mainRenderPass.w  = context.frameBufferWidth;
  context.mainRenderPass.h  = context.frameBufferHeight;

  //begin the render pass
  vulkanRenderPassBegin(commandBuffer,
                        &context.mainRenderPass,
                        context.swapchain.frameBuffers[context.imageIndex].handle);

  
  return true;
}

//-----------------------------------------------------RENDERER END FRAME---------------------------------------------------------

u8  rendererBackendEndFrame(f32 deltaTime){
  KTRACE("----------------------END FRAME-------------------");
  KDEBUG("deltatime : %f",deltaTime);
  VulkanCommandBuffer* commandBuffer  = &context.graphicsCommandBuffers[context.imageIndex];
  //end renderpass
  vulkanRenderPassEnd(commandBuffer, &context.mainRenderPass);
  //end commandBuffer
  vulkanCommandBufferEnd(commandBuffer);
  //make sure the previous frame is not using this image (IE : its fence is not being waited upon);
  if(context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE){ 
    vulkanFenceWait(&context,
                    context.imagesInFlight[context.imageIndex],
                    UINT64_MAX);
  }
  //mark the iamge as in use by the frame
  context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];
  //reset the fence
  vulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

  //submit the queue and wait for the opertion to complete.
  //begin Queue submission
  VkSubmitInfo  submitInfo  = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  //command buffers to be executed.
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer->handle;
  // the semaphores to be signaled when the queue is complete
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores  = &context.queueCompleteSemaphores[context.currentFrame];
  //wait sempahores ensure that the operation cannot begin untill the image is available
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = &context.imageAvailableSemaphores[context.currentFrame];
  //each SEMAPHORE waits on the corresponding pipeline stage to complete, 1:1 ratio.
  //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
  //writes from executing until the semaphore signal (IE : one frame is presented at a time)
  // this is how we take our triple buffering and make sure oen frame is presented at a time
  VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask  = flags;
  VkResult result = vkQueueSubmit(context.device.graphicsQueue,
                                  1,
                                  &submitInfo,
                                  context.inFlightFences[context.currentFrame].handle);
  if(result != VK_SUCCESS){
    UERROR("vkQueueSubmit failed");
    vkResultToString(result);
  }
  //end queue submission
  vulkanCommandBufferUpdateSubmitted(commandBuffer);


  // give the image back to swapchain
  result = vulkanSwapchainPresent(&context,
                         &context.swapchain,
                         context.device.graphicsQueue,
                         context.device.presentQueue,
                         context.queueCompleteSemaphores[context.currentFrame],
                         context.imageIndex);

  VK_CHECK(result, "SOMETHING WRONG WITH VulkanSwapchainPresent");
  return true;
}

//------------------------------------------------createCommandBuffers---------------------------------
VkResult createCommandBuffers(){
  KTRACE("-------------------CREATING COMMAND BUFFERS-------");
  VkResult  result = {0};
  if(!context.graphicsCommandBuffers){
    context.graphicsCommandBuffers  = DARRAY_RESERVE(VulkanCommandBuffer, context.swapchain.imageCount);
    for(u32 i = 0; i < context.swapchain.imageCount; ++i){
      kzeroMemory(&context.graphicsCommandBuffers[i],
                  sizeof(VulkanCommandBuffer));
    }
  }

  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    if(context.graphicsCommandBuffers[i].handle){
      result  = vulkanCommandBufferFree(&context,
                              context.device.graphicsCommandPool,
                              &context.graphicsCommandBuffers[i]);
      VK_CHECK2(result, __FUNCTION__);
    }
    kzeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
    result = vulkanCommandBufferAllocate(&context,
                                context.device.graphicsCommandPool, true,
                                &context.graphicsCommandBuffers[i]);

      VK_CHECK2(result, __FUNCTION__);
  }
  KINFO("CREATED COMMAND BUFFERS");
  return result;
}
//------------------------------------------------regenerateFrameBuffers------------------------------------
VkResult  regenerateFrameBuffers(VulkanSwapchain* swapchain, VulkanRenderPass* renderPass){
  KTRACE("-------------------REGENERATE SWAPCHAIN-----------");
  KDEBUG("swapchain : %p renderpass : %p", swapchain, renderPass);
  VkResult result = {0};

  for(u32 i = 0; i < swapchain->imageCount; ++i){
    u32 attachmentCount = 2;
    VkImageView attachments[] = {swapchain->views[i], swapchain->depthAttachment.view};

    result = vulkanFrameBufferCreate(&context, renderPass,
                            context.frameBufferWidth,
                            context.frameBufferHeight,
                            attachmentCount,
                            attachments, 
                            &context.swapchain.frameBuffers[i]);
    VK_CHECK2(result, __FUNCTION__);
  } 

  return result;
}

//------------------------------------------------RECREATE SWAPCHAIN------------------------------------
u8 recreateSwapchain(){
  KTRACE("-------------------RECREATING SWAPCHAIN-----------");
  // if already created do not try again
  VkResult result = {0};
  if(context.recreatingSwapchain){
    KDEBUG("recreting swapchin called when already recreating, booting !!");
    return false;
  }

  //detect if windows is too small to be drawn to
  if(context.frameBufferWidth == 0 || context.frameBufferHeight == 0){
    KDEBUG("recreateSwapchain called when window size is too small");
    return false;
  }

  // ------------------------------------------------------mark as recreating if dimensions are vaild
  context.recreatingSwapchain = true;

  // ------------------------------------------------------wait for any opertions to complete
  result = vkDeviceWaitIdle(context.device.logicalDevice);
  VK_CHECK(result, __FUNCTION__);

  //-------------------------------------------------------clear these out just in case
  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    context.imagesInFlight[i] = 0;
  }

  // requery support
  result = vulkanDeviceQuerySwapchainSupport(context.device.physicalDevice,
                                    context.surface,
                                    &context.device.swapchainSupport);
  VK_CHECK(result ,__FUNCTION__);
  result = vulkanDeviceDetectDepthFormat(&context.device);
  VK_CHECK(result ,__FUNCTION__);
  result = vulkanSwapchainRecreate(&context,
                          cachedFrameBufferWidth,
                          cachedFrameBufferHeight,
                          &context.swapchain);
  VK_CHECK(result ,__FUNCTION__);
  //sync the framebuffer size with the cached sizes
  context.frameBufferWidth  = cachedFrameBufferWidth;
  context.frameBufferHeight = cachedFrameBufferHeight;
  context.mainRenderPass.w  = context.frameBufferWidth;
  context.mainRenderPass.h  = context.frameBufferHeight;
  cachedFrameBufferWidth  = 0;
  cachedFrameBufferHeight = 0;

  //update frameBuffer size generation
  context.frameBufferSizeLastGeneration = context.frameBufferSizeGeneration;

  // ---------------------------------------------------------------------------------destroy commandBuffers
  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    result = vulkanCommandBufferFree(&context, 
                                     context.device.graphicsCommandPool,
                                     &context.graphicsCommandBuffers[i]);
    VK_CHECK(result, __FUNCTION__);
  }
  // ---------------------------------------------------------------------------------destroy framebuffers
  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    result = vulkanFrameBufferDestroy(&context,&context.swapchain.frameBuffers[i]);
    VK_CHECK(result, __FUNCTION__);
  }

  context.mainRenderPass.x  = 0;
  context.mainRenderPass.y  = 0;
  context.mainRenderPass.w  = context.frameBufferWidth;
  context.mainRenderPass.h  = context.frameBufferHeight;

  result = regenerateFrameBuffers(&context.swapchain, &context.mainRenderPass);
  VK_CHECK(result, __FUNCTION__);

  createCommandBuffers();

  //clear the recreating flag
  context.recreatingSwapchain = false;

  return true;
}

//-----------------------------------------------------RENDERER RESIZED---------------------------------------------------------


u8  rendererBackendResized(u16 width, u16 height){
TRACEFUNCTION;
KDEBUG("width : %"PRIu64" height : %"PRIu64"",width,height);
  UWARN("------RENDERER BACKEND RESIZE INITIALTED-------");
  cachedFrameBufferWidth   = width;
  cachedFrameBufferHeight  = height;
  context.frameBufferSizeGeneration++;

  UINFO("resized[%i %i %i]", width, height, context.frameBufferSizeGeneration);

  return true;
}

u8  rendererBackendShutdown(){
TRACEFUNCTION;
  vkDeviceWaitIdle(context.device.logicalDevice);

  KDEBUG("DESTROYING SYNC OBJECTS");
  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    if(context.imageAvailableSemaphores[i]){
      vkDestroySemaphore(context.device.logicalDevice,
                         context.imageAvailableSemaphores[i],
                         context.allocator);
      context.imageAvailableSemaphores[i] = 0;
    }
    if(context.queueCompleteSemaphores[i]){
      vkDestroySemaphore(context.device.logicalDevice,
                         context.queueCompleteSemaphores[i],
                         context.allocator);
      context.queueCompleteSemaphores[i] = 0;
    }

    vulkanFenceDestroy(&context, &context.inFlightFences[i]);
  }
  
  DARRAY_DESTROY(context.queueCompleteSemaphores);
  context.queueCompleteSemaphores = 0;
  DARRAY_DESTROY(context.imageAvailableSemaphores);
  context.imageAvailableSemaphores = 0;
  DARRAY_DESTROY(context.inFlightFences);
  context.inFlightFences = 0;
  DARRAY_DESTROY(context.imagesInFlight);
  context.imagesInFlight = 0;

  KINFO("DESTROYING COMMAND BUFFERS");
  for(u32 i = 0 ;i< context.swapchain.imageCount ; ++i){
    if(context.graphicsCommandBuffers[i].handle){
      vulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
      context.graphicsCommandBuffers[i].handle = 0;
    }
  }

  KINFO("DESTROYING FRAMEBUFFERS");
  for(u32 i = 0; i < context.swapchain.imageCount; ++i){
    vulkanFrameBufferDestroy(&context, &context.swapchain.frameBuffers[i]);
  }


  DARRAY_DESTROY(context.graphicsCommandBuffers);
  context.graphicsCommandBuffers = 0;

  KINFO("DESTROYING RENDERPASS");
  vulkanRenderPassDestroy(&context, &context.mainRenderPass);

  KINFO("DESTORYIIG Vulkan SWAPCHAIN");
  vulkanSwapchainDestroy(&context, &context.swapchain);

  KINFO("DESTOYING Vulkan DEVICE");
  vulkanDeviceDestroy(&context);

  KINFO("DESTROYING Vulkan SURFACE");
  if(context.surface){
    vkDestroySurfaceKHR(context.instance,context.surface ,context.allocator);
    context.surface = NULL;
  }

  KINFO("Destroying Vulkan debugger...");
  if (context.debugMessenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debugMessenger, context.allocator);
  }

  KINFO("Destroying Vulkan instance...");
  vkDestroyInstance(context.instance, context.allocator);
  return true;
}

i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags){
  TRACEFUNCTION;
  VkPhysicalDeviceMemoryProperties  memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);
  for(u32 i =0; i < memoryProperties.memoryTypeCount; ++i){
    if(typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags){
      KINFO("MEMORY TYPE FOUND");
      return i;
    }
  }

  UWARN("UNABLE TO FIND MEMORY TYPE");
  return -1;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
  VkDebugUtilsMessageTypeFlagsEXT message_types,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* user_data) {
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

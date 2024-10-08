#include  "rendererbackend.h"
#include  "renderertypes.h"
#include  "rendererutils.h"

#include  "vulkanplatform.h"
#include  "vulkandevice.h"
#include  "vulkanswapchain.h"
#include  "vulkanrenderpass.h"
#include  "vulkancommandbuffer.h"
#include  "vulkanframebuffer.h"
#include  "vulkanimage.h"
#include  "vulkanbuffer.h"
#include  "vulkanfence.h"

#include  "core/logger.h"
#include  "core/kmemory.h"

#include  "containers/darray.h"

#include  "math/mathtypes.h"

#include  "shaders/materialshader.h"

static    u32 cachedFrameBufferWidth    = 0;
static    u32 cachedFrameBufferHeight   = 0;

static    VulkanContext ctx;
static    const char* validationLayers = "VK_LAYER_KHRONOS_validation";

i32       findMemoryIndex       (u32 typeFilter, u32 propertyFlags);
VkResult  recreateSwapchain     ();
VkResult  createCommandBuffers  ();
VkResult  regenerateFrameBuffers(VulkanSwapchain* swapchain,VulkanRenderPass* renderPass);
VkResult  createBuffers         (VulkanContext* ctx);
VkResult  uploadDataRange       (VulkanContext* ctx, VkCommandPool pool, VkFence fence ,
                                 VkQueue queue, VulkanBuffer* buffer, u64 offset, u64 size, void* data);

//-----------------------------------------------------FIND MEMORY IDEX---------------------------------------------------------

i32
findMemoryIndex(u32 typeFilter, u32 propertyFlags)
{
  TRACEFUNCTION;
  VkPhysicalDeviceMemoryProperties  memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(ctx.device.physicalDevice, &memoryProperties);
  for(u32 i =0; i < memoryProperties.memoryTypeCount; ++i){
    if(typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags){
      KDEBUG("MEMORY TYPE FOUND");
      return i;
    }
  }
  UWARN("UNABLE TO FIND MEMORY TYPE");
  return -1;
}

b32
rendererBackendResized(u16 width, u16 height)
{
  TRACEFUNCTION;
  cachedFrameBufferWidth   = width;
  cachedFrameBufferHeight  = height;
  ctx.frameBufferSizeGeneration++;
  return TRUE;
}

b32
rendererBackendInitialize(RendererBackend* backend)
{
  TRACEFUNCTION;
  //function pointers
  ctx.findMemoryIndex = findMemoryIndex;
  //TODO
  ctx.allocator = NULL;
  //get applications info
  cachedFrameBufferWidth  = backend->applicationWidth;
  cachedFrameBufferHeight = backend->applicationHeight;

  ctx.frameBufferWidth    = (cachedFrameBufferWidth != 0 ? cachedFrameBufferWidth : 1280);
  ctx.frameBufferHeight   = (cachedFrameBufferHeight!= 0 ? cachedFrameBufferHeight: 1440);

  VkApplicationInfo appInfo   = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.apiVersion          = VK_API_VERSION_1_2;
  appInfo.pApplicationName    = backend->applicationName;
  appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName         = "KILL ENGINE";
  appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);

  VkResult result  = !VK_SUCCESS;
  KTRACE("----------------------REQUIRED EXTENSIONS------------------");
  const char** requiredExtensions   = (const char**)DARRAY_CREATE(const char*);
  DARRAY_PUSH(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME);
  DARRAY_PUSH(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  DARRAY_PUSH(requiredExtensions, &"VK_KHR_xcb_surface");
  result  =  checkAvailableExtensions(requiredExtensions);
  VK_CHECK_B32(result, "REQUIRED EXTENSIONS NOT AVAILABLE");

  KTRACE("----------------------REQUIRED LAYERS----------------------");
  const char**  requiredLayers  = (const char**)DARRAY_CREATE(const char*);
  DARRAY_PUSH(requiredLayers, &"VK_LAYER_KHRONOS_validation");
  result  =  checkAvailableLayers(requiredLayers);
  VK_CHECK_B32(result, "REQUIRED LAYERS NOT AVAILABLE");

  KTRACE("----------------------VULKAN INSTANCE----------------------");
  VkInstanceCreateInfo createInfo     = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  createInfo.pApplicationInfo         = &appInfo;
  createInfo.enabledExtensionCount    = DARRAY_LENGTH(requiredExtensions);
  createInfo.ppEnabledExtensionNames  = requiredExtensions;
  createInfo.enabledLayerCount        = DARRAY_LENGTH(requiredLayers);
  createInfo.ppEnabledLayerNames      = requiredLayers;

  ctx.allocator = 0;
  KTRACE("vkCreateInstace called");
  result  = vkCreateInstance(&createInfo, ctx.allocator, &(ctx.instance));
  VK_CHECK_B32(result, "FAILED TO CREATE INSTANCE : %s" ,vulkanResultToString(result));
  KINFO("VULKAN ISNTANCE CREATED");

  KTRACE("----------------------VULKAN DEBUGGER----------------------");
  result  = vulkanDebuggerCreate(&ctx);
  VK_CHECK_B32(result, "FAILED TO CREATE DEBUGGER");

  KTRACE("----------------------VULKAN SURFACE-----------------------");
  result  = vulkanSurfaceCreate(&ctx);
  VK_CHECK_B32(result, "FAILED TO CREATE SURFACE");

  KTRACE("----------------------VULKAN DEVICE------------------------");
  result =  vulkanDeviceCreate(&ctx);
  VK_CHECK_B32(result, "FAILED TO CREATE DEVICE !");

  KTRACE("----------------------VULKAN SWAPCHAIN---------------------");
  result = vulkanSwapchainCreate(&ctx, ctx.frameBufferWidth, ctx.frameBufferHeight, &ctx.swapchain);
  VK_CHECK_B32(result, "FAILED TO CREATE SWAPCHAIN");

  KTRACE("----------------------VULKAN RENDERPASS--------------------");
  result =  vulkanRenderPassCreate(&ctx,
                                   &ctx.mainRenderPass,
                                   0, 0, ctx.frameBufferWidth, ctx.frameBufferHeight,
                                   0.0f, 0.0f, 0.2f, 1.0f,
                                   1.0f, 0);
  VK_CHECK_B32(result, "FAILED TO CREATE RENDERPASS");

  KTRACE("----------------------SWAPCHAIN FRAMEBUFFERS---------------");
  ctx.swapchain.frameBuffers  = DARRAY_RESERVE(VulkanFrameBuffer, ctx.swapchain.imageCount);
  result = regenerateFrameBuffers(&ctx.swapchain, &ctx.mainRenderPass);
  VK_CHECK_B32(result, "FAILED TO CREATE INITIAL FRAMEBUFFERS");

  KTRACE("----------------------COMMAND BUFFERS----------------------");
  result = createCommandBuffers();
  VK_CHECK_B32(result, "FAILED TO CREATE COMMAND BUFFERS");


  KTRACE("----------------------SYNC OBJECTS-------------------------");
  ctx.imageAvailableSemaphores  = DARRAY_RESERVE(VkSemaphore, ctx.swapchain.maxFramesInFlight);
  ctx.queueCompleteSemaphores   = DARRAY_RESERVE(VkSemaphore, ctx.swapchain.maxFramesInFlight);
  ctx.inFlightFences            = DARRAY_RESERVE(VulkanFence, ctx.swapchain.maxFramesInFlight);

  for(u8 i = 0; i < ctx.swapchain.maxFramesInFlight; ++i){
    VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    result  = vkCreateSemaphore(ctx.device.logicalDevice,&semaphoreCreateInfo,ctx.allocator,&ctx.imageAvailableSemaphores[i]);
    result  = vkCreateSemaphore(ctx.device.logicalDevice,&semaphoreCreateInfo,ctx.allocator,&ctx.queueCompleteSemaphores[i]);

    //Create the fence in a signaled state, indicating that the FIRST FRAME HAS ALREADY BEED RENDERED.
    //this will prevent the application from waiting indefinitely for the first frame to render since it
    //cannot be renderer until a frame is RENDERED BEFORE it
    result = vulkanFenceCreate(&ctx, TRUE, &ctx.inFlightFences[i]);  
    VK_CHECK_B32(result, "FAILED TO CERATE SEMAPHORES");
  }

  // in flight images should not exist at this point, so CLEAR THE LIST , these are sorted in POINTERS
  // because the initial state shiould be zero , and will be 0 when not in use, ACTUAL FENCES ARE NOT 
  // OWNED BY THE LIST

  ctx.imagesInFlight  = DARRAY_RESERVE(VulkanFence, ctx.swapchain.imageCount);
  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    ctx.imagesInFlight[i] = 0;
  }

  KTRACE("----------------------BUILTIN SHADERS----------------------");
  result  = materialShaderCreate(&ctx, &ctx.materialShader);
  VK_CHECK_B32(result, "ERROR LOADING BUILTIN BASIC LIGHTING SHADER");



  KTRACE("----------------------CREATE OBEJCT BUFFERS---------------");
  result = createBuffers(&ctx);
  VK_CHECK_B32(result, "FAILED TO CREATE BUFFERS");

  KTRACE("----------------------TEST SLAP---------------------------");
  const u32 vertCount = 4;
  Vertex3D  verts[4]  = {0};

  const f32 f = 10.0f;
  verts[0].position.x =-0.5 * f;
  verts[0].position.y =-0.5 * f;
  verts[0].texcoord.x = 0.0f;
  verts[0].texcoord.y = 0.0f;

  verts[1].position.x = 0.5 * f;
  verts[1].position.y = 0.5 * f;
  verts[1].texcoord.x = 1.0f;
  verts[1].texcoord.y = 1.0f;

  verts[2].position.x =-0.5 * f;
  verts[2].position.y = 0.5 * f;
  verts[2].texcoord.x = 0.0f;
  verts[2].texcoord.y = 1.0f;

  verts[3].position.x = 0.5 * f;
  verts[3].position.y =-0.5 * f;
  verts[3].texcoord.x = 1.0f;
  verts[3].texcoord.y = 0.0f;

  const u32 indexCount  = 6;
  u32 indices[6] = {0,1,2,0,3,1};

  result = uploadDataRange(&ctx,ctx.device.graphicsCommandPool,0,ctx.device.graphicsQueue,&ctx.objectVertexBuffer,0,sizeof(Vertex3D) * vertCount, verts);
  VK_CHECK_B32(result, "failed to upload vertex data ");

  result = uploadDataRange(&ctx,ctx.device.graphicsCommandPool,0,ctx.device.graphicsQueue,&ctx.objectIndexBuffer,0,sizeof(u32) * indexCount, indices);
  VK_CHECK_B32(result, "failed to upload index data ");

  KINFO("Vulkan RENDERER BACKEND INITIALIZED");
  return TRUE;
}

//-----------------------------------------------------RENDERER BACKEND SHUTDOWN---------------------------------------------------------
b32
rendererBackendShutdown()
{
  TRACEFUNCTION;
  // destory in opposite order
  vkDeviceWaitIdle(ctx.device.logicalDevice);

  KDEBUG("DESTORYING BUFFERS");
  vulkanBufferDestroy(&ctx, &ctx.objectVertexBuffer);
  vulkanBufferDestroy(&ctx, &ctx.objectIndexBuffer);

  KDEBUG("DESTORYING MATERIAL SHADER");
  materialShaderDestroy(&ctx, &ctx.materialShader);

  KDEBUG("DESTROYING SYNC OBJECTS");
  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    if(ctx.imageAvailableSemaphores[i]){
      vkDestroySemaphore(ctx.device.logicalDevice,
                         ctx.imageAvailableSemaphores[i],
                         ctx.allocator);
      ctx.imageAvailableSemaphores[i] = 0;
    }
    if(ctx.queueCompleteSemaphores[i]){
      vkDestroySemaphore(ctx.device.logicalDevice,
                         ctx.queueCompleteSemaphores[i],
                         ctx.allocator);
      ctx.queueCompleteSemaphores[i] = 0;
    }

    vulkanFenceDestroy(&ctx, &ctx.inFlightFences[i]);
  }

  DARRAY_DESTROY(ctx.queueCompleteSemaphores);
  ctx.queueCompleteSemaphores = 0;
  DARRAY_DESTROY(ctx.imageAvailableSemaphores);
  ctx.imageAvailableSemaphores = 0;
  DARRAY_DESTROY(ctx.inFlightFences);
  ctx.inFlightFences = 0;
  DARRAY_DESTROY(ctx.imagesInFlight);
  ctx.imagesInFlight = 0;

  KINFO("DESTROYING COMMAND BUFFERS");
  for(u32 i = 0 ;i< ctx.swapchain.imageCount ; ++i){
    if(ctx.graphicsCommandBuffers[i].handle){
      vulkanCommandBufferFree(&ctx, ctx.device.graphicsCommandPool, &ctx.graphicsCommandBuffers[i]);
      ctx.graphicsCommandBuffers[i].handle = 0;
    }
  }

  KINFO("DESTROYING FRAMEBUFFERS");
  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    vulkanFrameBufferDestroy(&ctx, &ctx.swapchain.frameBuffers[i]);
  }
  DARRAY_DESTROY(ctx.graphicsCommandBuffers);
  ctx.graphicsCommandBuffers = 0;

  KINFO("DESTROYING RENDERPASS");
  vulkanRenderPassDestroy(&ctx, &ctx.mainRenderPass);

  KINFO("DESTROYING SWAPCHAIN");
  vulkanSwapchainDestroy(&ctx, &ctx.swapchain);

  KINFO("DESTROYING DEVICE");
  vulkanDeviceDestroy(&ctx);

  KINFO("DESTROYING SURFACE");
  if(ctx.surface){
    vkDestroySurfaceKHR(ctx.instance,ctx.surface ,ctx.allocator);
    ctx.surface = NULL;
  }

  KINFO("DESTROYING DEBUGGER");
  if (ctx.debugMessenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(ctx.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(ctx.instance, ctx.debugMessenger, ctx.allocator);
  }

  KINFO("DESTROYING INSTANCE");
  vkDestroyInstance(ctx.instance, ctx.allocator);
  return TRUE;
}

//-----------------------------------------------------RENDERER BEGIN FRAME---------------------------------------------------------

b32
rendererBackendBeginFrame(f32 deltaTime)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
  ctx.deltaTime = deltaTime;
  VulkanDevice* device  = &ctx.device;

  //check if recreating swap chain and boot out 
  if(ctx.recreatingSwapchain){
    KWARN("MIDST OF RECREATING SWAPCHAIN");
    result =  vkDeviceWaitIdle(device->logicalDevice);
    VK_CHECK_B32(result, "vkDeviceWaitIdle  failed while recreatingSwapchain");

    return FALSE;
  }

  //check if frameBuffer has been resized , if so then swapchain must be recreated 
  if(ctx.frameBufferSizeGeneration != ctx.frameBufferSizeLastGeneration){
    KWARN("FRAME BUFFER SIZE CAHANGED");
    result = vkDeviceWaitIdle(device->logicalDevice);
    //if swapchain recreation failed (ex : if the window was minimized);
    VK_CHECK_B32(result, "SWAPCHAIN RECRETION FAILED");

    KTRACE("-------------------RECREATING SWAPCHAIN------------------");
    VK_CHECK_B32(recreateSwapchain(),"SWAPCHAIN RECREATION FAILED");

    return FALSE;
  }

  if(!vulkanFenceWait(&ctx,&ctx.inFlightFences[ctx.currentFrame],1))KWARN("IN FLIGHT FENCE WAIT FAILURE WHILE BEGIN FRAME !");

  //THE SAME SEMAPHORE WILL BE WAITED ON BY THE QUEUE SUBMISSION TO ENSURE THIS IMAGE IS AVAILABLE.
  result =vulkanSwapchainAcquireNextImageIndex(&ctx,&ctx.swapchain,UINT64_MAX,ctx.imageAvailableSemaphores[ctx.currentFrame],0,&ctx.imageIndex);
  VK_CHECK_B32(result, "FAILED TO ACQUIRE NEXT IMAGE INDEX FROM SWAPCHAIN");

  //BEGIN RECORDING COMMANDS
  VulkanCommandBuffer* commandBuffer  =  &ctx.graphicsCommandBuffers[ctx.imageIndex];
  vulkanCommandBufferReset(commandBuffer);
  vulkanCommandBufferBegin(commandBuffer, FALSE, false, false);

  VkViewport viewport = {0};
  viewport.x          = 0.0f;
  viewport.y          = 0.0f;
  viewport.width      = (f32)ctx.frameBufferWidth;
  viewport.height     = (f32)ctx.frameBufferHeight;
  viewport.maxDepth   = 0.0f;
  viewport.minDepth   = 1.0f;

  VkRect2D scissoring;
  scissoring.offset.x = scissoring.offset.y = 0;
  scissoring.extent.width   = ctx.frameBufferWidth;
  scissoring.extent.height  = ctx.frameBufferHeight;

  vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissoring);

  ctx.mainRenderPass.w  = ctx.frameBufferWidth;
  ctx.mainRenderPass.h  = ctx.frameBufferHeight;

  //begin the render pass
  vulkanRenderPassBegin(commandBuffer,
                        &ctx.mainRenderPass,
                        ctx.swapchain.frameBuffers[ctx.imageIndex].handle);

  return TRUE;
}

//-----------------------------------------------------RENDERER END FRAME---------------------------------------------------------

b32
rendererBackendEndFrame(f32 deltaTime)
{
  TRACEFUNCTION;

  VulkanCommandBuffer* commandBuffer  = &ctx.graphicsCommandBuffers[ctx.imageIndex];
  vulkanRenderPassEnd(commandBuffer, &ctx.mainRenderPass);
  vulkanCommandBufferEnd(commandBuffer);

  //make sure the previous frame is not using this image (IE : its fence is not being waited upon);
  if(ctx.imagesInFlight[ctx.imageIndex] != VK_NULL_HANDLE){ 
    vulkanFenceWait(&ctx,
                    ctx.imagesInFlight[ctx.imageIndex],
                    UINT64_MAX);
  }
  //mark the iamge as in use by the frame
  ctx.imagesInFlight[ctx.imageIndex] = &ctx.inFlightFences[ctx.currentFrame];
  //reset the fence
  vulkanFenceReset(&ctx, &ctx.inFlightFences[ctx.currentFrame]);

  //submit the queue and wait for the opertion to complete.
  //begin Queue submission
  VkSubmitInfo  submitInfo  = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  //command buffers to be executed.
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer->handle;
  // the semaphores to be signaled when the queue is complete
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores  = &ctx.queueCompleteSemaphores[ctx.currentFrame];
  //wait sempahores ensure that the operation cannot begin untill the image is available
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = &ctx.imageAvailableSemaphores[ctx.currentFrame];
  //each SEMAPHORE waits on the corresponding pipeline stage to complete, 1:1 ratio.
  //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
  //writes from executing until the semaphore signal (IE : one frame is presented at a time)
  // this is how we take our triple buffering and make sure oen frame is presented at a time
  VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask  = flags;
  VkResult result = vkQueueSubmit(ctx.device.graphicsQueue,
                                  1,
                                  &submitInfo,
                                  ctx.inFlightFences[ctx.currentFrame].handle);
  VK_CHECK_B32(result, "failed to submit Queue");
  //end queue submission
  vulkanCommandBufferUpdateSubmitted(commandBuffer);


  // give the image back to swapchain
  result = vulkanSwapchainPresent(&ctx,&ctx.swapchain,ctx.device.graphicsQueue,ctx.device.presentQueue,ctx.queueCompleteSemaphores[ctx.currentFrame],ctx.imageIndex);

  VK_CHECK_B32(result, "failed to present swapchain");
  return TRUE;
}

//-----------------------------------------------------RENDERER UPDATE GLOBAL STATE-----------------------------------------------
b32
rendererUpdateGlobalState (Mat4 projection, Mat4 view, Vec3 viewPosition, Vec4 ambientColor, i32 mode)
{
  TRACEFUNCTION;

  VkResult result = !VK_SUCCESS;
  VulkanCommandBuffer* commandBuffer = &ctx.graphicsCommandBuffers[ctx.imageIndex];

  result  = materialShaderUse(&ctx, &ctx.materialShader);
  VK_CHECK_B32(result, "vulkanmaterialShaderUse failed in renderUpdateGlobalState");

  ctx.materialShader.globalUBO.projection = projection;
  ctx.materialShader.globalUBO.view       = view;
  //todo : other globalUBO properties
  result  = materialShaderUpdateGlobalObjectState(&ctx, &ctx.materialShader, ctx.deltaTime);
  VK_CHECK_B32(result, "vulkanmaterialShaderUpdateGlobalState failed in rendererUpdateGlobalState");
  return TRUE;
}

//-----------------------------------------------------VULKAN OBJECT UPDATE--------------------------------------------------------

b32
rendererUpdateObject( GeomteryRenderData data){
  TRACEFUNCTION;

  VkResult result = materialShaderUpdateLocalObjectState(&ctx, &ctx.materialShader, data);
  VK_CHECK_B32(result, "vulkanmaterialShaderUpdateObject failed");

  //test slap to draw geometry
  materialShaderUse(&ctx, &ctx.materialShader);

  VulkanCommandBuffer* commandBuffer = &ctx.graphicsCommandBuffers[ctx.imageIndex];
  VkDeviceSize offsets[1] = {0};
  //bind vertex buffer at offset
  vkCmdBindVertexBuffers(commandBuffer->handle, 0, 1, &ctx.objectVertexBuffer.handle, (VkDeviceSize*)offsets);

  //bind index buffer at offset
  vkCmdBindIndexBuffer  (commandBuffer->handle, ctx.objectIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

  //issue the draw
  vkCmdDrawIndexed(commandBuffer->handle, 6, 1, 0, 0, 0);

  return TRUE;
}

//-----------------------------------------------------RECREATE SWAPCHAIN------------------------------------------------------
VkResult
recreateSwapchain()
{
  TRACEFUNCTION;

  VkResult result = {0};

  if(ctx.recreatingSwapchain){
    KDEBUG("recreting swapchain called when already recreating, booting !!");
    return !VK_SUCCESS;
  }

  if(ctx.frameBufferWidth == 0 || ctx.frameBufferHeight == 0){
    KDEBUG("recreateSwapchain called when window size is too small");
    return !VK_SUCCESS;
  }

  ctx.recreatingSwapchain = TRUE;

  result = vkDeviceWaitIdle(ctx.device.logicalDevice);
  VK_CHECK_VERBOSE(result, "VKdEVICEwAITiDLE FAILED WHILE RECREATING SWAPCHAIN");

  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    ctx.imagesInFlight[i] = 0;
  }

  result = vulkanDeviceQuerySwapchainSupport(ctx.device.physicalDevice,ctx.surface,&ctx.device.swapchainSupport);
  VK_CHECK_RESULT(result, "FAILED TO QUERY SWAPCHAIN WHILE RECREATING SWAPCHAIN");

  result = vulkanDeviceDetectDepthFormat(&ctx.device);
  VK_CHECK_RESULT(result, "FAILED TO DETECT DEPTH FORMAT WHILE RECREATING SWAPCHAIN");

  result = vulkanSwapchainRecreate(&ctx,cachedFrameBufferWidth,cachedFrameBufferHeight,&ctx.swapchain);
  VK_CHECK_RESULT(result, "FAILED TO RECREATE SWAPCHIAN");

  ctx.frameBufferWidth  = cachedFrameBufferWidth;
  ctx.frameBufferHeight = cachedFrameBufferHeight;
  ctx.mainRenderPass.w  = ctx.frameBufferWidth;
  ctx.mainRenderPass.h  = ctx.frameBufferHeight;
  cachedFrameBufferWidth  = 0;
  cachedFrameBufferHeight = 0;

  ctx.frameBufferSizeLastGeneration = ctx.frameBufferSizeGeneration;

  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    vulkanCommandBufferFree(&ctx, 
                            ctx.device.graphicsCommandPool,
                            &ctx.graphicsCommandBuffers[i]);
  }
  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    vulkanFrameBufferDestroy(&ctx,&ctx.swapchain.frameBuffers[i]);
  }

  ctx.mainRenderPass.x  = 0;
  ctx.mainRenderPass.y  = 0;
  ctx.mainRenderPass.w  = ctx.frameBufferWidth;
  ctx.mainRenderPass.h  = ctx.frameBufferHeight;

  result = regenerateFrameBuffers(&ctx.swapchain, &ctx.mainRenderPass);
  VK_CHECK_RESULT(result, "failed to regenerate framebuffer while recreating swapchian");

  result = createCommandBuffers();
  VK_CHECK_RESULT(result, "failed to recreate command buffers while recreating swapchain");

  ctx.recreatingSwapchain = FALSE;

  KINFO("SWAPCHAIN RECREATED");
  return result;
}

//-----------------------------------------------------REGENERATE FRAME BUFFERS---------------------------------------------------------

VkResult  regenerateFrameBuffers(VulkanSwapchain* swapchain, VulkanRenderPass* renderPass){
  TRACEFUNCTION;

  VkResult result = {0};

  for(u32 i = 0; i < swapchain->imageCount; ++i){
    u32 attachmentCount = 2;
    VkImageView attachments[] = {swapchain->views[i], swapchain->depthAttachment.view};

    result = vulkanFrameBufferCreate(&ctx, renderPass,
                                     ctx.frameBufferWidth,
                                     ctx.frameBufferHeight,
                                     attachmentCount,
                                     attachments, 
                                     &ctx.swapchain.frameBuffers[i]);
    VK_CHECK_RESULT(result,"failed to create framebuffer for image %"PRIu32"",i);
  } 

  KINFO("SUCCESFULLY REGENRATED FRAMEBUFFERS");
  return result;
}

//-----------------------------------------------------CREATE COMMAND BUFFERS---------------------------------------------------------

VkResult
createCommandBuffers()
{
  TRACEFUNCTION;

  VkResult  result = !VK_SUCCESS;
  if(!ctx.graphicsCommandBuffers){
    ctx.graphicsCommandBuffers  = DARRAY_RESERVE(VulkanCommandBuffer, ctx.swapchain.imageCount);
    for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
      MEMZERO(&ctx.graphicsCommandBuffers[i]);
    }
  }

  for(u32 i = 0; i < ctx.swapchain.imageCount; ++i){
    if(ctx.graphicsCommandBuffers[i].handle){
      result  = vulkanCommandBufferFree(&ctx,
                                        ctx.device.graphicsCommandPool,
                                        &ctx.graphicsCommandBuffers[i]);
      VK_CHECK_RESULT(result, "failed to free grpahics Command Buffer : %"PRIu32"",i);
    }
    kzeroMemory(&ctx.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
    result = vulkanCommandBufferAllocate(&ctx,
                                         ctx.device.graphicsCommandPool, TRUE,
                                         &ctx.graphicsCommandBuffers[i]);

    VK_CHECK_RESULT(result, "failed to free allocate Command Buffer : %"PRIu32"",i);
  }

  KINFO("CREATED COMMAND BUFFERS");
  return result;
}

//-----------------------------------------------------CREATE BUFFERS---------------------------------------------------------

VkResult
createBuffers(VulkanContext* ctx)
{
  TRACEFUNCTION;

  VkResult result = !VK_SUCCESS;

  VkMemoryPropertyFlagBits memoryPropertyFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  const u64 vertexBufferSize  = sizeof(Vertex3D) * 1024 * 1024;
  result  = vulkanBufferCreate(ctx,
                               vertexBufferSize,
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               memoryPropertyFlags,
                               TRUE,
                               &ctx->objectVertexBuffer);
  VK_CHECK_RESULT(result, "failed to create vertex buffers");
  ctx->geometryVertexOffset = 0;

  KINFO("SUCCESSFULY CREATED GLOBAL VERTEX BUFFERS");
  const u64 indexBufferSize      = sizeof(u32) * 1024 * 1024;
  result  = vulkanBufferCreate(ctx,
                               indexBufferSize,
                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               memoryPropertyFlags,
                               TRUE,
                               &ctx->objectIndexBuffer);
  VK_CHECK_RESULT(result, "failed to index buffers");
  ctx->geometryIndexOffset  = 0;

  KINFO("SUCCESSFULY CREATED GLOBAL INDEX BUFFERS");
  return result;
}

//-----------------------------------------------------UPLAOAD DATA RANGE-----------------------------------------------------
//load data into a temporary buffer , that we are gonna simply call staging, has no other meaning and then copy to device local buffer
// DEFINE SOME GEOMETRY -> COPY TO STAGING BUFFER(on host) -> COPY TO DEVICE LOCAL BUFFER
// create a host visible staging buffer to upload to . mark it as the source of the ttransfer
VkResult
uploadDataRange(VulkanContext* ctx, VkCommandPool pool, VkFence fence ,
                VkQueue queue, VulkanBuffer* buffer, u64 offset, u64 size, void* data)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
  VkBufferUsageFlags  flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VulkanBuffer        staging;
  result  = vulkanBufferCreate(ctx, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, TRUE, &staging);
  VK_CHECK_RESULT(result, "vulkanBufferCreate   failed in uploadDataRange");

  result  = vulkanBufferLoadData(ctx, &staging, 0, size, 0, data);
  VK_CHECK_RESULT(result, "vulkanBufferLoadData failed in uploadDataRange");

  result  = vulkanBufferCopyTo(ctx, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);
  VK_CHECK_RESULT(result, "vulkanBufferCopyTo   failed in uploadDataRange");

  vulkanBufferDestroy(ctx, &staging);

  KINFO("SUCCESFULLY UPLOADED DATA RANGE");
  return result;
}

b32
rendererCreateTexture(Texture* texture, const u8* pixels)
{
  TRACEFUNCTION;

  VkResult result = !VK_SUCCESS;
  char* name          = texture->name;
  u32   width         = texture->width;
  u32   height        = texture->height;
  u32   channelCount  = texture->channelCount;
  KTRACE("CREATING TEXTURE '%s'", name);
  texture->internalData   = (TextureData*)kallocate(sizeof(TextureData), MEMORY_TAG_TEXTURE);

  TextureData*    data    = (TextureData*)texture->internalData;
  VkDeviceSize          imgsize = width* height * channelCount;
  VkFormat              imgfmt  =  VK_FORMAT_R8G8B8A8_UNORM;
  VulkanBuffer          staging = {0};
  VkBufferUsageFlags    usage   = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags memflags= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  KINFO("CREATING TEXTURE IMAGE FOR '%s'", name);
  {
    result  = vulkanBufferCreate(&ctx, imgsize, usage, memflags, TRUE, &staging);
    VK_CHECK_B32(result, "FAILED TO CREATE VulkanBuffer FOR TEXTURE '%s'",name);

    result  = vulkanBufferLoadData(&ctx, &staging, 0, imgsize, 0, pixels);
    VK_CHECK_B32(result, "FAILED TO LOAD VulkanBuffer   FOR TEXTURE '%s'",name);

    result  = vulkanImageCreate(&ctx, VK_IMAGE_TYPE_2D, width, height, imgfmt, VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                TRUE,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                &data->image);
    VK_CHECK_B32(result, "FAILED TO CREATE VulkanImage FOR TEXTURE '%s'",name);

    VulkanCommandBuffer tempbuf;
    VkCommandPool pool  = ctx.device.graphicsCommandPool;
    VkQueue       queue = ctx.device.graphicsQueue;

    result = vulkanCommandBufferStartSingleUse(&ctx, pool, &tempbuf);
    VK_CHECK_B32(result, "FAILED TO START SINGLE USE COMMAND BUFFER FOR CREATING TEXTURE");

    vulkanImageTransitionLayout (&ctx, &tempbuf, &data->image, imgfmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkanImageCopyFromBuffer   (&ctx, &data->image, staging.handle, &tempbuf);
    vulkanImageTransitionLayout (&ctx, &tempbuf, &data->image, imgfmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    result = vulkanCommandBufferStopSingleUse(&ctx, pool, &tempbuf, queue);
    VK_CHECK_B32(result, "FAILED TO STOP SINGLE USE COMMAND BUFFER FOR CREATING TEXTURE");

    vulkanBufferDestroy(&ctx, &staging);
  }
  KINFO("CREATING TEXTURE SAMPLER FOR '%s'", name);
  {
    VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    samplerInfo.magFilter         = VK_FILTER_LINEAR ;
    samplerInfo.minFilter         = VK_FILTER_LINEAR ;

    samplerInfo.addressModeU      = VK_SAMPLER_ADDRESS_MODE_REPEAT ;
    samplerInfo.addressModeV      = VK_SAMPLER_ADDRESS_MODE_REPEAT ;
    samplerInfo.addressModeW      = VK_SAMPLER_ADDRESS_MODE_REPEAT ;

    samplerInfo.anisotropyEnable  = VK_TRUE ;
    samplerInfo.maxAnisotropy     = 16 ;
    samplerInfo.borderColor       = VK_BORDER_COLOR_INT_OPAQUE_BLACK ;
    samplerInfo.unnormalizedCoordinates  =VK_FALSE;

    samplerInfo.compareEnable     = VK_FALSE;
    samplerInfo.compareOp         = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode        = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias        = 0.0f;
    samplerInfo.minLod            = 0.0f;
    samplerInfo.maxLod            = 0.0f;

    VkResult result = vkCreateSampler(ctx.device.logicalDevice, &samplerInfo, ctx.allocator, &data->sampler);
    VK_CHECK_B32(result, "FAILED TO CREATE IMAGE SAMPLER FOR '%s'",name);
  }
  texture->generation++;
  KINFO("SUCCESFULLY CREATED TEXTURE '%s'",name);
  return TRUE;
}

void 
rendererDestroyTexture(Texture *texture)
{
  TRACEFUNCTION;
  KDEBUG("DESTROYING TEXTURE  '%s'", texture->name);

  vkDeviceWaitIdle(ctx.device.logicalDevice);

  TextureData* data  = (TextureData*) texture->internalData;
  if(data){
    vulkanImageDestroy(&ctx, &data->image);
    MEMZERO(&data->image);

    vkDestroySampler(ctx.device.logicalDevice, data->sampler, ctx.allocator);
    data->sampler = 0;
    kfree(texture->internalData, sizeof(TextureData), MEMORY_TAG_TEXTURE);
  }
  MEMZERO(texture);
}

b32 
rendererCreateMaterial(Material* material)
{
  TRACEFUNCTION;
  if(material == NULL)
  {
    KERROR("CANNOT CREATE MATERIAL : nullptr PASSED");
    return FALSE;
  }
  VkResult result = materialShaderAcquireResources(&ctx, &ctx.materialShader, material);
  VK_CHECK_B32(result, "FAILED TO ACQUIRE SHADER RESOURCES");
  KINFO("SUCCESFULLY CREATED MATERIAL '%s'", material->name);
  return TRUE;
}

void  
rendererDestroyMaterial   (Material* material)
{
  TRACEFUNCTION;
  if(material == NULL)
  {
    KERROR("CANNOT DESTROY MATERIAL : nullptr Passed");
    return ;
  }
  if(material->internalId == INVALID_ID)
  {
    KWARN("rendererDestroyMaterial CALLED FOR A MATERIAL WITH INVLAID_ID, NOTHING WAS DONE");
    return;
  }
  materialShaderReleaseResources(&ctx, &ctx.materialShader, material);
  KINFO("SUCCESFULLY DESTROYED MATERIAL");
}


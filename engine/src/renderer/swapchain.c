#include  "swapchain.h"
#include  "device.h"
#include  "utils.h"
#include  "image.h"

#include  "core/kmemory.h"
#include "renderer/utils.h"
#include "renderer/vulkantypes.h"
#include <vulkan/vulkan_core.h>


VkResult create                   (VulkanContext*   context,
                                   u32 width, u32   height,
                                   VulkanSwapchain* swapchain);

VkResult destroy                  (VulkanContext*   context,
                                   VulkanSwapchain* swapchain);

VkResult vulkanSwapchainCreate    (VulkanContext*   context,
                                   u32 width, u32   height,
                                   VulkanSwapchain* outSwapchain)
{
  if(create(context,  width, height, outSwapchain) == VK_SUCCESS){
    UINFO("SWAPCHAIN CREATED");
    return VK_SUCCESS;
  }else{
    UINFO("SWAPCHAIN CREATION FAILED");
    return !VK_SUCCESS;
  }
}

VkResult vulkanSwapchainRecreate  (VulkanContext*   context,
                                   u32 width, u32   height,
                                   VulkanSwapchain* outSwapchain)
{
  destroy(context, outSwapchain);
  if(create(context,  width, height, outSwapchain) == VK_SUCCESS){
    UINFO("SWAPCHAIN CREATED");
    return VK_SUCCESS;
  }else{
    UINFO("SWAPCHAIN CREATION FAILED");
    return !VK_SUCCESS;
  }
}

VkResult vulkanSwapchainDestroy   (VulkanContext*   context,
                                   VulkanSwapchain* swapchain)
{
  return destroy(context, swapchain);
}

VkResult  vulkanSwapchainAcquireNextImageIndex (VulkanContext* context,
                                     VulkanSwapchain* swapchain,
                                     u64              timeoutNS, 
                                     VkSemaphore      imageAvailableSemaphore, 
                                     VkFence          fence, 
                                     u32*             outImageIndex)
{
  VkResult result = vkAcquireNextImageKHR(context->device.logicalDevice, swapchain->handle, timeoutNS, imageAvailableSemaphore, fence, outImageIndex);

  // trigger swapchian recreation and boot out of the render loop
  if(result == VK_ERROR_OUT_OF_DATE_KHR){
    UINFO("WINDOW RESIZED!");
    vulkanSwapchainRecreate(context, context->frameBufferWidth, context->frameBufferHeight,swapchain);
    return !VK_SUCCESS;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    VK_CHECK2(result, "FAILED TO ACQUIRE SWAPCHAIN IMAGE");
  }
  UINFO("SUCCESFULLY ACQUIRED IMAGE ");
  return VK_SUCCESS;
}

VkResult vulkanSwapchainPresent   (VulkanContext*   context,
                                   VulkanSwapchain* swapchain, 
                                   VkQueue          graphicsQueue,
                                   VkQueue          presentQueue,
                                   VkSemaphore      renderCompleteSemaphore,
                                   u32              presentImageIndex)
{
  VkPresentInfoKHR  presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount  = 1;
  presentInfo.pWaitSemaphores     = &renderCompleteSemaphore;
  presentInfo.swapchainCount      = 1;
  presentInfo.pSwapchains         = &swapchain->handle;
  presentInfo.pImageIndices       = &presentImageIndex;
  presentInfo.pResults            = 0;

  VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
  if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
    vulkanSwapchainRecreate(context, context->frameBufferWidth, context->frameBufferWidth, swapchain);
  }else if(result != VK_SUCCESS){
    UFATAL("FIALED TO PRESENT SWAPCHAIN IMAGE");
  }
  context->currentFrame = (context->currentFrame + 1) % swapchain->maxFramesInFlight;
  UINFO("SUCCESFULLY PRESENTED A FRAME");
  return result;
}
//-----------------------------------------INNER CREATE FUCNTION-----------------------------------
VkResult create   (VulkanContext*   context,
                   u32 width, u32   height,
                   VulkanSwapchain* swapchain)
{
  VkResult result = {0};
  VkExtent2D swapchainExtent  = {width, height};

  UINFO("GETTING IMAGE FORMATS");
  UDEBUG("----------------------FORMATS----------------------");
  swapchain->maxFramesInFlight = 2 ;
  //choose a swap surface format
  u8 found = false;
  for(u32 i = 0; i < context->device.swapchainSupport.formatCount; ++i){
    VkSurfaceFormatKHR format  = context->device.swapchainSupport.formats[i];
    //prefferedFormats
    if(format.format  ==  VK_FORMAT_B8G8R8A8_UNORM &&
      format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
      swapchain->imageFormat = format;
      found = true;
      break;
    }
  }
  if(!found){
    UFATAL("COUD NOT GET IMAGE FORMATS!");
    return !VK_SUCCESS;
  }
  UINFO("IMAGE FORMATS CHOSEN !");

  UDEBUG("----------------------PRESENT MODE----------------------");
  VkPresentModeKHR presentMode  = VK_PRESENT_MODE_FIFO_KHR;
  for(u32 i = 0;i < context->device.swapchainSupport.presentModesCount; ++i){
    VkPresentModeKHR mode = context->device.swapchainSupport.presentModes[i];
    if(mode ==  VK_PRESENT_MODE_MAILBOX_KHR){//triple buffering 
      presentMode = mode;
      break;
    }
  }
  // requery swapchain
  UDEBUG("--------------------SWAPCHAIN SUPPORT----------------------");
  UINFO("RECHECKING FOR SWAPCHAIN SPPORT");
  result = vulkanDeviceQuerySwapchainSupport(context->device.physicalDevice,
                                    context->surface,
                                    &context->device.swapchainSupport);
  VK_CHECK2(result, "could not query all swapchain requirements");

  UDEBUG("--------------------SWAPCHAIN EXTENT----------------------");
  UINFO("SETTING SWAPCHAIN EXTENT BOUNDS");
  //u32 checkwidth = context->device.swapchainSupport.capabilities.currentExtent.width;
  //u32 checkheight = context->device.swapchainSupport.capabilities.currentExtent.height;

  //swapchain extent  if whatever was passed to it is not valid then override it 
  // if(context->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX){
  //   swapchainExtent = context->device.swapchainSupport.capabilities.currentExtent;
  //   UERROR("value pased as swapchain extent in %s is unambiguious ",__FUNCTION__);
  // }

  //clamp to the value supported by the device
  //VkExtent2D min  = context->device.swapchainSupport.capabilities.maxImageExtent;
  //VkExtent2D max  = context->device.swapchainSupport.capabilities.maxImageExtent;

  //swapchainExtent.width = UCLAMP(swapchainExtent.width , min.width , max.width);
  //swapchainExtent.height= UCLAMP(swapchainExtent.height, min.height, max.height);

  UINFO("swapchain extent : width, height : %i, %i",swapchainExtent.width, swapchainExtent.height);
  //clamp the value of image count to max allowed by device
  u32 imageCount  = context->device.swapchainSupport.capabilities.minImageCount + 1;
  if(context->device.swapchainSupport.capabilities.maxImageCount > 0 && imageCount > context->device.swapchainSupport.capabilities.minImageCount){
    imageCount  = context->device.swapchainSupport.capabilities.minImageCount;
    UERROR("value passed as maxImageCount exceeds that maximagecount allowed, resetting !");
  }

  UDEBUG("--------------------SWAPCHAIN CREATE INFO----------");
  //SWAPCHAIN CREATE INFO
  VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  createInfo.surface  = context->surface;
  createInfo.minImageCount  = imageCount;
  createInfo.imageFormat  = swapchain->imageFormat.format;
  createInfo.imageColorSpace  = swapchain->imageFormat.colorSpace;
  createInfo.imageExtent  = swapchainExtent;
  createInfo.imageArrayLayers  = 1;
  createInfo.imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//something related to color buffer idk

  //setup the queue family indices
  if(context->device.graphicsQueueIndex == context->device.presentQueueIndex){
    u32 queueFamilyIndices[] = {
      (u32)context->device.graphicsQueueIndex,
      (u32)context->device.presentQueueIndex};
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount  = 2;
    createInfo.pQueueFamilyIndices  = queueFamilyIndices;
  }else{
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;// not gonna share the image subresource across different family queues
    // gonna be available to single family queue at a time
    createInfo.queueFamilyIndexCount  = 0;
    createInfo.pQueueFamilyIndices  = NULL;
  }

  createInfo.preTransform = context->device.swapchainSupport.capabilities.currentTransform;//potrait or landscape ?
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.clipped  = VK_TRUE;
  createInfo.oldSwapchain = NULL;

  result = vkCreateSwapchainKHR(context->device.logicalDevice, &createInfo, context->allocator, &swapchain->handle);
  UINFO("INITIALIZED SWAPCHAIN");
  VK_CHECK2(result, "COULD NOT CREATE SWAPCHAIN");
  //ONLY IN CASE OF SWAPCHAIN when created creates the images that it is going ti use as well 
  //all we have to do is to get the images from the swapchain, we DO NOT NEED TO CREATE THEM

  UDEBUG("--------------------SWAPCHAIN IMAGES----------");
  UINFO("GETTING SWAPCHAIN IMAGES");
  //start with zero frame index
  context->currentFrame = 0;

  //Images
  swapchain->imageCount = 0;
  result =  vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchain->imageCount, NULL );
  VK_CHECK2(result, "FAILED TO GET NUMBER OF SWAPCHAIN IMAGES"); 
  if(!swapchain->images){
    swapchain->images = (VkImage*)kallocate(sizeof(VkImage) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    MEMERR(swapchain->images);
  }
  if(!swapchain->views){
    swapchain->views  = (VkImageView*)kallocate(sizeof(VkImageView) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    MEMERR(swapchain->views);
  }
    result = vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchain->imageCount, swapchain->images);
    VK_CHECK2(result, "FAILED TO GET SWAPCHAIN IMAGES"); 

  UDEBUG("");
  UINFO("CREATING IMAGE VIEWS");
  //views
  for(u32 i = 0; i < swapchain->imageCount; ++i){
    VkImageViewCreateInfo viewInfo          = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image                          = swapchain->images[i];
    viewInfo.viewType                       = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                         = swapchain->imageFormat.format;
    viewInfo.subresourceRange.aspectMask    = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel  = 0;
    viewInfo.subresourceRange.levelCount    = 1;
    viewInfo.subresourceRange.baseArrayLayer= 0;
    viewInfo.subresourceRange.layerCount    = 1;

    result = vkCreateImageView(context->device.logicalDevice, &viewInfo, context->allocator, &swapchain->views[i]);
    VK_CHECK2(result,"FAILED TO CREATE IMAGE VIEWS");
    UINFO("CREATED IMAGE VIEW : %"PRIu32"",i);
  }

  UINFO("CHECKING FOR DEPTH FORMAT SUPPORT");
  //Depth resources
  if(vulkanDeviceDetectDepthFormat(&context->device) != VK_SUCCESS){
    context->device.depthFormat = VK_FORMAT_UNDEFINED;
    UFATAL("FAILED TO FIND A SUPPORTED DEPTH FORMAT ");
  }

  UINFO("CREATING DEPTH IMAGE");
  //CREATE THE DEPTH IMAGE
   result = vulkanImageCreate(context,
                    VK_IMAGE_TYPE_2D,
                    swapchainExtent.width,
                    swapchainExtent.height,
                    context->device.depthFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    true,
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    &swapchain->depthAttachment);

  return result;
}

VkResult destroy(VulkanContext* context,VulkanSwapchain* swapchain){

  vkDeviceWaitIdle(context->device.logicalDevice);
  vulkanImageDestroy(context, &swapchain->depthAttachment);

  for(u32 i = 0;i < swapchain->imageCount; ++i){
    vkDestroyImageView(context->device.logicalDevice, swapchain->views[i],context->allocator);
  }

  vkDestroySwapchainKHR(context->device.logicalDevice, swapchain->handle, context->allocator);
  return VK_SUCCESS;
}


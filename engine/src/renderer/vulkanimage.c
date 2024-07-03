#include  "vulkanimage.h"
#include  "rendererutils.h"

#include  "core/logger.h"

VkResult  vulkanImageCreate(VulkanContext*      context,
                            VkImageType         type,
                            u32                 width,
                            u32                 height,
                            VkFormat            format,
                            VkImageTiling       tiling,
                            VkImageUsageFlags   usage,
                            VkMemoryPropertyFlags memoryFlags,
                            u32                 createView,
                            VkImageAspectFlags  viewAspectFlags,
                            VulkanImage*        outImage)
{ //copy params
  TRACEFUNCTION;
  outImage->width = width;
  outImage->height=height;

  //creation info
  VkImageCreateInfo createInfo  = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  createInfo.imageType          = VK_IMAGE_TYPE_2D; 
  createInfo.extent.width       = width;
  createInfo.extent.height      = height;
  createInfo.extent.depth       = 1;
  createInfo.mipLevels          = 4;
  createInfo.arrayLayers        = 1;
  createInfo.format             = format;
  createInfo.tiling             = tiling;
  createInfo.initialLayout      = VK_IMAGE_LAYOUT_UNDEFINED;
  createInfo.usage              = usage;
  createInfo.samples            = VK_SAMPLE_COUNT_1_BIT;
  createInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

  KINFO("DEPTH VIEW IMAGE CREATED");
  VkResult result = vkCreateImage(context->device.logicalDevice,&createInfo , context->allocator, &outImage->handle);
  VK_CHECK_VERBOSE(result , "failed to create Image"); 

  VkMemoryRequirements memoryRequirements;
  KINFO("GETTING IMAGE MEMORY REQUIREMENTS");
  vkGetImageMemoryRequirements(context->device.logicalDevice, outImage->handle, &memoryRequirements);

  //                        function pointer
  i32 memoryType  = context->findMemoryIndex(memoryRequirements.memoryTypeBits, memoryFlags);
  if(memoryType == -1){
    KERROR("REQUIRED MEMORY TYPE NOT FOUND, IMAGE NOT VALID");
  }

  VkMemoryAllocateInfo memInfo  = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memInfo.allocationSize  = memoryRequirements.size;
  memInfo.memoryTypeIndex = memoryType;

  result =  vkAllocateMemory(context->device.logicalDevice, &memInfo, context->allocator, &outImage->memory);
  VK_CHECK_VERBOSE(result, "failed to allocate memory for iamge");

  result  = vkBindImageMemory(context->device.logicalDevice, outImage->handle, outImage->memory, 0);
  VK_CHECK_VERBOSE(result, "failed to bind memory for image");

  if(createView){
    outImage->view  = 0;
    result = vulkanImageViewCreate(context, format, outImage, viewAspectFlags);
    VK_CHECK_RESULT(result,"CREATED IMAGE BUT FAILED TO CREATE ITS VIEW");
    KINFO("IMAGE VIEW CREATED SUCCESFULLY!");
    return result;
  }
  KINFO("CREATED SWAPCHAIN IMAGE SUCCESFULLY!");
  return result;
}

VkResult  vulkanImageViewCreate(VulkanContext* context,
                                VkFormat format,
                                VulkanImage* image,
                                VkImageAspectFlags aspectFlags)
{
  TRACEFUNCTION;
  VkImageViewCreateInfo createInfo  = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  createInfo.image                          = image->handle;
  createInfo.viewType                       = VK_IMAGE_VIEW_TYPE_2D ;
  createInfo.format                         = format;
  createInfo.subresourceRange.aspectMask    = aspectFlags;
  //  MAKE CONFIGURABLE TODO
  createInfo.subresourceRange.baseMipLevel  = 0;
  createInfo.subresourceRange.levelCount    = 1;
  createInfo.subresourceRange.baseArrayLayer= 0;
  createInfo.subresourceRange.layerCount    = 1;

  VkResult result = vkCreateImageView(context->device.logicalDevice, &createInfo, context->allocator, &image->view);
  VK_CHECK_VERBOSE(result, "vkCreateImageView failed");
  return result;
}

void vulkanImageDestroy(VulkanContext* context, VulkanImage* image){
  if(image->view){
    vkDestroyImageView(context->device.logicalDevice, image->view, context->allocator);
    image->view = 0;
  }
  if(image->memory){
    vkFreeMemory(context->device.logicalDevice, image->memory, context->allocator);
    image->memory = 0;
  }
  if(image->handle){
    vkDestroyImage(context->device.logicalDevice, image->handle, context->allocator);
    image->handle = 0;
  }
}

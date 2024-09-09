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

  TRACEFUNCTION;
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
void      vulkanImageTransitionLayout(VulkanContext* context,
                                      VulkanCommandBuffer* commandBuffer,
                                      VulkanImage* image,
                                      VkFormat format,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout)
{

  TRACEFUNCTION;
  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = context->device.graphicsQueueIndex;
  barrier.dstQueueFamilyIndex = context->device.graphicsQueueIndex;
  barrier.image = image->handle;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount   = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount   = 1;
//baseArrayLayers + layerCount shoudl be == to array layers of the image when it was created

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destStage;

  if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destStage   = VK_PIPELINE_STAGE_TRANSFER_BIT;

  }else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destStage   = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  }else{
    KFATAL("UNSOPPRTED LAYOUT");
    return;
  }

  vkCmdPipelineBarrier(commandBuffer->handle, sourceStage, destStage, 0,0,0,0,0,1,&barrier);
}

void      vulkanImageCopyFromBuffer(VulkanContext* context,
                                    VulkanImage* image,
                                    VkBuffer buffer,
                                    VulkanCommandBuffer* commandBuffer)
{
  TRACEFUNCTION;
  VkBufferImageCopy region  = {0};
  region.bufferOffset = 0;
  region.bufferRowLength  = 0;
  region.bufferImageHeight  = 0;

  region.imageSubresource.aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel    = 0;
  region.imageSubresource.baseArrayLayer  = 0;
  region.imageSubresource.layerCount  = 1;

  region.imageExtent.width  = image->width;
  region.imageExtent.height = image->height;
  region.imageExtent.depth  = 1;

  vkCmdCopyBufferToImage(commandBuffer->handle, buffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
}


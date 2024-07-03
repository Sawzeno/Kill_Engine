#include  "vulkanbuffer.h"
#include  "vulkancommandbuffer.h"
#include  "rendererutils.h"

#include  "core/logger.h"
#include  "core/kmemory.h"

VkResult  vulkanBufferCreate      (VulkanContext*         context,
                                   u64                    size,
                                   VkBufferUsageFlagBits  usage,
                                   u32                    memoryPropertyFlags,
                                   bool                   bindOnCreate,
                                   VulkanBuffer*          outBuffer)
{
  TRACEFUNCTION;
  KDEBUG("context : %p size : %"PRIu64"usage : %p  memoryProprtyFlags : %"PRIu32" bindOnCreate : %d outBuffer : %p",context, size,usage,memoryPropertyFlags, bindOnCreate, outBuffer);

  VkResult result = !VK_SUCCESS;
  kzeroMemory(outBuffer, sizeof(VulkanBuffer));
  outBuffer->totalSize  = size;
  outBuffer->usage      = usage;
  outBuffer->memoryPropertyFlags= memoryPropertyFlags;

  VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferInfo.size       = size;
  bufferInfo.usage      = usage;
  bufferInfo.sharingMode= VK_SHARING_MODE_EXCLUSIVE;

  result  = vkCreateBuffer(context->device.logicalDevice, &bufferInfo, context->allocator, &outBuffer->handle);
  VK_CHECK_VERBOSE(result, "vkCreateBuffer failed");

  //GATHER MEMORY REQUIREMENTS;
  VkMemoryRequirements memoryRequirements = {0};
  vkGetBufferMemoryRequirements(context->device.logicalDevice, outBuffer->handle, &memoryRequirements);

  outBuffer->memoryIndex  = context->findMemoryIndex(memoryRequirements.memoryTypeBits, outBuffer->memoryPropertyFlags);
  if(outBuffer->memoryIndex == -1){
    KERROR("unable to create vulkan buffer because the required memory type index was not found");
    return !VK_SUCCESS;
  }
  //ALLOCATE MEMORY INFO
  VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex= (u32)outBuffer->memoryIndex;

  //ALLOCATE THE MEMORY
  result  = vkAllocateMemory(context->device.logicalDevice, &memoryAllocateInfo, context->allocator, &outBuffer->memory);
  VK_CHECK_VERBOSE(result, "vkAllocateMemory failed in vulkanBufferCreate");

  if(bindOnCreate){
    result = vulkanBufferBind(context, outBuffer, 0);
    VK_CHECK_RESULT(result, "failed to bind vulkan buffer on create");
  }
  return result;
}

VkResult  vulkanBufferResize      (VulkanContext*         context,
                                   u64                    newSize,
                                   VulkanBuffer*          buffer,
                                   VkQueue                queue,
                                   VkCommandPool          commandPool){
  TRACEFUNCTION;
  KDEBUG("context : %p newSize : %"PRIu64" buffer: %p queue : %p commandPool : %p",context,newSize,buffer,queue,commandPool);
  VkResult  result    =!VK_SUCCESS;
  // create new buffer
  VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCreateInfo.size = newSize;
  bufferCreateInfo.usage= buffer->usage;
  bufferCreateInfo.sharingMode= VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer newBuffer;
  result  = vkCreateBuffer(context->device.logicalDevice, &bufferCreateInfo, context->allocator, &newBuffer);
  VK_CHECK_VERBOSE(result, "vkCreateBuffer failed");

  //gather memory requirements
  VkMemoryRequirements requirements = {0};
  vkGetBufferMemoryRequirements(context->device.logicalDevice, newBuffer, &requirements);

  //allocate memory info
  VkMemoryAllocateInfo memAllocateInfo  = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memAllocateInfo.allocationSize  = requirements.size;
  memAllocateInfo.memoryTypeIndex = (u32)buffer->memoryIndex;

  //allocateMemory
  VkDeviceMemory newMemory;
  result  = vkAllocateMemory(context->device.logicalDevice, &memAllocateInfo, context->allocator, &newMemory);
  VK_CHECK_VERBOSE(result, "vkAllocateMemory failed");

  //bind the new buffers memory
  result  = vkBindBufferMemory(context->device.logicalDevice, newBuffer,newMemory,  0);
  VK_CHECK_VERBOSE(result, "vkBindBufferMemory failed");

  //copy over the data
  result = vulkanBufferCopyTo(context,  commandPool, NULL, queue, buffer->handle, 0, newBuffer, 0, buffer->totalSize);
  VK_CHECK_RESULT(result, "failed to copy vulkan buffer while resizing");
  //make sure anything using these is finished
  result = vkDeviceWaitIdle(context->device.logicalDevice);
  VK_CHECK_VERBOSE(result, "vkDeviceWaitIdle failed");

  //destroy the old buffer
  if(buffer->memory){
    vkFreeMemory(context->device.logicalDevice, buffer->memory, context->allocator);
    buffer->memory = 0;
  }
  if(buffer->handle){
    vkDestroyBuffer(context->device.logicalDevice, buffer->handle, context->allocator);
    buffer->handle  = 0;
  }

  //set new properties
  buffer->totalSize = newSize;
  buffer->memory    = newMemory;
  buffer->handle    = newBuffer;

  return result;
}

VkResult  vulkanBufferBind        (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset){
  TRACEFUNCTION;
  KDEBUG("context : %p buffer : %p offset "PRIu64"",context, buffer, offset);
  VkResult result =    vkBindBufferMemory(context->device.logicalDevice, buffer->handle, buffer->memory, offset);
  VK_CHECK_VERBOSE(result, "vkBindBufferMemory failed");
  return result;
}

VkResult  vulkanBufferDestroy     (VulkanContext*         context,
                                   VulkanBuffer*          buffer){
  TRACEFUNCTION;
  KDEBUG("context : %p buffer : %p",context, buffer);
  if(buffer->memory){
    vkFreeMemory(context->device.logicalDevice, buffer->memory, context->allocator);
    buffer->memory  = 0;
  }
  if(buffer->handle){
    vkDestroyBuffer(context->device.logicalDevice, buffer->handle, context->allocator);
    buffer->handle  = 0;
  }
  buffer->totalSize = 0;
  buffer->usage     = 0;
  buffer->isLocked  = 0;

  return VK_SUCCESS;
}

void*     vulkanBufferLockMemory  (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset,
                                   u64                    size,
                                   u32                    flags){
TRACEFUNCTION;
  KDEBUG("context : %p buffer : %p offset : %"PRIu64" size : %"PRIu64" flags : %"PRIu32"", context, buffer, offset, size, flags);
  void* data;
  if(vkMapMemory(context->device.logicalDevice, buffer->memory, offset,  size,  flags, &data) != VK_SUCCESS){
    KERROR("vkMapMemory failed in vulkanBufferLockMemory");
    return NULL;
  }
  return data;
}

VkResult  vulkanBufferUnlockMemory(VulkanContext*         context,
                                   VulkanBuffer*          buffer){
TRACEFUNCTION;
  KDEBUG("context : %p context buffer : %p", context, buffer);
  vkUnmapMemory(context->device.logicalDevice, buffer->memory);
  return VK_SUCCESS;
}

VkResult  vulkanBufferLoadData    (VulkanContext*         context,
                                   VulkanBuffer*          buffer,
                                   u64                    offset,
                                   u64                    size,
                                   u32                    flags,
                                   const void*            data){
  TRACEFUNCTION;
  KDEBUG("context : %p buffer : %p offset : %"PRIu64" size : %"PRIu64" flags : %"PRIu32" data : %p", context, buffer, offset, size, flags, data);
  void* dataptr;
  VkResult result = vkMapMemory(context->device.logicalDevice, buffer->memory, offset, size, flags, &dataptr);
  VK_CHECK_VERBOSE(result, "vkMapMemory failed in vulkanBufferLoadData");
  kcopyMemory   (dataptr, data, size, __FUNCTION__);
  KDEBUG("vkUnmapMemory called");
  vkUnmapMemory (context->device.logicalDevice, buffer->memory);
  return VK_SUCCESS;
}

VkResult  vulkanBufferCopyTo      (VulkanContext*         context,
                                   VkCommandPool          pool,
                                   VkFence                fence,
                                   VkQueue                queue,
                                   VkBuffer               source,
                                   u64                    sourceOffset,
                                   VkBuffer               dest,
                                   u64                    destOffset,
                                   u64                    size){
  TRACEFUNCTION;
  KDEBUG("context : %p pool : %p fence : %p queue : %p source : %p sourceOffset : %"PRIu32" dest : %p destOffset : "PRIu32" size : "PRIu32"",context,&pool,&fence,&queue,&source,sourceOffset,&dest,destOffset,size);
  VkResult result = !VK_SUCCESS;

  //create a one time use command buffer
  VulkanCommandBuffer tempCommandBuffer;
  result = vulkanCommandBufferStartSingleUse(context, pool, &tempCommandBuffer);
  VK_CHECK_RESULT(result, "failed to copy single use buffer");

  //prepare the copy command and add it to command buffer
  VkBufferCopy copyRegion;
  copyRegion.srcOffset  = sourceOffset;
  copyRegion.dstOffset  = destOffset;
  copyRegion.size       = size;

  KDEBUG("vkCmdCopyBuffer called");
  vkCmdCopyBuffer(tempCommandBuffer.handle, source, dest, 1, &copyRegion);

  //submit the buffer for execution and wait for it to complete
  result  = vulkanCommandBufferStopSingleUse(context, pool, &tempCommandBuffer, queue);
  VK_CHECK_RESULT(result, "failed to copy Single use buffer");

  return result;
}

#include  "vulkanfence.h"
#include  "rendererutils.h"

#include  "core/logger.h"


VkResult  vulkanFenceCreate(VulkanContext* context,
                            u8  createSignaled,
                            VulkanFence* outFence){
  TRACEFUNCTION;
  VkResult result = {0};
  // make sure to signal the fence 
  outFence->isSignaled = createSignaled;
  VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  if(outFence->isSignaled){
    createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  }
  result  = vkCreateFence(context->device.logicalDevice,
                          &createInfo, 
                          context->allocator, 
                          &outFence->handle);
  VK_CHECK_VERBOSE(result, "vkCreateFence failed");
  KINFO("SCCESFULLY CREATED FENCE");

  return result;
}

VkResult  vulkanFenceDestroy(VulkanContext* context, 
                             VulkanFence* fence){
  TRACEFUNCTION;
  if(fence->handle){
    vkDestroyFence(context->device.logicalDevice, fence->handle, context->allocator);
  }
  fence->isSignaled = false;
  return VK_SUCCESS;
}

u8  vulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeoutNS){
  VkResult  result  = {0};
  if(!fence->isSignaled){
    result  = vkWaitForFences(context->device.logicalDevice,
                              1, &fence->handle,
                              true, timeoutNS);
    VK_CHECK_FENCE(result);
  }else{
    // if already singaled NO NOT WAIT 
    KINFO("FENCE WAS SUCCESFULLY EXECUTED");
     return true;
  }
  return false;
}

VkResult  vulkanFenceReset(VulkanContext* context, VulkanFence* fence){
  TRACEFUNCTION;
  VkResult result = {0};
  if(fence->isSignaled){
    result  = vkResetFences(context->device.logicalDevice, 1, &fence->handle);
    VK_CHECK_VERBOSE(result , "vkResetFences failed");
    fence->isSignaled = false;
  }
  return result;
}

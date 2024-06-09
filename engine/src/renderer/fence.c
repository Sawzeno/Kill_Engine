#include "fence.h"

#include  "utils.h"
#include  "defines.h"
#include <vulkan/vulkan_core.h>


VkResult  vulkanFenceCreate(vulkanContext* context,
                            u8  createSignaled,
                            vulkanFence* outFence){
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
  VK_CHECK2(result, "FAILED TO CREATE FENCE");
  UINFO("SCCESFULLY CREATED FENCE");

  return result;
}

VkResult  vulkanFenceDestroy(vulkanContext* context, 
                             vulkanFence* fence){
  if(fence->handle){
    vkDestroyFence(context->device.logicalDevice, fence->handle, context->allocator);
  }
  fence->isSignaled = false;
  return VK_SUCCESS;
}

u8  vulkanFenceWait(vulkanContext* context, vulkanFence* fence, u64 timeoutNS){
  VkResult  result  = {0};
  if(!fence->isSignaled){
    result  = vkWaitForFences(context->device.logicalDevice,
                              1, &fence->handle,
                              true, timeoutNS);
    VK_CHECK_FENCE(result);
  }else{
    // if alreasdy singaled NO NOT WAIT 
    UINFO("FENCE WAS SUCCESFULLY EXECUTED");
     return true;
  }
  return false;
}

VkResult  vulkanFenceReset(vulkanContext* context, vulkanFence* fence){
  VkResult result = {0};
  if(fence->isSignaled){
    result  = vkResetFences(context->device.logicalDevice, 1, &fence->handle);
    VK_CHECK2(result , "FAILED TO RESET FENCE");
    fence->isSignaled = false;
  }
  return result;
}

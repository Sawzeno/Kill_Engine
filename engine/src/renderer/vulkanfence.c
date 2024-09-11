#include  "vulkanfence.h"
#include  "rendererutils.h"

#include  "core/logger.h"


VkResult  vulkanFenceCreate(VulkanContext* context,
                            b32  createSignaled,
                            VulkanFence* outFence){
  TRACEFUNCTION;
  VkResult result = {0};
  // make sure to signal the fence 
  outFence->isSignaled = createSignaled;
  VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  if(outFence->isSignaled == TRUE){
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
  if(fence->handle != NULL){
    vkDestroyFence(context->device.logicalDevice, fence->handle, context->allocator);
  }
  fence->isSignaled = FALSE;
  return VK_SUCCESS;
}

b32  vulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeoutNS){
  VkResult  result  = {0};
  if(!fence->isSignaled)
  {
    result  = vkWaitForFences(context->device.logicalDevice,
                              1,
                              &fence->handle,
                              TRUE,
                              timeoutNS);
    switch(result){                                                
      case VK_SUCCESS:                                              
        fence->isSignaled = TRUE;                                   
        KINFO("FENCE WAS SUCCESSFULLY EXECUTED");                   
        return TRUE;                                                
      case VK_TIMEOUT:                                              
        KWARN("vkFenceWait -> FENCE TIMED OUT");                    
        break;                                                      
      case VK_ERROR_DEVICE_LOST:                                    
        KERROR("vkFenceWait -> DEVICE LOST");                       
        break;                                                      
      case VK_ERROR_OUT_OF_HOST_MEMORY:                             
        KERROR("vkFenceWait -> VK_ERROR_OUT_OF_HOST_MEMORY");       
        break;                                                      
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:                           
        KERROR("vkFenceWait -> VK_ERROR_OUT_OF_DEVICE_MEMORY");     
        break;                                                      
      default:                                                      
        KERROR("vkFenceWait -> UNKNOWN ERROR HAS OCCURRED");        
        break;                                                      
    }
  }else{
    KINFO("FENCE IS ALREADY BUSY");
    return TRUE;
  }
  return FALSE;
}

VkResult  vulkanFenceReset(VulkanContext* context, VulkanFence* fence){
  TRACEFUNCTION;
  VkResult result = {0};
  if(fence->isSignaled){
    result  = vkResetFences(context->device.logicalDevice, 1, &fence->handle);
    VK_CHECK_VERBOSE(result , "vkResetFences failed");
    fence->isSignaled = FALSE;
  }
  return result;
}

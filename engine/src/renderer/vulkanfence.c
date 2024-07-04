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
  if(!fence->isSignaled)
  {
    result  = vkWaitForFences(context->device.logicalDevice,
                              1,
                              &fence->handle,
                              true,
                              timeoutNS);
    switch(result){                                                
      case VK_SUCCESS:                                              
        fence->isSignaled = true;                                   
        UINFO("FENCE WAS SUCCESSFULLY EXECUTED");                   
        KINFO("FENCE WAS SUCCESSFULLY EXECUTED");                   
        return true;                                                
      case VK_TIMEOUT:                                              
        UWARN("vkFenceWait -> FENCE TIMED OUT");                    
        KWARN("vkFenceWait -> FENCE TIMED OUT");                    
        break;                                                      
      case VK_ERROR_DEVICE_LOST:                                    
        UERROR("vkFenceWait -> DEVICE LOST");                       
        KERROR("vkFenceWait -> DEVICE LOST");                       
        break;                                                      
      case VK_ERROR_OUT_OF_HOST_MEMORY:                             
        UERROR("vkFenceWait -> VK_ERROR_OUT_OF_HOST_MEMORY");       
        KERROR("vkFenceWait -> VK_ERROR_OUT_OF_HOST_MEMORY");       
        break;                                                      
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:                           
        UERROR("vkFenceWait -> VK_ERROR_OUT_OF_DEVICE_MEMORY");     
        KERROR("vkFenceWait -> VK_ERROR_OUT_OF_DEVICE_MEMORY");     
        break;                                                      
      default:                                                      
        UERROR("vkFenceWait -> UNKNOWN ERROR HAS OCCURRED");        
        KERROR("vkFenceWait -> UNKNOWN ERROR HAS OCCURRED");        
        break;                                                      
    }
  }else{
    KINFO("FENCE IS ALREADY BUSY");
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

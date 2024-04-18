#include  "rendererbackend.h"

#include  "defines.h"
#include  <vulkan/vulkan.h>
#include  <vulkan/vulkan_core.h>

static vulkanContext context;

u8  rendererBackendInitialize(const char* applicationName , platformState* platState){

  //TODO
  context.allocator = NULL;
  VkApplicationInfo appInfo   = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.apiVersion          = VK_API_VERSION_1_2;
  appInfo.pApplicationName    = applicationName;
  appInfo.applicationVersion  = VK_MAKE_VERSION(1,0,0);
  appInfo.pEngineName         = "KILL ENGINE";
  appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo createInfo     = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  createInfo.pApplicationInfo         = &appInfo;
  createInfo.enabledExtensionCount    = 0;
  createInfo.ppEnabledExtensionNames  = 0;
  createInfo.enabledLayerCount        = 0;
  createInfo.ppEnabledLayerNames      = 0;

  VkResult result = vkCreateInstance(&createInfo, NULL, &context.instance);

  if(result != VK_SUCCESS){
    UERROR("vkCreateInstance failed : %u" ,result);
    return false;
  }
  UINFO("VULKAN RENDERER BACKEND INITIALIZED")
  return true;
}

u8  rendererBackendBeginFrame(f32 deltaTime){
  return true;
}

u8  rendererBackendEndFrame(f32 deltaTime){
  return true;
}

u8  rendererBackendResized(){
  return true;
}
u8  rendererBackendShutdown(){
  return true;
}


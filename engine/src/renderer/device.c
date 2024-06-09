#include "device.h"
#include  "utils.h"
#include "core/kmemory.h"
#include "vulkantypes.h"
#include "containers/darray.h"
#include "defines.h"
#include <string.h>
#include <vulkan/vulkan_core.h>

typedef struct vulkanPhysicalDeviceRequirements vulkanPhysicalDeviceRequirements;
typedef struct vulkanPhysicalDeviceQueueFamilyInfo vulkanPhysicalDeviceQueueFamilyInfo;

struct vulkanPhysicalDeviceRequirements {
  u8 graphics;
  u8 transfer;
  u8 present;
  u8 compute;
  u8 samplerAnisotropy;
  u8 discreteGPU;
  const char** deviceExtensionNames;
};

//you can think of queue families as amjor road for transporting data , some of these roads ahve stops for everything
//like swapchain present computer,grpahics, , other road onlt stop at compute other only for transfer operation
//different hardware support differnet number of therse roads, thet all support atleast one queue
//but as the application size increases you want to divert data traffic 
struct vulkanPhysicalDeviceQueueFamilyInfo {
  u32 graphicsFamilyIndex;
  u32 transferFamilyIndex;
  u32 presentFamilyIndex;
  u32 computeFamilyIndex;
};

VkResult physicalDeviceMeetsRequirements(
  VkPhysicalDevice device,
  VkSurfaceKHR surface,
  const VkPhysicalDeviceProperties* properties,
  const VkPhysicalDeviceFeatures* features,
  const vulkanPhysicalDeviceRequirements* requirements,
  vulkanPhysicalDeviceQueueFamilyInfo* outQueueFamilyInfo,
  vulkanSwapchainSupportInfo* outSwapchainSupport);

VkResult vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice physicalDevice,
                                           VkSurfaceKHR surface,
                                           vulkanSwapchainSupportInfo *outSupportInfo) {

  UDEBUG("----------------------DEVICE SURFACE CAPABILTITIES---------");
  VkResult result = {0};
  result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &outSupportInfo->capabilities);
  VK_CHECK2(result, "failed to get device surface capabilities");

  UDEBUG("----------------------SURFACE FORMATS----------------------");
  result  = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, 0);
  VK_CHECK2(result, "failed to get device formats count");

  if (outSupportInfo->formatCount != 0) {
    outSupportInfo->formats = kallocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, outSupportInfo->formats);
    VK_CHECK2(result, "failed to get device formats available");
  }

  UDEBUG("----------------------PRESENT MODES-_---------------------");
  result  = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &outSupportInfo->presentModesCount, 0);
  VK_CHECK2(result, "failed to get present modes count");

  if (outSupportInfo->presentModesCount != 0) {
    outSupportInfo->presentModes = kallocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModesCount, MEMORY_TAG_RENDERER);
    result  = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                        surface,
                                                        &outSupportInfo->presentModesCount,
                                                        outSupportInfo->presentModes);
    VK_CHECK2(result, "failed to get present modes");
  }

  UINFO("QUERIED SWAPCHAIN SUPPORT SUCCESFULLY");
  return VK_SUCCESS;
}


VkResult physicalDeviceMeetsRequirements(
  VkPhysicalDevice device,
  VkSurfaceKHR surface,
  const VkPhysicalDeviceProperties* properties,
  const VkPhysicalDeviceFeatures* features,
  const vulkanPhysicalDeviceRequirements* requirements,
  vulkanPhysicalDeviceQueueFamilyInfo* outQueueFamilyInfo,
  vulkanSwapchainSupportInfo* outSwapchainSupport) {

  VkResult  result = {0};
  // Evaluate device properties to determine the needs of our application
  outQueueFamilyInfo->graphicsFamilyIndex = (u32)-1;
  outQueueFamilyInfo->presentFamilyIndex = (u32)-1;
  outQueueFamilyInfo->computeFamilyIndex = (u32)-1;
  outQueueFamilyInfo->transferFamilyIndex = (u32)-1;

  // Check for discreteGPU
  if (requirements->discreteGPU) {
    if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      UINFO("NODE DICSCRETE GPU PRESENTA");
      return !VK_SUCCESS;
    }
  }

  UDEBUG("--------------------QUEUE PROPERTIES----------------------");
  UINFO("CHECKING FOR REQUIRED QUEUES");
  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
  VkQueueFamilyProperties* queueFamilies = kallocate(sizeof(VkQueueFamilyProperties) * queueFamilyCount, MEMORY_TAG_RENDERER);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

  UINFO("GRAPHICS | PRESENT | COMPUTE | TRANSFER | NAME");
  u8 minTransferScore = 255;
  for (u32 i = 0; i < queueFamilyCount; ++i) {
    u8 currentTransferScore = 0;

    // Graphics?
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      outQueueFamilyInfo->graphicsFamilyIndex = i;
      ++currentTransferScore;
    }
    // Compute?
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      outQueueFamilyInfo->computeFamilyIndex = i;
      ++currentTransferScore;
    }
    // Transfer?
    if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      // Take the index if it is the lowest, this increases the likelihood that it is a dedicated transfer queue
      if (currentTransferScore <= minTransferScore) {
        minTransferScore = currentTransferScore;
        outQueueFamilyInfo->transferFamilyIndex = i;
      }
    }

    // Present?
    VkBool32 supportsPresent = VK_FALSE;
    result =   vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent);
    VK_CHECK2(result, "failed to check physical device suface support");
    if (supportsPresent) {
      outQueueFamilyInfo->presentFamilyIndex = i;
    }
  }

  UINFO("       %d |       %d |       %d |        %d | %s",
        outQueueFamilyInfo->graphicsFamilyIndex != -1,
        outQueueFamilyInfo->presentFamilyIndex  != -1,
        outQueueFamilyInfo->computeFamilyIndex  != -1,
        outQueueFamilyInfo->transferFamilyIndex != -1,
        properties->deviceName);

  kfree(queueFamilies, sizeof(VkQueueFamilyProperties) * queueFamilyCount, MEMORY_TAG_RENDERER);

  if (
    (!requirements->graphics ||( outQueueFamilyInfo->graphicsFamilyIndex != -1)) ||
    (!requirements->present  ||( outQueueFamilyInfo->presentFamilyIndex  != -1)) ||
    (!requirements->compute  ||( outQueueFamilyInfo->computeFamilyIndex  != -1)) ||
    (!requirements->transfer ||( outQueueFamilyInfo->transferFamilyIndex != -1)))
  {
    UINFO("DEVICE MEETS QUEUE REQUIREMENTS");

    UINFO("GRAPHICS FAMILY INDEX: %i", outQueueFamilyInfo->graphicsFamilyIndex);
    UINFO("PRESENT FAMILY INDEX: %i", outQueueFamilyInfo->presentFamilyIndex);
    UINFO("COMPUTE FAMILY INDEX: %i", outQueueFamilyInfo->computeFamilyIndex);
    UINFO("TRANSFER FAMILY INDEX: %i", outQueueFamilyInfo->transferFamilyIndex);

    UDEBUG("--------------------SWAPCHAIN SUPPORT----------------------");
    UINFO("CHECKING FOR SWAPCHAIN SPPORT");
    result = vulkanDeviceQuerySwapchainSupport(device, surface, outSwapchainSupport);
    VK_CHECK2(result, "could not query all swapchain requirements");

    if (outSwapchainSupport->formatCount < 1 || outSwapchainSupport->presentModesCount < 1) {
      if (outSwapchainSupport->formats) {
        kfree(outSwapchainSupport->formats, sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
      }
      if (outSwapchainSupport->presentModes) {
        kfree(outSwapchainSupport->formats, sizeof(VkPresentModeKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
      }
      UINFO("REQUIRED SWAPCHAIN SUPPORT NOT PRESENT");
      return !VK_SUCCESS;
    }

    UINFO("DEVICE MEETS SWAPCHAIN REQUIREMENTS");

    UDEBUG("--------------------DEVICE EXTENSIONS----------------------");
    UINFO("CHECKING FOR DEVICE EXTENSIONS");
    if (requirements->deviceExtensionNames) {
      u32 availableExtensionCount = 0;
      VkExtensionProperties* availableExtensions = NULL;
      result =  vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount, NULL);
      VK_CHECK2( result, "could not get number of deviceExtensionProperties");

      if (availableExtensionCount != 0) {
        availableExtensions = kallocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        result = vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount, availableExtensions);
        VK_CHECK2(result, "could not get device extensions");

        u32 requiredExtensionsCount = darrayLength(requirements->deviceExtensionNames);
        for (u32 i = 0; i < requiredExtensionsCount; ++i) {
          u8 found = false;
          for (u32 j = 0; j < availableExtensionCount; ++j) {
            if (strcmp((requirements->deviceExtensionNames)[i], availableExtensions[j].extensionName) == 0) {
              found = true;
              break;
            }
          }
          if (!found) {
            UERROR("REQUIRED DEVICE EXTENSIONS NOT FOUND: %s", requirements->deviceExtensionNames[i]);
            kfree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
            return !VK_SUCCESS;
          }
        }
      }
      kfree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
    }
    UINFO("DEVICE HAS REQUIRED EXTENSIONS");

    UDEBUG("--------------------SAMPLER ANISOTROPY----------------------");
    UINFO("CHECKING FOR SMAPLER ANISOTROPY");
    if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
      UINFO("DEVICE DOES NOT SUPPORT SAMPLER ANISOTROPY");
      return !VK_SUCCESS;
    }

    UINFO("DEVICE MEETS ALL REQUIREMENTS!");
    return VK_SUCCESS;
  }

  UINFO("DEVICE DOES NOT MEET QUEUE REQUIREMENTS");
  return !VK_SUCCESS;
}


VkResult selectPhysicalDevice(vulkanContext* context) {

  VkResult  result = {0};
  u32 physicalDeviceCount = 0;
  result =  vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, NULL);
  VK_CHECK2(result, "could not enumerate physical devices");
  if (physicalDeviceCount == 0) {
    UFATAL("No devices which support Vulkan were found.");
    return false;
  }

  VkPhysicalDevice physicalDevices[physicalDeviceCount];
  result = vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices);
  VK_CHECK2(result, "could not enumerate physical devices");

  for (u32 i = 0; i < physicalDeviceCount; ++i) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

    UDEBUG("----------------------FEATURES----------------------");
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

    UDEBUG("----------------------MEMMORY----------------------");
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

    UDEBUG("----------------------REQUIREMENTS----------------------");
    vulkanPhysicalDeviceRequirements requirements = {0};
    requirements.graphics = true;
    requirements.present = true;
    requirements.transfer = true;
    requirements.compute = true;
    requirements.discreteGPU = true;
    requirements.samplerAnisotropy = true;
    requirements.deviceExtensionNames = darrayCreate(const char*);
    darrayPush(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    UINFO("CHECKING IF %s MEETS REQUIREMENTS", properties.deviceName);
    vulkanPhysicalDeviceQueueFamilyInfo queueInfo = {0};
    result = physicalDeviceMeetsRequirements(physicalDevices[i], context->surface, &properties, &features, &requirements, &queueInfo, &context->device.swapchainSupport);
    if (result == VK_SUCCESS) {
      UINFO("SELECTED DEVICE: '%s'", properties.deviceName);
      switch (properties.deviceType) {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
          UINFO("GPU TYPE IS UNKNOWN");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
          UINFO("GPU IS INTEGRATED");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
          UINFO("GPU IS DISCRETE");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
          UINFO("GPU IS VIRTUAL");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
          UINFO("USING CPU!!");
          break;
      }

      UINFO("GPU DRIVER VERSION: %d.%d.%d",
            VK_VERSION_MAJOR(properties.driverVersion),
            VK_VERSION_MINOR(properties.driverVersion),
            VK_VERSION_PATCH(properties.driverVersion));

      UINFO("VULKAN API VERSION: %d.%d.%d",
            VK_VERSION_MAJOR(properties.apiVersion),
            VK_VERSION_MINOR(properties.apiVersion),
            VK_VERSION_PATCH(properties.apiVersion));

      // MEMORY INFORMATION
      for (u32 i = 0; i < memory.memoryHeapCount; ++i) {
        f32 memSizeGIB = (((f32)memory.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f);
        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          UINFO("LOCAL GPU MEM: %.2f GIB", memSizeGIB);
        } else {
          UINFO("SHARED SYSTEM MEM: %.2f GIB", memSizeGIB);
        }
      }

      context->device.physicalDevice = physicalDevices[i];
      context->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
      context->device.presentQueueIndex = queueInfo.presentFamilyIndex;
      context->device.transferQueueIndex = queueInfo.transferFamilyIndex;

      context->device.properties = properties;
      context->device.features = features;
      context->device.memory = memory;

      break;
    }else{
      UERROR("DEVICE DID NOT MEET RQUIREMENTS ");
    }
  }//for loop

  // Ensure a device has been selected
  if (!context->device.physicalDevice) {
    UERROR("NO PHYSICAL DEVICES COULD MEET THE REQUIREMENTS.");
    return !VK_SUCCESS;
  }

  UINFO("PHYSICAL DEVICE HAS BEEN SELECTED!");
  return VK_SUCCESS;
}


VkResult vulkanDeviceCreate(vulkanContext* context) {
  VkResult result = {0};

  UINFO("SETTING UP PHYSICAL DEVICE");
  result  = selectPhysicalDevice(context);
  VK_CHECK2(result, "FAILED TO SELECT A PHYSICAL DEVICE");


  UINFO("SETTING UP LOGICAL DEVICE");
  u8  presentSharedGraphicsQueue  = context->device.graphicsQueueIndex == context->device.presentQueueIndex;
  u8  transferSharedGraphicsQueue = context->device.graphicsQueueIndex == context->device.transferQueueIndex;
  u32 indexCount = 1;
  if(!presentSharedGraphicsQueue){
    indexCount++;
  }
  if(!transferSharedGraphicsQueue){
    indexCount++;
  }

  u32 indices[4] = {0};
  u8 index = 0;
  indices[index++] = context->device.graphicsQueueIndex;
  if(!presentSharedGraphicsQueue){
    indices[index++] = context->device.presentQueueIndex;
  }
  if(!transferSharedGraphicsQueue){
    indices[index++] = context->device.transferQueueIndex;
  }

  UDEBUG("----------------------QUEUE CREATE INFO---------------");
  VkDeviceQueueCreateInfo queueCreateInfos[4] = {0};
  for(u32 i = 0; i < indexCount; ++i){
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex  = indices[i];
    queueCreateInfos[i].queueCount        = 1;
    //                  FUTURE STUFF
    // if(indices[i] == context->device.graphicsQueueIndex){
    //   queueCreateInfos[i].queueCount  = 2;
    // }
    queueCreateInfos[i].flags = 0;
    queueCreateInfos[i].pNext = 0;

    f32 queuePriority = 1.0f;
    queueCreateInfos[i].pQueuePriorities  = &queuePriority;
  }

  VkPhysicalDeviceFeatures deviceFeatures  = {0};
  deviceFeatures.samplerAnisotropy  = true;

  UDEBUG("----------------------LOGICAL DEVICE CREATE INFO-------");
  VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  createInfo.queueCreateInfoCount = indexCount;
  createInfo.pQueueCreateInfos    = queueCreateInfos;
  createInfo.pEnabledFeatures     = &deviceFeatures;
  const char* extenstionName      = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  createInfo.enabledExtensionCount= 1;
  createInfo.ppEnabledExtensionNames  = &extenstionName;
  //deprecated
  createInfo.enabledLayerCount    = 0;
  createInfo.ppEnabledLayerNames  = 0;

  result = vkCreateDevice(context->device.physicalDevice,
                          &createInfo,
                          context->allocator,
                          &context->device.logicalDevice);
  VK_CHECK2(result,  "COULD NOT CREATE LOGICAL DEVICE");
  UINFO("LOGICAL DEVICE CREATED!");

  UDEBUG("----------------------DEVICE QUEUES-------");
  UINFO("GETTING DEVICE QUEUES");
  vkGetDeviceQueue(context->device.logicalDevice,
                   context->device.graphicsQueueIndex,
                   0,
                   &context->device.graphicsQueue);

  vkGetDeviceQueue(context->device.logicalDevice,
                   context->device.presentQueueIndex,
                   0,
                   &context->device.presentQueue);

  vkGetDeviceQueue(context->device.logicalDevice,
                   context->device.transferQueueIndex,
                   0,
                   &context->device.transferQueue);

  UINFO("QUEUS OBTAIINED");

  UDEBUG("----------------------COMMAND POOLS-------");
  UINFO("GETTING COMMAND POOLS");
  VkCommandPoolCreateInfo poolCreateInfo  = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  poolCreateInfo.queueFamilyIndex = context->device.graphicsQueueIndex;
  //allows any command buffer allocateeed from a pool to be indivisually reset to initial state;
  poolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  result  = vkCreateCommandPool(context->device.logicalDevice, &poolCreateInfo,
                                context->allocator, &context->device.graphicsCommandPool);
  VK_CHECK2(result, "FAILED TO CREATE COMMAND POOLS");
  UINFO("GRPAHICS COMMAND POOL HAS BEEN CREATED");
  return result;
}

VkResult vulkanDeviceDestroy(vulkanContext* context) {

  UINFO("DESTROYING COMMAND POOLS");
  vkDestroyCommandPool(context->device.logicalDevice,
                       context->device.graphicsCommandPool,
                       context->allocator);

  context->device.graphicsQueue = NULL;
  context->device.presentQueue = NULL;
  context->device.transferQueue = NULL;

  UINFO("DESTROYING LOGICAL DEVICE");
  if(context->device.logicalDevice){
    vkDestroyDevice(context->device.logicalDevice, context->allocator);
    context->device.logicalDevice = NULL;
  }

  UINFO("RELEASING PHYSICAL DEVICE RESOURCES");

  if(context->device.swapchainSupport.formats){
    kfree(context->device.swapchainSupport.formats,
          sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.formatCount,
          MEMORY_TAG_RENDERER);
    context->device.swapchainSupport.formats = NULL;
    context->device.swapchainSupport.formatCount = 0;
  }

  if(context->device.swapchainSupport.presentModes){
    kfree(context->device.swapchainSupport.presentModes,
          sizeof(VkPresentModeKHR) * context->device.swapchainSupport.presentModesCount,
          MEMORY_TAG_RENDERER);
    context->device.swapchainSupport.presentModes = NULL;
    context->device.swapchainSupport.presentModesCount  = 0;
  }

  kzeroMemory(&context->device.swapchainSupport.capabilities,
              sizeof(context->device.swapchainSupport.capabilities));
  context->device.presentQueueIndex = -1;
  context->device.graphicsQueueIndex = -1;
  context->device.transferQueueIndex = -1;


  return VK_SUCCESS;
}

VkResult  vulkanDeviceDetectDepthFormat(vulkanDevice* device){

  //format cadidates
  const u64 candidateCount  = 3;
  VkFormat  candidates[3] = {
    VK_FORMAT_D32_SFLOAT,                 //32 bit signed folating point format in the depth component
    VK_FORMAT_D32_SFLOAT_S8_UINT,         //two component format -> 32 signed float bits in depth & 8 unsigned int in stencil
    VK_FORMAT_D24_UNORM_S8_UINT};         //two component fromat -> 24 unsigned normalized bits in depth & 8 unsigned int  in stencil

  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for(u64 i = 0; i < candidateCount; ++i){
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i], &properties);
    if((properties.linearTilingFeatures & flags) == flags){
      device->depthFormat = candidates[i];
      UINFO("DEPTH FORMAT SPPORTED");
      return VK_SUCCESS;
    }else if((properties.optimalTilingFeatures & flags) == flags){
      device->depthFormat = candidates[i];
      UINFO("DEPTH FORMAT SPPORTED");
      return VK_SUCCESS;
    }
  }

  return !VK_SUCCESS;
}

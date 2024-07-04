#include  "vulkandevice.h"
#include  "vulkantypes.h"
#include  "rendererutils.h"

#include  "core/kmemory.h"
#include  "core/logger.h"
#include  "containers/darray.h"

typedef struct VulkanPhysicalDeviceRequirements     VulkanPhysicalDeviceRequirements;
typedef struct VulkanPhysicalDeviceQueueFamilyInfo  VulkanPhysicalDeviceQueueFamilyInfo;

struct VulkanPhysicalDeviceRequirements {
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
struct VulkanPhysicalDeviceQueueFamilyInfo {
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
  const VulkanPhysicalDeviceRequirements* requirements,
  VulkanPhysicalDeviceQueueFamilyInfo* outQueueFamilyInfo,
  VulkanSwapchainSupportInfo* outSwapchainSupport);

VkResult 
vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice            physicalDevice,
                                  VkSurfaceKHR                surface,
                                  VulkanSwapchainSupportInfo *outSupportInfo)
{
TRACEFUNCTION;
  KTRACE("----------------------DEVICE SURFACE CAPABILTITIES---------");
  VkResult result = {0};
  result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &outSupportInfo->capabilities);
  VK_CHECK_VERBOSE(result, "failed to get device surface capabilities");

  KTRACE("----------------------SURFACE FORMATS----------------------");
  result  = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, 0);
  VK_CHECK_VERBOSE(result, "failed to get device formats count");

  if (outSupportInfo->formatCount != 0) {
    outSupportInfo->formats = kallocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &outSupportInfo->formatCount, outSupportInfo->formats);
    VK_CHECK_VERBOSE(result, "failed to get device formats available");
  }

  KTRACE("----------------------PRESENT MODES-----------------------");
  result  = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &outSupportInfo->presentModesCount, 0);
  VK_CHECK_VERBOSE(result, "failed to get present modes count");

  if (outSupportInfo->presentModesCount != 0) {
    outSupportInfo->presentModes = kallocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModesCount, MEMORY_TAG_RENDERER);
    result  = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                        surface,
                                                        &outSupportInfo->presentModesCount,
                                                        outSupportInfo->presentModes);
    VK_CHECK_VERBOSE(result, "failed to get present modes");
  }

  KINFO("QUERIED SWAPCHAIN SUPPORT SUCCESFULLY");
  return VK_SUCCESS;
}


VkResult
physicalDeviceMeetsRequirements(VkPhysicalDevice                        device,
                                VkSurfaceKHR                            surface,
                                const VkPhysicalDeviceProperties*       properties,
                                const VkPhysicalDeviceFeatures*         features,
                                const VulkanPhysicalDeviceRequirements* requirements,
                                VulkanPhysicalDeviceQueueFamilyInfo*    outQueueFamilyInfo,
                                VulkanSwapchainSupportInfo*             outSwapchainSupport)
{
TRACEFUNCTION;
  VkResult  result = {0};
  // Evaluate device properties to determine the needs of our application
  outQueueFamilyInfo->graphicsFamilyIndex = (u32)-1;
  outQueueFamilyInfo->presentFamilyIndex = (u32)-1;
  outQueueFamilyInfo->computeFamilyIndex = (u32)-1;
  outQueueFamilyInfo->transferFamilyIndex = (u32)-1;

  // Check for discreteGPU
  if (requirements->discreteGPU) {
    if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      KINFO("NO DICSCRETE GPU PRESENT");
      return !VK_SUCCESS;
    }
  }

  KTRACE("--------------------QUEUE PROPERTIES----------------------");
  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
  VkQueueFamilyProperties queueFamilies[32]; 
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

  KDEBUG("GRAPHICS | PRESENT | COMPUTE | TRANSFER | NAME");
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
    VK_CHECK_VERBOSE(result, "failed to check physical device suface support");
    if (supportsPresent) {
      outQueueFamilyInfo->presentFamilyIndex = i;
    }
  }

  KDEBUG("       %d |       %d |       %d |        %d | %s",
        outQueueFamilyInfo->graphicsFamilyIndex != -1,
        outQueueFamilyInfo->presentFamilyIndex  != -1,
        outQueueFamilyInfo->computeFamilyIndex  != -1,
        outQueueFamilyInfo->transferFamilyIndex != -1,
        properties->deviceName);


  if (
    (!requirements->graphics ||( outQueueFamilyInfo->graphicsFamilyIndex != -1)) ||
    (!requirements->present  ||( outQueueFamilyInfo->presentFamilyIndex  != -1)) ||
    (!requirements->compute  ||( outQueueFamilyInfo->computeFamilyIndex  != -1)) ||
    (!requirements->transfer ||( outQueueFamilyInfo->transferFamilyIndex != -1)))
  {
    KINFO("DEVICE MEETS QUEUE REQUIREMENTS");

    KDEBUG("GRAPHICS FAMILY INDEX: %i", outQueueFamilyInfo->graphicsFamilyIndex);
    KDEBUG("PRESENT FAMILY INDEX: %i", outQueueFamilyInfo->presentFamilyIndex);
    KDEBUG("COMPUTE FAMILY INDEX: %i", outQueueFamilyInfo->computeFamilyIndex);
    KDEBUG("TRANSFER FAMILY INDEX: %i", outQueueFamilyInfo->transferFamilyIndex);

    KTRACE("--------------------SWAPCHAIN SUPPORT----------------------");
    result = vulkanDeviceQuerySwapchainSupport(device, surface, outSwapchainSupport);
    VK_CHECK_VERBOSE(result, "could not query all swapchain requirements");

    if (outSwapchainSupport->formatCount < 1 || outSwapchainSupport->presentModesCount < 1) {
      if (outSwapchainSupport->formats) {
        kfree(outSwapchainSupport->formats, sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
      }
      if (outSwapchainSupport->presentModes) {
        kfree(outSwapchainSupport->formats, sizeof(VkPresentModeKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
      }
      KINFO("REQUIRED SWAPCHAIN SUPPORT NOT PRESENT");
      return !VK_SUCCESS;
    }

    KINFO("DEVICE MEETS SWAPCHAIN REQUIREMENTS");

    KTRACE("--------------------DEVICE EXTENSIONS----------------------");
    if (requirements->deviceExtensionNames) {
      u32 availableExtensionCount = 0;
      VkExtensionProperties* availableExtensions = NULL;
      result =  vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount, NULL);
      VK_CHECK_VERBOSE( result, "could not get number of deviceExtensionProperties");

      if (availableExtensionCount != 0) {
        availableExtensions = kallocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        result = vkEnumerateDeviceExtensionProperties(device, NULL, &availableExtensionCount, availableExtensions);
        VK_CHECK_VERBOSE(result, "could not get device extensions");

        u32 requiredExtensionsCount = DARRAY_LENGTH(requirements->deviceExtensionNames);
        for (u32 i = 0; i < requiredExtensionsCount; ++i) {
          u8 found = false;
          for (u32 j = 0; j < availableExtensionCount; ++j) {
            if (strcmp((requirements->deviceExtensionNames)[i], availableExtensions[j].extensionName) == 0) {
              found = true;
              break;
            }
          }
          if (!found) {
            KERROR("REQUIRED DEVICE EXTENSIONS NOT FOUND: %s", requirements->deviceExtensionNames[i]);
            kfree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
            return !VK_SUCCESS;
          }
        }
      }
      kfree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
    }
    KINFO("DEVICE HAS REQUIRED EXTENSIONS");

    KTRACE("--------------------SAMPLER ANISOTROPY----------------------");
    if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
      KINFO("DEVICE DOES NOT SUPPORT SAMPLER ANISOTROPY");
      return !VK_SUCCESS;
    }

    KINFO("DEVICE MEETS ALL REQUIREMENTS!");
    return VK_SUCCESS;
  }

  KINFO("DEVICE DOES NOT MEET QUEUE REQUIREMENTS");
  return !VK_SUCCESS;
}

VkResult
selectPhysicalDevice(VulkanContext* context)
{
TRACEFUNCTION;
  VkResult  result = {0};
  u32 physicalDeviceCount = 0;
  result =  vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, NULL);
  VK_CHECK_VERBOSE(result, "vkEnumeratePhysicalDevices failed");
  if (physicalDeviceCount == 0) {
    KFATAL("No devices which support Vulkan were found.");
    return !VK_SUCCESS;
  }

  const int maxDeviceCount = 32;
  VkPhysicalDevice physicalDevices[maxDeviceCount];
  result = vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices);
  VK_CHECK_VERBOSE(result, "vkEnumeratePhysicalDevices failed");

  for (u32 i = 0; i < physicalDeviceCount; ++i) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

    KDEBUG("----------------------FEATURES----------------------");
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

    KDEBUG("----------------------MEMMORY----------------------");
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

    KDEBUG("----------------------REQUIREMENTS----------------------");
    VulkanPhysicalDeviceRequirements requirements = {0};
    requirements.graphics = true;
    requirements.present = true;
    requirements.transfer = true;
    requirements.compute = true;
    requirements.discreteGPU = true;
    requirements.samplerAnisotropy = true;
    requirements.deviceExtensionNames = DARRAY_CREATE(const char*);
    DARRAY_PUSH(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    KINFO("CHECKING IF %s MEETS REQUIREMENTS", properties.deviceName);
    VulkanPhysicalDeviceQueueFamilyInfo queueInfo = {0};
    result = physicalDeviceMeetsRequirements(physicalDevices[i], context->surface, &properties, &features, &requirements, &queueInfo, &context->device.swapchainSupport);
    if (result == VK_SUCCESS) {
      KINFO("SELECTED DEVICE: '%s'", properties.deviceName);
      switch (properties.deviceType) {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
          KINFO("GPU TYPE IS UNKNOWN");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
          KINFO("GPU IS INTEGRATED");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
          KINFO("GPU IS DISCRETE");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
          KINFO("GPU IS VIRTUAL");
          break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
          KINFO("USING CPU!!");
          break;
      }

      KDEBUG("GPU DRIVER VERSION: %d.%d.%d",
            VK_VERSION_MAJOR(properties.driverVersion),
            VK_VERSION_MINOR(properties.driverVersion),
            VK_VERSION_PATCH(properties.driverVersion));

      KDEBUG("Vulkan API VERSION: %d.%d.%d",
            VK_VERSION_MAJOR(properties.apiVersion),
            VK_VERSION_MINOR(properties.apiVersion),
            VK_VERSION_PATCH(properties.apiVersion));

      // MEMORY INFORMATION
      for (u32 i = 0; i < memory.memoryHeapCount; ++i) {
        f32 memSizeGIB = (((f32)memory.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f);
        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          KINFO("LOCAL GPU MEM: %.2f GIB", memSizeGIB);
        } else {
          KINFO("SHARED SYSTEM MEM: %.2f GIB", memSizeGIB);
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
      KERROR("DEVICE DID NOT MEET RQUIREMENTS ");
    }
  }//for loop

  // Ensure a device has been selected
  if (!context->device.physicalDevice) {
    KERROR("NO PHYSICAL DEVICES COULD MEET THE REQUIREMENTS.");
    return !VK_SUCCESS;
  }

  KINFO("PHYSICAL DEVICE HAS BEEN SELECTED!");
  return VK_SUCCESS;
}

VkResult
vulkanDeviceCreate(VulkanContext* context)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;

  KINFO("SETTING UP PHYSICAL DEVICE");
  result  = selectPhysicalDevice(context);
  VK_CHECK_RESULT(result, "FAILED TO SELECT A PHYSICAL DEVICE");


  KINFO("SETTING UP LOGICAL DEVICE");
  u8  presentSharedGraphicsQueue  = context->device.graphicsQueueIndex == context->device.presentQueueIndex;
  u8  transferSharedGraphicsQueue = context->device.graphicsQueueIndex == context->device.transferQueueIndex;
  u32 indexCount = 1;
  if(!presentSharedGraphicsQueue){
    indexCount++;
  }
  if(!transferSharedGraphicsQueue){
    indexCount++;
  }

  u32 indices[32] = {0};
  u8 index = 0;
  indices[index++] = context->device.graphicsQueueIndex;
  if(!presentSharedGraphicsQueue){
    indices[index++] = context->device.presentQueueIndex;
  }
  if(!transferSharedGraphicsQueue){
    indices[index++] = context->device.transferQueueIndex;
  }

  KTRACE("----------------------QUEUE CREATE INFO---------------");
  VkDeviceQueueCreateInfo queueCreateInfos[32] = {0};
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

  KTRACE("----------------------LOGICAL DEVICE CREATE INFO-------");
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
  VK_CHECK_VERBOSE(result,  "COULD NOT CREATE LOGICAL DEVICE");
  KINFO("LOGICAL DEVICE CREATED!");

  KTRACE("------------------DEVICE QUEUES--------------------");
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

  KINFO("QUEUS OBTAIINED");

  KTRACE("------------------COMMAND POOLS---------------------");
  VkCommandPoolCreateInfo poolCreateInfo  = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  poolCreateInfo.queueFamilyIndex = context->device.graphicsQueueIndex;
  //allows any command buffer allocateeed from a pool to be indivisually reset to initial state;
  poolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  result  = vkCreateCommandPool(context->device.logicalDevice, &poolCreateInfo,
                                context->allocator, &context->device.graphicsCommandPool);
  VK_CHECK_VERBOSE(result, "FAILED TO CREATE COMMAND POOLS");
  KINFO("GRPAHICS COMMAND POOL HAS BEEN CREATED");
  KINFO("VULKAN DEVICE CREATED");
  return result;
}

VkResult
vulkanDeviceDestroy(VulkanContext* context)
{
TRACEFUNCTION;
  KINFO("DESTROYING COMMAND POOLS");
  vkDestroyCommandPool(context->device.logicalDevice,
                       context->device.graphicsCommandPool,
                       context->allocator);

  context->device.graphicsQueue = NULL;
  context->device.presentQueue = NULL;
  context->device.transferQueue = NULL;

  KINFO("DESTROYING LOGICAL DEVICE");
  if(context->device.logicalDevice){
    vkDestroyDevice(context->device.logicalDevice, context->allocator);
    context->device.logicalDevice = NULL;
  }

  KINFO("RELEASING PHYSICAL DEVICE RESOURCES");

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

  KINFO("SUCCESSFULY DESTROYED VULKAN DEVICE");
  return VK_SUCCESS;
}

VkResult
vulkanDeviceDetectDepthFormat(VulkanDevice* device)
{
TRACEFUNCTION;
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
      KINFO("DEPTH FORMAT SPPORTED");
      return VK_SUCCESS;
    }else if((properties.optimalTilingFeatures & flags) == flags){
      device->depthFormat = candidates[i];
      KINFO("DEPTH FORMAT SPPORTED");
      return VK_SUCCESS;
    }
  }
  KINFO("DEPTH FORMAT NOT SPPORTED");
  return !VK_SUCCESS;
}

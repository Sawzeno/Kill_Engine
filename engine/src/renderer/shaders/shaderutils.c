#include  "shaderutils.h"
#include  "renderer/rendererutils.h"
#include  "core/logger.h"
#include "core/kmemory.h"
#include "platform/filesystem.h"

VkResult createShaderModule(VulkanContext* context,
                       const char* shaderName,
                       const char* stageName,
                       VkShaderStageFlagBits stageFlag,
                       u64  stageIndex,
                       VulkanShaderStage* shaderStagesArr)
{
  char file[256];
  snprintf(file, 128, "/home/gon/Developer/Kill_Engine/bin/%s.%s.spv",shaderName, stageName);
  UDEBUG("READING FILE %s",file);
  kzeroMemory(&shaderStagesArr[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
  shaderStagesArr[stageIndex].createInfo.sType  = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  FileHandle handle;
  if(!KFileOpen(file, FILE_MODE_READ, true, &handle)){
    UERROR("FAILED TO OPEN SHADER MODULE");
    return !VK_SUCCESS;
  }
  
  u8* fileBuffer  = {0};
  if(!KFileReadAllBytes(&handle, &fileBuffer)){
    UERROR("FAILED TO READ SHADER MODULE");
    return !VK_SUCCESS;
  }

  shaderStagesArr[stageIndex].createInfo.codeSize = handle.size;
  shaderStagesArr[stageIndex].createInfo.pCode    = (u32*)fileBuffer;

  u64 fileSize  = handle.size;
  KFileClose(&handle);

  VK_CHECK2(vkCreateShaderModule(context->device.logicalDevice,
                                 &shaderStagesArr[stageIndex].createInfo,
                                 context->allocator,
                                 &shaderStagesArr[stageIndex].handle),
            "failed to create Shader Module");
  kzeroMemory(&shaderStagesArr[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
  shaderStagesArr[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStagesArr[stageIndex].shaderStageCreateInfo.stage = stageFlag;
  shaderStagesArr[stageIndex].shaderStageCreateInfo.module= shaderStagesArr[stageIndex].handle;
  shaderStagesArr[stageIndex].shaderStageCreateInfo.pName = "main";

  if(fileBuffer){
    kfree(fileBuffer, sizeof(u8) * fileSize, MEMORY_TAG_STRING);
    fileBuffer  = 0;
  }

  return VK_SUCCESS;
}



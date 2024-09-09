#include  "shaderutils.h"

#include "core/kstring.h"
#include  "core/logger.h"
#include  "core/kmemory.h"
#include  "core/filesystem.h"
#include  "renderer/rendererutils.h"

VkResult createShaderModule(VulkanContext* context,
                       const char* shaderName,
                       const char* stageName,
                       VkShaderStageFlagBits stageFlag,
                       u64  stageIndex,
                       VulkanShaderStage* shaderStagesArr)
{
  char file[1024];
  kstrfmt(file, "/home/gon/Developer/Kill_Engine/bin/%s.%s.spv", shaderName, stageName);

  kzeroMemory(&shaderStagesArr[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
  shaderStagesArr[stageIndex].createInfo.sType  = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  FileHandle handle;

  if(KFileOpen(file, FILE_MODE_READ, true, &handle) != KFILE_RET_SUCCESS) return !VK_SUCCESS;
  KINFO("ACQUIRED HANDLE ON SHADER MODULE");

  u8* fileBuffer  = kallocate(handle.size * sizeof(u8),MEMORY_TAG_STRING );

  if(KFileReadAllBytes(&handle, &fileBuffer) != KFILE_RET_SUCCESS) return !VK_SUCCESS;
  KINFO("SUCCESFULLY READ SHADER MODULE");

  shaderStagesArr[stageIndex].createInfo.codeSize = handle.size;
  shaderStagesArr[stageIndex].createInfo.pCode    = (u32*)fileBuffer;

  KFileClose(&handle);

  VkResult result = vkCreateShaderModule(context->device.logicalDevice,
                                 &shaderStagesArr[stageIndex].createInfo,
                                 context->allocator,
                                 &shaderStagesArr[stageIndex].handle);
  VK_CHECK_VERBOSE(result, "vkCreateShaderModule failed");

  kzeroMemory(&shaderStagesArr[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
  shaderStagesArr[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStagesArr[stageIndex].shaderStageCreateInfo.stage = stageFlag;
  shaderStagesArr[stageIndex].shaderStageCreateInfo.module= shaderStagesArr[stageIndex].handle;
  shaderStagesArr[stageIndex].shaderStageCreateInfo.pName = "main";

  if(fileBuffer){
    kfree(fileBuffer, handle.size * sizeof(u8), MEMORY_TAG_STRING);
  }

  return VK_SUCCESS;
}



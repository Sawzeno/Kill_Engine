#include  "vulkanshaderobject.h"
#include "renderer/rendererutils.h"
#include  "shaderutils.h"
#include  "core/logger.h"
#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"


VkResult  vulkanObjectShaderCreate  (VulkanContext* context, VulkanObjectShader* outShader){
  //shader module init per stage

  VkResult  result  = {0};
  char stageTypesStrs [OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
  VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    result = createShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stageTypesStrs[i], stageTypes[i], i, outShader->stages);
    if(result != VK_SUCCESS){
      UERROR("UNABLE TO CREATE SHADER MODULE %s FOR %s | %d : %s",
             stageTypesStrs[i],
             BUILTIN_SHADER_NAME_OBJECT,
             result,
             vkResultToString(result));
      return result;
    }
    UDEBUG("CREATED SHADER MODULE %s FOR %s",stageTypesStrs[i],BUILTIN_SHADER_NAME_OBJECT);
  }
  //descriptors
  return result;
}
VkResult  vulkanObjectShaderDestroy (VulkanContext* context, VulkanObjectShader* shader);

VkResult  vulkanObjectShaderUse     (VulkanContext* context, VulkanObjectShader* shader);


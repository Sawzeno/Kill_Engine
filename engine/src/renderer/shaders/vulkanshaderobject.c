#include  "vulkanshaderobject.h"
#include  "renderer/vulkanpipeline.h"
#include "math/mathtypes.h"
#include "renderer/rendererutils.h"
#include  "shaderutils.h"
#include  "core/logger.h"
#include <vulkan/vulkan_core.h>
#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"


VkResult  vulkanObjectShaderCreate  (VulkanContext* context, VulkanObjectShader* outShader){
  TRACEFUNCTION;
  KDEBUG("context : %p outShader : %p",context,outShader);
  //shader module init per stage

  VkResult  result  = !VK_SUCCESS;
  char stageTypesStrs [OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
  VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    result = createShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stageTypesStrs[i], stageTypes[i], i, outShader->stages);
    VK_CHECK_RESULT(result,"UNABLE TO CREATE SHADER MODULE %s FOR %s | %d : %s",
             stageTypesStrs[i],
             BUILTIN_SHADER_NAME_OBJECT,
             result,
             vulkanResultToString(result));
    KDEBUG("CREATED SHADER MODULE %s FOR %s",stageTypesStrs[i],BUILTIN_SHADER_NAME_OBJECT);
  }

  UWARN("TODO DESCRIPTORS");

  VkViewport viewport;
  viewport.x  = 0.0f;
  viewport.y  = 0.0f;
  viewport.width  = (f32)context->frameBufferWidth;
  viewport.height = (f32)context->frameBufferHeight;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor;
  scissor.offset.x  = scissor.offset.y  = 0;
  scissor.extent.width  = context->frameBufferWidth;
  scissor.extent.height = context->frameBufferHeight;

  u32 offset  = 0;
  const i32 attributeCount  = 1;
  VkVertexInputAttributeDescription attributeDescriptions[attributeCount] = {0};
  //POSITION
  VkFormat formats[attributeCount] = {
    VK_FORMAT_R32G32B32_SFLOAT,
  };
  u64 sizes[attributeCount] = {
    sizeof(Vec3)
  };
  for(u32 i = 0; i < attributeCount; ++i){
    attributeDescriptions[i].binding  = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format   = formats[i];
    attributeDescriptions[i].offset   = offset;
    offset += sizes[i];
  }
  UWARN("TODO DESCRIPTOR SET LAYOUT");

  //STAGES
  //should match the numbers of shader->stages
  VkPipelineShaderStageCreateInfo StageCreateInfos[OBJECT_SHADER_STAGE_COUNT]  = {0};
  for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    StageCreateInfos[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
    StageCreateInfos[i]       = outShader->stages[i].shaderStageCreateInfo;
  }

  KINFO("CREATING VULKAN GRAPHICS PIPELINE");
  result  = vulkanGraphicsPipelineCreate(context,
                                         &context->mainRenderPass,
                                         attributeCount,
                                         attributeDescriptions,
                                         0,
                                         0,
                                         OBJECT_SHADER_STAGE_COUNT,
                                         StageCreateInfos,
                                         viewport,
                                         scissor,
                                         false,
                                         &outShader->pipeline);
  VK_CHECK_RESULT(result, "FAILED TO CREATE SHADER OBJECT");
  KINFO("SUCCESFULLY CRAEATED SHADER OBJECT");
  return result;
}
VkResult  vulkanObjectShaderDestroy (VulkanContext* context, VulkanObjectShader* shader){
 TRACEFUNCTION; 
  KDEBUG("context : %p shader : %p",context, shader);
  vulkanGraphicsPipelineDestroy(context, &shader->pipeline);
  VkResult result = !VK_SUCCESS;
  //destroy shader modules
  for(u32 i  = 0 ; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
    shader->stages[i].handle  = NULL;
  }
  return result;
}

VkResult  vulkanObjectShaderUse     (VulkanContext* context, VulkanObjectShader* shader){
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
  u32 imageIndex  = context->imageIndex;
  result = vulkanGraphicsPipelineBind(&context->graphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
  VK_CHECK_RESULT(result, "vulkanGraphicsPipelineBind failed in vulkanObjectShaderUse");

  return result;
}


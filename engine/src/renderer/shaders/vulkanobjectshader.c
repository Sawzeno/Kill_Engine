#include  "vulkanobjectshader.h"
#include  "shaderutils.h"

#include  "renderer/renderertypes.h"
#include  "renderer/rendererutils.h"
#include  "renderer/vulkanpipeline.h"
#include  "renderer/vulkanbuffer.h"

#include  "core/logger.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"


VkResult
vulkanObjectShaderCreate(VulkanContext* context, VulkanObjectShader* outShader)
{
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

  KTRACE("----------------------UNIFORM BUFFER OBJECT----------------------");
  //UBO-------------> UNIFORM BUFFER OBJECT
  //Global Descriptors
  VkDescriptorSetLayoutBinding globalUBOSetLayoutBinding  = {0};
  globalUBOSetLayoutBinding.binding            = 0;
  globalUBOSetLayoutBinding.descriptorCount    = 1;
  globalUBOSetLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  globalUBOSetLayoutBinding.pImmutableSamplers = 0;
  globalUBOSetLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo globalUBOSetLayoutCreateInfo= {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  globalUBOSetLayoutCreateInfo.bindingCount    = 1;
  globalUBOSetLayoutCreateInfo.pBindings       = &globalUBOSetLayoutBinding;

  result  = vkCreateDescriptorSetLayout(context->device.logicalDevice,
                                        &globalUBOSetLayoutCreateInfo,
                                        context->allocator,
                                        &outShader->globalDescriptorSetLayout);

  VK_CHECK_VERBOSE(result, "vkCreateDescriptorSetLayout failed in vulkanShaderOjbectCreate while trying to create globalUBO"); 

  VkDescriptorPoolSize  globalUBOPoolSize      = {0};
  globalUBOPoolSize.type                       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  globalUBOPoolSize.descriptorCount            = context->swapchain.imageCount;

  VkDescriptorPoolCreateInfo  globalUBOPoolCreateInfo         = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  globalUBOPoolCreateInfo.poolSizeCount        = 1;
  globalUBOPoolCreateInfo.pPoolSizes           = &globalUBOPoolSize;
  globalUBOPoolCreateInfo.maxSets              = context->swapchain.imageCount;

  result  = vkCreateDescriptorPool(context->device.logicalDevice,
                                   &globalUBOPoolCreateInfo,
                                   context->allocator,
                                   &outShader->globalDescriptorPool);
  VK_CHECK_VERBOSE(result, "vkCreateDescriptorPool failed in vulkanObjectShaderCreate while trying to create globalUBO")

  KTRACE("----------------------GRAPHICS PIPELINE----------------------");
  //viewport
  VkViewport viewport;
  viewport.x  = 0.0f;
  viewport.y  = 0.0f;
  viewport.width  = (f32)context->frameBufferWidth;
  viewport.height = (f32)context->frameBufferHeight;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  //scissor
  VkRect2D scissor;
  scissor.offset.x  = scissor.offset.y  = 0;
  scissor.extent.width  = context->frameBufferWidth;
  scissor.extent.height = context->frameBufferHeight;

  //attributes
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
  // descriptor set layouts
  const i32 descriptorSetLayoutCount  = 1;
  VkDescriptorSetLayout descriptorSetLayouts[descriptorSetLayoutCount]  = {outShader->globalDescriptorSetLayout};

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
                                         descriptorSetLayoutCount,
                                         descriptorSetLayouts,
                                         OBJECT_SHADER_STAGE_COUNT,
                                         StageCreateInfos,
                                         viewport,
                                         scissor,
                                         false,
                                         &outShader->pipeline);
  VK_CHECK_RESULT(result, "FAILED TO CREATE GRAPHICS PIPELINE FOR OBJECT SHADER");

  //create Uniform Buffer
  result=vulkanBufferCreate(context,
                            sizeof(GlobalUniformObject),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true,
                            &outShader->globalUniformBuffer);
  VK_CHECK_RESULT(result,"vulkanBufferCreate failed in vulkanObjectShaderCreate while tryin to create globalUBO");

  // allocate global descriptor sets
  VkDescriptorSetLayout globalUBOSetLayouts[3]  = {
    outShader->globalDescriptorSetLayout,
    outShader->globalDescriptorSetLayout,
    outShader->globalDescriptorSetLayout,};

  VkDescriptorSetAllocateInfo globalUBOAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  globalUBOAllocateInfo.descriptorPool      = outShader->globalDescriptorPool;
  globalUBOAllocateInfo.descriptorSetCount  = 3;
  globalUBOAllocateInfo.pSetLayouts         = globalUBOSetLayouts;

  result  = vkAllocateDescriptorSets(context->device.logicalDevice, &globalUBOAllocateInfo, outShader->globalDescriptorSets);
  VK_CHECK_VERBOSE(result, "vkAllocateDescriptorSets failed in vulkanObjectShaderCreate while trying to allocate globalUBO");
  KINFO("SUCCESFULLY CRAEATED SHADER OBJECT");
  return result;
}

VkResult
vulkanObjectShaderDestroy(VulkanContext* context, VulkanObjectShader* shader)
{
  TRACEFUNCTION; 
  KDEBUG("context : %p shader : %p",context, shader);

  //destroy uniform buffer
  vulkanBufferDestroy(context, &shader->globalUniformBuffer);

  //destroy gpaphics pipeline
  vulkanGraphicsPipelineDestroy(context, &shader->pipeline);

  //destroy global descriptor pool
  vkDestroyDescriptorPool(context->device.logicalDevice, shader->globalDescriptorPool, context->allocator);

  //destory descriptor set layouts
  vkDestroyDescriptorSetLayout(context->device.logicalDevice, shader->globalDescriptorSetLayout, context->allocator);

  VkResult result = !VK_SUCCESS;
  //destroy shader modules
  for(u32 i  = 0 ; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
    shader->stages[i].handle  = NULL;
  }
  KINFO("SUCCESFULLY DESTROYED SHADER OBJECT");
  return result;
}

VkResult
vulkanObjectShaderUse(VulkanContext* context, VulkanObjectShader* shader)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
  u32 imageIndex  = context->imageIndex;
  result = vulkanGraphicsPipelineBind(&context->graphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
  VK_CHECK_RESULT(result, "vulkanGraphicsPipelineBind failed in vulkanObjectShaderUse");

  KINFO("SUCCESFULLY USED SHADER OBJECT");
  return result;
}

VkResult
vulkanObjectShaderUpdateGlobalState(VulkanContext* context, VulkanObjectShader* shader)
{
  TRACEFUNCTION;

  u32 imageIndex                   = context->imageIndex;
  VkCommandBuffer commandBuffer    = context->graphicsCommandBuffers[imageIndex].handle;
  VkDescriptorSet globalDescriptor = shader->globalDescriptorSets[imageIndex];

  // bind the global descriptor set to updated
  vkCmdBindDescriptorSets(commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.pipelineLayout,
                          0,
                          1,
                          &globalDescriptor,
                          0,
                          0);
  //configure the descriptors for the given index
  u32 range = sizeof(GlobalUniformObject);
  u64 offset= 0;

  //copy data to buffer
  VkResult result   = vulkanBufferLoadData(context, &shader->globalUniformBuffer, offset, range, 0, &shader->globalUBO);
  VK_CHECK_RESULT(result, "vulkanBufferLoadData failed in vulkanObjectShaderUpdateGlobalState while trying to load data for globalUBO");

  //update descriptor sets
  VkDescriptorBufferInfo  bufferInfo  = {0};
  bufferInfo.buffer = shader->globalUniformBuffer.handle;
  bufferInfo.offset = offset;
  bufferInfo.range  = range;

  VkWriteDescriptorSet descriptorWrite  = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptorWrite.dstSet                = shader->globalDescriptorSets[imageIndex];
  descriptorWrite.dstBinding            = 0;
  descriptorWrite.dstArrayElement       = 0;
  descriptorWrite.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount       = 1;
  descriptorWrite.pBufferInfo           = &bufferInfo;

  vkUpdateDescriptorSets(context->device.logicalDevice, 1, &descriptorWrite, 0, 0);
  KINFO("SUCCESFULLY UPDATED SHADER OBJECT GLOBAL STATE");
  return result;
}

VkResult
vulkanObjectShaderUpdateObject(VulkanContext* context, VulkanObjectShader* shader,  Mat4 model)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
  
  u32 imageIndex                = context->imageIndex;
  VkCommandBuffer commandBuffer = context->graphicsCommandBuffers[imageIndex].handle;

  vkCmdPushConstants(commandBuffer, shader->pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &model);
  return VK_SUCCESS;
}

#include  "vulkanobjectshader.h"
#include "core/kmemory.h"
#include "math/kmath.h"
#include  "shaderutils.h"

#include  "renderer/renderertypes.h"
#include  "renderer/rendererutils.h"
#include  "renderer/vulkanpipeline.h"
#include  "renderer/vulkanbuffer.h"

#include  "core/logger.h"
#include <vulkan/vulkan_core.h>

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"
#define ATTRIBUTE_COUNT 2

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
  //-------------------------------------------------------------------------------------------GLOBAL UBO----------------------------
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

  VK_CHECK_VERBOSE(result, "vkCreateDescriptorSetLayout failed while trying to create globalUBO"); 

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
  VK_CHECK_VERBOSE(result, "vkCreateDescriptorPool failed while trying to create globalUBO")

  //-------------------------------------------------------------------------------------------LOCAL UBO----------------------------
  const u32 localSamplerCount = 1;
  VkDescriptorType descriptorTypes[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT] = {
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,    // binding 0 -> uniform Buffer
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  //binding 1 -> diffuse sampler layout
  };

  VkDescriptorSetLayoutBinding bindings[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT] = {0};
  for(u32 i = 0; i < VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT; ++i){
    bindings[i].binding         = i;
    bindings[i].descriptorCount = 1;
    bindings[i].descriptorType  = descriptorTypes[i];
    bindings[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  VkDescriptorSetLayoutCreateInfo localUBOSetLayoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  localUBOSetLayoutCreateInfo.bindingCount  = VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT;
  localUBOSetLayoutCreateInfo.pBindings     = bindings;
  result  = vkCreateDescriptorSetLayout(context->device.logicalDevice,
                                        &localUBOSetLayoutCreateInfo,
                                        context->allocator,
                                        &outShader->localDescriptorSetLayout);
  VK_CHECK_RESULT(result, "vkCreateDecriptorSetLayout failed while creating localObjectDescriptors");

  VkDescriptorPoolSize  localUBOPoolSizes[2];
  // first sectio will be used for unifor, buffers
  localUBOPoolSizes[0].type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  localUBOPoolSizes[0].descriptorCount  = VULKAN_OBJECT_MAX_OBJECT_COUNT;
  // second section will be sued for image Samplers
  localUBOPoolSizes[1].type             = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  localUBOPoolSizes[1].descriptorCount  = localSamplerCount * VULKAN_OBJECT_MAX_OBJECT_COUNT;
  
  VkDescriptorPoolCreateInfo localUBOPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  localUBOPoolCreateInfo.poolSizeCount  = 2;
  localUBOPoolCreateInfo.pPoolSizes     = localUBOPoolSizes;
  localUBOPoolCreateInfo.maxSets        = VULKAN_OBJECT_MAX_OBJECT_COUNT;

  result  = vkCreateDescriptorPool(context->device.logicalDevice,
                                   &localUBOPoolCreateInfo,
                                   context->allocator,
                                   &outShader->localDescriptorPool);
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
  VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_COUNT] = {0};

  //POSITION
  VkFormat formats[ATTRIBUTE_COUNT] = {
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT,
  };

  u64 sizes[ATTRIBUTE_COUNT] = {
    sizeof(Vec3),
    sizeof(Vec2),
  };

  for(u32 i = 0; i < ATTRIBUTE_COUNT; ++i){
    attributeDescriptions[i].binding  = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format   = formats[i];
    attributeDescriptions[i].offset   = offset;
    offset += sizes[i];
  }
  // descriptor set layouts
  const i32 descriptorSetLayoutCount  = 2;
  VkDescriptorSetLayout descriptorSetLayouts[descriptorSetLayoutCount]  = {
    outShader->globalDescriptorSetLayout,
    outShader->localDescriptorSetLayout,
  };

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
                                         ATTRIBUTE_COUNT,
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

  //create Uniform Buffer for global UBO
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

  // create uniform buffer for local objects
  result=vulkanBufferCreate(context,
                            sizeof(LocalUniformObject),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true,
                            &outShader->localUniformBuffer);
  VK_CHECK_RESULT(result, "vulkanBufferCreate failed while trying to create lcoalUniformBuffer for shader");

   KINFO("SUCCESFULLY CRAEATED SHADER OBJECT");
  return result;
}

VkResult
vulkanObjectShaderDestroy(VulkanContext* context, VulkanObjectShader* shader)
{
  TRACEFUNCTION; 
  KDEBUG("context : %p shader : %p",context, shader);
  VkDevice logicalDevice  = context->device.logicalDevice;

  //destroy local descriptor pools
  vkDestroyDescriptorPool(logicalDevice, shader->localDescriptorPool, context->allocator);

  //destroy local descriptor set layout
  vkDestroyDescriptorSetLayout(logicalDevice, shader->localDescriptorSetLayout, context->allocator);

  //destroy uniform buffers
  vulkanBufferDestroy(context, &shader->globalUniformBuffer);
  vulkanBufferDestroy(context, &shader->localUniformBuffer);

  //destroy gpaphics pipeline
  vulkanGraphicsPipelineDestroy(context, &shader->pipeline);

  //destroy global descriptor pool
  vkDestroyDescriptorPool(logicalDevice, shader->globalDescriptorPool, context->allocator);

  //destory descriptor set layouts
  vkDestroyDescriptorSetLayout(logicalDevice, shader->globalDescriptorSetLayout, context->allocator);

  VkResult result = !VK_SUCCESS;
  //destroy shader modules
  for(u32 i  = 0 ; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    vkDestroyShaderModule(logicalDevice, shader->stages[i].handle, context->allocator);
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
vulkanObjectShaderUpdateGlobalState(VulkanContext* context, VulkanObjectShader* shader, f32 deltaTime)
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
vulkanObjectShaderUpdateObject(VulkanContext* context, VulkanObjectShader* shader,  GeometryRenderData data)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;

  u32 imageIndex                = context->imageIndex;
  VkCommandBuffer commandBuffer = context->graphicsCommandBuffers[imageIndex].handle;

  vkCmdPushConstants(commandBuffer, shader->pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &data.model);

  //obtain material data
  VulkanObjectShaderLocalObjectState* objectState = &shader->localObjectStates[data.objectId];
  VkDescriptorSet localObjectDescriptorSet        = objectState->descriptorSets[imageIndex];


  // TODO update this part later
  VkWriteDescriptorSet descriptorWrites[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT];
  kzeroMemory(&descriptorWrites, sizeof(VkWriteDescriptorSet) * VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT);
  u32 descriptorCount = 0;
  u32 descriptorIndex = 0;

  // descriptor 0 ==> uniform buffer
  u32 range = sizeof(LocalUniformObject);
  u32 offset= sizeof(LocalUniformObject) * data.objectId;
  LocalUniformObject  LBO;

  // test slap
  static f32 accumulator  = 0.0f;
  accumulator += context->frameDeltaTime;
  f32 s = (ksin(accumulator) + 1.0f)/2.0f; // scale the range from -1 -> 1 to i -> 0
  LBO.diffuseColor  = vec4Create(s, s, s, 1.0);


  // bind the descriptor set to be updated, or in case the shader changed.
  vkCmdBindDescriptorSets(commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.pipelineLayout,
                          1,
                          1,
                          &localObjectDescriptorSet,
                          0,
                          0);

  vulkanBufferLoadData(context,
                       &shader->localUniformBuffer,
                       offset,
                       range,
                       0,
                       &LBO);

  // only do this if the descriptor has not been updated
  if(objectState->descriptorStates[descriptorIndex].generations[imageIndex] == INVALID_ID){
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shader->localUniformBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range  = range;

    VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptor.dstSet = localObjectDescriptorSet;
    descriptor.dstBinding = descriptorIndex;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor.descriptorCount  = 1;
    descriptor.pBufferInfo  = &bufferInfo;

    descriptorWrites[descriptorCount] = descriptor;
    descriptorCount++;

    // udpate the frame generation . in this case it is only needed once since this is a buffer
    objectState->descriptorStates[descriptorCount].generations[imageIndex]  = 1;
  }

  descriptorIndex++;

  // SAMPLERS
  const u32 samplerCount  = 1;
  VkDescriptorImageInfo imageInfos[1];

  for( u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex){
    Texture* tex  = data.textures[samplerIndex];
    u32* descriptorGeneration = &objectState->descriptorStates[descriptorIndex].generations[imageIndex];

    // check if the texture needs udpating first (in case of hot reloading , gpu eneds to be told the texture has been updated)
    if(tex && (*descriptorGeneration != tex->generation || *descriptorGeneration == INVALID_ID)){
      VulkanTextureData*  internalData  = (VulkanTextureData*)tex->internalData;

      // assign view and sampler
      imageInfos[samplerIndex].imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfos[samplerIndex].imageView    = internalData->image.view;
      imageInfos[samplerIndex].sampler      = internalData->sampler;

      VkWriteDescriptorSet  descriptor      = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      descriptor.dstSet                     = localObjectDescriptorSet;
      descriptor.dstBinding                 = descriptorIndex;
      descriptor.descriptorType             = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor.descriptorCount            = 1;
      descriptor.pImageInfo                 = &imageInfos[samplerIndex];

      descriptorWrites[descriptorCount]     = descriptor;
      ++descriptorCount;

      //sync frame generation if not using a default texture
      if(tex->generation != INVALID_ID){
        *descriptorGeneration = tex->generation;
      }
      ++descriptorIndex;
    }
  }

  if(descriptorCount > 0){
    vkUpdateDescriptorSets(context->device.logicalDevice, descriptorCount, descriptorWrites, 0, 0);
  }

  return VK_SUCCESS;
}

VkResult
vulkanObjectShaderAcquireResources(VulkanContext* context, VulkanObjectShader* shader, u32* outObjectId)
{
  TRACEFUNCTION;
  // TODO free list
  *outObjectId = shader->localUniformBufferIndex;
  shader->localUniformBufferIndex++;

  u32 objectId = *outObjectId;
  VulkanObjectShaderLocalObjectState* objectState = &shader->localObjectStates[objectId];
  for(u32 i = 0; i < VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT; ++i){
    for(u32 j = 0; j < 3; ++j){
      objectState->descriptorStates[i].generations[j] = INVALID_ID;
    }
  }

  VkDescriptorSetLayout layouts[3] = {
    shader->localDescriptorSetLayout,
    shader->localDescriptorSetLayout,
    shader->localDescriptorSetLayout,
  };

  // allocate descriptor sets
  VkDescriptorSetAllocateInfo allocInfo  = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocInfo.descriptorPool  = shader->localDescriptorPool;
  allocInfo.descriptorSetCount  = 3;
  allocInfo.pSetLayouts = layouts;

  VkResult result = vkAllocateDescriptorSets(context->device.logicalDevice, &allocInfo, objectState->descriptorSets);
  VK_CHECK_RESULT(result, "failed to allocate descriptor sets for local objects in shader");

  KTRACE("SUCCESFULLY ACQUIRES SHADER RESOURCES");
  return VK_SUCCESS;
}

void
vulkanObjectShaderReleaseResoruces  (VulkanContext* context, VulkanObjectShader* shader, u32 objectId)
{

  VulkanObjectShaderLocalObjectState* objectState = &shader->localObjectStates[objectId];

  const u32 descriptorSetCount = 3;
  VkResult result =  vkFreeDescriptorSets(context->device.logicalDevice,
                                          shader->localDescriptorPool,
                                          descriptorSetCount,
                                          objectState->descriptorSets);

  if(result != VK_SUCCESS){
    UERROR("error freeing object shader local object sets");
  }

  for(u32 i = 0; i < VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT; ++i){
    for(u32 j = 0; j < 3; ++j){
      objectState->descriptorStates[i].generations[j] = INVALID_ID;
    }
  }
  // TODO add the object id to free list
}


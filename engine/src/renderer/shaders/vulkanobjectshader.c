#include  "vulkanobjectshader.h"
#include "math/kmath.h"
#include "math/mathtypes.h"
#include  "shaderutils.h"

#include  "renderer/renderertypes.h"
#include  "renderer/rendererutils.h"
#include  "renderer/vulkanpipeline.h"
#include  "renderer/vulkanbuffer.h"

#include  "core/logger.h"
#include <vulkan/vulkan_core.h>

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"
/*
1.  Create a VkBuffer of object Data
2.  Allocate memory for the buffer and bind it (VkMemoryAllocateInfo -> VkDeviceMemory)(vkAllocateMemory -> vkBindBufferMemory)
3.  Update the Buffer with data (vkMapMemory -> *data(any), memcpy -> data => objectData(where we want), vkUnmapMemory);
4.  Create a descriptor set Layout |(VkDescriptorSetLayoutBinding | -> vkDescriptorSetLayoutCreateInfo -> vkDescriptorSetLayout)
                                   | CORRESPONDS TO THE BINDING   |
5.  Create a descriptorPool(vkDescriptorPoolSize ->vkDescriptorPoolCreateInfo -> vkDescriptorPool);
6.  Allocate a Descriptor Set(vkDescriptorAllocateInfo -> vkDescriptorSet);
7.  Update the Descriptor Set with the buffer you created(vkDescriptorBufferInfo -> vkWriteDescriptorSet)
8.  Bind the Descriptor Set in CommandBuffer (vkCmdBindDescriptorSets);
*/
// Summary:
// 1. Create a Vulkan buffer and allocate memory for it.
// 2. Load data into the buffer.
// 3. Define and create a descriptor set layout for shaders.
// 4. Create a descriptor pool for managing descriptor sets.
// 5. Allocate descriptor sets from the pool.
// 6. Update descriptor sets with buffer or image bindings.
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




  KTRACE("--------------------------------------------------------------------------GLOBAL OBJECT BINDING----------------------");
  //UBO-------------> UNIFORM BUFFER OBJECT
  //Global Descriptors
  VkDescriptorSetLayoutBinding globalUBOSetLayoutBinding  = {0};
  globalUBOSetLayoutBinding.binding            = 0; //matches the binding in glsl or location hlsl
  globalUBOSetLayoutBinding.descriptorCount    = 1;
  globalUBOSetLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  globalUBOSetLayoutBinding.pImmutableSamplers = 0;
  globalUBOSetLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo globalObjectDescriptorSetLayout= {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  globalObjectDescriptorSetLayout.bindingCount    = 1;
  globalObjectDescriptorSetLayout.pBindings       = &globalUBOSetLayoutBinding;
  result  = vkCreateDescriptorSetLayout(context->device.logicalDevice,&globalObjectDescriptorSetLayout,NULL,&outShader->globalDescriptorSetLayout);
  VK_CHECK_VERBOSE(result, "vkCreateDescriptorSetLayout failed in vulkanShaderOjbectCreate while trying to create globalDescriptorSetLayout"); 

  // Used for global items such as view/projection matrix
  VkDescriptorPoolSize  globalObjectPoolSize      = {0};
  globalObjectPoolSize.type                       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  globalObjectPoolSize.descriptorCount            = context->swapchain.imageCount;

  VkDescriptorPoolCreateInfo  globalObjectPoolCreateInfo         = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  globalObjectPoolCreateInfo.poolSizeCount        = 1;
  globalObjectPoolCreateInfo.pPoolSizes           = &globalObjectPoolSize;
  globalObjectPoolCreateInfo.maxSets              = context->swapchain.imageCount;
  result  = vkCreateDescriptorPool(context->device.logicalDevice,&globalObjectPoolCreateInfo,NULL,&outShader->globalDescriptorPool);
  VK_CHECK_VERBOSE(result, "vkCreateDescriptorPool failed in vulkanObjectShaderCreate while trying to create globalUBO")

  KTRACE("----------------------------------------------------------------------------LOCAL OBJECT BIINDING----------------------");
  u32 localSamplerCount = 1;
  VkDescriptorType descriptorTypes[OBJECT_SHADER_DESCRIPTOR_COUNT] = {
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // binding 0 - uniform BUffer
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // diffuseSamplerLayout
  };
  VkDescriptorSetLayoutBinding bindings[OBJECT_SHADER_DESCRIPTOR_COUNT] = {0};
  for(u32 i = 0; i <  OBJECT_SHADER_DESCRIPTOR_COUNT; ++i){
    bindings[i].binding = i;
    bindings[i].descriptorCount = 1;
    bindings[i].descriptorType  = descriptorTypes[i];
    bindings[i].stageFlags  = VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  VkDescriptorSetLayoutCreateInfo localDescriptorSetLayout  = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  localDescriptorSetLayout.bindingCount = OBJECT_SHADER_DESCRIPTOR_COUNT;
  localDescriptorSetLayout.pBindings    = bindings;
  result  = vkCreateDescriptorSetLayout(context->device.logicalDevice, &localDescriptorSetLayout,NULL, &outShader->localDescriptorSetLayout);
  VK_CHECK_VERBOSE(result, "vkCreateDescriptorSetLayout failed in vulkanShaderOjbectCreate while trying to create localDescriptoSetLayout"); 

  VkDescriptorPoolSize  localDescriptorPoolSizes[2];
  localDescriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  localDescriptorPoolSizes[0].descriptorCount  = MAX_LOCAL_OBJECT_COUNT;
  localDescriptorPoolSizes[1].type    =VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  localDescriptorPoolSizes[1].descriptorCount = localSamplerCount * MAX_LOCAL_OBJECT_COUNT;

  VkDescriptorPoolCreateInfo localDescriptorPoolCreateInfo  = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  localDescriptorPoolCreateInfo.pPoolSizes  = localDescriptorPoolSizes;
  localDescriptorPoolCreateInfo.poolSizeCount = 2;
  localDescriptorPoolCreateInfo.maxSets = MAX_LOCAL_OBJECT_COUNT;
  result = vkCreateDescriptorPool(context->device.logicalDevice, &localDescriptorPoolCreateInfo, NULL, &outShader->localDescriptorPool);


  KTRACE("----------------------GRAPHICS PIPELINE----------------------");
  //viewport
  VkViewport viewport;
  viewport.x  = 0.0f;
  viewport.y  = (f32)context->frameBufferWidth;
  viewport.width  = (f32)context->frameBufferWidth;
  viewport.height = -(f32)context->frameBufferHeight;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // SCISSOR
  VkRect2D scissor;
  scissor.offset.x  = scissor.offset.y  = 0;
  scissor.extent.width  = context->frameBufferWidth;
  scissor.extent.height = context->frameBufferHeight;

  //  ATTRIBUTES
  // Describes an attribute of an vertex in the vertex buffer
  // Used to specify the layout of the vertex data such as format and location
  u32 offset  = 0;
#define ATTRIBUTE_COUNT 2
  VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_COUNT] = {0};
  //POSITION, TEXCOORD
  VkFormat formats[ATTRIBUTE_COUNT] = {
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT,
  };
  u64 sizes[ATTRIBUTE_COUNT] = {
    sizeof(Vec3),
    sizeof(Vec2),
  };
  for(u32 i = 0; i < ATTRIBUTE_COUNT; ++i){
    attributeDescriptions[i].binding  = 0;          // binding number of the vertex buffer that contains this attribute
    attributeDescriptions[i].location = i;          // location in the vertex  shader that this attribute correspoonds to
    attributeDescriptions[i].format   = formats[i]; // the format of the data
    attributeDescriptions[i].offset   = offset;     // the byte offset of the attribute in the vertex data
    offset += sizes[i];
  }

  // DESCRIPTOR SET LAYOUTS
  const i32 descriptorSetLayoutCount  = 2;
  VkDescriptorSetLayout descriptorSetLayouts[2]  = {
    outShader->globalDescriptorSetLayout,
    outShader->localDescriptorSetLayout,
  };

  // STAGES
  // should match the numbers of shader->stages
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

  KTRACE("------------------------------CREATING GLOBAL UNIFORM BUFFER----------------------");
  result=vulkanBufferCreate(context,
                            sizeof(GlobalUniformObject),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true,
                            &outShader->globalUniformBuffer);
  VK_CHECK_RESULT(result,"vulkanBufferCreate failed in vulkanObjectShaderCreate while tryin to create globalUBO");

  KTRACE("------------------------------CREATING LOCAL  UNIFORM BUFFER----------------------");
  result=vulkanBufferCreate(context,
                            sizeof(LocalUniformObject),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true,
                            &outShader->localUniformBuffer);
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

  // LOCAL  BUFFER AND OBEJCTS
  vulkanBufferDestroy(context, &shader->localUniformBuffer);
  vkDestroyDescriptorPool(context->device.logicalDevice, shader->localDescriptorPool, context->allocator);
  vkDestroyDescriptorSetLayout(context->device.logicalDevice, shader->localDescriptorSetLayout, context->allocator);

  // GLOBAL BUFFER AND OBEJCTS
  vulkanBufferDestroy(context, &shader->globalUniformBuffer);
  vkDestroyDescriptorPool(context->device.logicalDevice, shader->globalDescriptorPool, context->allocator);
  vkDestroyDescriptorSetLayout(context->device.logicalDevice, shader->globalDescriptorSetLayout, context->allocator);

  vulkanGraphicsPipelineDestroy(context, &shader->pipeline);
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
vulkanObjectShaderUpdateGlobalState(VulkanContext* context, VulkanObjectShader* shader, f32 deltaTime)
{
  TRACEFUNCTION;

  UINFO("UPDATE GLOBAL BUFFERS");
  u32 imageIndex                   = context->imageIndex;
  VkCommandBuffer commandBuffer    = context->graphicsCommandBuffers[imageIndex].handle;
  VkDescriptorSet globalDescriptor = shader->globalDescriptorSets[imageIndex];

  // this call which send tha data to the GPU
  // bind the global descriptor set to updated
  vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,shader->pipeline.pipelineLayout,0,1,&globalDescriptor,0,0);

  //configure the descriptors for the given index
  u32 range = sizeof(GlobalUniformObject);
  u64 offset= 0;

  //copy data to buffer
  VkResult result   = vulkanBufferLoadData(context, &shader->globalUniformBuffer, offset, range, 0, &shader->globalUBO);
  VK_CHECK_RESULT(result, "vulkanBufferLoadData failed in vulkanObjectShaderUpdateGlobalState while trying to load data for globalUBO");

  //update descriptor sets
  VkDescriptorBufferInfo  bufferInfo  = {0};
  bufferInfo.buffer = shader->globalUniformBuffer.handle;   // bkBuffer with the data that needs to be passed to the shader
  bufferInfo.offset = offset;
  bufferInfo.range  = range;

  VkWriteDescriptorSet descriptorWrite  = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptorWrite.dstSet                = shader->globalDescriptorSets[imageIndex];
  descriptorWrite.dstBinding            = 0;  //matches the binding or location in glsl or hlsl
  descriptorWrite.dstArrayElement       = 0;
  descriptorWrite.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount       = 1;
  descriptorWrite.pBufferInfo           = &bufferInfo;

  vkUpdateDescriptorSets(context->device.logicalDevice, 1, &descriptorWrite, 0, 0);
  KINFO("SUCCESFULLY UPDATED SHADER OBJECT GLOBAL STATE");
  return result;
}


//---------------------------------------------------------UPDATE OBJECT--------------------------------------------------


VkResult
vulkanObjectShaderUpdateLocalState(VulkanContext* context, VulkanObjectShader* shader,  GeomteryRenderData data)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;

  UINFO("UPDATE LOCAL BUFFERS");
  u32 imageIndex                = context->imageIndex;
  VkCommandBuffer commandBuffer = context->graphicsCommandBuffers[imageIndex].handle;

  vkCmdPushConstants(commandBuffer, shader->pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &data.model); // 128 bytes limit

  //OBTAIN MATERIAL DATA
  LocalObjectState* localObjectState = &shader->localObjectStates[data.objectId];
  VkDescriptorSet localDescriptorSet  = localObjectState->descriptorSets[imageIndex];

  //UPDATE
  VkWriteDescriptorSet descriptorWrites[OBJECT_SHADER_DESCRIPTOR_COUNT] = {0};
  u32 descriptorCount = 0;
  u32 descriptorIndex = 0;

  // descriptor 0 (UNIFORM BUFFER)
  u32 range = sizeof(LocalUniformObject);
  u64 offset= sizeof(LocalUniformObject) * data.objectId;

  LocalUniformObject obj = {0};
  static f32 accumaltor = 0.0f;
  accumaltor  += context->deltaTime;
  f32 s = (ksin(accumaltor) + 1.0f)/2.0f;
  obj.diffuseColor  = vec4Create(s, s, s, 1.0f);

  vulkanBufferLoadData(context, &shader->localUniformBuffer, offset, range, 0, &obj);
  UINFO("UPDATING UNIFORM ");
  if(localObjectState->descriptorStates[descriptorIndex].generations[imageIndex] == INVALID_ID){
    VkDescriptorBufferInfo bufferInfo = {0};
    bufferInfo.buffer = shader->localUniformBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range  = range;

    VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptor.dstSet           = localDescriptorSet;
    descriptor.dstBinding       = descriptorIndex;
    descriptor.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor.descriptorCount  = 1;
    descriptor.pBufferInfo      = &bufferInfo;

    descriptorWrites[descriptorCount] = descriptor;
    descriptorCount++;

    localObjectState->descriptorStates[descriptorIndex].generations[imageIndex] = 1;
  }
  descriptorIndex++;

  UINFO("UPDATING SAMPLER");

  const u32 samplerCount  = 1;
  VkDescriptorImageInfo imageInfos[1];

  for(u32 i =0 ;i < samplerCount; ++i){
    Texture* tex  = data.textures[i];
    u32* descriptorGeneration = &localObjectState->descriptorStates[descriptorIndex].generations[imageIndex];

    if(tex && (*descriptorGeneration != tex->generation || *descriptorGeneration == INVALID_ID)){
      VulkanTextureData* internalData = (VulkanTextureData*)tex->internalData;


      imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfos[i].imageView   = internalData->image.view;
      imageInfos[i].sampler     = internalData->sampler;

      VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      descriptor.dstSet         = localDescriptorSet;
      descriptor.dstBinding     = descriptorIndex;
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor.descriptorCount= 1;
      descriptor.pImageInfo     = &imageInfos[i];

      descriptorWrites[descriptorCount] = descriptor;
      descriptorCount++;

      if(tex->generation != INVALID_ID){
        *descriptorGeneration = tex->generation;
      }
      descriptorIndex++;
    }
  }

  if(descriptorCount > 0){
    vkUpdateDescriptorSets(context->device.logicalDevice, descriptorCount, descriptorWrites, 0, 0);
  }

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipelineLayout, 1, 1, &localDescriptorSet, 0,0);
  return VK_SUCCESS;
}


bool      vulkanObjectShaderAcquireResources  (VulkanContext* context, VulkanObjectShader* shader, u32* outObjectId){
  *outObjectId  = shader->localUniformBufferIndex;
  shader->localUniformBufferIndex++;

  u32 objectId = *outObjectId;

  LocalObjectState* objectState = &shader->localObjectStates[objectId];

  for(u32 i = 0; i < OBJECT_SHADER_DESCRIPTOR_COUNT; ++i){
    for( u32 j =0; j < 3 ; ++j){
      objectState->descriptorStates[i].generations[j] = INVALID_ID;
    }
  }

  VkDescriptorSetLayout layouts[3] = {
    shader->localDescriptorSetLayout,
    shader->localDescriptorSetLayout,
    shader->localDescriptorSetLayout,
  };

  VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocInfo.descriptorPool  = shader->localDescriptorPool;
  allocInfo.descriptorSetCount  = 3;
  allocInfo.pSetLayouts = layouts;

  VkResult result = vkAllocateDescriptorSets(context->device.logicalDevice, &allocInfo, objectState->descriptorSets);
  VK_CHECK_BOOL(result, "ERROR ALLOCATIING DESCRIPTOR SETS IN SHADER");

  return true;
}

void      vulkanObjectShaderReleaseResources  (VulkanContext* context, VulkanObjectShader* shader, u32  objectId){
  LocalObjectState* objectState = &shader->localObjectStates[objectId];

  const u32 count = 3;

  VkResult result = vkFreeDescriptorSets(context->device.logicalDevice, shader->localDescriptorPool, count, objectState->descriptorSets);

  if(result != VK_SUCCESS){
    UERROR("ERROR FREEING SHADER DESCRIPTOR SETS");
    return;
  }
}

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
#define ATTRIBUTE_COUNT 2
#define OBJECT_SHADER_SAMPLER_COUNT 1

/*
1. Create a Vulkan buffer and allocate memory for it.
2. Load data into the buffer.
3. Define and create a descriptor set layout for shaders.
4. Create a descriptor pool for managing descriptor sets.
5. Allocate descriptor sets from the pool.
6. Update descriptor sets with buffer or image bindings.
*/

VkResult
ObjectShaderCreate(VulkanContext* ctx, VulkanObjectShader* outShader)
{
  TRACEFUNCTION;
  MEMZERO(outShader);
  VkResult  result  = !VK_SUCCESS;

  KINFO("CREATING SHADER MODULES");
  {
    char stageTypesStrs [OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){
      result = createShaderModule(ctx, BUILTIN_SHADER_NAME_OBJECT, stageTypesStrs[i], stageTypes[i], i, outShader->stages);
      VK_CHECK_RESULT(result,"UNABLE TO CREATE SHADER MODULE %s FOR %s | %d : %s",
                      stageTypesStrs[i],
                      BUILTIN_SHADER_NAME_OBJECT,
                      result,
                      vulkanResultToString(result));
      KDEBUG("CREATED SHADER MODULE %s FOR %s",stageTypesStrs[i],BUILTIN_SHADER_NAME_OBJECT);
    }
  }
  KINFO("CREATING GLOBAL OBJECT BINDING");
  {
    VkDescriptorType descriptorTypes[OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT] = {
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };
    u32 descriptorCounts[OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT] = {
      ctx->swapchain.imageCount
    };

    VkDescriptorSetLayoutBinding  layoutBindings[OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT]  = {0};
    for(u32 i = 0; i < OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT; ++i){
      layoutBindings[i].binding              = i;
      layoutBindings[i].descriptorCount      = 1;
      layoutBindings[i].descriptorType       = descriptorTypes[i];
      layoutBindings[i].stageFlags           = VK_SHADER_STAGE_VERTEX_BIT;
      layoutBindings[i].pImmutableSamplers   = NULL;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo= {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount       = OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT;
    layoutInfo.pBindings          = layoutBindings;

    result  = vkCreateDescriptorSetLayout(ctx->device.logicalDevice,&layoutInfo,ctx->allocator,&outShader->globalDescriptorSetLayout);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE GLOBAL DESCRIPTOR LAYOUT"); 

    // Used for global items such as view/projection matrix
    VkDescriptorPoolSize          poolSizes[OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT] = {0};
    for(u32 i = 0; i < OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT; ++i){
      poolSizes[i].type           = descriptorTypes[i];
      poolSizes[i].descriptorCount= descriptorCounts[i];
    }

    VkDescriptorPoolCreateInfo    poolInfo  = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount        = OBJECT_SHADER_GLOBAL_DESCRIPTOR_COUNT;
    poolInfo.pPoolSizes           = poolSizes;
    poolInfo.maxSets              = ctx->swapchain.imageCount;
    poolInfo.flags                = 0;

    result  = vkCreateDescriptorPool(ctx->device.logicalDevice,&poolInfo,ctx->allocator,&outShader->globalDescriptorPool);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE GLOBAL DESCRIPTOR POOL");
  }
  KINFO("CREATING LOCAL OBJECT BINDING");
  {
    VkDescriptorType descriptorTypes[OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = {
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };
    u32 descriptorCounts[OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = {
      MAX_LOCAL_OBJECT_COUNT,
      OBJECT_SHADER_SAMPLER_COUNT * MAX_LOCAL_OBJECT_COUNT
    };

    VkDescriptorSetLayoutBinding layoutBindings[OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = {0};
    for(u32 i = 0; i < OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT; ++i){
      layoutBindings[i].binding             = i;
      layoutBindings[i].descriptorCount     = 1;
      layoutBindings[i].descriptorType      = descriptorTypes[i];
      layoutBindings[i].stageFlags          = VK_SHADER_STAGE_FRAGMENT_BIT;
      layoutBindings[i].pImmutableSamplers  = NULL;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount       = OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT;
    layoutInfo.pBindings          = layoutBindings;
    result  = vkCreateDescriptorSetLayout(ctx->device.logicalDevice,&layoutInfo,ctx->allocator,&outShader->localDescriptorSetLayout);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE LOCAL  DESCRIPTOR LAYOUT"); 

    VkDescriptorPoolSize          poolSizes[OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = {0};
    for(u32 i = 0; i < OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT; ++i){
      poolSizes[i].type           = descriptorTypes[i];
      poolSizes[i].descriptorCount= descriptorCounts[i];
    }

    VkDescriptorPoolCreateInfo    poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount        = OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT;
    poolInfo.pPoolSizes           = poolSizes;
    poolInfo.maxSets              = MAX_LOCAL_OBJECT_COUNT;
    poolInfo.flags                = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    result  = vkCreateDescriptorPool(ctx->device.logicalDevice,&poolInfo,ctx->allocator,&outShader->localDescriptorPool);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE LOCAL  DESCRIPTOR POOL");
  }
  KINFO("CREATING GRAPHICS PIPELINE");
  {

    VkViewport viewport;
    viewport.x        = 0.0f;
    viewport.y        = (f32)ctx->frameBufferWidth;
    viewport.width    = (f32)ctx->frameBufferWidth;
    viewport.height   =-(f32)ctx->frameBufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x  = scissor.offset.y  = 0;
    scissor.extent.width  = ctx->frameBufferWidth;
    scissor.extent.height = ctx->frameBufferHeight;

    // Describe an ATTRIBUTE of an vertex in the vertex buffer Used to specify the layout of the vertex data such as format and location
    u32 offset  = 0;
    VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_COUNT] = {0};
    VkFormat formats[ATTRIBUTE_COUNT] = {
      VK_FORMAT_R32G32B32_SFLOAT, // LOCATION
      VK_FORMAT_R32G32_SFLOAT,    // TEXCOORD
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
    VkPipelineShaderStageCreateInfo StageCreateInfos[OBJECT_SHADER_STAGE_COUNT]  = {0};
    for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i){
      StageCreateInfos[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
      StageCreateInfos[i]       = outShader->stages[i].shaderStageCreateInfo;
    }
    result  = vulkanGraphicsPipelineCreate(ctx,&ctx->mainRenderPass,ATTRIBUTE_COUNT,attributeDescriptions,
                                           descriptorSetLayoutCount,descriptorSetLayouts,
                                           OBJECT_SHADER_STAGE_COUNT,StageCreateInfos,
                                           viewport,scissor,false,&outShader->pipeline);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE GRAPHICS PIPELINE FOR OBJECT SHADER");
  } 

  KINFO("CREATING OBJECT SHADER UNIFORM BUFFERS");
  {
    result=vulkanBufferCreate(ctx,
                              sizeof(GlobalUniformObject),
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              true,&outShader->globalUniformBuffer);
    VK_CHECK_RESULT(result, "FAILED TO CREATE GLOBAL UNIFORM OBJECT BUFFER");

    result=vulkanBufferCreate(ctx,
                              sizeof(LocalUniformObject),
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              true,&outShader->localUniformBuffer);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE LOCAL UNIFORM OBJECT BUFFER");
  }
  KINFO("ALLOCATIING OBJECT SHADER DESCRIPTOR SETS");
  {
    // allocate global descriptor sets
    VkDescriptorSetLayout globalUBOSetLayouts[3]  = {
      outShader->globalDescriptorSetLayout,
      outShader->globalDescriptorSetLayout,
      outShader->globalDescriptorSetLayout,};

    VkDescriptorSetAllocateInfo globalUBOAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    globalUBOAllocateInfo.descriptorPool      = outShader->globalDescriptorPool;
    globalUBOAllocateInfo.descriptorSetCount  = 3;
    globalUBOAllocateInfo.pSetLayouts         = globalUBOSetLayouts;

    result  = vkAllocateDescriptorSets(ctx->device.logicalDevice, &globalUBOAllocateInfo, outShader->globalDescriptorSets);
    VK_CHECK_VERBOSE(result, "FAILED TO CREATE OBJECT SHADER DESCRIPTOR SETS");
  }
  KINFO("SUCCESFULLY CRAEATED OBEJCT SHADER");
  return result;
}


VkResult
ObjectShaderUse(VulkanContext* ctx, VulkanObjectShader* shader)
{
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
  u32 imageIndex  = ctx->imageIndex;
  result = vulkanGraphicsPipelineBind(&ctx->graphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
  VK_CHECK_RESULT(result, "vulkanGraphicsPipelineBind failed in vulkanObjectShaderUse");

  KINFO("SUCCESFULLY USED OBJECT SHADER ");
  return result;
}

VkResult
ObjectShaderUpdateGlobalState(VulkanContext* ctx, VulkanObjectShader* shader, f32 deltaTime)
{
  TRACEFUNCTION;

  u32 imageIndex                   = ctx->imageIndex;
  VkCommandBuffer commandBuffer    = ctx->graphicsCommandBuffers[imageIndex].handle;
  VkDescriptorSet globalDescriptor = shader->globalDescriptorSets[imageIndex];

  // this call which send tha data to the GPU
  // bind the global descriptor set to updated
  vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,shader->pipeline.pipelineLayout,0,1,&globalDescriptor,0,0);

  //configure the descriptors for the given index
  u32 range = sizeof(GlobalUniformObject);
  u64 offset= 0;

  //copy data to buffer
  VkResult result   = vulkanBufferLoadData(ctx, &shader->globalUniformBuffer, offset, range, 0, &shader->globalUBO);
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

  vkUpdateDescriptorSets(ctx->device.logicalDevice, 1, &descriptorWrite, 0, 0);

  KINFO("OBJECT SHADER GLOBAL STATES UPDATED !");
  return result;
}

//---------------------------------------------------------UPDATE OBJECT--------------------------------------------------

VkResult
ObjectShaderUpdateLocalState(VulkanContext* ctx, VulkanObjectShader* shader,  GeomteryRenderData data)
{
  TRACEFUNCTION;

  VkResult result = !VK_SUCCESS;
  u32 imageIndex                = ctx->imageIndex;
  VkCommandBuffer commandBuffer = ctx->graphicsCommandBuffers[imageIndex].handle;

  vkCmdPushConstants(commandBuffer, shader->pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &data.model); // 128 bytes limit

  LocalObjectState* localObjectState = &shader->localObjectStates[data.objectId];
  VkDescriptorSet localDescriptorSet = localObjectState->descriptorSets[imageIndex];

  VkWriteDescriptorSet descriptorWrites[OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT] = {0};
  u32 descriptorCount = 0;
  u32 descriptorIndex = 0;

  u32 range = sizeof(LocalUniformObject);
  u64 offset= sizeof(LocalUniformObject) * data.objectId;

  LocalUniformObject obj = {0};
  static f32 accumaltor = 0.0f;
  accumaltor  += ctx->deltaTime;
  f32 s = (ksin(accumaltor) + 1.0f)/2.0f;
  obj.diffuseColor  = vec4Create(s, s, s, 1.0f);

  vulkanBufferLoadData(ctx, &shader->localUniformBuffer, offset, range, 0, &obj);

  KTRACE("UPDATING OBJECT SHADER UNIFORMS");
  {
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
  }
  KTRACE("UPDATING OBJECT SHADER SAMPLERS");
  {
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
  }

  if(descriptorCount > 0) vkUpdateDescriptorSets(ctx->device.logicalDevice, descriptorCount, descriptorWrites, 0, 0);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipelineLayout, 1, 1, &localDescriptorSet, 0,0);

  KINFO("OBJECT SHADER LOCAL STATES UPDATED !");
  return VK_SUCCESS;
}

bool
ObjectShaderAcquireResources  (VulkanContext* ctx, VulkanObjectShader* shader, u32* outObjectId)
{
  TRACEFUNCTION;
  *outObjectId  = shader->localUniformBufferIndex;
  shader->localUniformBufferIndex++;

  u32 objectId = *outObjectId;

  LocalObjectState* objectState = &shader->localObjectStates[objectId];

  for(u32 i = 0; i < OBJECT_SHADER_LOCAL_DESCRIPTOR_COUNT; ++i){
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

  VkResult result = vkAllocateDescriptorSets(ctx->device.logicalDevice, &allocInfo, objectState->descriptorSets);
  VK_CHECK_BOOL(result, "ERROR ALLOCATIING DESCRIPTOR SETS IN SHADER");
  KINFO("SUCCESFULLY ACQUIRED OBJECT SHADER RESOURCES");
  return true;
}

void      
ObjectShaderReleaseResources  (VulkanContext* ctx, VulkanObjectShader* shader, u32  objectId){
  TRACEFUNCTION;
  LocalObjectState* objectState = &shader->localObjectStates[objectId];

  const u32 count = 3;

  VkResult result = vkFreeDescriptorSets(ctx->device.logicalDevice, shader->localDescriptorPool, count, objectState->descriptorSets);

  if(result != VK_SUCCESS){
    UERROR("ERROR FREEING SHADER DESCRIPTOR SETS");
    return;
  }
  UINFO("FREED OBJECT SHADER DESCRIPTOR SETS");
}

VkResult
ObjectShaderDestroy(VulkanContext* ctx, VulkanObjectShader* shader)
{
  TRACEFUNCTION; 

  vulkanBufferDestroy(ctx, &shader->localUniformBuffer);
  vkDestroyDescriptorPool(ctx->device.logicalDevice, shader->localDescriptorPool, ctx->allocator);
  vkDestroyDescriptorSetLayout(ctx->device.logicalDevice, shader->localDescriptorSetLayout, ctx->allocator);

  vulkanBufferDestroy(ctx, &shader->globalUniformBuffer);
  vkDestroyDescriptorPool(ctx->device.logicalDevice, shader->globalDescriptorPool, ctx->allocator);
  vkDestroyDescriptorSetLayout(ctx->device.logicalDevice, shader->globalDescriptorSetLayout, ctx->allocator);

  vulkanGraphicsPipelineDestroy(ctx, &shader->pipeline);
  VkResult result = !VK_SUCCESS;
  //destroy shader modules
  for(u32 i  = 0 ; i < OBJECT_SHADER_STAGE_COUNT; ++i){
    vkDestroyShaderModule(ctx->device.logicalDevice, shader->stages[i].handle, ctx->allocator);
    shader->stages[i].handle  = NULL;
  }
  KINFO("SUCCESFULLY DESTROYED OBJECT SHADER");
  return result;
}

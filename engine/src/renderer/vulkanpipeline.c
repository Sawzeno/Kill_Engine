#include  "vulkanpipeline.h"
#include  "renderer/rendererutils.h"
#include  "math/mathtypes.h"

#include  "core/logger.h"

VkResult  vulkanGraphicsPipelineCreate(VulkanContext* context,
                                       VulkanRenderPass* renderpass,
                                       u32 attributeCount,
                                       VkVertexInputAttributeDescription* attributes,
                                       u32 descriptorSetLayoutCount,
                                       VkDescriptorSetLayout* descriptorSetLayouts,
                                       u32 stageCount,
                                       VkPipelineShaderStageCreateInfo* stages,
                                       VkViewport viewport,
                                       VkRect2D scissor,
                                       bool isWireframe,
                                       VulkanPipeline* outPipeline){
  TRACEFUNCTION;
  VkResult result = !VK_SUCCESS;
KDEBUG("context : %p renderpass : %p attributeCount : %"PRIu32" attributes : %p descriptorSetLayoutCount : %"PRIu32" descriptorSetLayouts : %p stageCount : %"PRIu32" stages : %p viewport : %p scissor : %p isWireframe %p outPipeline %p",context, renderpass, attributeCount, attributes, descriptorSetLayoutCount, descriptorSetLayouts, stageCount, stages, viewport, scissor, isWireframe, outPipeline);
  //VIEWPORT STATE
  VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO}; 
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports    = &viewport;
  viewportStateCreateInfo.scissorCount  = 1;
  viewportStateCreateInfo.pScissors     = &scissor;

  //RASTERIZER STATE
  VkPipelineRasterizationStateCreateInfo  rasterizationStateCreateInfo  = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizationStateCreateInfo.depthClampEnable         = VK_FALSE;
  rasterizationStateCreateInfo.rasterizerDiscardEnable  = VK_FALSE;
  rasterizationStateCreateInfo.polygonMode              = isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  rasterizationStateCreateInfo.lineWidth                = 1.0f;
  rasterizationStateCreateInfo.cullMode                 = VK_CULL_MODE_BACK_BIT;
  rasterizationStateCreateInfo.frontFace                = VK_FRONT_FACE_CLOCKWISE;
  rasterizationStateCreateInfo.depthBiasEnable          = VK_FALSE;
  rasterizationStateCreateInfo.depthBiasConstantFactor  = 0.0f;
  rasterizationStateCreateInfo.depthBiasClamp           = 0.0f;
  rasterizationStateCreateInfo.depthBiasSlopeFactor     = 0.0f;

  //MULTSISAMPLING
  VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo  = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisamplingStateCreateInfo.sampleShadingEnable  = VK_FALSE;
  multisamplingStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisamplingStateCreateInfo.minSampleShading     = 1.0f;
  multisamplingStateCreateInfo.pSampleMask          = 0;
  multisamplingStateCreateInfo.alphaToCoverageEnable= VK_FALSE;
  multisamplingStateCreateInfo.alphaToOneEnable     = VK_FALSE;

  //DEPTH AND STENCIL TESTING
  VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo  = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depthStencilStateCreateInfo.depthTestEnable       = VK_TRUE;
  depthStencilStateCreateInfo.depthWriteEnable      = VK_TRUE;
  depthStencilStateCreateInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
  depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
  depthStencilStateCreateInfo.depthTestEnable       = VK_FALSE;

  //COLORBLEND
  VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendAttachmentState.blendEnable             = VK_TRUE;
  colorBlendAttachmentState.srcColorBlendFactor     = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstColorBlendFactor     = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.colorBlendOp            = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.srcAlphaBlendFactor     = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstAlphaBlendFactor     = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.alphaBlendOp            = VK_BLEND_OP_ADD;

  colorBlendAttachmentState.colorWriteMask          = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo  = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendStateCreateInfo.logicOpEnable           = VK_FALSE;
  colorBlendStateCreateInfo.logicOp                 = VK_LOGIC_OP_COPY;
  colorBlendStateCreateInfo.attachmentCount         = 1;
  colorBlendStateCreateInfo.pAttachments            = &colorBlendAttachmentState;

  //DYNAMIC STATE
  const u32 dynamicStateCount = 3;
  VkDynamicState dynamicStates[dynamicStateCount] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_LINE_WIDTH,
  };

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicStateCreateInfo.dynamicStateCount  = 3;
  dynamicStateCreateInfo.pDynamicStates     = dynamicStates;

  //VERTEX INPUT
  VkVertexInputBindingDescription vertexBindingDescription  = {0};
  vertexBindingDescription.binding  = 0;  //Binding Index
  vertexBindingDescription.stride   = sizeof(Vertex3D);
  vertexBindingDescription.inputRate= VK_VERTEX_INPUT_RATE_VERTEX; //move to next data entry for each i

  //ATTRIBUTES
  VkPipelineVertexInputStateCreateInfo  vertexInputCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInputCreateInfo.vertexBindingDescriptionCount   = 1;
  vertexInputCreateInfo.pVertexBindingDescriptions      = &vertexBindingDescription;
  vertexInputCreateInfo.vertexAttributeDescriptionCount = attributeCount;
  vertexInputCreateInfo.pVertexAttributeDescriptions    = attributes;

  //INPUT ASSEMBLY
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo  = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyCreateInfo.topology                      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyCreateInfo.primitiveRestartEnable        = VK_FALSE;

  //PIPELINE LAYOUT
  VkPipelineLayoutCreateInfo  layoutCreateInfo  = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  //DESCRIPTOR SET LAYOUTS
  layoutCreateInfo.setLayoutCount = descriptorSetLayoutCount;
  layoutCreateInfo.pSetLayouts    = descriptorSetLayouts;
  
  //CREATE THE PIPELINE LAYOUT
  result  = vkCreatePipelineLayout(context->device.logicalDevice, &layoutCreateInfo, context->allocator, &outPipeline->pipelineLayout);
  VK_CHECK_VERBOSE(result, "vkCreatePipelineLayout failed");
  UINFO("SUCCESFULLY CEATED PIPELINE LAYOUT");
  
  //CREATE PIPELINE
  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipelineCreateInfo.stageCount         = stageCount;
  pipelineCreateInfo.pStages            = stages;
  pipelineCreateInfo.pVertexInputState  = &vertexInputCreateInfo;
  pipelineCreateInfo.pInputAssemblyState= &inputAssemblyCreateInfo;

  pipelineCreateInfo.pViewportState     = &viewportStateCreateInfo;
  pipelineCreateInfo.pRasterizationState= &rasterizationStateCreateInfo;
  pipelineCreateInfo.pMultisampleState  = &multisamplingStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
  pipelineCreateInfo.pColorBlendState   = &colorBlendStateCreateInfo;
  pipelineCreateInfo.pDynamicState      = &dynamicStateCreateInfo;
  pipelineCreateInfo.pTessellationState = NULL;

  pipelineCreateInfo.layout             = outPipeline->pipelineLayout;

  pipelineCreateInfo.renderPass         = renderpass->handle;
  pipelineCreateInfo.subpass            = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex  = -1;

  result  = vkCreateGraphicsPipelines(context->device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, context->allocator, &outPipeline->handle);

  VK_CHECK_VERBOSE(result, "vkCreateGraphicsPipelines failed");
  UINFO("SUCCESSFULY CREATED GRPAHICS PIPELINE");
  return result;
}

VkResult vulkanGraphicsPipelineDestroy(VulkanContext* context, VulkanPipeline* pipeline){
  TRACEFUNCTION;
  KDEBUG("context : %p pipeline : %p", context, pipeline);
  if(pipeline){
    if(pipeline->handle){
      vkDestroyPipeline(context->device.logicalDevice, pipeline->handle, context->allocator);
      pipeline->handle  = 0;
    }else{
      UWARN("pipeline handle is null can't destroy");
    }
    if(pipeline->pipelineLayout){
      vkDestroyPipelineLayout(context->device.logicalDevice, pipeline->pipelineLayout, context->allocator);
      pipeline->pipelineLayout  = 0;
    }else{
      UWARN("pipeline layout is null, can't destroy");
    }
    return VK_SUCCESS;
  }
  KERROR("CANT DESTROY ANY PIPELINE AS NULL PASSED!");
  return !VK_SUCCESS;
}

VkResult vulkanGraphicsPipelineBind(VulkanCommandBuffer* commandBuffer, VkPipelineBindPoint bindPoint, VulkanPipeline* pipeline){
  TRACEFUNCTION;
  KDEBUG("commandBuffer : %p bindPoint : %p pipeline : %p",commandBuffer, bindPoint, pipeline);
  vkCmdBindPipeline(commandBuffer->handle, bindPoint,  pipeline->handle);
  return VK_SUCCESS;
}

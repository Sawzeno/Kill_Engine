#pragma once

#include "vulkantypes.h"


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
                                       VulkanPipeline* outPipeline);

VkResult vulkanGraphicsPipelineDestroy(VulkanContext* context, VulkanPipeline* pipeline);

VkResult vulkanGraphicsPipelineBind(VulkanCommandBuffer* commandBuffer, VkPipelineBindPoint bindPoint, VulkanPipeline* pipeline);

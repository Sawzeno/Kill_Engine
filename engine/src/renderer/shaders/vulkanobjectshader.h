#pragma once

#include  "renderer/vulkantypes.h"

VkResult  vulkanObjectShaderCreate            (VulkanContext* context, VulkanObjectShader* outShader);
VkResult  vulkanObjectShaderDestroy           (VulkanContext* context, VulkanObjectShader* shader);

VkResult  vulkanObjectShaderUse               (VulkanContext* context, VulkanObjectShader* shader);
VkResult  vulkanObjectShaderUpdateGlobalState (VulkanContext* context, VulkanObjectShader* shader, f32 deltaTime);
VkResult  vulkanObjectShaderUpdateObject      (VulkanContext* context, VulkanObjectShader*,GeometryRenderData data);

VkResult  vulkanObjectShaderAcquireResources  (VulkanContext* context, VulkanObjectShader* shader, u32* outObjectId);
void      vulkanObjectShaderReleaseResoruces  (VulkanContext* context, VulkanObjectShader* shader, u32 objectId);


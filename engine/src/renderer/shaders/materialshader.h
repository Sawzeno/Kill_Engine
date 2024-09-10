#pragma once

#include  "renderer/vulkantypes.h"

VkResult  materialShaderCreate            (VulkanContext* ctx, VulkanMaterialShader* outShader);
VkResult  materialShaderDestroy           (VulkanContext* ctx, VulkanMaterialShader* shader);
VkResult  materialShaderUse               (VulkanContext* ctx, VulkanMaterialShader* shader);
VkResult  materialShaderUpdateGlobalState (VulkanContext* ctx, VulkanMaterialShader* shader, f32 deltaTime);
VkResult  materialShaderUpdateLocalState  (VulkanContext* ctx, VulkanMaterialShader*,GeomteryRenderData data);
bool      materialShaderAcquireResources  (VulkanContext* ctx, VulkanMaterialShader* shader, u32* outObjectId);
void      materialShaderReleaseResources  (VulkanContext* ctx, VulkanMaterialShader* shader, u32  objectId);

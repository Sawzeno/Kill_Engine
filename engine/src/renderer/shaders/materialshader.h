#pragma once

#include  "renderer/vulkantypes.h"

VkResult  materialShaderCreate            (VulkanContext* ctx, MaterialShader* outShader);
VkResult  materialShaderDestroy           (VulkanContext* ctx, MaterialShader* shader);
VkResult  materialShaderUse               (VulkanContext* ctx, MaterialShader* shader);
VkResult  materialShaderUpdateGlobalObjectState (VulkanContext* ctx, MaterialShader* shader, f32 deltaTime);
VkResult  materialShaderUpdateLocalObjectState  (VulkanContext* ctx, MaterialShader*,GeomteryRenderData data);
VkResult  materialShaderAcquireResources  (VulkanContext* ctx, MaterialShader* shader, Material* material);
VkResult  materialShaderReleaseResources  (VulkanContext* ctx, MaterialShader* shader, Material* material);

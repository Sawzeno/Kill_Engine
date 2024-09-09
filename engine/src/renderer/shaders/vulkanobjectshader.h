#pragma once

#include  "renderer/vulkantypes.h"

VkResult  ObjectShaderCreate            (VulkanContext* context, VulkanObjectShader* outShader, Texture* defaultDiffuse);
VkResult  ObjectShaderDestroy           (VulkanContext* context, VulkanObjectShader* shader);

VkResult  ObjectShaderUse               (VulkanContext* context, VulkanObjectShader* shader);
VkResult  ObjectShaderUpdateGlobalState (VulkanContext* context, VulkanObjectShader* shader, f32 deltaTime);
VkResult  ObjectShaderUpdateLocalState  (VulkanContext* context, VulkanObjectShader*,GeomteryRenderData data);

bool      ObjectShaderAcquireResources  (VulkanContext* context, VulkanObjectShader* shader, u32* outObjectId);
void      ObjectShaderReleaseResources  (VulkanContext* context, VulkanObjectShader* shader, u32  objectId);

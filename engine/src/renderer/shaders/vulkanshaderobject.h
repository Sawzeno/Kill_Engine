#pragma once
#include  "renderer/vulkantypes.h"


VkResult  vulkanObjectShaderCreate  (VulkanContext* context, VulkanObjectShader* outShader);
VkResult  vulkanObjectShaderDestroy (VulkanContext* context, VulkanObjectShader* shader);

VkResult  vulkanObjectShaderUse     (VulkanContext* context, VulkanObjectShader* shader);


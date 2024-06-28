#pragma once

#include  "renderer/vulkantypes.h"

VkResult createShaderModule(VulkanContext* context,
                        const char* shaderName,
                        const char* stageName,
                        VkShaderStageFlagBits stageFlag,
                        u64  stageIndex,
                        VulkanShaderStage* shaderStagesArr);

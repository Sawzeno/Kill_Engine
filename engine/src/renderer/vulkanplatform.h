#pragma once

#include  "defines.h"

typedef struct PlatformState platformState;
typedef struct VulkanContext VulkanContext;

u8  createVulkanSurface(platformState* platState, VulkanContext* context);

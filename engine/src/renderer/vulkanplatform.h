#pragma once

#include  "defines.h"

typedef struct platformState platformState;
typedef struct vulkanContext vulkanContext;

u8  createvulkanSurface(platformState* platState, vulkanContext* context);

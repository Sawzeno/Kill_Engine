#pragma once

#include  "rendererbackend.h"

u8  initializeRenderer(u64*requiredMemory, void* state);

u8  rendererOnResized(u16 width , u16 height);

u8  rendererDrawFrame(RenderPacket* packet);

u8  shutdownRenderer();

#pragma once

#include  "rendererbackend.h"

bool  initializeRenderer(u64*requiredMemory, void* state);

bool  rendererOnResized(u16 width , u16 height);

bool  rendererDrawFrame(RenderPacket* packet);

bool  shutdownRenderer();

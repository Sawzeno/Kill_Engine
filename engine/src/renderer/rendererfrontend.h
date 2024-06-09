#pragma once

#include "platform/platform.h"
#include  "rendererbackend.h"

u8  initializeRenderer(const char* applicationName , platformState* platState);

u8  shutdownRenderer();

u8  rendererOnResized(u16 width , u16 height);

u8  rendererDrawFrame(renderPacket* packet);

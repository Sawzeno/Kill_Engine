#pragma once

#include  "rendererbackend.h"

bool  initializeRenderer(u64*requiredMemory, void* state);
void  shutdownRenderer  ();
bool  rendererResize    (u16 width , u16 height);
bool  rendererDrawFrame (RenderPacket* packet);
void  rendererSetView   (Mat4 view);

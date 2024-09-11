#pragma once

#include  "rendererbackend.h"

bool  rendererInitalize(u64*requiredMemory, void* statePtr);
void  rendererShutdown  ();
bool  rendererResize    (u16 width , u16 height);
bool  rendererDrawFrame (RenderPacket* packet);
void  rendererSetView   (Mat4 view);

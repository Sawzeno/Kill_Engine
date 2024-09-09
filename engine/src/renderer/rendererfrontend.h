#pragma once

#include  "rendererbackend.h"

bool  initializeRenderer(u64*requiredMemory, void* state);
void  shutdownRenderer  ();
bool  rendererResize    (u16 width , u16 height);
bool  rendererDrawFrame (RenderPacket* packet);
void  rendererSetView   (Mat4 view);

void rendererCreateTexture(const char *name,
                           bool autoRelease,
                           i32 width,
                           u32 height,
                           i32 channelCount,
                           const u8* pixels,
                           u32 hasTransparency,
                           Texture* tex);

void rendererDestroyTexture( Texture* tex);

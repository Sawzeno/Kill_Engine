#pragma once

#include  "rendererbackend.h"

bool  initializeRenderer(u64*requiredMemory, void* state);
void  shutdownRenderer  ();
bool  rendererResize    (u16 width , u16 height);
bool  rendererDrawFrame (RenderPacket* packet);
void  rendererSetView   (Mat4 view);

bool  rendererCreateTexture(const char* name,
                            bool autoRelease,
                            i32 width,
                            i32 height,
                            i32 channelCount,
                            const u8* pixels,
                            bool hasTransparency,
                            Texture* outTexture);

void  rendererDestroyTexture(Texture* texture);

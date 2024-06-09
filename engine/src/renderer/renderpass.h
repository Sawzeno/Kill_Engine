#pragma once

#include  "vulkantypes.h"

VkResult  vulkanRenderPassCreate(vulkanContext* context,
                            vulkanRenderPass* outRenderPass,
                            f32 x, f32 y, f32 w, f32 h,
                            f32 r, f32 g, f32 b, f32 a,
                            f32 depth, u32 stencil);

VkResult  vulkanRenderPassDestroy (vulkanContext* context,  vulkanRenderPass* renderPass);

//COMMAND BUFFER HOLDS A LIST OF COMMMADNS THAT WILL BE EXECUTED BY A QUEUE
//RENDERPASS NEEDS ONE OF THESE TO PERFORM THEIR OPERATIONS
VkResult  vulkanRenderPassBegin   (vulkanCommandBuffer* commandBuffer,
                                   vulkanRenderPass* renderPass, VkFramebuffer frameBuffer);  

VkResult  vulkanRenderPassEnd     (vulkanCommandBuffer* commandBuffer, vulkanRenderPass* renderPass);

/*
 * vulkan uses functions fro draw calls and simmilar that are fed into a COMMANDbUFFER
 * WHICH are a set of commands and those commands are fed into QUEUE which is the fed
 * to the GPU to be executed  
 * ALL of this is done in a very asynchonous manner , it allows us to submit all the 
 * stuff to the GPU at once to be executed in a very efficient manner
 *
 * COMMAND BUFFERS arent something that we can just create , they are allocated from 
 * a COMMAND POOL which is a pre allocated pool of COMMAND BUFFERS tha we can take any 
 * that are not being currently used 
 *
 * when we first create a COMMAND BUFFER it initial state is set to COMMAND_BUFFER_STATE_NOT_ALLOCATED
 * because a command buffer HANDLE must be allocated from the pool once that happend we switch to
 * COMMAND_BUFFER_READY which can then go to COMMAND_BUFFER_RECORDING, which means we are able to 
 * ISSUE COMMANDS to the COMMAND_BUFFER, we then end the recording with 
 * COMMAND_BUFFER_RECORDING_ENDED it is then COMMAND_BUFFER_SUBMITTED until its is 
 * COMPLETELY EXECUTED and then we go back to READY
 * */


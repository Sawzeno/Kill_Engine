#include  "vulkanrenderpass.h"
#include  "rendererutils.h"

#include  "core/logger.h"
#include  "core/kmemory.h"

VkResult  vulkanRenderPassCreate(VulkanContext*     context,
                                 VulkanRenderPass*  outRenderPass,
                                 f32 x, f32 y, f32 w, f32 h,
                                 f32 r, f32 g, f32 b, f32 a,
                                 f32 depth, u32     stencil){
  TRACEFUNCTION;
  VkResult  result  = 0;

  outRenderPass->x =x ;
  outRenderPass->y =y ;
  outRenderPass->w =w ;
  outRenderPass->h =h ;
  outRenderPass->r =r ;
  outRenderPass->g =g ;
  outRenderPass->b =b ;
  outRenderPass->a =a ;
  outRenderPass->depth = depth;
  outRenderPass->stencil = stencil;

  //Main Subpass
  VkSubpassDescription subpass  = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  //Attachements
  const u32 attachmentDesciptionCount = 2;//----goes here in attachementDescriptions
  VkAttachmentDescription attachmentDesciptions[2] = {0};

  KDEBUG("-----------------------COLOR ATTACHMENT________");
  // color attachment
  VkAttachmentDescription colorAttachment = {0};
  colorAttachment.format          = context->swapchain.imageFormat.format;
  colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;// we are gonna use this for something else goin down the road
  colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;//this is gonna be undefined
  colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;//we do not expect any initial layout before render pass starts
  colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//transitioned to after the render pass
  colorAttachment.flags           = 0;

  attachmentDesciptions[0]  = colorAttachment;

  VkAttachmentReference  colorAttachmentReference  = {0};// specifies what attachent we are going to use for the color attachement
  colorAttachmentReference.attachment = 0;  //Attachement description array index
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  subpass.colorAttachmentCount  = 1;
  subpass.pColorAttachments     = &colorAttachmentReference;

  KDEBUG("-----------------------DEPTH ATTACHMENT-------------");
  //depth attachment
  VkAttachmentDescription depthAttachment = {0};
  depthAttachment.format          = context->device.depthFormat;
  depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;// we are gonna use this for something else goin down the road
  depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_DONT_CARE;//this is gonna be undefined
  depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;//we do not expect any initial layout before render pass starts
  depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//transitioned to after the render pass
  depthAttachment.flags           = 0;

  attachmentDesciptions[1] = depthAttachment;

  //depth attachment reference
  VkAttachmentReference depthAttachmentReference  = {0};
  depthAttachmentReference.attachment = 1;
  depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  //other attachment types (input , resolve , preserve)
  //renderPassCreate

  //depth stencil data
  subpass.pDepthStencilAttachment = &depthAttachmentReference;

  //Input from shader
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;

  //Attachments used for multisampling color attachments
  subpass.pResolveAttachments = NULL;

  //Attachments not used in this subpass , but must be preserved for the next
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments    = NULL;

  KDEBUG("-----------------------RENDER PASS DEPENDENCY----------");
  //Render pass depenedencies [what our source subpass is ,if we had more than one subapss]
  VkSubpassDependency subpassDependency = {0};
  subpassDependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass      = 0;
  subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask   = 0;
  subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  //what memory access we want 
  subpassDependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency.dependencyFlags = 0;

  KDEBUG("-----------------------RENDER PASS CREATE INFO---------");
  //render pass create
  VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  createInfo.attachmentCount        = attachmentDesciptionCount;
  createInfo.pAttachments           = attachmentDesciptions;
  createInfo.subpassCount           = 1;
  createInfo.pSubpasses             = &subpass;
  createInfo.dependencyCount        = 1;
  createInfo.pDependencies          = &subpassDependency;
  createInfo.flags                  = 0;
  createInfo.pNext                  = 0;

  result  = vkCreateRenderPass(context->device.logicalDevice, &createInfo, context->allocator, &outRenderPass->handle);
  VK_CHECK_VERBOSE(result, "vkCreateRenderPass failed");
  KINFO("SUCCESFULLY CREATED RENDERPASS");
  return VK_SUCCESS;
}

VkResult  vulkanRenderPassDestroy (VulkanContext* context,  VulkanRenderPass* renderPass){
  TRACEFUNCTION;
  if(renderPass && renderPass->handle){
    vkDestroyRenderPass(context->device.logicalDevice, renderPass->handle, context->allocator);
    renderPass->handle = NULL;
    return VK_SUCCESS;
  }else{
    KFATAL("NULL PASSED TO %s",__FUNCTION__);
    return !VK_SUCCESS;
  }
}

VkResult  vulkanRenderPassBegin   (VulkanCommandBuffer* commandBuffer,
                                   VulkanRenderPass* renderPass, VkFramebuffer frameBuffer){
TRACEFUNCTION;
  KDEBUG("-----------------------RENDER PASS BEGIN------------");
  VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  beginInfo.renderPass  = renderPass->handle;
  beginInfo.framebuffer = frameBuffer;
  beginInfo.renderArea.offset.x = 0;
  beginInfo.renderArea.offset.y = 0;
  beginInfo.renderArea.extent.width = renderPass->w;
  beginInfo.renderArea.extent.height= renderPass->h;
  
  VkClearValue clearValues[2];
  kzeroMemory(clearValues, sizeof(VkClearValue) * 2);
  clearValues[0].color.float32[0] = renderPass->r;
  clearValues[0].color.float32[1] = renderPass->g;
  clearValues[0].color.float32[2] = renderPass->b;
  clearValues[0].color.float32[3] = renderPass->a;

  clearValues[1].depthStencil.depth = renderPass->depth;
  clearValues[1].depthStencil.stencil= renderPass->stencil;

  beginInfo.clearValueCount = 2;
  beginInfo.pClearValues    = clearValues;

  vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
  commandBuffer->state  = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
  return VK_SUCCESS;
} 

VkResult  vulkanRenderPassEnd     (VulkanCommandBuffer* commandBuffer, VulkanRenderPass* renderPass){
  TRACEFUNCTION;
  vkCmdEndRenderPass(commandBuffer->handle);
  commandBuffer->state  = COMMAND_BUFFER_STATE_RECORDING;
  return VK_SUCCESS;
}

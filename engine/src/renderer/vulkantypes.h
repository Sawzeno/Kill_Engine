#pragma once

#include  "defines.h"
#include  "renderertypes.h"

#include  <vulkan/vulkan_core.h>

#define   OBJECT_SHADER_STAGE_COUNT 2
// descriptors per object
#define   VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT 2
#define   VULKAN_OBJECT_MAX_OBJECT_COUNT 1024

typedef struct  VulkanDevice        VulkanDevice;
typedef struct  VulkanContext       VulkanContext;
typedef struct  VulkanSwapchainSupportInfo VulkanSwapchainSupportInfo;
typedef struct  VulkanSwapchain     VulkanSwapchain;
typedef struct  VulkanImage         VulkanImage;
typedef struct  VulkanRenderPass    VulkanRenderPass;
typedef struct  VulkanCommandBuffer VulkanCommandBuffer;
typedef struct  VulkanFrameBuffer   VulkanFrameBuffer;
typedef struct  VulkanFence         VulkanFence;
typedef struct  VulkanObjectShader  VulkanObjectShader;
typedef struct  VulkanShaderStage   VulkanShaderStage;
typedef struct  VulkanObjectShaderLocalObjectState VulkanObjectShaderLocalObjectState;
typedef struct  VulkanDescriptorState VulkanDescriptorState;
typedef struct  VulkanPipeline      VulkanPipeline;
typedef struct  VulkanBuffer        VulkanBuffer;
typedef struct  VulkanTextureData   VulkanTextureData;
//---------------------------------------------------------------------------ENUMS
typedef enum  VulkanRenderPassState {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
}VulkanRenderPassState;

typedef enum VulkanCommandBufferState{
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED,
}VulkanCommandBufferState;
//---------------------------------------------------------------------------STRUCTS
struct VulkanBuffer{
  u64                           totalSize;
  VkBuffer                      handle;
  VkBufferUsageFlagBits         usage;
  bool                          isLocked;
  VkDeviceMemory                memory;
  i32                           memoryIndex;
  u32                           memoryPropertyFlags;
};

struct VulkanShaderStage{
  VkShaderModuleCreateInfo      createInfo;
  VkShaderModule                handle;
  VkPipelineShaderStageCreateInfo  shaderStageCreateInfo;
};

struct VulkanPipeline{
  VkPipeline                    handle;
  VkPipelineLayout              pipelineLayout;
};

struct VulkanDescriptorState{
  u32 generations[3];                                                 // One per frame, has the descriptor been updated or not 
};

// all isntances will be one per object
struct VulkanObjectShaderLocalObjectState{
  VkDescriptorSet descriptorSets[3];                                        // per frame
  VulkanDescriptorState descriptorStates[VULKAN_OBJECT_MAX_OBJECT_COUNT];   // per descriptor
};

struct VulkanObjectShader{
  VulkanShaderStage             stages[OBJECT_SHADER_STAGE_COUNT];    //vertex , fragment shader stages

  VkDescriptorPool              globalDescriptorPool;
  VkDescriptorSetLayout         globalDescriptorSetLayout;
  VkDescriptorSet               globalDescriptorSets[3];              //One Descriptor Set per Frame, max 3 for triple buffering

  VulkanBuffer                  globalUniformBuffer;                  //Glboal Uniform Buffer
  GlobalUniformObject           globalUBO;                            //Global Uniform Object

  VkDescriptorPool              localDescriptorPool;
  VkDescriptorSetLayout         localDescriptorSetLayout;
  VulkanBuffer                  localUniformBuffer;                  // one large buffer thats gonna handle all the object buffers
  // TODO manage some kind of free list instead
  u32                           localUniformBufferIndex;
  // TODO make dynamic
  VulkanObjectShaderLocalObjectState localObjectStates[VULKAN_OBJECT_MAX_OBJECT_COUNT];
  VulkanPipeline                pipeline; 
};

struct VulkanFence{
  VkFence                       handle;
  u8                            isSignaled;
};

struct  VulkanFrameBuffer{
  VkFramebuffer                 handle;
  u32                           attachmentCount;
  VkImageView*                  attachments;
  VulkanRenderPass*             renderPass;
};

struct VulkanCommandBuffer{
  VkCommandBuffer               handle;
  VulkanCommandBufferState      state;
};

struct VulkanRenderPass{
  VkRenderPass                  handle;
  f32                           x, y, w, h;
  f32                           r, g, b, a;
  f32                           depth;
  u32                           stencil;
  VulkanRenderPassState         state;
};

struct VulkanImage{
  VkImage                       handle;             //handle to the image
  VkDeviceMemory                memory;             //handle to the memory allocated for the image
  VkImageView                   view;               //image  view
  u32                           width;              //width  of image
  u32                           height;             //height of image
};

struct VulkanTextureData{
  VulkanImage image;
  VkSampler   sampler;
};

struct VulkanSwapchain{
  VkSwapchainKHR                handle;             // Handle to the swapchain object
  VkSurfaceFormatKHR            imageFormat;        // Format of the swapchain images
  u16                           maxFramesInFlight;  // Maximum number of frames that can be processed simultaneously
  u32                           imageCount;         // Number of images in the swapchain
  VkImage*                      images;             // Array of swapchain images
  VkImageView*                  views;              // Array of image views corresponding to the swapchain images
  VulkanImage                   depthAttachment;
  VulkanFrameBuffer*            frameBuffers;
};

struct VulkanSwapchainSupportInfo{
  VkSurfaceCapabilitiesKHR      capabilities;       // Capabilities of the surface
  VkSurfaceFormatKHR*           formats;            // Array of available surface formats
  VkPresentModeKHR*             presentModes;       // Array of presentation modes
  u32                           formatCount;        // Number of available formats
  u32                           presentModesCount;  // Number of available presentation modes
};

struct VulkanDevice{
  VkPhysicalDevice              physicalDevice;     // Handle to the physical device
  VkDevice                      logicalDevice;      // Handle to the logical device
  VulkanSwapchainSupportInfo    swapchainSupport;   // Swapchain support details for the device
 
  i32                           graphicsQueueIndex; // Index of the graphics queue family
  i32                           presentQueueIndex;  // Index of the present queue family
  i32                           transferQueueIndex; // Index of the transfer queue family
  
  VkQueue                       graphicsQueue;      // Handle to the graphics queue
  VkQueue                       presentQueue;       // Handle to the present queue
  VkQueue                       transferQueue;      // Handle to the transfer queue
  
  VkCommandPool                 graphicsCommandPool;

  VkPhysicalDeviceProperties    properties;         // Properties of the physical device
  VkPhysicalDeviceFeatures      features;           // Features supported by the physical device
  VkPhysicalDeviceMemoryProperties memory;          // Memory properties of the physical device
  VkFormat                      depthFormat;        // Depth buffer Format, specifying the layout and bit depth of the depth data
};

struct VulkanContext{
  f32                           frameDeltaTime;
  u32                           frameBufferWidth;    // Framebuffer width
  u32                           frameBufferHeight;   // Framebuffer height
  u64                           frameBufferSizeGeneration;
  u64                           frameBufferSizeLastGeneration;
  VkInstance                    instance;            // Handle to the Vulkan instance
  VkAllocationCallbacks*        allocator;           // Custom memory allocator (optional)
  VkDebugUtilsMessengerEXT      debugMessenger;     // Handle to the debug messenger
  VulkanDevice                  device;              // Vulkan device (physical and logical)
  VkSurfaceKHR                  surface;             // Handle to the window surface
  VulkanSwapchain               swapchain;           // Swapchain for image presentation
  VulkanRenderPass              mainRenderPass;      // mainRenderPass for creation
  u32                           imageIndex;          // Index of the current swapchain image
  u32                           currentFrame;        // Index of the current frame being processed
  u8                            recreatingSwapchain; // if swapchain is beign recreated or not
  VulkanCommandBuffer*          graphicsCommandBuffers;//darray
  VkSemaphore*                  imageAvailableSemaphores;//darray
  VkSemaphore*                  queueCompleteSemaphores;//darray
  u32                           inFlightFenceCount;  
  VulkanFence*                  inFlightFences;
  VulkanFence**                 imagesInFlight;        //holds pointer to fences which exist and aew owned elsewhere
  VulkanObjectShader            objectShader;
  VulkanBuffer                  objectVertexBuffer;
  VulkanBuffer                  objectIndexBuffer;
  u64                           geometryVertexOffset;
  u64                           geometryIndexOffset;
  i32(*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);
};


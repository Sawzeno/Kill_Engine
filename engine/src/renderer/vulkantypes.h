#pragma once

#include  "defines.h"
#include  "renderertypes.h"

#include  <vulkan/vulkan_core.h>

#define   MATERIAL_SHADER_STAGE_COUNT 2
#define   MATERIAL_SHADER_GLOBAL_DESCRIPTOR_COUNT 1
#define   MATERIAL_SHADER_LOCAL_DESCRIPTOR_COUNT  2 
#define   MATERIAL_SHADER_SAMPLER_COUNT 1
#define   MAX_MATERIAL_COUNT 1024

typedef struct  VulkanContext       VulkanContext;
typedef struct  VulkanDevice        VulkanDevice;
typedef struct  VulkanSwapchainSupportInfo VulkanSwapchainSupportInfo;
typedef struct  VulkanSwapchain     VulkanSwapchain;
typedef struct  VulkanCommandBuffer VulkanCommandBuffer;
typedef struct  VulkanRenderPass    VulkanRenderPass;
typedef struct  VulkanFrameBuffer   VulkanFrameBuffer;
typedef struct  VulkanFence         VulkanFence;

typedef struct  VulkanBuffer        VulkanBuffer;
typedef struct  VulkanImage         VulkanImage;
typedef struct  VulkanPipeline      VulkanPipeline;
typedef struct  VulkanShaderStage   VulkanShaderStage;
typedef struct  vulkanDescriptorState vulkanDescriptorState;

typedef struct  MaterialShader          MaterialShader;
typedef struct  MaterialInstanceState   MaterialInstanceState;
typedef struct  TextureData             TextureData;

//---------------------------------------------------------------------------ENUMS
typedef enum  VulkanRenderPassState {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
}VULKAN_RENDER_PASS_STATE;

typedef enum VulkanCommandBufferState{
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED,
}VULKAN_COMMAND_BUFFER_STATE;

struct VulkanBuffer{
  u64                   totalSize;
  VkBuffer              handle;
  VkBufferUsageFlagBits usage;
  bool                  isLocked;
  VkDeviceMemory        memory;
  i32                   memoryIndex;
  u32                   memoryPropertyFlags;
};

struct VulkanShaderStage{
  VkShaderModuleCreateInfo  createInfo;
  VkShaderModule            handle;
  VkPipelineShaderStageCreateInfo  shaderStageCreateInfo;
};

struct VulkanPipeline{
  VkPipeline                handle;
  VkPipelineLayout          pipelineLayout;
};

struct vulkanDescriptorState{
  u32 generations[3];
  u32 ids[3];
};

struct MaterialInstanceState {
  VkDescriptorSet descriptorSets[3];
  vulkanDescriptorState descriptorStates[MATERIAL_SHADER_LOCAL_DESCRIPTOR_COUNT];
};

struct MaterialShader{
  MaterialInstanceState     instanceStates[MAX_MATERIAL_COUNT];
  VulkanShaderStage         stages[MATERIAL_SHADER_STAGE_COUNT];

  VulkanPipeline            pipeline; 
  TEXTURE_USE               samplerUses[MATERIAL_SHADER_SAMPLER_COUNT];

  VkDescriptorSet           globalDescriptorSets[3];
  GlobalUniformObject       globalUBO;              //Feed this to a buffer which is tied to a descriptor

  VkDescriptorPool          globalDescriptorPool;
  VkDescriptorSetLayout     globalDescriptorSetLayout;
  VulkanBuffer              globalUniformBuffer;
  
  VkDescriptorPool          localDescriptorPool;
  VkDescriptorSetLayout     localDescriptorSetLayout;
  VulkanBuffer              localUniformBuffer;
  u32                       localUniformBufferIndex;
};

struct VulkanFence{
  VkFence                   handle;
  b32                        isSignaled;
};

struct  VulkanFrameBuffer{
  VkFramebuffer     handle;
  u32               attachmentCount;
  VkImageView*      attachments;
  VulkanRenderPass* renderPass;
};

struct VulkanCommandBuffer{
  VkCommandBuffer               handle;
  VULKAN_COMMAND_BUFFER_STATE      state;
};

struct VulkanRenderPass{
  VkRenderPass                  handle;
  f32                           x, y, w, h;
  f32                           r, g, b, a;
  f32                           depth;
  u32                           stencil;
  VULKAN_RENDER_PASS_STATE         state;
};

struct VulkanImage{
  VkImage                       handle;             //handle to the image
  VkDeviceMemory                memory;             //handle to the memory allocated for the image
  VkImageView                   view;               //image  view
  u32                           width;              //width  of image
  u32                           height;             //height of image
};

struct TextureData{
  VulkanImage                   image;
  VkSampler                     sampler;
};

struct VulkanSwapchain{
  VkSwapchainKHR                handle;             // Handle to the swapchain object
  VkSurfaceFormatKHR            imageFormat;        // Format of the swapchain images
  u32                           maxFramesInFlight;  // Maximum number of frames that can be processed simultaneously
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
  VkInstance                    instance;            // Handle to the Vulkan instance
  VulkanDevice                  device;              // Vulkan device (physical and logical)
  VkAllocationCallbacks*        allocator;           // Custom memory allocator (optional)
  VkDebugUtilsMessengerEXT      debugMessenger;     // Handle to the debug messenger
  VkSurfaceKHR                  surface;             // Handle to the window surface
  VulkanSwapchain               swapchain;           // Swapchain for image presentation
  VulkanRenderPass              mainRenderPass;      // mainRenderPass for creation
  VulkanCommandBuffer*          graphicsCommandBuffers;//darray
  VkSemaphore*                  imageAvailableSemaphores;//darray
  VkSemaphore*                  queueCompleteSemaphores;//darray
  VulkanFence*                  inFlightFences;
  VulkanFence**                 imagesInFlight;        //holds pointer to fences which exist and aew owned elsewhere
  MaterialShader                materialShader;
  VulkanBuffer                  objectVertexBuffer;
  VulkanBuffer                  objectIndexBuffer;
  u64                           frameBufferSizeGeneration;
  u64                           frameBufferSizeLastGeneration;
  u64                           geometryVertexOffset;
  u64                           geometryIndexOffset;
  u32                           inFlightFenceCount;  
  f32                           deltaTime;
  u32                           frameBufferWidth;    // Framebuffer width
  u32                           frameBufferHeight;   // Framebuffer height
  u32                           imageIndex;          // Index of the current swapchain image
  u32                           currentFrame;        // Index of the current frame being processed
  b32                           recreatingSwapchain; // if swapchain is beign recreated or not
  i32                           (*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);
};


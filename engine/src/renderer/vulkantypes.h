#pragma once

#include  "defines.h"
#include  <vulkan/vulkan_core.h>





typedef struct  vulkanDevice        vulkanDevice;
typedef struct  vulkanContext       vulkanContext;
typedef struct  vulkanSwapchainSupportInfo vulkanSwapchainSupportInfo;
typedef struct  vulkanSwapchain     vulkanSwapchain;
typedef struct  vulkanImage         vulkanImage;
typedef struct  vulkanRenderPass    vulkanRenderPass;
typedef struct  vulkanCommandBuffer vulkanCommandBuffer;
typedef struct  vulkanFrameBuffer   vulkanFrameBuffer;
typedef struct  vulkanFence         vulkanFence;

typedef enum  vulkanRenderPassState {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
}vulkanRenderPassState;

typedef enum vulkanCommandBufferState{
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED,
}vulkanCommandBufferState;

struct vulkanFence{
  VkFence handle;
  u8      isSignaled;
};

struct  vulkanFrameBuffer{
  VkFramebuffer     handle;
  u32               attachmentCount;
  VkImageView*      attachments;
  vulkanRenderPass* renderPass;
};

struct vulkanCommandBuffer{
  VkCommandBuffer               handle;
  vulkanCommandBufferState      state;
};

struct vulkanRenderPass{
  VkRenderPass                  handle;
  f32                           x, y, w, h;
  f32                           r, g, b, a;
  f32                           depth;
  u32                           stencil;
  vulkanRenderPassState         state;
};

struct vulkanImage{
  VkImage                       handle;             //handle to the image
  VkDeviceMemory                memory;             //handle to the memory allocated for the image
  VkImageView                   view;               //image  view
  u32                           width;              //width  of image
  u32                           height;             //height of image
};

struct vulkanSwapchain{
  VkSwapchainKHR                handle;             // Handle to the swapchain object
  VkSurfaceFormatKHR            imageFormat;        // Format of the swapchain images
  u16                           maxFramesInFlight;  // Maximum number of frames that can be processed simultaneously
  u32                           imageCount;         // Number of images in the swapchain
  VkImage*                      images;             // Array of swapchain images
  VkImageView*                  views;              // Array of image views corresponding to the swapchain images
  vulkanImage                   depthAttachment;
  vulkanFrameBuffer*            frameBuffers;
};

struct vulkanSwapchainSupportInfo{
  VkSurfaceCapabilitiesKHR      capabilities;       // Capabilities of the surface
  VkSurfaceFormatKHR*           formats;            // Array of available surface formats
  VkPresentModeKHR*             presentModes;       // Array of presentation modes
  u32                           formatCount;        // Number of available formats
  u32                           presentModesCount;  // Number of available presentation modes
};

struct vulkanDevice{
  VkPhysicalDevice              physicalDevice;     // Handle to the physical device
  VkDevice                      logicalDevice;      // Handle to the logical device
  vulkanSwapchainSupportInfo    swapchainSupport;   // Swapchain support details for the device
 
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

struct vulkanContext{
  u32                           frameBufferWidth;    // Framebuffer width
  u32                           frameBufferHeight;   // Framebuffer height
  u64                           frameBufferSizeGeneration;
  u64                           frameBufferSizeLastGeneration;
  VkInstance                    instance;            // Handle to the Vulkan instance
  VkAllocationCallbacks*        allocator;           // Custom memory allocator (optional)
  VkSurfaceKHR                  surface;             // Handle to the window surface
  VkDebugUtilsMessengerEXT      debugMessenger;     // Handle to the debug messenger
  vulkanDevice                  device;              // Vulkan device (physical and logical)
  vulkanSwapchain               swapchain;           // Swapchain for image presentation
  vulkanRenderPass              mainRenderPass;      // mainRenderPass for creation
  u32                           imageIndex;          // Index of the current swapchain image
  u32                           currentFrame;        // Index of the current frame being processed
  u8                            recreatingSwapchain; // if swapchain is beign recreated or not
  vulkanCommandBuffer*          graphicsCommandBuffers;//darray
  VkSemaphore*                  imageAvailableSemaphores;//darray
  VkSemaphore*                  queueCompleteSemaphores;//darray
  u32                           inFlightFenceCount;  
  vulkanFence*                  inFlightFences;
  vulkanFence**                 imagesInFlight;        //holds pointer to fences which exist and aew owned elsewhere
  i32(*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);
};


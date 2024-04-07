#define   GLFW_INCLUDE_VULKAN
#include  <stdio.h>
#include  <GLFW/glfw3.h>

#include  "test.h"



int test() {
  if (!glfwInit()) {
    printf("Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", NULL, NULL);

  if (!window) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  printf("GLFW window created successfully\n");

  printf("GLFW window created successfully\n");

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

  printf("%d extensions supported\n", extensionCount);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

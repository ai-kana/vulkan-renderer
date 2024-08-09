#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

extern VkSurfaceKHR surface;

VkResult create_surface();

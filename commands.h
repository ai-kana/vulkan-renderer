#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 2

extern VkQueue graphics_queue;
extern VkQueue present_queue;

extern VkCommandBuffer* command_buffers;
extern VkCommandPool command_pool;

VkResult create_command_buffers();
VkResult create_command_pool();
VkResult record_command_buffer(VkCommandBuffer* buffer, uint32_t image_index);

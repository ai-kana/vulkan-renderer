#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "commands.h"

extern VkSemaphore* image_available_semaphore;
extern VkSemaphore* render_finished_semaphore;
extern VkFence* in_flight_fence;

VkResult create_sync_objects();

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>

extern VkDevice logical_device;
extern VkPhysicalDevice physical_device;

struct optional_uint32_t {
    uint32_t value;
    bool assigned;
};

struct queue_family_indices {
    struct optional_uint32_t graphics_family;
    struct optional_uint32_t present_family;
};

struct queue_family_indices find_queue_families(VkPhysicalDevice* device);

VkResult init_device();
VkResult create_logical_device();

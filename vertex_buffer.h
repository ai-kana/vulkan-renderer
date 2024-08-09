#pragma once

#include "linmath.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct vertex {
    vec2 position;
    vec3 color;
};

#define VERTICES_SIZE 3
extern const struct vertex vertices[VERTICES_SIZE];
extern VkBuffer vertex_buffer;
extern VkDeviceMemory vertex_buffer_memory;

VkVertexInputBindingDescription get_binding_description();
VkVertexInputAttributeDescription* get_attribute_description(uint32_t* size);
VkResult create_vertex_buffer();

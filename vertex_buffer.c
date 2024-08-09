#include "devices.h"
#include <stdint.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>
#include "vertex_buffer.h"

const struct vertex vertices[VERTICES_SIZE] = {
    {{0.f, -0.5f}, {1.f, 0.f, 0.f}},
    {{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
    {{-0.5f, 0.5f}, {0.f, 0.f, 1.f}},
};

VkBuffer vertex_buffer;
VkDeviceMemory vertex_buffer_memory;

VkVertexInputBindingDescription get_binding_description() {
    VkVertexInputBindingDescription description;
    description.binding = 0;
    description.stride = sizeof(struct vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return description;
}

VkVertexInputAttributeDescription* get_attribute_description(uint32_t* size) {
    const uint32_t s = 2;
    *size = 2;
    VkVertexInputAttributeDescription* description = malloc(sizeof(VkVertexInputAttributeDescription) * s);
    description[0].binding = 0;
    description[0].location = 0;
    description[0].format = VK_FORMAT_R32G32_SFLOAT;
    // This is the worst shit ever
    description[0].offset = offsetof(struct vertex, position);

    description[1].binding = 0;
    description[1].location = 1;
    description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    description[1].offset = offsetof(struct vertex, color);

    return description;
}

static uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if (!(type_filter & (1 << i))) {
            continue;
        }

        if ((memory_properties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    return -1;
}

VkResult create_vertex_buffer() {
    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertices[0]) * VERTICES_SIZE,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult result =  vkCreateBuffer(logical_device, &create_info, NULL, &vertex_buffer);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(logical_device, vertex_buffer, &memory_requirements);

    VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, property_flags)
    };

    result = vkAllocateMemory(logical_device, &allocate_info, NULL, &vertex_buffer_memory);
    if (result != VK_SUCCESS) {
        return result;
    }

    vkBindBufferMemory(logical_device, vertex_buffer, vertex_buffer_memory, 0);

    void* data;
    vkMapMemory(logical_device, vertex_buffer_memory, 0, create_info.size, 0, &data);
    {
        memcpy(data, vertices, create_info.size);
    }
    vkUnmapMemory(logical_device, vertex_buffer_memory);

    return VK_SUCCESS;
}

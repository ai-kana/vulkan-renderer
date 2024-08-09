#include "commands.h"
#include "devices.h"
#include "graphics_pipeline.h"
#include "swap_chain.h"
#include "vertex_buffer.h"
#include <stdlib.h>

VkQueue graphics_queue;
VkQueue present_queue;

VkCommandBuffer* command_buffers;
VkCommandPool command_pool;

VkResult record_command_buffer(VkCommandBuffer* buffer, uint32_t image_index) {
    VkCommandBufferBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkBeginCommandBuffer(*buffer, &info);

    VkClearValue clear_color = {{{0.f, 0.f, 0.f, 0.1f}}};
    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = swap_chain_frame_buffers[image_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = swap_chain_extent,
        .clearValueCount = 1,
        .pClearValues = &clear_color
    };

    vkCmdBeginRenderPass(*buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    {
        vkCmdBindPipeline(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        VkViewport viewport = {
            .x = 0.f,
            .y = 0.f,
            .width = swap_chain_extent.width,
            .height = swap_chain_extent.height,
        };
        vkCmdSetViewport(*buffer, 0, 1, &viewport);

        VkRect2D scissors = {
            .offset = {0, 0},
            .extent = swap_chain_extent,
        };
        vkCmdSetScissor(*buffer, 0, 1, &scissors);

        VkBuffer vertex_buffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(*buffer, 0, 1, vertex_buffers, offsets);

        vkCmdDraw(*buffer, VERTICES_SIZE, 1, 0, 0);
    }
    vkCmdEndRenderPass(*buffer);
    return vkEndCommandBuffer(*buffer);
}

VkResult create_command_pool() {
    struct queue_family_indices indices = find_queue_families(&physical_device);
    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family.value,
    };

    return vkCreateCommandPool(logical_device, &create_info, NULL, &command_pool);
}

VkResult create_command_buffers() {
    command_buffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    return vkAllocateCommandBuffers(logical_device, &buffer_info, command_buffers);
}

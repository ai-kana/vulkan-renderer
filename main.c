#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "devices.h"
#include "graphics_pipeline.h"
#include "surfaces.h"
#include "main.h"
#include "sync_objects.h"
#include "swap_chain.h"
#include "vertex_buffer.h"
#include "window.h"
#include "commands.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>

VkInstance instance;
static uint32_t current_frame = 0;

static VkResult create_instance() {
    struct VkApplicationInfo application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Meow",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Meowgine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t extensions_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);

    struct VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = extensions_count,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
    };

    return vkCreateInstance(&create_info, NULL, &instance);
}

static VkResult init_vulkan() {
    VkResult result;
    result = create_instance();
    if (result != VK_SUCCESS) {
        puts("Failed to create instance");
        return result;
    }

    result = create_surface();
    if (result != VK_SUCCESS) {
        puts("Failed to create surface");
        return result;
    }

    result = init_device();
    if (result != VK_SUCCESS) {
        puts("Failed to create device");
        return result;
    }

    result = create_logical_device();
    if (result != VK_SUCCESS) {
        puts("Failed to create logical device");
        return result;
    }

    result = create_swap_chain();
    if (result != VK_SUCCESS) {
        puts("Failed to create swap chain");
        return result;
    }

    result = create_image_view();
    if (result != VK_SUCCESS) {
        puts("Failed to create image view");
        return result;
    }

    result = create_render_pass();
    if (result != VK_SUCCESS) {
        puts("Failed to create render pass");
        return result;
    }

    result = create_graphics_pipeline();
    if (result != VK_SUCCESS) {
        puts("Failed to create graphics pipeline");
        return result;
    }

    result = create_frame_buffer();
    if (result != VK_SUCCESS) {
        puts("Failed to create frame buffers");
        return result;
    }

    result = create_command_pool();
    if (result != VK_SUCCESS) {
        puts("Failed to create command pool");
        return result;
    }

    result = create_vertex_buffer();
    if (result != VK_SUCCESS) {
        puts("Failed to create vertex buffer");
        return result;
    }

    result = create_command_buffers();
    if (result != VK_SUCCESS) {
        puts("Failed to create command buffer");
        return result;
    }

    result = create_sync_objects();
    if (result != VK_SUCCESS) {
        puts("Failed to create sync objects");
        return result;
    }

    return VK_SUCCESS;
}

static void draw_frame() {
    vkWaitForFences(logical_device, 1, &in_flight_fence[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR((logical_device), swap_chain, UINT64_MAX, image_available_semaphore[current_frame], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return;
    }
    vkResetFences(logical_device, 1, &in_flight_fence[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);
    record_command_buffer(&command_buffers[current_frame], image_index);

    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitSemaphores = &image_available_semaphore[current_frame],
        .waitSemaphoreCount = 1,
        .pWaitDstStageMask = &flags,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffers[current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished_semaphore[current_frame],
    };

    vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence[current_frame]);
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_semaphore[current_frame],
        .pSwapchains = &swap_chain,
        .swapchainCount = 1,
        .pImageIndices = &image_index,
    };
    vkQueuePresentKHR(present_queue, &present_info);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

static void main_loop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        draw_frame();
    }

    vkDeviceWaitIdle(logical_device);
}

static void cleanup_swap_chain() {
    for (uint32_t i = 0; i < swap_chain_images_count; i++) {
        vkDestroyFramebuffer(logical_device, swap_chain_frame_buffers[i], NULL);
    }

    for (uint32_t i = 0; i < swap_chain_images_count; i++) {
        vkDestroyImageView(logical_device, swap_chain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(logical_device, swap_chain, NULL);
}

static void cleanup() {
    cleanup_swap_chain();

    vkDestroyBuffer(logical_device, vertex_buffer, NULL);
    vkFreeMemory(logical_device, vertex_buffer_memory, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(logical_device, image_available_semaphore[i], NULL);
        vkDestroySemaphore(logical_device, render_finished_semaphore[i], NULL);
        vkDestroyFence(logical_device, in_flight_fence[i], NULL);
    }

    free(image_available_semaphore);
    free(render_finished_semaphore);
    free(in_flight_fence);

    vkDestroyCommandPool(logical_device, command_pool, NULL);
    free(command_buffers);

    vkDestroyPipeline(logical_device, pipeline, NULL);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, NULL);
    vkDestroyRenderPass(logical_device, render_pass, NULL);

    vkDestroyDevice(logical_device, NULL);

    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    free(swap_chain_images);
    free(swap_chain_image_views);

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    init_window();

    if (init_vulkan() != VK_SUCCESS) {
        return 1;
    }

    main_loop();
    cleanup();

    return 0;
}

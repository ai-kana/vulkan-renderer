#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

extern VkSwapchainKHR swap_chain;
extern VkImage* swap_chain_images;
extern uint32_t swap_chain_images_count;

extern VkImageView* swap_chain_image_views;

extern VkFormat swap_chain_format;
extern VkExtent2D swap_chain_extent;

extern VkFramebuffer* swap_chain_frame_buffers;

struct swap_chain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;

    VkSurfaceFormatKHR* formats;
    uint32_t formats_count;

    VkPresentModeKHR* present_modes;
    uint32_t present_modes_count;
};

struct swap_chain_support_details query_swap_chain_details(VkPhysicalDevice* device);

VkResult create_swap_chain();
void recreate_swap_chain();
VkResult create_image_view();
VkResult create_frame_buffer();

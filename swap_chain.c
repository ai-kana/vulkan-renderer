#include "swap_chain.h"
#include "devices.h"
#include "graphics_pipeline.h"
#include "surfaces.h"
#include "window.h"
#include <limits.h>
#include <stdlib.h>

VkSwapchainKHR swap_chain;
VkImage* swap_chain_images;
uint32_t swap_chain_images_count;

VkImageView* swap_chain_image_views;

VkFormat swap_chain_format;
VkExtent2D swap_chain_extent;

VkFramebuffer* swap_chain_frame_buffers;

static uint32_t clamp(uint32_t number, uint32_t min, uint32_t max) {
    if (number < min) {
        return min;
    }

    if (number > max) {
        return max;
    }

    return number;
}

static VkExtent2D choose_swap_chain_extent(VkSurfaceCapabilitiesKHR* capabilities) {
    if (capabilities->currentExtent.width != UINT_MAX) {
        return capabilities->currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = {
        .width = width,
        .height = height,
    };

    extent.width = clamp(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    extent.height = clamp(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);
    return extent;
}

static VkPresentModeKHR choose_swap_chain_present_mode(VkPresentModeKHR* modes, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        VkPresentModeKHR mode = modes[i];
        if (mode != VK_PRESENT_MODE_MAILBOX_KHR) {
            continue;
        }

        return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkSurfaceFormatKHR choose_swap_chain_surface_format(VkSurfaceFormatKHR* formats, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        VkSurfaceFormatKHR format = formats[i];
        if (format.format != VK_FORMAT_R8G8B8A8_SRGB) {
            continue;
        }

        if (format.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            continue;
        }

        return format;
    }

    return formats[0];
}

VkResult create_frame_buffer() {
    swap_chain_frame_buffers = malloc(sizeof(swap_chain_frame_buffers) * swap_chain_images_count);

    for (uint32_t i = 0; i < swap_chain_images_count; i++) {
        VkImageView* image_view = &swap_chain_image_views[i];

        VkFramebufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = image_view,
            .width = swap_chain_extent.width,
            .height = swap_chain_extent.height,
            .layers = 1,
        };

        VkResult result = vkCreateFramebuffer(logical_device, &create_info, NULL, &swap_chain_frame_buffers[i]);
        if (result != VK_SUCCESS) {
            return result;
        }
    }

    return VK_SUCCESS;
}


VkResult create_image_view() {
    swap_chain_image_views = malloc(sizeof(VkImageView) * swap_chain_images_count);
    for (uint32_t i = 0; i < swap_chain_images_count; i++) {
        VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swap_chain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swap_chain_format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };
        VkResult result = vkCreateImageView(logical_device, &create_info, NULL, &swap_chain_image_views[i]);
        if (result != VK_SUCCESS) {
            return result;
        }
    }

    return VK_SUCCESS;
}

struct swap_chain_support_details query_swap_chain_details(VkPhysicalDevice* device) {
    struct swap_chain_support_details details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, surface, &details.formats_count, NULL);
    if (details.formats_count != 0) {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * details.formats_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(*device, surface, &details.formats_count, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, surface, &details.present_modes_count, NULL); 
    if (details.present_modes_count != 0) {
        details.present_modes = malloc(sizeof(VkPresentModeKHR) * details.present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(*device, surface, &details.present_modes_count, details.present_modes);
    }

    return details;
}

VkResult create_swap_chain() {
    struct swap_chain_support_details details = query_swap_chain_details(&physical_device);

    VkSurfaceFormatKHR surface_format = choose_swap_chain_surface_format(details.formats, details.formats_count);
    VkPresentModeKHR present_mode = choose_swap_chain_present_mode(details.present_modes, details.present_modes_count);
    VkExtent2D extent = choose_swap_chain_extent(&details.capabilities);

    uint32_t image_count = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
        image_count = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    struct queue_family_indices indices = find_queue_families(&physical_device);
    uint32_t family_indices[2] = {
        indices.graphics_family.value, 
        indices.present_family.value
    };

    if (indices.graphics_family.value != indices.present_family.value) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult result = vkCreateSwapchainKHR(logical_device, &create_info, NULL, &swap_chain);
    if (result != VK_SUCCESS) {
        return result;
    }

    vkGetSwapchainImagesKHR(logical_device, swap_chain, &swap_chain_images_count, NULL);
    swap_chain_images = malloc(sizeof(VkImage) * swap_chain_images_count);
    vkGetSwapchainImagesKHR(logical_device, swap_chain, &swap_chain_images_count, swap_chain_images);

    swap_chain_format = surface_format.format;
    swap_chain_extent = extent;

    return result;
}

void recreate_swap_chain() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(logical_device);

    create_swap_chain();
    create_image_view();
    create_frame_buffer();
}

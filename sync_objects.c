#include "sync_objects.h"
#include "commands.h"
#include "devices.h"
#include <stdlib.h>

VkSemaphore* image_available_semaphore;
VkSemaphore* render_finished_semaphore;
VkFence* in_flight_fence;

VkResult create_sync_objects() {
    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    render_finished_semaphore = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    image_available_semaphore = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    in_flight_fence = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    VkResult result;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        result = vkCreateSemaphore(logical_device, &semaphore_info, NULL, &render_finished_semaphore[i]);
        if (result != VK_SUCCESS)
        {
            return result;
        }

        result = vkCreateSemaphore(logical_device, &semaphore_info, NULL, &image_available_semaphore[i]);
        if (result != VK_SUCCESS)
        {
            return result;
        }

        result = vkCreateFence(logical_device, &fence_info, NULL, &in_flight_fence[i]);
        if (result != VK_SUCCESS)
        {
            return result;
        }
    }

    return VK_SUCCESS;
}

#include "devices.h"
#include "commands.h"
#include "main.h"
#include "surfaces.h"
#include "swap_chain.h"
#include <stdlib.h>
#include <string.h>

VkDevice logical_device;
VkPhysicalDevice physical_device;

static const char* device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static bool check_extension_support(VkPhysicalDevice* device) {
    size_t count = sizeof(device_extensions) / sizeof(char*);

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(*device, NULL, &extension_count, NULL);

    VkExtensionProperties* available = malloc(extension_count * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(*device, NULL, &extension_count, available);

    uint32_t matches = 0;
    for (uint32_t i = 0; i < count; i++) {
        for (uint32_t n = 0; n < extension_count; n++) { 
            if (strcmp(device_extensions[i], available[n].extensionName) == 0) {
                matches++;
                break;
            }
        }
    }

    free(available);

    return matches == count;
}

static bool device_suitable(VkPhysicalDevice* device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(*device, &properties);
    
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(*device, &features);
    struct queue_family_indices indices = find_queue_families(device);
    if (!indices.graphics_family.assigned) {
        return false;
    }

    bool supports_extensions = check_extension_support(device);
    bool supports_swap_chain = false;
    if (supports_extensions) {
        struct swap_chain_support_details details = query_swap_chain_details(device);
        supports_swap_chain = details.formats_count != 0 && details.present_modes_count != 0;

        if (details.formats_count > 0) {
            free(details.formats);
        }

        if (details.present_modes_count > 0) {
            free(details.present_modes);
        }
    }

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader && supports_swap_chain;
}


VkResult create_logical_device() {
    struct queue_family_indices indices = find_queue_families(&physical_device);
    float priority = 1.f;

    uint32_t unique_count = 1;
    if (indices.graphics_family.value != indices.present_family.value) {
        unique_count = 2;
    }

    VkDeviceQueueCreateInfo queue_create_infos[2];
    VkDeviceQueueCreateInfo graphics_queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.graphics_family.value,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    VkDeviceQueueCreateInfo present_queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.present_family.value,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    queue_create_infos[0] = graphics_queue_create_info;
    queue_create_infos[1] = present_queue_create_info;

    VkPhysicalDeviceFeatures features;
    memset(&features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));

    size_t extension_count = sizeof(device_extensions) / sizeof(char*);
    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = unique_count,
        .pEnabledFeatures = &features,
        .enabledLayerCount = 0,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = device_extensions,
    };

    VkResult out = vkCreateDevice(physical_device, &device_create_info, NULL, &logical_device);
    if (out != VK_SUCCESS) {
        return out;
    }

    vkGetDeviceQueue(logical_device, indices.graphics_family.value, 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, indices.present_family.value, 0, &present_queue);

    return VK_SUCCESS;
}

struct queue_family_indices find_queue_families(VkPhysicalDevice* device) {
    struct queue_family_indices indices;
    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &family_count, NULL);

    VkQueueFamilyProperties* families = malloc(sizeof(VkQueueFamilyProperties) * family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &family_count, families);
    for (uint32_t i = 0; i < family_count; i++) {
        VkQueueFamilyProperties family = families[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family.value = i;
            indices.graphics_family.assigned = true;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, surface, &present_support);
        if (present_support) {
            indices.present_family.value = i;
            indices.present_family.assigned = true;
        }
        
        if (indices.graphics_family.assigned && indices.present_family.assigned) {
            break;
        }
    }

    free(families);

    return indices;
}


VkResult init_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (device_count == 0) {
        return !VK_SUCCESS;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    physical_device = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice device = devices[i];
        if (!device_suitable(&device)) {
            continue;
        }

        physical_device = device;
        break;
    }

    free(devices);

    if (physical_device == VK_NULL_HANDLE) {
        return !VK_SUCCESS;
    }

    return VK_SUCCESS;
}

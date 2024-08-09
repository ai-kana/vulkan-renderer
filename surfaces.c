#include "surfaces.h"
#include "main.h"
#include "window.h"

VkSurfaceKHR surface;

VkResult create_surface() {
    return glfwCreateWindowSurface(instance, window, NULL, &surface);
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

extern VkRenderPass render_pass;
extern VkPipelineLayout pipeline_layout;
extern VkPipeline pipeline;

VkResult create_render_pass();
VkResult create_graphics_pipeline();

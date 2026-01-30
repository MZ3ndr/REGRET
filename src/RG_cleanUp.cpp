#include "main.h"
#include <vulkan/vulkan_core.h>
REGRET::RG_Result REGRET::context::CleanUp(){
    vkDestroyDevice(vulkan.getDevice(),nullptr);
    vkDestroySurfaceKHR(vulkan.getInstance(),vulkan.getSurface(),nullptr);
    vkDestroyInstance(vulkan.getInstance(), nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();

    return RG_Result::RG_SUCCESS;
};
#include "main.h"

REGRET::RG_Result REGRET::context::CleanUp(){
    vkDeviceWaitIdle(vulkan._device);
    vkDestroyCommandPool(vulkan._device, vulkan._commandPool, nullptr);
    for(auto framebuffer : vulkan._swapChainFramebuffers){
        vkDestroyFramebuffer(vulkan._device,framebuffer,nullptr);
    }
    vkDestroyPipeline(vulkan._device, vulkan._graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vulkan._device,vulkan._pipelinelayout, nullptr);
    vkDestroyRenderPass(vulkan._device,vulkan._renderPass,nullptr);
    for(auto imageView : vulkan._swapChainImageViews){
        vkDestroyImageView(vulkan._device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vulkan._device, vulkan._swapChain, nullptr);
    vkDestroyDevice(vulkan._device,nullptr);
    vkDestroySurfaceKHR(vulkan._instance,vulkan._surface,nullptr);
    vkDestroyInstance(vulkan._instance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();

    return RG_Result::RG_SUCCESS;
};
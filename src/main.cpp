//created 19.1.2026 21:00
#include "main.h"
#include "logging/log.h"
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan_core.h>


void test(){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::cout << "available extensions:\n";
    for (const auto& extension : extensions) {
        LOG(extension.extensionName);
    }
}

REGRET::RG_Result REGRET::context::Vulkan::CheckValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName : vulkan.validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return RG_Result::RG_FAIL;
        }
    }
    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::context::VulkanInit(){
    #ifdef DEBUG
        vulkan.enableVkValidationLayers = true;
        if(vulkan.CheckValidationLayerSupport()!=RG_Result::RG_SUCCESS){
            ERROR("validation layers requested, but not available!");
            return RG_Result::RG_Validation_ERR;
        }
    #endif
    vulkan.CreateInstance();
    return RG_Result::RG_SUCCESS;
};

REGRET::RG_Result REGRET::context::CleanUp(){
    vkDestroyInstance(this->vulkan.instance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();

    return RG_Result::RG_SUCCESS;
};

REGRET::RG_Result REGRET::context::WindowInit(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(winSettings.width,winSettings.height,winSettings.title, winSettings.monitor, winSettings.share);
    if(!this->window){
        return RG_Result::RG_WINDOW_NOT_CREATED;
    }
    return RG_Result::RG_SUCCESS;
};

void REGRET::context::init(){
    Logger logger{};
    logger.Clear();
    WindowInit();
    VulkanInit();
}

void UpdateLoop(REGRET::context *ctx){
    while(!glfwWindowShouldClose(ctx->window)){
        glfwPollEvents();

    }
}

int main() {
    REGRET::context ctx{};
    ctx.init();
    UpdateLoop(&ctx);
    ctx.CleanUp();
    ERROR("test");
};

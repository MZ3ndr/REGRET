//created 19.1.2026 21:00
#include "main.h"
#include "common/common.h"
#include "logging/log.h"
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>

// test [[someAnchor|test]]
// test 2 [[#someAnchor|test2]]




void test(){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    LOG("available extensions:\n");
    for (const auto& extension : extensions) {
        LOG(extension.extensionName);
    }
}

void test2(){
    int count = 10;
    array<int> arr(count);
    LOG(arr.data());
    int *p = arr.data();
    arr.resize(count);
    *p = 4;
    ++p;
    *p = 2;
    p[2] = 100;
    for (const auto& elem : arr) {
        LOG(elem);
    }
}

REGRET::RG_Result REGRET::context::Vulkan::CheckValidationLayerSupport(){
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    array<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    availableLayers.resize(layerCount);
    for (const char* layerName : this->validationLayers) {
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

    res = vulkan.CreateInstance();
    RG_check(res);
    res = CreateSurface();
    RG_check(res);
    res = vulkan.SelectPhysicalDevice();
    RG_check(res);
    res = vulkan.CreateLogicalDevice();
    RG_check(res);
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
    #if defined(_WIN32)
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(window);
        createInfo.hinstance = GetModuleHandle(nullptr);
    #elif defined(__linux__)
        VkWaylandSurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        createInfo.display = glfwGetWaylandDisplay();
        createInfo.surface = glfwGetWaylandWindow(window);
    #endif
    return RG_Result::RG_SUCCESS;
};

REGRET::RG_Result REGRET::context::init(){
    Logger logger{};
    logger.Clear();
    WARN("Initiating Window...");
    res = WindowInit();
    WARN("Window DONE");
    RG_check(res);
    WARN("Initiating Vulkan...");
    res = VulkanInit();
    WARN("Vulkan DONE");
    RG_check(res);
    WARN("SETUP DONE");
    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result  UpdateLoop(REGRET::context *ctx){
    while(!glfwWindowShouldClose(ctx->window)){
        glfwPollEvents();


    }
    return REGRET::RG_Result::RG_SUCCESS;
}

int main() {
    REGRET::context ctx{};
    REGRET::RG_Result res = REGRET::RG_Result::RG_FAIL;
    WARN("Using: ",UsingWindows ? "Windows" : "Linux");
    res = ctx.init();
    RG_mainERR(res);
    res = REGRET::RG_Result::RG_FAIL;
    res = UpdateLoop(&ctx);
    RG_mainERR(res);
    ctx.CleanUp();
    if(res == REGRET::RG_Result::RG_SUCCESS){
        LOG(res);
    }else{
        ERROR(res);
    }
};

//created 19.1.2026 21:00
//first rendering (RGB Triangle) 27.9.2026 5:00
#include "main.h"
#include "common/common.h"

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

REGRET::RG_Result REGRET::RG_Vulkan::CheckValidationLayerSupport(){
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

REGRET::RG_Result REGRET::RG_Vulkan::VulkanInit(GLFWwindow* window){

    res = CreateInstance();
    RG_check(res);
    res = CreateSurface(window);
    RG_check(res);
    res = SelectPhysicalDevice();
    RG_check(res);
    res = CreateLogicalDevice();
    RG_check(res);
    res = CreateSwapChain(window);
    RG_check(res);
    res = CreateImageViews();
    RG_check(res);
    res = CreateRenderPass();
    RG_check(res);
    res = CreateGraphicsPipeine();
    RG_check(res);
    res = CreateFramebuffer();
    RG_check(res);
    res = CreateCommandPool();
    RG_check(res);
    res = CreateCommandBuffer();
    RG_check(res);
    res = CreateSyncObjects();
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
    glfwFocusWindow(window);
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
    res = vulkan.VulkanInit(window);
    WARN("Vulkan DONE");
    RG_check(res);
    WARN("SETUP DONE");
    return RG_Result::RG_SUCCESS;
}

void REGRET::context::drawFrame(){
    vkWaitForFences(vulkan._device, 1, &vulkan._inFlightFence,VK_TRUE, UINT64_MAX);
    vkResetFences(vulkan._device,1,&vulkan._inFlightFence);
    u32 imageIndex;
    vkAcquireNextImageKHR(vulkan._device, vulkan._swapChain, UINT64_MAX, vulkan._imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(vulkan._commandBuffer,0);
    vulkan.recordCommandBuffer(vulkan._commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {vulkan._imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkan._commandBuffer;

    VkSemaphore signalSemaphores[] = {vulkan._renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(vkQueueSubmit(vulkan._graphicsQueue,1,&submitInfo,vulkan._inFlightFence) != VK_SUCCESS){
        RG_ERR_BREAK("fialed to submir draw command buffer!")
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vulkan._swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(vulkan._presentQueue,&presentInfo);
}

REGRET::RG_Result  UpdateLoop(REGRET::context *ctx){
    while(!glfwWindowShouldClose(ctx->window)){
        glfwPollEvents();
        ctx->drawFrame();

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


#include "RG_vulkan.h"
#include "common/common.h"

REGRET::RG_Result REGRET::RG_Vulkan::Debug(){
    #ifdef DEBUG
    enableVkValidationLayers = true;
        if(CheckValidationLayerSupport()!=RG_Result::RG_SUCCESS){
            throw std::runtime_error("validation layers requested, but not available!");
            return RG_Result::RG_Validation_ERR;
        }
    #endif
    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::RG_Vulkan::CreateInstance(){

    uint32_t glfwExtentionCount = 0;
    const char** glfwExtentions;
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    array<const char*> requiredExtensions(glfwExtentionCount+1);
    for(uint32_t i = 0; i < glfwExtentionCount; i++) {
        requiredExtensions.add(glfwExtentions[i]);
    }

    requiredExtensions.add(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    VkApplicationInfo RegretInfo{};
    RegretInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    RegretInfo.pApplicationName = "Regret";
    RegretInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    RegretInfo.pEngineName = "Regret-Engine";
    RegretInfo.engineVersion = VK_MAKE_VERSION(0, 1,0);
    RegretInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &RegretInfo;
    createInfo.enabledExtensionCount = glfwExtentionCount;
    createInfo.ppEnabledExtensionNames = glfwExtentions;
    createInfo.enabledLayerCount = 0;
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    if(enableVkValidationLayers){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
    if(res != VkResult::VK_SUCCESS){
       ERROR("Failed to create vkInstance.  ERROR: ", res);
       return RG_Result::RG_INSTANCE_CREATE_ERR;
    }
    return RG_Result::RG_SUCCESS;
}

int rateDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int score = 0;
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        score += 1000;
    }
    score += deviceProperties.limits.maxImageDimension2D;
    if(!deviceFeatures.geometryShader){
        return 0;
    }
    return score;
}

//aka findQueueFamilies
QueueFamilyindices REGRET::RG_Vulkan::QueueFamilySupport(VkPhysicalDevice candidate)
{
    QueueFamilyindices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queueFamilyCount, nullptr);

    array<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queueFamilyCount, queueFamilies.data());
    queueFamilies.resize(queueFamilyCount);
    VkBool32 presentSupport = false;
    int i = 0;
    for(const auto& queueFamilie : queueFamilies){
        if(queueFamilie.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        };
        vkGetPhysicalDeviceSurfaceSupportKHR(candidate,i,surface,&presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        };
        if(indices.isComplete()){
            break;
        }
        i++;
    }
    return indices;
}

bool REGRET::RG_Vulkan::isDeviceSuitable(VkPhysicalDevice candidate)
{
    QueueFamilyindices indices = QueueFamilySupport(candidate);
    bool extentionsSupported = checkDeviceExtentionSupport(candidate);
    bool swapChainAdequate = false;
    if(extentionsSupported){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(candidate);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.isComplete() && extentionsSupported && swapChainAdequate;
}

REGRET::RG_Result REGRET::RG_Vulkan::SelectPhysicalDevice()
{

    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);
    if(deviceCount == 0){
        ERROR("No physical Device supporting Vulkan!");
        ERROR("Maybe use directX 11/12 ?"); //TODO: directX 11/12
        return RG_Result::RG_MISSING_PHYSICAL_DEVICE_SUPPORT;
    }

    array<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    devices.resize(deviceCount);
    std::multimap<int,VkPhysicalDevice> candidates;
    for(const auto& device : devices){
        if(isDeviceSuitable(device)){
            int score = rateDevice(device);
            candidates.insert(std::make_pair(score, device));
        }
    }
    if(candidates.rbegin()->first > 0 && candidates.rbegin()->second != VK_NULL_HANDLE){
        physicalDevice = candidates.rbegin()->second;
    }
    else{
        ERROR("Failed to find suitable GPU! All scores = 0.");
        return RG_Result::RG_MISSING_PHYSICAL_DEVICE_SUPPORT;
    };

    return RG_Result::RG_SUCCESS;
};

REGRET::RG_Result REGRET::RG_Vulkan::CreateLogicalDevice()
{
    float queuePriority = 1.0f; // priority of this queue relative to other queues
    QueueFamilyindices indices = QueueFamilySupport(physicalDevice);
    array<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    for(u32 queueFamily : uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.add(queueCreateInfo);
    }
    VkPhysicalDeviceFeatures deviceFeatures{};
    //[[#TODO|TODO]]: queue device Features for [[queueFeatures|later]]

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if( enableVkValidationLayers ){
        createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }else{
        createInfo.enabledLayerCount = 0;
    };
    VkResult result = VkResult::VK_ERROR_INITIALIZATION_FAILED;
    result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if(result != VkResult::VK_SUCCESS){
        ERROR("Failed to create logical Device!");
        return RG_Result::RG_LOGICAL_DEVICE_ERR;
    }
    vkGetDeviceQueue(device,indices.graphicsFamily.value(),0,&graphicsQueue );
    vkGetDeviceQueue(device,indices.presentFamily.value(),0,&presentQueue );

    return RG_Result::RG_SUCCESS;
};

REGRET::RG_Result REGRET::RG_Vulkan::CreateSurface(GLFWwindow* window)
{
    VkResult result;
    result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if(result != VK_SUCCESS){
        ERROR("Failed to create Window Surface!");
        return RG_Result::RG_SURFACE_NOT_CREATED;
    }
    return RG_Result::RG_SUCCESS;
}

bool REGRET::RG_Vulkan::checkDeviceExtentionSupport(VkPhysicalDevice candidate){
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(candidate,nullptr, &extensionCount,nullptr);
    array<VkExtensionProperties> availableExtentions(extensionCount);
    vkEnumerateDeviceExtensionProperties(candidate,nullptr, &extensionCount,availableExtentions.data());
    availableExtentions.resize(extensionCount);
    std::set<std::string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());
    for(const auto& extension : availableExtentions){
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

SwapChainSupportDetails REGRET::RG_Vulkan::querySwapChainSupport(VkPhysicalDevice candidate){
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(candidate, surface, &details.capabilities);
    u32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(candidate, surface, &formatCount, nullptr);
    if(formatCount != 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(candidate, surface, &formatCount, details.formats.data());
    }

    u32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(candidate, surface, &presentModeCount, nullptr);
    if(presentModeCount != 0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(candidate, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void SwapChainChoose::chooseSwapSurfaceFormat(const array<VkSurfaceFormatKHR>& availableFormats){
    for(const auto& availableFormat : availableFormats){
        if( availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ){
            surfaceFormat = availableFormat;
            return;
        }
    }
    surfaceFormat = availableFormats[0];
};

void SwapChainChoose::chooseSwapPresentMode(const array<VkPresentModeKHR>& availablePresentModes){
    for(const auto& availablePresentMode : availablePresentModes){
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR){
            this->presentMode = availablePresentMode;
        }
    }
    this->presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChainChoose::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,GLFWwindow* window){
    if(capabilities.currentExtent.width != std::numeric_limits<u32>::max()){
        this->extent = capabilities.currentExtent;
    }
    else{
        int width,height;
        glfwGetFramebufferSize(window,&width, &height);
        VkExtent2D actualExtent = {
            static_cast<u32>(width),
            static_cast<u32>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        this->extent = actualExtent;
    }
}


REGRET::RG_Result REGRET::RG_Vulkan::CreateSwapChain(GLFWwindow* window){
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    SwapChainChoose swapChainInfo(window,swapChainSupport);
    u32 imageCount = swapChainSupport.capabilities.minImageCount+1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount){
        imageCount = swapChainSupport.capabilities.maxImageCount;
    };

    swapChainImageFormat = swapChainInfo.surfaceFormat.format;
    swapChainExtent      = swapChainInfo.extent;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = swapChainInfo.surfaceFormat.format;
    createInfo.imageColorSpace  = swapChainInfo.surfaceFormat.colorSpace;
    createInfo.imageExtent      = swapChainInfo.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyindices indices = QueueFamilySupport(physicalDevice);
    u32 QueueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily){
        createInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount    = 2;
        createInfo.pQueueFamilyIndices      = QueueFamilyIndices;
    }else{
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = swapChainInfo.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device,&createInfo,nullptr,&swapChain);
    if(result != VK_SUCCESS){
        ERROR("Couldnt create Swapchain!");
        return RG_Result::RG_SWAPCHAIN_CREATE_ERR;
    };
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::RG_Vulkan::CreateImageViews(){
    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0; i < swapChainImages.size(); i++){
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        createInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel    = 0;
        createInfo.subresourceRange.levelCount      = 1;
        createInfo.subresourceRange.baseArrayLayer  = 0;
        createInfo.subresourceRange.layerCount      = 1;

        if(vkCreateImageView(device,&createInfo,nullptr,&swapChainImageViews[i]) != VK_SUCCESS){
            ERROR("Failed to create image views!");
            return RG_Result::RG_SWAPCHAIN_IMAGE_VIEW_CREATE_ERR;
        }
    }
    return RG_Result::RG_SUCCESS;
};

REGRET::RG_Result REGRET::RG_Vulkan::CreateFramebuffer(){
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainImageViews.size(); i++ ){
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(device,&framebufferInfo,nullptr,&swapChainFramebuffers[i]) != VK_SUCCESS ){
            RG_ERR_BREAK("failed to create Framebuffer!");
        }
    }


    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::RG_Vulkan::CreateCommandPool(){
    QueueFamilyindices queueFamilyIndices = QueueFamilySupport(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(device,&poolInfo,nullptr,&commandPool) != VK_SUCCESS){
        RG_ERR_BREAK("failed to create Command Pool!");
    }

    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::RG_Vulkan::CreateCommandBuffer(){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(device,&allocInfo,&commandBuffer) != VK_SUCCESS){
        RG_ERR_BREAK("failed to create command buffers!")
    }

    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::RG_Vulkan::recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex){
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    if(vkBeginCommandBuffer(commandBuffer,&beginInfo) != VK_SUCCESS){
        RG_ERR_BREAK("failed to begin recording command buffer!")
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f,0.0f,0.0f,1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer,0,1,&viewport);

    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer,0,1,&scissor);

    vkCmdDraw(commandBuffer,3,1,0,0);

    vkCmdEndRenderPass(commandBuffer);
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        RG_ERR_BREAK("failed to record command buffer!")
    }

    return RG_Result::RG_SUCCESS;
}



//TODO: clean up for that shit
REGRET::RG_Result REGRET::RG_Vulkan::CreateSyncObjects(){
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if(
        vkCreateSemaphore(device,&semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device,&semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device,&fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS
    ){
        RG_ERR_BREAK("failed to create semaphores!");
    }
    return RG_Result::RG_SUCCESS;
};
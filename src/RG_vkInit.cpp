#include "main.h"
REGRET::RG_Result REGRET::context::Vulkan::CreateInstance(){

    uint32_t glfwExtentionCount = 0;
    const char** glfwExtentions;
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);
    std::vector<const char*> requiredExtensions;
    for(uint32_t i = 0; i < glfwExtentionCount; i++) {
        requiredExtensions.emplace_back(glfwExtentions[i]);
    }

    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

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
       ERROR("Failed to create vkInstance. Error: ", res);
       return RG_Result::RG_INSTANCE_CREATE_ERR;
    }
    return RG_Result::RG_SUCCESS;
}
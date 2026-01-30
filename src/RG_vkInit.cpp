#include "common/common.h"
#include "logging/log.h"
#include "main.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

REGRET::RG_Result REGRET::context::Vulkan::Debug(){
    #ifdef DEBUG
    enableVkValidationLayers = true;
        if(CheckValidationLayerSupport()!=RG_Result::RG_SUCCESS){
            throw std::runtime_error("validation layers requested, but not available!");
            return RG_Result::RG_Validation_ERR;
        }
    #endif
    return RG_Result::RG_SUCCESS;
}

REGRET::RG_Result REGRET::context::Vulkan::CreateInstance(){

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
QueueFamilyindices REGRET::context::Vulkan::QueueFamilySupport(VkPhysicalDevice candidate)
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

bool REGRET::context::Vulkan::isDeviceSuitable(VkPhysicalDevice candidate)
{
    QueueFamilyindices indices = QueueFamilySupport(candidate);
    return indices.graphicsFamily.has_value();
}

REGRET::RG_Result REGRET::context::Vulkan::SelectPhysicalDevice()
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

REGRET::RG_Result REGRET::context::Vulkan::CreateLogicalDevice()
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
    createInfo.enabledExtensionCount = 0;
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

REGRET::RG_Result REGRET::context::CreateSurface()
{
    VkResult result;
    result = glfwCreateWindowSurface(vulkan.getInstance(), window,nullptr,&vulkan.getSurface());
    if(result != VK_SUCCESS){
        ERROR("Failed to create Window Surface!");
        return RG_Result::RG_SURFACE_NOT_CREATED;
    }
    return RG_Result::RG_SUCCESS;
}

bool REGRET::context::Vulkan::checkDeviceExtentionSupport(){
    return true;
}
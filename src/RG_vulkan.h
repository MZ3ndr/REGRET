#include "common/common.h"
#include <vulkan/vulkan_core.h>


struct QueueFamilyindices{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;
    bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
struct QueueFamilys{
    QueueFamilyindices QueueFamilySupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    array<VkSurfaceFormatKHR> formats;
    array<VkPresentModeKHR> presentModes;
};


namespace REGRET {
    class RG_Vulkan{
    private:
        bool enableVkValidationLayers = false;
        const array<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const array<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        // Helper ----
        QueueFamilyindices QueueFamilySupport(VkPhysicalDevice candidate);
        bool isDeviceSuitable(VkPhysicalDevice candidate);
        bool checkDeviceExtentionSupport(VkPhysicalDevice candidate);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice candidate);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const array<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const array<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,GLFWwindow* window);
        //------------
        //TODO Vulkan internal cleanup
        VkDevice device;
        VkInstance instance{};
        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain;
        array<VkImage> swapChainImages;
        RG_Result CheckValidationLayerSupport();
        RG_Result CreateInstance();
        RG_Result SelectPhysicalDevice();
        RG_Result CreateLogicalDevice();
        RG_Result Debug();
        RG_Result CreateSurface(GLFWwindow* window);
        RG_Result CreateSwapChain(GLFWwindow* window);
        public:
        RG_Result VulkanInit(GLFWwindow* window);
        inline const VkInstance getInstance()const {return this->instance;};
        inline VkSurfaceKHR& getSurface() {return this->surface;};
        inline const VkDevice getDevice()const {return this->device;};
        inline const VkSwapchainKHR getSwapChain() const {return this->swapChain;};

    };


}



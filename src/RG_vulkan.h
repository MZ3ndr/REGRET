#include "common/common.h"
#include "logging/log.h"

#define VERT_SHADER_DIR "shaders/testTriangle_vert.spv"
#define FRAG_SHADER_DIR "shaders/testTriangle_frag.spv"


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

struct SwapChainChoose{
    void chooseSwapSurfaceFormat(const array<VkSurfaceFormatKHR>& availableFormats);
    void chooseSwapPresentMode(const array<VkPresentModeKHR>& availablePresentModes);
    void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,GLFWwindow* window);
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    SwapChainChoose(GLFWwindow* window,SwapChainSupportDetails &SupportDetails){
        chooseSwapSurfaceFormat(SupportDetails.formats);
        chooseSwapPresentMode(SupportDetails.presentModes);
        chooseSwapExtent(SupportDetails.capabilities, window);
    };
};

namespace REGRET {
    class RG_Vulkan{
        //Copy/Move could result in unexpected
        //issues because of _device and similar
        RG_Vulkan(const RG_Vulkan&) = delete;
        RG_Vulkan& operator=(const RG_Vulkan&) = delete;
        RG_Vulkan(RG_Vulkan&&) = delete;
        RG_Vulkan& operator=(RG_Vulkan&&) = delete;
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
        VkShaderModule createShaderModule(const std::vector<char>& code);
        //------------
        //TODO Vulkan internal cleanup
        VkDevice device;
        VkInstance instance{};
        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain;
        array<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        array<VkImageView> swapChainImageViews;
        VkRenderPass renderPass;
        VkPipelineLayout pipelinelayout;
        VkPipeline graphicsPipeline;
        array<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;
        //======================================
        RG_Result CheckValidationLayerSupport();
        RG_Result CreateInstance();
        RG_Result SelectPhysicalDevice();
        RG_Result CreateLogicalDevice();
        RG_Result Debug();
        RG_Result CreateSurface(GLFWwindow* window);
        RG_Result CreateSwapChain(GLFWwindow* window);
        RG_Result CreateImageViews();
        RG_Result CreateRenderPass();
        RG_Result CreateGraphicsPipeine();
        RG_Result CreateFramebuffer();
        RG_Result CreateCommandPool();
        RG_Result CreateCommandBuffer();
        RG_Result CreateSyncObjects();

    public:
        RG_Vulkan() = default;
        RG_Result VulkanInit(GLFWwindow* window);
        RG_Result recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);

        const VkDevice& _device = device;
        const VkInstance& _instance = instance;
        const VkSurfaceKHR& _surface = surface;
        const VkSwapchainKHR& _swapChain = swapChain;
        const array<VkImageView>& _swapChainImageViews = swapChainImageViews;
        const VkPipelineLayout& _pipelinelayout = pipelinelayout;
        const VkRenderPass& _renderPass = renderPass;
        const VkPipeline& _graphicsPipeline = graphicsPipeline;
        const array<VkFramebuffer>& _swapChainFramebuffers = swapChainFramebuffers;
        const VkCommandPool& _commandPool = commandPool;
        const VkCommandBuffer& _commandBuffer = commandBuffer;
        const VkQueue& _graphicsQueue = graphicsQueue;
        const VkQueue& _presentQueue = presentQueue;
        const VkSemaphore& _imageAvailableSemaphore = imageAvailableSemaphore;
        const VkSemaphore& _renderFinishedSemaphore = renderFinishedSemaphore;
        const VkFence& _inFlightFence = inFlightFence;

        //TODO better




    };


}



#pragma  once
#include "./common/common.h"
#include "./logging/log.h"

// Rendering
// Engine for
// Gloriously
// Raw
// Experiments in
// Technology

#define RG_check(res) if(res != RG_Result::RG_SUCCESS){return res;};
#define RG_mainERR(res) if(res != REGRET::RG_Result::RG_SUCCESS){ERROR("[REGRET] EXIT with Errorcode: ",res); return static_cast<std::underlying_type_t<REGRET::RG_Result>>(res);;}


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

namespace REGRET {
    static RG_Result res = RG_Result::RG_FAIL;
    inline int to_int(RG_Result r) {
        return static_cast<std::underlying_type_t<RG_Result>>(r);
    }

    class context {
        private:
            struct windowSettings{
                int height = 800;
                int width = 800;
                const char *title = "Regret";
                GLFWmonitor *monitor = nullptr;
                GLFWwindow *share = nullptr;
            }winSettings;
            struct Vulkan{
                private:
                    bool enableVkValidationLayers = false;
                    const array<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
                    const array<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

                    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
                    VkQueue graphicsQueue;
                    VkQueue presentQueue;
                    
                    QueueFamilyindices QueueFamilySupport(VkPhysicalDevice candidate);
                    bool isDeviceSuitable(VkPhysicalDevice candidate);
                    bool checkDeviceExtentionSupport();
                    //TODO Vulkan internal cleanup
                    VkDevice device;
                    VkInstance instance{};
                    VkSurfaceKHR surface;
                    public:
                    RG_Result CheckValidationLayerSupport();
                    RG_Result CreateInstance();
                    RG_Result SelectPhysicalDevice();
                    RG_Result CreateLogicalDevice();
                    RG_Result Debug();

                    inline const VkInstance getInstance()const {return this->instance;};
                    inline VkSurfaceKHR& getSurface() {return this->surface;};
                    inline const VkDevice getDevice()const {return this->device;};

            }vulkan;

            RG_Result CreateSurface();

            RG_Result VulkanInit();

            RG_Result WindowInit();

        public:
            GLFWwindow* window;
            RG_Result CleanUp();
            RG_Result init();

    };

};

//[[TODO|TODO]]:
//[[#queueFeatures|Queue Features]]



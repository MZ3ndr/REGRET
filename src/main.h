#pragma  once
#include "./common/common.h"
#include "./logging/log.h"

// Rendering
// Engine for
// Gloriously
// Raw
// Experiments in
// Technology

namespace REGRET {
    enum class RG_Result {
        RG_SUCCESS,
        RG_FAIL,
        RG_WINDOW_NOT_CREATED,
        RG_INSTANCE_CREATE_ERR,
        RG_Validation_ERR

    };


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
                bool enableVkValidationLayers = false;
                const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

                VkInstance instance{};

                RG_Result CheckValidationLayerSupport();
                RG_Result CreateInstance();
                RG_Result selectPhysicalDevice();
            }static vulkan;


            RG_Result VulkanInit();

            RG_Result WindowInit();

        public:
            GLFWwindow* window;
            RG_Result CleanUp();
            void init();
    };

}
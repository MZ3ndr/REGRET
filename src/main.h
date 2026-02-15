#pragma  once
#include "./common/common.h"
#include "./logging/log.h"
#include "RG_vulkan.h"

// Rendering
// Engine for
// Gloriously
// Raw
// Experiments in
// Technology

#define RG_check(res) if(res != RG_Result::RG_SUCCESS){return res;};
#define RG_mainERR(res) if(res != REGRET::RG_Result::RG_SUCCESS){ERROR("[REGRET] EXIT with Errorcode: ",res); return static_cast<std::underlying_type_t<REGRET::RG_Result>>(res);;}

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
            RG_Vulkan vulkan;
            RG_Result WindowInit();

        public:
            GLFWwindow* window;
            RG_Result CleanUp();
            RG_Result init();

    };

};

//[[TODO|TODO]]:
//[[#queueFeatures|Queue Features]]



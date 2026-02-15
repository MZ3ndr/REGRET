#pragma once
#if defined (_WIN32)
    static bool UsingWindows = true;
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif defined(__linux__)
    static bool UsingWindows = false;
    #define VK_USE_PLATFORM_WAYLAND_KHR
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #include <GLFW/glfw3native.h>
#endif
#include "templates.h"
#include <sys/types.h>
#include <type_traits>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <utility>
#include <vector>
#include <map>
#include <optional>
#include <stdexcept>
#include <set>
#include <algorithm>
#include <cstddef>
#include <limits>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

//7 von 32

namespace REGRET {
    enum class RG_Result {
        RG_FAIL = -1,
        RG_SUCCESS,
        RG_WINDOW_NOT_CREATED,
        RG_INSTANCE_CREATE_ERR,
        RG_Validation_ERR,
        RG_MISSING_PHYSICAL_DEVICE_SUPPORT,
        RG_LOGICAL_DEVICE_ERR,
        RG_SURFACE_NOT_CREATED,
        RG_SWAPCHAIN_CREATE_ERR,
    };
}




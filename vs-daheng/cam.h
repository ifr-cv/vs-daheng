#pragma once
#include "GxIAPI.h"
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>


/// @brief 将GX_STATUS转换为字符串
static inline std::string GX_STATUStoString(GX_STATUS stat) {
    switch (stat)
    {
#define ENUM_STR(v,desc) case GX_STATUS_LIST::v:return #v ", " desc;
        ENUM_STR(GX_STATUS_SUCCESS, "成功");
        ENUM_STR(GX_STATUS_ERROR, "不期望发生的未明确指明的内部错误");
        ENUM_STR(GX_STATUS_NOT_FOUND_TL, "找不到TL库");
        ENUM_STR(GX_STATUS_NOT_FOUND_DEVICE, "找不到设备");
        ENUM_STR(GX_STATUS_OFFLINE, "当前设备为掉线状态");
        ENUM_STR(GX_STATUS_INVALID_PARAMETER, "无效参数,一般是指针为NULL或输入的IP等参数格式无效");
        ENUM_STR(GX_STATUS_INVALID_HANDLE, "无效句柄");
        ENUM_STR(GX_STATUS_INVALID_CALL, "无效的接口调用,专指软件接口逻辑错误");
        ENUM_STR(GX_STATUS_INVALID_ACCESS, "功能当前不可访问或设备访问模式错误");
        ENUM_STR(GX_STATUS_NEED_MORE_BUFFER, "用户申请的buffer不足:读操作时用户输入buffersize小于实际需要");
        ENUM_STR(GX_STATUS_ERROR_TYPE, "用户使用的FeatureID类型错误，比如整型接口使用了浮点型的功能码");
        ENUM_STR(GX_STATUS_OUT_OF_RANGE, "用户写入的值越界");
        ENUM_STR(GX_STATUS_NOT_IMPLEMENTED, "当前不支持的功能");
        ENUM_STR(GX_STATUS_NOT_INIT_API, "没有调用初始化接口");
        ENUM_STR(GX_STATUS_TIMEOUT, "超时错误");
#undef ENUM_STR
    default:
        return "UNKNOWN, " + std::to_string(stat);
    }
}
constexpr const char* ExtractFileName(const char* path) {
    int idx = 0;
    for (int i = 0; path[i]; i++) {
        if (path[i] == '/' || path[i] == '\\') idx = i + 1;
    }
    return path + idx;
}


constexpr auto LOG_lvl = 1;
#define LOG_type(lvl, prefix, type, v, suffix)                                                                                                                \
    if constexpr (lvl >= LOG_lvl) do {                                                                                                                        \
            static constexpr const auto filename = ExtractFileName(__FILE__);                                                                                 \
            std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());                                                 \
            auto tm = *std::localtime(&currentTime);                                                                                                          \
            prefix << '[' << std::put_time(&tm, "%y%m%d-%H:%M:%S") << "] " type " " << filename << ":" << __func__ << ":" << __LINE__ << "  " << v << suffix; \
    } while (0)

#define LOG_DEBUG(v) LOG_type(0, std::cout, "[DEBUG]", v, std::endl)
#define LOG_INFO(v) LOG_type(1, std::cout, "[ INFO]", v, std::endl)
#define LOG_WARN(v) LOG_type(2, std::cout, "[ WARN]", v, std::endl)
#define LOG_ERROR(v) LOG_type(3, std::cout, "[ERROR]", v, std::endl)
#define LOG_FATAL(v) LOG_type(4, std::cout, "[FATAL]", v, std::endl)

/// @brief 输出Gx日志
/// @return 是否成功执行
template<bool exit_on_bad = false, bool force = false>
static bool logGxStatus(const std::string& str, GX_STATUS status) {
    if (!force && status == GX_STATUS_SUCCESS) {
        LOG_DEBUG(str << ": " << status << '(' << GX_STATUStoString(status) << ')');
        return true;
    }
    else if constexpr (exit_on_bad) {
        LOG_FATAL(str << ": " << status << '(' << GX_STATUStoString(status) << ')');
        exit(status == GX_STATUS_SUCCESS ? 1 : status);
    }
    else {
        LOG_WARN(str << ": " << status << '(' << GX_STATUStoString(status) << ')');
        return false;
    }
}

#define IFR_GX_CHECK(expr) logGxStatus<false>(#expr, expr)
#define IFR_GX_CHECK_E(expr) logGxStatus<true>(#expr, expr)


class Camera {
    GX_DEV_HANDLE m_hDevice = {};///< 相机句柄
    uint8_t m_pixelColorType;    ///< 像素颜色格式 (0~2bit = 格式, RG,GB,GR,BG), (3~7bit = 位深-1)
    int64_t m_nImageWidth, m_nImageHeight, m_bColorFilter;

public:

    /// @brief 初始化相机
    void initCamera();

    /// @brief 开启相机
    void startCamera();

protected:
    void onUpdate(GX_FRAME_CALLBACK_PARAM* pFrameData);
public:
    /// @brief 停止相机
    void stopCamera();

    /// @brief 关闭相机
    void closeCamera();

private:
    static void GxCallback(GX_FRAME_CALLBACK_PARAM* pFrameData) {
        reinterpret_cast<Camera*>(pFrameData->pUserParam)->onUpdate(pFrameData);
    }
};
#include "cam.h"
#include <thread>


void Camera::initCamera() {
    LOG_INFO("Initing Camera...");
    GX_OPEN_PARAM openParam;
    uint32_t nDeviceNum = 0;
    openParam.accessMode = GX_ACCESS_EXCLUSIVE;
    openParam.openMode = GX_OPEN_INDEX;
    openParam.pszContent = new char[2]{ '1', '\0' }; //会导致2字节的内存泄漏, 忽略
    // 初始化库
    IFR_GX_CHECK_E(GXInitLib());

    // 枚举设备列表
    auto emStatus = GXUpdateDeviceList(&nDeviceNum, 1000);
    if ((emStatus != GX_STATUS_SUCCESS) || (nDeviceNum <= 0)) {
        logGxStatus<true, true>("Can not found any camera", emStatus);
    }

    //打开设备
    IFR_GX_CHECK_E(GXOpenDevice(&openParam, &m_hDevice));

    // set_param_use_trigger();
    // set_param_fps();

    //设置采集模式连续采集
    IFR_GX_CHECK(GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS));
    IFR_GX_CHECK(GXSetInt(m_hDevice, GX_INT_ACQUISITION_SPEED_LEVEL, 1));
    IFR_GX_CHECK(GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF));
    //关闭自动曝光
    IFR_GX_CHECK(GXSetEnum(m_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF));


    IFR_GX_CHECK_E(GXGetInt(m_hDevice, GX_INT_SENSOR_WIDTH, &m_nImageWidth));  // 获取宽度
    IFR_GX_CHECK_E(GXGetInt(m_hDevice, GX_INT_SENSOR_HEIGHT, &m_nImageHeight));// 获取高度
    IFR_GX_CHECK_E(GXSetInt(m_hDevice, GX_INT_OFFSET_X, 0));
    IFR_GX_CHECK_E(GXSetInt(m_hDevice, GX_INT_OFFSET_Y, 0));
    IFR_GX_CHECK_E(GXSetInt(m_hDevice, GX_INT_WIDTH, m_nImageWidth));  // 获取宽度
    IFR_GX_CHECK_E(GXSetInt(m_hDevice, GX_INT_HEIGHT, m_nImageHeight));// 获取高度

    LOG_INFO("camera size ("<< m_nImageWidth<<"x"<< m_nImageHeight<<")");
    if (!(m_nImageHeight > 0 && m_nImageWidth > 0)) {
        logGxStatus<true, true>("Open camera fail", emStatus);
    }
    //判断相机是否支持bayer格式
    IFR_GX_CHECK_E(GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_bColorFilter));
    if (m_bColorFilter != GX_COLOR_FILTER_NONE) {
        int64_t m_nPixelColorFilter = 0;
        IFR_GX_CHECK_E(GXGetEnum(m_hDevice, GX_ENUM_PIXEL_SIZE, &m_nPixelColorFilter));
        if (m_nPixelColorFilter != GX_PIXEL_SIZE_BPP8) {
            LOG_FATAL("Non 8-bit pixel format not supported " << m_nPixelColorFilter);
            exit(-1);
        }
        m_pixelColorType = static_cast<uint8_t>(((m_nPixelColorFilter - 1) << 2) + (m_bColorFilter - 1));
        //GX_PIXEL_COLOR_FILTER_ENTRY
        //GX_PIXEL_SIZE_ENTRY
    }
    else {
        LOG_FATAL("Non Bayer format not supported");
        exit(-1);
    }

    //设置采集 buffer 个数
    IFR_GX_CHECK(GXSetAcqusitionBufferNumber(m_hDevice, 50));


    LOG_INFO("Success Init Camera");
}

void Camera::startCamera() {
    LOG_INFO("Starting Camera...");
    IFR_GX_CHECK_E(GXRegisterCaptureCallback(m_hDevice, this, GxCallback));
    IFR_GX_CHECK_E(GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START));
    LOG_INFO("Success Start Camera");
}

void Camera::stopCamera() {
    LOG_INFO("Stopping Camera...");
    IFR_GX_CHECK_E(GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP));
    IFR_GX_CHECK_E(GXUnregisterCaptureCallback(m_hDevice));
    LOG_INFO("Success Stop Camera");
}
void Camera::closeCamera() {
    LOG_INFO("Closing Camera...");
    IFR_GX_CHECK_E(GXCloseDevice(m_hDevice));
    IFR_GX_CHECK_E(GXCloseLib());
    LOG_INFO("Success Close Camera");
}

void Camera::onUpdate(GX_FRAME_CALLBACK_PARAM* data) {
    if (data->status != GX_FRAME_STATUS_SUCCESS)return;// 有可能有残帧, 跳过
    
    LOG_INFO("Frame_Data: " << data->nFrameID << ", "
        << data->nHeight << "*" << data->nWidth << ", "
        << data->nTimestamp
    );
}
  
int main() {
    Camera camera;
    camera.initCamera();
    camera.startCamera();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
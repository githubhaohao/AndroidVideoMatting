//
// Created by 字节流动 on 2022/1/7.
//

#ifndef NCNN_ANDROID_ROBUSTVIDEOMATTING_MAIN_BODYSEG_H
#define NCNN_ANDROID_ROBUSTVIDEOMATTING_MAIN_BODYSEG_H
#include <ImageDef.h>
#include <android/looper.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <android/asset_manager_jni.h>
#include <android/sensor.h>
#include "LogUtil.h"
#include "nanodet.h"

class BodySeg {
public:
    BodySeg(){}
    ~BodySeg(){}

    void Init(AAssetManager* mgr);

    void SetCameraOrt(int ort);

    void Process(NativeImage *pNV21, NativeImage *pOutI420, NativeImage *pOutMask);

    void UnInit();

private:
    void LoadModel(AAssetManager* mgr);

private:
    int camera_facing;
    int camera_orientation;
    mutable int accelerometer_orientation;
    ASensorManager* sensor_manager = nullptr;
    mutable ASensorEventQueue* sensor_event_queue = nullptr;
    const ASensor* accelerometer_sensor = nullptr;
    NanoDet* nanoDet = nullptr;
};


#endif //NCNN_ANDROID_ROBUSTVIDEOMATTING_MAIN_BODYSEG_H

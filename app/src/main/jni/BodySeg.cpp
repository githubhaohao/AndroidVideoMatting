//
// Created by 字节流动 on 2022/1/7.
//

#include <opencv2/imgproc/types_c.h>
#include "BodySeg.h"

void BodySeg::Init(AAssetManager *mgr) {
    LOGCATE("BodySeg::Init");
    accelerometer_orientation = 0;
    sensor_manager = ASensorManager_getInstance();
    accelerometer_sensor = ASensorManager_getDefaultSensor(sensor_manager, ASENSOR_TYPE_ACCELEROMETER);

    LoadModel(mgr);
}
static const int NDKCAMERAWINDOW_ID = 233;
void BodySeg::Process(NativeImage *pNV21, NativeImage *pOutI420, NativeImage *pOutMask) {

    // resolve orientation from camera_orientation and accelerometer_sensor
    int nv21_height = pNV21->height;
    int nv21_width  = pNV21->width;
    {
        if (!sensor_event_queue)
        {
            sensor_event_queue = ASensorManager_createEventQueue(sensor_manager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), NDKCAMERAWINDOW_ID, 0, 0);

            ASensorEventQueue_enableSensor(sensor_event_queue, accelerometer_sensor);
        }

        int id = ALooper_pollAll(0, 0, 0, 0);
        if (id == NDKCAMERAWINDOW_ID)
        {
            ASensorEvent e[8];
            ssize_t num_event = 0;
            while (ASensorEventQueue_hasEvents(sensor_event_queue) == 1)
            {
                num_event = ASensorEventQueue_getEvents(sensor_event_queue, e, 8);
                if (num_event < 0)
                    break;
            }

            if (num_event > 0)
            {
                float acceleration_x = e[num_event - 1].acceleration.x;
                float acceleration_y = e[num_event - 1].acceleration.y;
                float acceleration_z = e[num_event - 1].acceleration.z;
//                 __android_log_print(ANDROID_LOG_WARN, "NdkCameraWindow", "x = %f, y = %f, z = %f", x, y, z);

                if (acceleration_y > 7)
                {
                    accelerometer_orientation = 0;
                }
                if (acceleration_x < -7)
                {
                    accelerometer_orientation = 90;
                }
                if (acceleration_y < -7)
                {
                    accelerometer_orientation = 180;
                }
                if (acceleration_x > 7)
                {
                    accelerometer_orientation = 270;
                }
            }
        }
    }

    // roi crop and rotate nv21
    int nv21_roi_x = 0;
    int nv21_roi_y = 0;
    int nv21_roi_w = 0;
    int nv21_roi_h = 0;
    int roi_x = 0;
    int roi_y = 0;
    int roi_w = 0;
    int roi_h = 0;
    int rotate_type = 0;
    int render_w = 0;
    int render_h = 0;
    int render_rotate_type = 0;
    {
        int win_w = pNV21->height;//横着的图像
        int win_h = pNV21->width;

        if (accelerometer_orientation == 90 || accelerometer_orientation == 270)
        {
            std::swap(win_w, win_h);
        }

        const int final_orientation = (camera_orientation + accelerometer_orientation) % 360;

        if (final_orientation == 0 || final_orientation == 180)
        {
            if (win_w * nv21_height > win_h * nv21_width)
            {
                roi_w = nv21_width;
                roi_h = (nv21_width * win_h / win_w) / 2 * 2;
                roi_x = 0;
                roi_y = ((nv21_height - roi_h) / 2) / 2 * 2;
            }
            else
            {
                roi_h = nv21_height;
                roi_w = (nv21_height * win_w / win_h) / 2 * 2;
                roi_x = ((nv21_width - roi_w) / 2) / 2 * 2;
                roi_y = 0;
            }

            nv21_roi_x = roi_x;
            nv21_roi_y = roi_y;
            nv21_roi_w = roi_w;
            nv21_roi_h = roi_h;
        }
        if (final_orientation == 90 || final_orientation == 270)
        {
            if (win_w * nv21_width > win_h * nv21_height)
            {
                roi_w = nv21_height;
                roi_h = (nv21_height * win_h / win_w) / 2 * 2;
                roi_x = 0;
                roi_y = ((nv21_width - roi_h) / 2) / 2 * 2;
            }
            else
            {
                roi_h = nv21_width;
                roi_w = (nv21_width * win_w / win_h) / 2 * 2;
                roi_x = ((nv21_height - roi_w) / 2) / 2 * 2;
                roi_y = 0;
            }

            nv21_roi_x = roi_y;
            nv21_roi_y = roi_x;
            nv21_roi_w = roi_h;
            nv21_roi_h = roi_w;
        }

        if (camera_facing == 0)
        {
            if (camera_orientation == 0 && accelerometer_orientation == 0)
            {
                rotate_type = 2;
            }
            if (camera_orientation == 0 && accelerometer_orientation == 90)
            {
                rotate_type = 7;
            }
            if (camera_orientation == 0 && accelerometer_orientation == 180)
            {
                rotate_type = 4;
            }
            if (camera_orientation == 0 && accelerometer_orientation == 270)
            {
                rotate_type = 5;
            }
            if (camera_orientation == 90 && accelerometer_orientation == 0)
            {
                rotate_type = 5;
            }
            if (camera_orientation == 90 && accelerometer_orientation == 90)
            {
                rotate_type = 2;
            }
            if (camera_orientation == 90 && accelerometer_orientation == 180)
            {
                rotate_type = 7;
            }
            if (camera_orientation == 90 && accelerometer_orientation == 270)
            {
                rotate_type = 4;
            }
            if (camera_orientation == 180 && accelerometer_orientation == 0)
            {
                rotate_type = 4;
            }
            if (camera_orientation == 180 && accelerometer_orientation == 90)
            {
                rotate_type = 5;
            }
            if (camera_orientation == 180 && accelerometer_orientation == 180)
            {
                rotate_type = 2;
            }
            if (camera_orientation == 180 && accelerometer_orientation == 270)
            {
                rotate_type = 7;
            }
            if (camera_orientation == 270 && accelerometer_orientation == 0)
            {
                rotate_type = 7;
            }
            if (camera_orientation == 270 && accelerometer_orientation == 90)
            {
                rotate_type = 4;
            }
            if (camera_orientation == 270 && accelerometer_orientation == 180)
            {
                rotate_type = 5;
            }
            if (camera_orientation == 270 && accelerometer_orientation == 270)
            {
                rotate_type = 2;
            }
        }
        else
        {
            if (final_orientation == 0)
            {
                rotate_type = 1;
            }
            if (final_orientation == 90)
            {
                rotate_type = 6;
            }
            if (final_orientation == 180)
            {
                rotate_type = 3;
            }
            if (final_orientation == 270)
            {
                rotate_type = 8;
            }
        }

        if (accelerometer_orientation == 0)
        {
            render_w = roi_w;
            render_h = roi_h;
            render_rotate_type = 1;
        }
        if (accelerometer_orientation == 90)
        {
            render_w = roi_h;
            render_h = roi_w;
            render_rotate_type = 8;
        }
        if (accelerometer_orientation == 180)
        {
            render_w = roi_w;
            render_h = roi_h;
            render_rotate_type = 3;
        }
        if (accelerometer_orientation == 270)
        {
            render_w = roi_h;
            render_h = roi_w;
            render_rotate_type = 6;
        }
    }

    // crop and rotate nv21
    cv::Mat nv21_croprotated(roi_h + roi_h / 2, roi_w, CV_8UC1);
    {
        const unsigned char* srcY = pNV21->ppPlane[0] + nv21_roi_y * nv21_width + nv21_roi_x;
        unsigned char* dstY = nv21_croprotated.data;
        ncnn::kanna_rotate_c1(srcY, nv21_roi_w, nv21_roi_h, nv21_width, dstY, roi_w, roi_h, roi_w, rotate_type);

        const unsigned char* srcUV = pNV21->ppPlane[0] + nv21_width * nv21_height + nv21_roi_y * nv21_width / 2 + nv21_roi_x;
        unsigned char* dstUV = nv21_croprotated.data + roi_w * roi_h;
        ncnn::kanna_rotate_c2(srcUV, nv21_roi_w / 2, nv21_roi_h / 2, nv21_width, dstUV, roi_w / 2, roi_h / 2, roi_w, rotate_type);
    }
    LOGCATE("BodySeg::Process accelerometer_orientation=%d, camera_orientation=%d", accelerometer_orientation, camera_orientation);

    // nv21_croprotated to rgb
    cv::Mat rgb(roi_h, roi_w, CV_8UC3);
    cv::Mat mask(roi_h, roi_w, CV_8UC1);
    ncnn::yuv420sp2rgb(nv21_croprotated.data, roi_w, roi_h, rgb.data);
    LOGCATE("BodySeg::Process nv21[w,h]=[%d, %d], render[w,h]=[%d, %d]", nv21_width, nv21_height, render_w, render_h);

    //on_image_render(rgb);
    if(nanoDet != nullptr) {
        //std::vector<Object> objects;
        //nanoDet->detect(rgb, objects);
        nanoDet->draw(rgb, mask);
    }
    // rotate to native window orientation
    cv::Mat rgb_render(render_h, render_w, CV_8UC3);
    cv::Mat mask_render(render_h, render_w, CV_8UC1);
    ncnn::kanna_rotate_c3(rgb.data, roi_w, roi_h, rgb_render.data, render_w, render_h, render_rotate_type);
    ncnn::kanna_rotate_c1(mask.data, roi_w, roi_h, mask_render.data, render_w, render_h, render_rotate_type);

    memcpy(pOutMask->ppPlane[0], mask_render.data, render_h * render_w);

    NativeImage image;
    image.format = IMAGE_FORMAT_I420;
    image.width  = render_w;
    image.height = render_h;
    cv::Mat nv21Img;
    cv::cvtColor(rgb_render, nv21Img, CV_RGB2YUV_I420);

    image.ppPlane[0] = nv21Img.data;
    image.ppPlane[1] = image.ppPlane[0] + image.width * image.height;
    image.ppPlane[2] = image.ppPlane[1] + image.width * image.height / 4;
    image.pLineSize[0] = image.width;
    image.pLineSize[1] = image.width / 2;
    image.pLineSize[2] = image.width / 2;

    NativeImageUtil::CopyNativeImage(&image, pOutI420);
    //NativeImageUtil::DumpNativeImage(&image, "/sdcard", "BodySeg");
}

void BodySeg::UnInit() {
    LOGCATE("BodySeg::UnInit");
    if (accelerometer_sensor)
    {
        ASensorEventQueue_disableSensor(sensor_event_queue, accelerometer_sensor);
        accelerometer_sensor = nullptr;
    }

    if (sensor_event_queue)
    {
        ASensorManager_destroyEventQueue(sensor_manager, sensor_event_queue);
        sensor_event_queue = nullptr;
    }

    if(nanoDet != nullptr) {
        delete nanoDet;
        nanoDet = nullptr;
    }

}

void BodySeg::SetCameraOrt(int ort) {
    LOGCATE("BodySeg::SetCameraOrt ort=%d", ort);
    camera_orientation = ort;
    if(ort == 270) {
        camera_facing = 0;
    }
    else
    {
        camera_facing = 1;
    }

}

void BodySeg::LoadModel(AAssetManager *mgr) {
    LOGCATE("BodySeg::LoadModel mgr=%p", mgr);
    const char* modeltypes[] =
            {
                    "rvm-512",
                    "rvm-640",
            };

    const int target_sizes[] =
            {
                    512,
                    640,
            };

    const float mean_vals[][3] =
            {
                    {123.675f, 116.28f,  103.53f},
                    {123.675f, 116.28f,  103.53f},
            };

    const float norm_vals[][3] =
            {
                    {0.01712475f, 0.0175f, 0.01742919f},
                    {0.01712475f, 0.0175f, 0.01742919f},
            };

    int modelid = 0;
    int cpugpu = 0;
    const char* modeltype = modeltypes[(int)modelid];
    int target_size = target_sizes[(int)modelid];
    bool use_gpu = (int)cpugpu == 1;
    if(nanoDet == nullptr) {
        nanoDet = new NanoDet();
        nanoDet->load(mgr, modeltype, target_size, mean_vals[(int)modelid], norm_vals[(int)modelid], use_gpu);
    }
}


//
// Created by 字节流动 on 2022/1/5.
//
#include <cstdio>
#include <cstring>
#include <android/asset_manager_jni.h>
#include "util/LogUtil.h"
#include "util/ImageDef.h"
#include "NativeContext.h"
#include "jni.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1CreateContext(JNIEnv *env,
                                                                             jobject thiz) {
    NativeContext::CreateContext(env, thiz);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1DestroyContext(JNIEnv *env,
                                                                              jobject thiz) {
    NativeContext::DeleteContext(env, thiz);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1Init(JNIEnv *env, jobject thiz, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) return pContext->Init(mgr);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1UnInit(JNIEnv *env, jobject thiz) {
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) return pContext->UnInit();
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1OnPreviewFrame(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jint format,
                                                                              jbyteArray data,
                                                                              jint width,
                                                                              jint height,
                                                                              jbyteArray outData) {
    int len = env->GetArrayLength (data);
    unsigned char* buf = new unsigned char[len];
    env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte*>(buf));
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    NativeImage *pSegResult;
    if(pContext) pContext->OnPreviewFrame(format, buf, width, height, &pSegResult);
    delete[] buf;
    env->DeleteLocalRef(data);

    jbyte* outBytes = env->GetByteArrayElements(outData, 0);
    jsize outBytesSize = env->GetArrayLength(outData);
    env->SetByteArrayRegion(outData, 0, outBytesSize,reinterpret_cast<const jbyte *>(pSegResult->ppPlane[0]));
    env->ReleaseByteArrayElements(outData, outBytes, 0);
    //NativeImageUtil::DumpNativeImage(pSegResult, "/sdcard", "SegResult");

//    NativeImage image;
//    image.format = format;
//    image.width  = width;
//    image.height = height;
//    image.ppPlane[0] = buf;
//    image.ppPlane[1] = buf + width * height;
//    NativeImageUtil::DumpNativeImage(&image, "/sdcard", "camera");
//    delete[] buf;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1SetTransformMatrix(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jfloat translate_x,
                                                                                  jfloat translate_y,
                                                                                  jfloat scale_x,
                                                                                  jfloat scale_y,
                                                                                  jint degree,
                                                                                  jint mirror) {
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->SetTransformMatrix(translate_x, translate_y, scale_x, scale_y, degree, mirror);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1OnSurfaceCreated(JNIEnv *env,
                                                                                jobject thiz) {
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->OnSurfaceCreated();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1OnSurfaceChanged(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jint width,
                                                                                jint height) {
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->OnSurfaceChanged(width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1OnDrawFrame(JNIEnv *env,
                                                                           jobject thiz) {
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->OnDrawFrame();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1SetFilterData(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jint index,
                                                                             jint format,
                                                                             jint width,
                                                                             jint height,
                                                                             jbyteArray bytes) {
    int len = env->GetArrayLength (bytes);
    uint8_t* buf = new uint8_t[len];
    env->GetByteArrayRegion(bytes, 0, len, reinterpret_cast<jbyte*>(buf));
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->SetLUTImage(index, format, width, height, buf);
    delete[] buf;
    env->DeleteLocalRef(bytes);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1SetFragShader(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jint index,
                                                                             jstring str) {
    int length = env->GetStringUTFLength(str);
    const char* cStr = env->GetStringUTFChars(str, JNI_FALSE);
    char *buf = static_cast<char *>(malloc(length + 1));
    memcpy(buf, cStr, length + 1);
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->SetFragShader(index, buf, length + 1);
    free(buf);
    env->ReleaseStringUTFChars(str, cStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_ncnnbodyseg_media_CameraNativeContext_native_1ReadDataFromTextureId(JNIEnv *env,
                                                                                     jobject thiz,
                                                                                     jint oes_texture_id,
                                                                                     jint texWidth,
                                                                                     jint texHeight,
                                                                                     jbyteArray outData) {
    RGB2NV21Helper *rgb2Nv21Helper = new RGB2NV21Helper();
    //测试从OES纹理中读取数据
    NativeImage image;
    image.format = IMAGE_FORMAT_NV21;
    image.width  = texWidth;
    image.height = texHeight;
    NativeImageUtil::AllocNativeImage(&image);
    rgb2Nv21Helper->Init(image.width, image.height);
    rgb2Nv21Helper->Draw(oes_texture_id, &image);
    //保存到 /sdcard/IMG_480x640_readOESTexture_0.NV21
    NativeImageUtil::DumpNativeImage(&image, "/sdcard", "readOESTexture");

    //读完之后送去做分割渲染
    NativeImage *pSegResult;
    NativeContext *pContext = NativeContext::GetContext(env, thiz);
    if(pContext) pContext->OnPreviewFrame(IMAGE_FORMAT_NV21, image.ppPlane[0], image.width, image.height, &pSegResult);

    NativeImageUtil::FreeNativeImage(&image);
    rgb2Nv21Helper->UnInit();
    delete rgb2Nv21Helper;

    jbyte* outBytes = env->GetByteArrayElements(outData, 0);
    jsize outBytesSize = env->GetArrayLength(outData);
    env->SetByteArrayRegion(outData, 0, outBytesSize,reinterpret_cast<const jbyte *>(pSegResult->ppPlane[0]));
    env->ReleaseByteArrayElements(outData, outBytes, 0);


}
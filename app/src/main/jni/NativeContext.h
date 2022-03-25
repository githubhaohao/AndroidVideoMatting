/**
 *
 * Created by 公众号：字节流动 on 2021/3/16.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef OPENGLCAMERA2_BYTEFLOWRENDERCONTEXT_H
#define OPENGLCAMERA2_BYTEFLOWRENDERCONTEXT_H

#include <cstdint>
#include <jni.h>
#include <net.h>
#include "GLCameraRender.h"

#define RECORDER_TYPE_SINGLE_VIDEO  0 //仅录制视频
#define RECORDER_TYPE_SINGLE_AUDIO  1 //仅录制音频
#define RECORDER_TYPE_AV            2 //同时录制音频和视频,打包成 MP4 文件

class NativeContext
{
public:
	NativeContext();

	~NativeContext();

	static void CreateContext(JNIEnv *env, jobject instance);

	static void StoreContext(JNIEnv *env, jobject instance, NativeContext *pContext);

	static void DeleteContext(JNIEnv *env, jobject instance);

	static NativeContext* GetContext(JNIEnv *env, jobject instance);

	static void OnGLRenderFrame(void *ctx, NativeImage * pImage);

	int Init(AAssetManager* mgr);

	int UnInit();

	void OnPreviewFrame(int format, uint8_t *pBuffer, int width, int height, NativeImage **ppSegResult);

	void SetTransformMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree, int mirror);

	//OpenGL callback
	void OnSurfaceCreated();

	void OnSurfaceChanged(int width, int height);

	void OnDrawFrame();

	void ReadImgFromOesTexture(int oesTextureId, NativeImage *pOutImg);

	//加载滤镜素材图像
	void SetLUTImage(int index, int format, int width, int height, uint8_t *pData);

	//加载着色器脚本
	void SetFragShader(int index, char *pShaderStr, int strSize);

private:
	static jfieldID s_ContextHandle;
	TransformMatrix m_transformMatrix;
	mutex m_mutex;
};


#endif //OPENGLCAMERA2_BYTEFLOWRENDERCONTEXT_H

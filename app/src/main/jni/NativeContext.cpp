/**
 *
 * Created by 公众号：字节流动 on 2021/3/16.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <LogUtil.h>
#include <ImageDef.h>
#include "NativeContext.h"

jfieldID NativeContext::s_ContextHandle = 0L;

NativeContext::NativeContext() {
	GLCameraRender::GetInstance();
}

NativeContext::~NativeContext()
{
	GLCameraRender::ReleaseInstance();
}

void NativeContext::CreateContext(JNIEnv *env, jobject instance)
{
	LOGCATE("NativeContext::CreateContext");
	NativeContext *pContext = new NativeContext();
	StoreContext(env, instance, pContext);
}

void NativeContext::StoreContext(JNIEnv *env, jobject instance, NativeContext *pContext)
{
	LOGCATE("NativeContext::StoreContext");
	jclass cls = env->GetObjectClass(instance);
	if (cls == NULL)
	{
		LOGCATE("NativeContext::StoreContext cls == NULL");
		return;
	}

	s_ContextHandle = env->GetFieldID(cls, "mNativeContextHandle", "J");
	if (s_ContextHandle == NULL)
	{
		LOGCATE("NativeContext::StoreContext s_ContextHandle == NULL");
		return;
	}

	env->SetLongField(instance, s_ContextHandle, reinterpret_cast<jlong>(pContext));

}


void NativeContext::DeleteContext(JNIEnv *env, jobject instance)
{
	LOGCATE("NativeContext::DeleteContext");
	if (s_ContextHandle == NULL)
	{
		LOGCATE("NativeContext::DeleteContext Could not find render context.");
		return;
	}

	NativeContext *pContext = reinterpret_cast<NativeContext *>(env->GetLongField(
			instance, s_ContextHandle));
	if (pContext)
	{
		delete pContext;
	}
	env->SetLongField(instance, s_ContextHandle, 0L);
}

NativeContext *NativeContext::GetContext(JNIEnv *env, jobject instance)
{
	LOGCATE("NativeContext::GetContext");

	if (s_ContextHandle == NULL)
	{
		LOGCATE("NativeContext::GetContext Could not find render context.");
		return NULL;
	}

	NativeContext *pContext = reinterpret_cast<NativeContext *>(env->GetLongField(
			instance, s_ContextHandle));
	return pContext;
}

int NativeContext::Init(AAssetManager* mgr)
{
	GLCameraRender::GetInstance()->Init(mgr);
	GLCameraRender::GetInstance()->SetRenderCallback(this, OnGLRenderFrame);
	return 0;
}

int NativeContext::UnInit()
{
	GLCameraRender::GetInstance()->UnInit();

	return 0;
}

void NativeContext::OnPreviewFrame(int format, uint8_t *pBuffer, int width, int height, NativeImage **ppSegResult)
{
	LOGCATE("NativeContext::UpdateFrame format=%d, width=%d, height=%d, pData=%p",
			format, width, height, pBuffer);
	NativeImage nativeImage;
	nativeImage.format = format;
	nativeImage.width = width;
	nativeImage.height = height;
	nativeImage.ppPlane[0] = pBuffer;

    switch (format)
	{
		case IMAGE_FORMAT_NV12:
		case IMAGE_FORMAT_NV21:
			nativeImage.ppPlane[1] = nativeImage.ppPlane[0] + width * height;
			nativeImage.pLineSize[0] = width;
            nativeImage.pLineSize[1] = width;
			break;
		case IMAGE_FORMAT_I420:
			nativeImage.ppPlane[1] = nativeImage.ppPlane[0] + width * height;
			nativeImage.ppPlane[2] = nativeImage.ppPlane[1] + width * height / 4;
            nativeImage.pLineSize[0] = width;
            nativeImage.pLineSize[1] = width / 2;
            nativeImage.pLineSize[2] = width / 2;
			break;
		default:
			break;
	}

    GLCameraRender::GetInstance()->RenderVideoFrame(&nativeImage, ppSegResult);
}

void NativeContext::SetTransformMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree, int mirror)
{
	m_transformMatrix.translateX = translateX;
	m_transformMatrix.translateY = translateY;
	m_transformMatrix.scaleX = scaleX;
	m_transformMatrix.scaleY = scaleY;
	m_transformMatrix.degree = degree;
	m_transformMatrix.mirror = mirror;
	GLCameraRender::GetInstance()->UpdateMVPMatrix(&m_transformMatrix);
}

void NativeContext::OnSurfaceCreated()
{
	GLCameraRender::GetInstance()->OnSurfaceCreated();
}

void NativeContext::OnSurfaceChanged(int width, int height)
{
	GLCameraRender::GetInstance()->OnSurfaceChanged(width, height);
}

void NativeContext::OnDrawFrame()
{
	GLCameraRender::GetInstance()->OnDrawFrame();
}

void NativeContext::OnGLRenderFrame(void *ctx, NativeImage *pImage) {
	LOGCATE("NativeContext::OnGLRenderFrame ctx=%p, pImage=%p", ctx, pImage);
	NativeContext *context = static_cast<NativeContext *>(ctx);
	std::unique_lock<std::mutex> lock(context->m_mutex);
//	if(context->m_pVideoRecorder != nullptr)
//		context->m_pVideoRecorder->OnFrame2Encode(pImage);
//
//	if(context->m_pAVRecorder != nullptr)
//		context->m_pAVRecorder->OnFrame2Encode(pImage);
}

void
NativeContext::SetLUTImage(int index, int format, int width, int height, uint8_t *pData) {
	LOGCATE("NativeContext::SetLUTImage index=%d, format=%d, width=%d, height=%d, pData=%p",
			index, format, width, height, pData);
	NativeImage nativeImage;
	nativeImage.format = format;
	nativeImage.width = width;
	nativeImage.height = height;
	nativeImage.ppPlane[0] = pData;
	nativeImage.pLineSize[0] = width * 4; //RGBA

	//GLCameraRender::GetInstance()->SetLUTImage(index, &nativeImage);
}

void NativeContext::SetFragShader(int index, char *pShaderStr, int strSize) {
	//GLCameraRender::GetInstance()->SetFragShaderStr(index, pShaderStr, strSize);
}

void NativeContext::ReadImgFromOesTexture(int oesTextureId, NativeImage *pOutImg) {
	LOGCATE("NativeContext::ReadImgFromOesTexture [oesTextureId,pOutImg] = [%d, %p]", oesTextureId, pOutImg);
	GLCameraRender::GetInstance()->ReadImgFromTexture(oesTextureId, pOutImg);
}



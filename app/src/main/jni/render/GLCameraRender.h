/**
 *
 * Created by 公众号：字节流动 on 2022/3/12.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef LEARNFFMPEG_MASTER_GLCAMERARENDER_H
#define LEARNFFMPEG_MASTER_GLCAMERARENDER_H
#include <thread>
#include <ImageDef.h>
#include <GLES3/gl3.h>
#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include <vec2.hpp>
#include <BaseGLRender.h>
#include <vector>
#include "../BodySeg.h"

using namespace glm;
using namespace std;

#define MATH_PI 3.1415926535897932384626433832802
#define TEXTURE_NUM 3

#define SHADER_INDEX_ORIGIN  0
#define SHADER_INDEX_DMESH   1
#define SHADER_INDEX_GHOST   2
#define SHADER_INDEX_CIRCLE  3
#define SHADER_INDEX_ASCII   4
#define SHADER_INDEX_SPLIT   5
#define SHADER_INDEX_MATTE   6
#define SHADER_INDEX_LUT_A   7
#define SHADER_INDEX_LUT_B   8
#define SHADER_INDEX_LUT_C   9
#define SHADER_INDEX_NE      10  //Negative effect

typedef void (*OnRenderFrameCallback)(void*, NativeImage*);

class GLCameraRender: public BaseGLRender {
public:
    //初始化预览帧的宽高
    virtual void Init(AAssetManager* mgr);
    //渲染一帧视频
    virtual void RenderVideoFrame(NativeImage *pImage, NativeImage **ppSegResult);
    virtual void UnInit();

    //GLSurfaceView 的三个回调
    virtual void OnSurfaceCreated();
    virtual void OnSurfaceChanged(int w, int h);
    virtual void OnDrawFrame();

    static GLCameraRender *GetInstance();
    static void ReleaseInstance();

    //更新变换矩阵，Camera预览帧需要进行旋转
    virtual void UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY);
    virtual void UpdateMVPMatrix(TransformMatrix * pTransformMatrix);
    virtual void SetTouchLoc(float touchX, float touchY) {
        m_TouchXY.x = touchX / m_ScreenSize.x;
        m_TouchXY.y = touchY / m_ScreenSize.y;
    }

    //添加好滤镜之后，视频帧的回调，然后将带有滤镜的视频帧放入编码队列
    void SetRenderCallback(void *ctx, OnRenderFrameCallback callback) {
        m_CallbackContext = ctx;
        m_RenderFrameCallback = callback;
    }

private:
    GLCameraRender();
    virtual ~GLCameraRender();
    bool CreateFrameBufferObj();

    void GetRenderFrameFromFBO();

    static std::mutex m_Mutex;
    static GLCameraRender* s_Instance;
    GLuint m_ProgramObj = GL_NONE;
    GLuint m_FBOProgramObj = GL_NONE;
    GLuint m_TextureIds[TEXTURE_NUM];
    GLuint m_VaoId = GL_NONE;
    GLuint m_VboIds[3];
    GLuint m_FboTextureId = GL_NONE;
    GLuint m_FboId = GL_NONE;

    NativeImage m_RenderImage;
    glm::mat4 m_MVPMatrix;
    TransformMatrix m_transformMatrix;

    int m_FrameIndex;
    vec2 m_TouchXY;
    vec2 m_ScreenSize;

    OnRenderFrameCallback m_RenderFrameCallback = nullptr;
    void *m_CallbackContext = nullptr;

    BodySeg *m_BodySeg = nullptr;
    NativeImage m_MaskImage;
    GLuint m_MaskTextureId = GL_NONE;
};


#endif //LEARNFFMPEG_MASTER_GLCAMERARENDER_H

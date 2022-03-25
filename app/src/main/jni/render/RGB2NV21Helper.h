
#ifndef NDK_OPENGLES_3_0_RGB2NV21SAMPLE_H
#define NDK_OPENGLES_3_0_RGB2NV21SAMPLE_H

#include "ImageDef.h"

class RGB2NV21Helper
{
public:
	RGB2NV21Helper();
	~RGB2NV21Helper();
	void Init(int width, int height);
	void Draw(int rgbaTextureId, NativeImage *pOutImg);
	void UnInit();
private:
	bool CreateFrameBufferObj();

	GLuint m_FboTextureId;
	GLuint m_FboId;
	GLuint m_VaoIds[2] = {GL_NONE};
	GLuint m_VboIds[4] = {GL_NONE};
	GLuint m_FboProgramObj;
	GLuint m_FboVertexShader;
	GLuint m_FboFragmentShader;
	GLint m_FboSamplerLoc;
	glm::vec2 m_ImgSize;
};


#endif //NDK_OPENGLES_3_0_RGB2NV21SAMPLE_H


#include <AGL/agl.h>

class SkEGLContext {
public:
	SkEGLContext();
	~SkEGLContext();

	bool init(int width, int height);

private:
	AGLContext	fAGLContext;
};

SkEGLContext::SkEGLContext() : fAGLContext(NULL) {
}

SkEGLContext::~SkEGLContext() {
	if (fAGLContext) {
		aglDestroyContext(fAGLContext);
	}
}

bool SkEGLContext::init(int width, int height) {
	GLint major, minor;
	AGLContext ctx;

	aglGetVersion(&major, &minor);
	SkDebugf("---- agl version %d %d\n", major, minor);

	const GLint pixelAttrs[] = {
		AGL_RGBA,
		AGL_STENCIL_SIZE, 8,
/*
		AGL_SAMPLE_BUFFERS_ARB, 1,
		AGL_MULTISAMPLE,
		AGL_SAMPLES_ARB, 2,
*/
		AGL_ACCELERATED,
		AGL_NONE
	};
	AGLPixelFormat format = aglChoosePixelFormat(NULL, 0, pixelAttrs);
	//AGLPixelFormat format = aglCreatePixelFormat(pixelAttrs);
	SkDebugf("----- agl format %p\n", format);
	ctx = aglCreateContext(format, NULL);
	SkDebugf("----- agl context %p\n", ctx);
	aglDestroyPixelFormat(format);

	SkDebugf("---- sizeof aglcontext %d\n", sizeof(AGLContext));
/*
	static const GLint interval = 1;
	aglSetInteger(ctx, AGL_SWAP_INTERVAL, &interval);
*/

	aglSetCurrentContext(ctx);
	fAGLContext = ctx;

	// Now create our FBO render target

	GLuint fboID;
	GLuint cbID;
	glGenFramebuffersEXT(1, &fboID);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
	glGenRenderbuffers(1, &cbID);
	glBindRenderbuffer(GL_RENDERBUFFER, cbID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cbID);
	return true;
}

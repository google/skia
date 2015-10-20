#include <math.h>
#include <jni.h>
#include <android/bitmap.h>

#include "SKString.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "renderer.h"
#include "../gpu/GRContext.h"

#include "../gpu/gl/SkGlContext.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include "SkTime.h"
#include "logger.h"


SkAutoTUnref<GrContext> g_Context;
SkPath* fPath;
GrRenderTarget*         fCurRenderTarget;

GLuint fRenderbuffer;
GLuint fStencilbuffer;
GLuint fFramebuffer;
GLuint fTexture;
GLint fWidth;
GLint fHeight;

GLuint mPositionHandle;
GLuint mColorHandle;
GLuint mVertexBufferID;
GLuint mSamplerLoc;

GLuint vertexCount = 4;
GLuint vertexStride = 3 * 4; // 4 bytes per vertex
GLuint mProgram;
static float squareCoords[] = { -1.0f, 1.0f, 0.0f,   // top left
		-1.0f, -1.0f, 0.0f,   // bottom left
		1.0f, 1.0f, 0.0f, // top right

		1.0f, -1.0f, 0.0f,   // bottom right
		};

static float textureCoords[] = { 0.0f, 1.0f,  // top left
		0.0f, 0.0f,   // bottom left
		1.0f, 1.0f, // top right

		1.0f, 0.0f   // bottom right
		};

#define LOG_TAG "skiaDraw"

void releaseSkia() {
	g_Context.reset(NULL);

	delete fCurRenderTarget;

	glDeleteRenderbuffers(1, &fRenderbuffer);
	glDeleteFramebuffers(1, &fFramebuffer);
	glDeleteProgram(mProgram);

	delete fPath;
	fPath = NULL;
}

GrContext* getContext() {

	if (g_Context.get() == NULL) {

		glClearColor(1.0f, 0.0f, .5f, 1.0f);
		SkAutoTUnref<const GrGLInterface> glInterface;
		// all these guys use the native interface
		glInterface.reset(GrGLCreateNativeInterface());

		const GrGLInterface* fCurIntf = (GrGLInterfaceRemoveNVPR(glInterface));

		g_Context.reset(
				GrContext::Create(kOpenGL_GrBackend,
						(GrBackendContext) fCurIntf));

		glClearColor(.5f, 0.0f, .5f, 1.0f);

	}

	return g_Context;
}

void initializeSKiaSurface(int width, int height) {

	glClearColor(1.0, 0., 0., 1.);

	static const char* vertexShaderCode = "attribute vec3 vPosition;"
			"varying highp vec2 imageCoordinates;"
			"void main() { "
			"  gl_Position.xyz = vPosition; "
			"  gl_Position.w = 1.;"
			" imageCoordinates.xy = vPosition.xy+ vec2(1,1);"
			" imageCoordinates.xy /= vec2(2,2);"
			"}";

	static const char* fragmentShaderCode = "precision mediump float; "
			"uniform vec4 vColor; "
			"uniform sampler2D inputImageTexture;\n"
			"varying highp vec2 imageCoordinates;"
			"void main() { "
			"  gl_FragColor =  texture2D(inputImageTexture, imageCoordinates); "
			"}";


		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffers(1, &fFramebuffer);
		int error = glGetError();

		glBindFramebuffer(GL_FRAMEBUFFER, fFramebuffer);
		//    glEnable(GL_STENCIL_TEST);

		glGenRenderbuffers(1, &fRenderbuffer);
		glGenRenderbuffers(1, &fStencilbuffer);

		// Allocate color buffer backing based on the current layer size
		glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height);
		error = glGetError();

		/*	   glBindRenderbuffer(GL_RENDERBUFFER, fStencilbuffer);
		 glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		 error = glGetError(); */

		glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_RENDERBUFFER, fRenderbuffer);
		error = glGetError();

		/*	glBindRenderbuffer(GL_RENDERBUFFER, fStencilbuffer);
		 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fStencilbuffer);
		 error = glGetError(); */

		error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (error != GL_FRAMEBUFFER_COMPLETE) {
			//   NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,
				&fWidth);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT,
				&fHeight);

		glGenTextures(1, &fTexture);
		glBindTexture(GL_TEXTURE_2D, fTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, fTexture, 0);

		GLint drawFboId = 0, readFboId = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &drawFboId);
		glGetIntegerv(GL_RENDERBUFFER_BINDING, &readFboId);

				// create empty OpenGL ES Program
		mProgram = glCreateProgram();

		// create a vertex shader type (GLES20.GL_VERTEX_SHADER)
		// or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
		int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		int length = strlen(vertexShaderCode);

		glShaderSource(vertexShader, 1, &vertexShaderCode, &length);
		error = glGetError();
		glCompileShader(vertexShader);
		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 2048;

			//The maxLength includes the NULL character
			GLchar infoLog[maxLength];
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength,
					&infoLog[0]);

			//We don't need the shader anymore.
			glDeleteShader(vertexShader);

			LOG_ERROR("%s", infoLog);

			//In this simple program, we'll just leave
			return;
		}

		int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		length = strlen(fragmentShaderCode);

		glShaderSource(fragmentShader, 1, &fragmentShaderCode, &length);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 2048;

			//The maxLength includes the NULL character
			GLchar infoLog[maxLength];
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength,
					&infoLog[0]);

			//We don't need the shader anymore.
			glDeleteShader(fragmentShader);
			//Either of them. Don't leak shaders.
			glDeleteShader(vertexShader);

			LOG_ERROR("%s", infoLog);

			//In this simple program, we'll just leave
			return;
		}

		// add the vertex shader to program
		glAttachShader(mProgram, vertexShader);

		// add the fragment shader to program
		glAttachShader(mProgram, fragmentShader);

		// creates OpenGL ES program executables
		glLinkProgram(mProgram);
		GLint p_ok;
		glGetProgramiv(mProgram, GL_LINK_STATUS, &p_ok);
		error = glGetError();
		if (error != GL_NONE) {
			LOG_ERROR("linkProgram returned error %d while trying to draw",
					error);
		}

		// get handle to vertex shader's vPosition member
		mPositionHandle = glGetAttribLocation(mProgram, "vPosition");
		error = glGetError();
		if (error != GL_NONE) {
			LOG_ERROR("vPosition returned error %d while trying to draw",
					error);
		}
		// get handle to fragment shader's vColor member
		mColorHandle = glGetUniformLocation(mProgram, "vColor");
		error = glGetError();
		if (error != GL_NONE) {
			LOG_ERROR("vColorHandle returned error %d while trying to draw",
					error);
		}

		// Get the sampler location
		mSamplerLoc = glGetUniformLocation(mProgram, "inputImageTexture");

		error = glGetError();
		if (error != GL_NONE) {
			LOG_ERROR("mSamplerLoc returned error %d while trying to draw",
					error);
		}

		// number of coordinates per vertex in this array
		static int COORDS_PER_VERTEX = 3;

		glGenBuffers(1, &mVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareCoords), squareCoords,
				GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		GrContext* pContext = getContext();


		SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

		GrBackendRenderTargetDesc desc;
			desc.fWidth = width;
			desc.fHeight = height;
			desc.fConfig = kRGBA_8888_GrPixelConfig;
			glGetIntegerv(GL_SAMPLES, &desc.fSampleCnt);
			glGetIntegerv(GL_STENCIL_BITS, &desc.fStencilBits);
			desc.fRenderTargetHandle = drawFboId;

			desc.fSampleCnt = 0;

			GrGLint buffer;



		fCurRenderTarget =
				pContext->textureProvider()->wrapBackendRenderTarget(desc);


		glClearColor(.25f, .25f, .25f, .25f);
		glClear (GL_COLOR_BUFFER_BIT);
		glFlush();



	fPath = new SkPath;
}

void DoSkiaDraw() {

	g_Context->resetContext();

    SkSurfaceProps props( 0 , kUnknown_SkPixelGeometry);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(fCurRenderTarget, &props));
    SkCanvas* canvas = surface->getCanvas();


	canvas->drawColor(0x00000000);
	SkPaint p;
	p.setAntiAlias(true);
	p.setColor(0xffff00ff);
	p.setStyle(SkPaint::kStroke_Style);
	p.setStrokeWidth(40);
	p.setStrokeJoin(SkPaint::Join::kRound_Join);
	p.setStrokeCap(SkPaint::Cap::kRound_Cap);

	SkColor colors[2] = { 0xffff0000, 0x00000000 };
	SkPoint points[2];
	points[0].Make(0, 0);
	points[1].Make(30, 0);

	static float intervals[2] = { 1, 1 };

	static bool bDrawRect = true;

	canvas->drawPath(*fPath, p);
	SkRect r = { 300, 400, 600, 600 };

	if ( bDrawRect )
	{
		p.setColor(0xAA11EEAA);
		canvas->drawRect(r, p);
		bDrawRect = false;
	}

	g_Context->flush();



}

void DrawBitmap(EGLDisplay display, EGLSurface surface, EGLContext context,
		long elapsedTime) {



	glBindFramebuffer(GL_FRAMEBUFFER, fFramebuffer);

	glDisable (GL_SCISSOR_TEST);
	glClearColor(0, 0, 0, 0);
	//  glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, fWidth, fHeight);

	DoSkiaDraw();

	// grab skia values
	GLenum type;
	GLint size;
	GLsizei length;

	char name[255];


	GLint currentProgram;
	glGetIntegerv( GL_CURRENT_PROGRAM, &currentProgram );
	glGetActiveAttrib(	currentProgram, mPositionHandle, sizeof(name), &length, &size, &type, name );


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (!eglMakeCurrent(display, surface, surface, context)) {
		LOG_ERROR("eglMakeCurrent() returned error %d while trying to draw",
				eglGetError());
	}
	// Add program to OpenGL ES environment
	glUseProgram(mProgram);

	glClearColor(1., 0., 0., 1.f);
	glDisable(GL_CULL_FACE);
	//glClear (GL_COLOR_BUFFER_BIT);


	// Bind the texture
	glActiveTexture (GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fTexture);

	// Set the texture sampler to texture unit to 0
	glUniform1i(mSamplerLoc, 0);


	glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);

	// Enable a handle to the triangle vertices
	glEnableVertexAttribArray(mPositionHandle);

	// Prepare the triangle coordinate data
	glVertexAttribPointer(mPositionHandle, 4, GL_FLOAT, GL_FALSE, vertexStride,
			0);

	// Set color for drawing the triangle
//	float color[] = { 1, 1, 1, 1 };
//	glUniform4fv(mColorHandle, 1, color);

	// Draw the triangle
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex array - this call screws up SKIA :(
	//glDisableVertexAttribArray(mPositionHandle);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);

	// Enable a handle to the triangle vertices
	glDisableVertexAttribArray(mPositionHandle);



	glFlush();

}

void addSkiaPoint(float x, float y) {

	fPath->lineTo(SkPoint::Make(x, y));
}

void startSkiaLine(float x, float y) {
	delete fPath;
	fPath = new SkPath;
	fPath->moveTo(SkPoint::Make(x, y));
}

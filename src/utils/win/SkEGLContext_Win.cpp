#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <GL/GL.h>

#include "SkEGLContext.h"
#include "SkTypes.h"

#define SK_EGL_DECLARE_PROC(F) SkEGL ## F ## Proc SkEGL ## F = NULL;
#define SK_EGL_GET_PROC(F) SkEGL ## F = (SkEGL ## F ## Proc) \
        wglGetProcAddress("gl" #F);
#define SK_EGL_GET_PROC_SUFFIX(F, S) SkEGL ## F = (SkEGL ## F ## Proc) \
        wglGetProcAddress("gl" #F #S);

#define SK_EGL_FRAMEBUFFER 0x8D40
#define SK_EGL_RENDERBUFFER 0x8D41
#define SK_EGL_COLOR_ATTACHMENT0 0x8CE0
#define SK_EGL_DEPTH_STENCIL 0x84F9
#define SK_EGL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define SK_EGL_FRAMEBUFFER_COMPLETE 0x8CD5

#define SK_EGL_FUNCTION_TYPE __stdcall
typedef void (SK_EGL_FUNCTION_TYPE *SkEGLGenFramebuffersProc) (GLsizei n, GLuint *framebuffers);
typedef void (SK_EGL_FUNCTION_TYPE *SkEGLBindFramebufferProc) (GLenum target, GLuint framebuffer);
typedef void (SK_EGL_FUNCTION_TYPE *SkEGLGenRenderbuffersProc) (GLsizei n, GLuint *renderbuffers);
typedef void (SK_EGL_FUNCTION_TYPE *SkEGLBindRenderbufferProc) (GLenum target, GLuint renderbuffer);
typedef void (SK_EGL_FUNCTION_TYPE *SkEGLRenderbufferStorageProc) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (SK_EGL_FUNCTION_TYPE *SkEGLFramebufferRenderbufferProc) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef GLenum (SK_EGL_FUNCTION_TYPE *SkEGLCheckFramebufferStatusProc) (GLenum target);

SkEGLContext::SkEGLContext() :
        fWindow(NULL)
        , fDeviceContext(NULL)
        , fGlRenderContext(0) {
}

SkEGLContext::~SkEGLContext() {
    if (this->fGlRenderContext) {
        wglDeleteContext(this->fGlRenderContext);
    }
    if (this->fWindow && this->fDeviceContext) {
        ReleaseDC(this->fWindow, this->fDeviceContext);
    }
    if (this->fWindow) {
        DestroyWindow(this->fWindow);
    }
}

bool skEGLCheckExtension(const char* ext,
                         const char* extensionString) {
    int extLength = strlen(ext);

    while (true) {
        int n = strcspn(extensionString, " ");
        if (n == extLength && 0 == strncmp(ext, extensionString, n)) {
            return true;
        }
        if (0 == extensionString[n]) {
            return false;
        }
        extensionString += n+1;
    }

    return false;
}

bool SkEGLContext::init(const int width, const int height) {
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = (WNDPROC) DefWindowProc;
    wc.lpszClassName = TEXT("Griffin");
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
 
    if (!RegisterClass(&wc)) {
        SkDebugf("Could not register window class.\n");
        return false;
    }

    if (!(
        this->fWindow = CreateWindow(
        TEXT("Griffin"),
        TEXT("The Invisible Man"),
        WS_OVERLAPPEDWINDOW,
        10, 10,                 // x, y
        200, 200,               // width, height
        NULL, NULL,             // parent, menu
        hInstance, NULL)        // hInstance, param
        ))
    {
        SkDebugf("Could not create window.\n");
        return false;
    }

    if (!(this->fDeviceContext = GetDC(this->fWindow))) {
        SkDebugf("Could not get device context.\n");
        return false;
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int pixelFormat = 0;
    if (!(pixelFormat = ChoosePixelFormat(this->fDeviceContext, &pfd))) {
	    SkDebugf("No matching pixel format descriptor.\n");
        return false;
    }
    
    if (!SetPixelFormat(this->fDeviceContext, pixelFormat, &pfd)) {
	    SkDebugf("Could not set the pixel format %d.\n", pixelFormat);
        return false;
    }
    
    if (!(this->fGlRenderContext = wglCreateContext(this->fDeviceContext))) {
	    SkDebugf("Could not create rendering context.\n");
        return false;
    }

    if (!(wglMakeCurrent(this->fDeviceContext, this->fGlRenderContext))) {
        SkDebugf("Could not set the context.\n");
        return false;
    }

    //TODO: in the future we need to use this context
    // to test for WGL_ARB_create_context
    // and then create a new window / context.

    //Setup the framebuffers
    const char* glExts =
        reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    if (!skEGLCheckExtension(
          "GL_EXT_framebuffer_object"
          , glExts))
    {
        SkDebugf("GL_EXT_framebuffer_object not found.\n");
        return false;
    }
    SK_EGL_DECLARE_PROC(GenFramebuffers)
    SK_EGL_DECLARE_PROC(BindFramebuffer)
    SK_EGL_DECLARE_PROC(GenRenderbuffers)
    SK_EGL_DECLARE_PROC(BindRenderbuffer)
    SK_EGL_DECLARE_PROC(RenderbufferStorage)
    SK_EGL_DECLARE_PROC(FramebufferRenderbuffer)
    SK_EGL_DECLARE_PROC(CheckFramebufferStatus)

    SK_EGL_GET_PROC_SUFFIX(GenFramebuffers, EXT)
    SK_EGL_GET_PROC_SUFFIX(BindFramebuffer, EXT)
    SK_EGL_GET_PROC_SUFFIX(GenRenderbuffers, EXT)
    SK_EGL_GET_PROC_SUFFIX(BindRenderbuffer, EXT)
    SK_EGL_GET_PROC_SUFFIX(RenderbufferStorage, EXT)
    SK_EGL_GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT)
    SK_EGL_GET_PROC_SUFFIX(CheckFramebufferStatus, EXT)

    GLuint fboID;
    GLuint cbID;
    GLuint dsID;
    SkEGLGenFramebuffers(1, &fboID);
    SkEGLBindFramebuffer(SK_EGL_FRAMEBUFFER, fboID);
    SkEGLGenRenderbuffers(1, &cbID);
    SkEGLBindRenderbuffer(SK_EGL_RENDERBUFFER, cbID);
    SkEGLRenderbufferStorage(SK_EGL_RENDERBUFFER, GL_RGBA, width, height);
    SkEGLFramebufferRenderbuffer(SK_EGL_FRAMEBUFFER
                                 , SK_EGL_COLOR_ATTACHMENT0
                                 , SK_EGL_RENDERBUFFER, cbID);
    SkEGLGenRenderbuffers(1, &dsID);
    SkEGLBindRenderbuffer(SK_EGL_RENDERBUFFER, dsID);
    SkEGLRenderbufferStorage(SK_EGL_RENDERBUFFER, SK_EGL_DEPTH_STENCIL
                             , width, height);
    SkEGLFramebufferRenderbuffer(SK_EGL_FRAMEBUFFER
                                 , SK_EGL_DEPTH_STENCIL_ATTACHMENT
                                 , SK_EGL_RENDERBUFFER
                                 , dsID);
    glViewport(0, 0, width, height);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    GLenum status = SkEGLCheckFramebufferStatus(SK_EGL_FRAMEBUFFER);
    return SK_EGL_FRAMEBUFFER_COMPLETE == status;
}

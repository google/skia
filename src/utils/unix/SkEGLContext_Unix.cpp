#include "SkEGLContext.h"
#include "SkTypes.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

#define SK_GL_GET_PROC(T, F) T F = NULL; \
        F = (T) glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(#F));

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    ctxErrorOccurred = true;
    return 0;
}

SkEGLContext::SkEGLContext() : context(NULL), display(NULL), pixmap(0), glxPixmap(0) {
}

SkEGLContext::~SkEGLContext() {
    if (this->display) {
        glXMakeCurrent(this->display, 0, 0);

        if (this->context)
            glXDestroyContext(this->display, this->context);

        if (this->glxPixmap)
            glXDestroyGLXPixmap(this->display, this->glxPixmap);

        if (this->pixmap)
            XFreePixmap(this->display, this->pixmap);

        XCloseDisplay(this->display);
    }
}

bool SkEGLContext::init(const int width, const int height) {
    Display *display = XOpenDisplay(0);
    this->display = display;

    if (!display) {
        SkDebugf("Failed to open X display.\n");
        return false;
    }

    // Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_PIXMAP_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };

    int glx_major, glx_minor;

    // FBConfigs were added in GLX version 1.3.
    if (!glXQueryVersion( display, &glx_major, &glx_minor) ||
            ( (glx_major == 1) && (glx_minor < 3) ) || (glx_major < 1))
    {
        SkDebugf("Invalid GLX version.");
        return false;
    }

    //SkDebugf("Getting matching framebuffer configs.\n");
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display),
                                          visual_attribs, &fbcount);
    if (!fbc) {
        SkDebugf("Failed to retrieve a framebuffer config.\n");
        return false;
    }
    //SkDebugf("Found %d matching FB configs.\n", fbcount);

    // Pick the FB config/visual with the most samples per pixel
    //SkDebugf("Getting XVisualInfos.\n");
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

    int i;
    for (i = 0; i < fbcount; ++i) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);

            //SkDebugf("  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
            //       " SAMPLES = %d\n",
            //        i, (unsigned int)vi->visualid, samp_buf, samples);

            if (best_fbc < 0 || (samp_buf && samples > best_num_samp))
                best_fbc = i, best_num_samp = samples;
            if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
                worst_fbc = i, worst_num_samp = samples;
        }
        XFree(vi);
    }

    GLXFBConfig bestFbc = fbc[best_fbc];

    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree(fbc);

    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
    //SkDebugf("Chosen visual ID = 0x%x\n", (unsigned int)vi->visualid);

    Pixmap pixmap = XCreatePixmap(
        display, RootWindow(display, vi->screen), width, height, vi->depth
    );

    this->pixmap = pixmap;
    if (!pixmap) {
        SkDebugf("Failed to create pixmap.\n");
        return false;
    }

    GLXPixmap glxPixmap = glXCreateGLXPixmap(display, vi, pixmap);
    this->glxPixmap = glxPixmap;

    // Done with the visual info data
    XFree(vi);

    // Create the context
    GLXContext ctx = 0;

    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails.
    //
    // Note this error handler is global.
    // All display connections in all threads of a process use the same
    // error handler, so be sure to guard against other threads issuing
    // X commands while this code is running.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) =
        XSetErrorHandler(&ctxErrorHandler);

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString(
        display, DefaultScreen(display)
    );
    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if (!gluCheckExtension(
          reinterpret_cast<const GLubyte*>("GLX_ARB_create_context")
          , reinterpret_cast<const GLubyte*>(glxExts)))
    {
        //SkDebugf("GLX_ARB_create_context not found."
        //       " Using old-style GLX context.\n");
        ctx = glXCreateNewContext(display, bestFbc, GLX_RGBA_TYPE, 0, True);

    } else {
        //SkDebugf("Creating context.\n");

        SK_GL_GET_PROC(PFNGLXCREATECONTEXTATTRIBSARBPROC, glXCreateContextAttribsARB)
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };
        ctx = glXCreateContextAttribsARB(
            display, bestFbc, 0, True, context_attribs
        );

        // Sync to ensure any errors generated are processed.
        XSync(display, False);
        if (!ctxErrorOccurred && ctx) {
           //SkDebugf( "Created GL 3.0 context.\n" );
        } else {
            // Couldn't create GL 3.0 context.
            // Fall back to old-style 2.x context.
            // When a context version below 3.0 is requested,
            // implementations will return the newest context version compatible
            // with OpenGL versions less than version 3.0.

            // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
            context_attribs[1] = 1;
            // GLX_CONTEXT_MINOR_VERSION_ARB = 0
            context_attribs[3] = 0;

            ctxErrorOccurred = false;

            //SkDebugf("Failed to create GL 3.0 context."
            //       " Using old-style GLX context.\n");
            ctx = glXCreateContextAttribsARB(
                display, bestFbc, 0, True, context_attribs
            );
        }
    }

    // Sync to ensure any errors generated are processed.
    XSync(display, False);

    // Restore the original error handler
    XSetErrorHandler(oldHandler);

    if (ctxErrorOccurred || !ctx) {
        SkDebugf("Failed to create an OpenGL context.\n");
        return false;
    }
    this->context = ctx;

    // Verify that context is a direct context
    if (!glXIsDirect(display, ctx)) {
        //SkDebugf("Indirect GLX rendering context obtained.\n");
    } else {
        //SkDebugf("Direct GLX rendering context obtained.\n");
    }

    //SkDebugf("Making context current.\n");
    if (!glXMakeCurrent(display, glxPixmap, ctx)) {
      SkDebugf("Could not set the context.\n");
      return false;
    }

    //Setup the framebuffers
    const GLubyte* glExts = glGetString(GL_EXTENSIONS);
    if (!gluCheckExtension(
          reinterpret_cast<const GLubyte*>("GL_EXT_framebuffer_object")
          , glExts))
    {
      SkDebugf("GL_EXT_framebuffer_object not found.\n");
      return false;
    }
    SK_GL_GET_PROC(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffersEXT)
    SK_GL_GET_PROC(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebufferEXT)
    SK_GL_GET_PROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffersEXT)
    SK_GL_GET_PROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbufferEXT)
    SK_GL_GET_PROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorageEXT)
    SK_GL_GET_PROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbufferEXT)
    SK_GL_GET_PROC(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT)

    GLuint fboID;
    GLuint cbID;
    GLuint dsID;
    glGenFramebuffersEXT(1, &fboID);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
    glGenRenderbuffersEXT(1, &cbID);
    glBindRenderbufferEXT(GL_RENDERBUFFER, cbID);
    glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cbID);
    glGenRenderbuffersEXT(1, &dsID);
    glBindRenderbufferEXT(GL_RENDERBUFFER, dsID);
    glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsID);
    glViewport(0, 0, width, height);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
    return GL_FRAMEBUFFER_COMPLETE == status;
}

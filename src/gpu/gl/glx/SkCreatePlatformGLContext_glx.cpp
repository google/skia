
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glu.h>

namespace {

/* Note: Skia requires glx 1.3 or newer */

/* This struct is taken from a mesa demo.  Please update as required */
static const struct { int major, minor; } gl_versions[] = {
   {1, 0},
   {1, 1},
   {1, 2},
   {1, 3},
   {1, 4},
   {1, 5},
   {2, 0},
   {2, 1},
   {3, 0},
   {3, 1},
   {3, 2},
   {3, 3},
   {4, 0},
   {4, 1},
   {4, 2},
   {4, 3},
   {4, 4},
   {0, 0} /* end of list */
};
#define NUM_GL_VERSIONS SK_ARRAY_COUNT(gl_versions)

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    ctxErrorOccurred = true;
    return 0;
}

class GLXGLContext : public SkGLContext {
public:
    GLXGLContext(GrGLStandard forcedGpuAPI);
    ~GLXGLContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    GLXContext fContext;
    Display* fDisplay;
    Pixmap fPixmap;
    GLXPixmap fGlxPixmap;
};

GLXGLContext::GLXGLContext(GrGLStandard forcedGpuAPI)
    : fContext(nullptr)
    , fDisplay(nullptr)
    , fPixmap(0)
    , fGlxPixmap(0) {

    fDisplay = XOpenDisplay(0);

    if (!fDisplay) {
        SkDebugf("Failed to open X display.\n");
        this->destroyGLContext();
        return;
    }

    // Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_PIXMAP_BIT,
        None
    };

    int glx_major, glx_minor;

    // FBConfigs were added in GLX version 1.3.
    if (!glXQueryVersion(fDisplay, &glx_major, &glx_minor) ||
            ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
        SkDebugf("GLX version 1.3 or higher required.\n");
        this->destroyGLContext();
        return;
    }

    //SkDebugf("Getting matching framebuffer configs.\n");
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(fDisplay, DefaultScreen(fDisplay),
                                          visual_attribs, &fbcount);
    if (!fbc) {
        SkDebugf("Failed to retrieve a framebuffer config.\n");
        this->destroyGLContext();
        return;
    }
    //SkDebugf("Found %d matching FB configs.\n", fbcount);

    // Pick the FB config/visual with the most samples per pixel
    //SkDebugf("Getting XVisualInfos.\n");
    int best_fbc = -1, best_num_samp = -1;

    int i;
    for (i = 0; i < fbcount; ++i) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(fDisplay, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(fDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(fDisplay, fbc[i], GLX_SAMPLES, &samples);

            //SkDebugf("  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
            //       " SAMPLES = %d\n",
            //        i, (unsigned int)vi->visualid, samp_buf, samples);

            if (best_fbc < 0 || (samp_buf && samples > best_num_samp))
                best_fbc = i, best_num_samp = samples;
        }
        XFree(vi);
    }

    GLXFBConfig bestFbc = fbc[best_fbc];

    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree(fbc);

    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig(fDisplay, bestFbc);
    //SkDebugf("Chosen visual ID = 0x%x\n", (unsigned int)vi->visualid);

    fPixmap = XCreatePixmap(fDisplay, RootWindow(fDisplay, vi->screen), 10, 10, vi->depth);

    if (!fPixmap) {
        SkDebugf("Failed to create pixmap.\n");
        this->destroyGLContext();
        return;
    }

    fGlxPixmap = glXCreateGLXPixmap(fDisplay, vi, fPixmap);

    // Done with the visual info data
    XFree(vi);

    // Create the context

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
        fDisplay, DefaultScreen(fDisplay)
    );


    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if (!gluCheckExtension(reinterpret_cast<const GLubyte*>("GLX_ARB_create_context"),
                           reinterpret_cast<const GLubyte*>(glxExts))) {
        if (kGLES_GrGLStandard != forcedGpuAPI) {
            fContext = glXCreateNewContext(fDisplay, bestFbc, GLX_RGBA_TYPE, 0, True);
        }
    } else {
        //SkDebugf("Creating context.\n");
        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
            (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddressARB((GrGLubyte*)"glXCreateContextAttribsARB");

        if (kGLES_GrGLStandard == forcedGpuAPI) {
            if (gluCheckExtension(
                    reinterpret_cast<const GLubyte*>("GLX_EXT_create_context_es2_profile"),
                    reinterpret_cast<const GLubyte*>(glxExts))) {
                static const int context_attribs_gles[] = {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
                    None
                };
                fContext = glXCreateContextAttribsARB(fDisplay, bestFbc, 0, True,
                                                      context_attribs_gles);
            }
        } else {
            // Well, unfortunately GLX will not just give us the highest context so instead we have
            // to do this nastiness
            for (i = NUM_GL_VERSIONS - 2; i > 0 ; i--) {
                /* don't bother below GL 3.0 */
                if (gl_versions[i].major == 3 && gl_versions[i].minor == 0) {
                    break;
                }
                // On Nvidia GPUs, to use Nv Path rendering we need a compatibility profile for the
                // time being.
                // TODO when Nvidia implements NVPR on Core profiles, we should start requesting
                // core here
                static const int context_attribs_gl[] = {
                      GLX_CONTEXT_MAJOR_VERSION_ARB, gl_versions[i].major,
                      GLX_CONTEXT_MINOR_VERSION_ARB, gl_versions[i].minor,
                      GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                      None
                };
                fContext =
                        glXCreateContextAttribsARB(fDisplay, bestFbc, 0, True, context_attribs_gl);

                // Sync to ensure any errors generated are processed.
                XSync(fDisplay, False);

                if (!ctxErrorOccurred && fContext) {
                    break;
                }
                // try again
                ctxErrorOccurred = false;
            }

            // Couldn't create GL 3.0 context.
            // Fall back to old-style 2.x context.
            // When a context version below 3.0 is requested,
            // implementations will return the newest context version
            // compatible with OpenGL versions less than version 3.0.
            if (ctxErrorOccurred || !fContext) {
                static const int context_attribs_gl_fallback[] = {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
                    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                    None
                };

                ctxErrorOccurred = false;

                fContext = glXCreateContextAttribsARB(fDisplay, bestFbc, 0, True,
                                                      context_attribs_gl_fallback);
            }
        }
    }

    // Sync to ensure any errors generated are processed.
    XSync(fDisplay, False);

    // Restore the original error handler
    XSetErrorHandler(oldHandler);

    if (ctxErrorOccurred || !fContext) {
        SkDebugf("Failed to create an OpenGL context.\n");
        this->destroyGLContext();
        return;
    }

    // Verify that context is a direct context
    if (!glXIsDirect(fDisplay, fContext)) {
        //SkDebugf("Indirect GLX rendering context obtained.\n");
    } else {
        //SkDebugf("Direct GLX rendering context obtained.\n");
    }

    //SkDebugf("Making context current.\n");
    if (!glXMakeCurrent(fDisplay, fGlxPixmap, fContext)) {
      SkDebugf("Could not set the context.\n");
        this->destroyGLContext();
        return;
    }

    SkAutoTUnref<const GrGLInterface> gl(GrGLCreateNativeInterface());
    if (nullptr == gl.get()) {
        SkDebugf("Failed to create gl interface");
        this->destroyGLContext();
        return;
    }

    if (!gl->validate()) {
        SkDebugf("Failed to validate gl interface");
        this->destroyGLContext();
        return;
    }

    this->init(gl.detach());
}


GLXGLContext::~GLXGLContext() {
    this->teardown();
    this->destroyGLContext();
}

void GLXGLContext::destroyGLContext() {
    if (fDisplay) {
        glXMakeCurrent(fDisplay, 0, 0);

        if (fContext) {
            glXDestroyContext(fDisplay, fContext);
            fContext = nullptr;
        }

        if (fGlxPixmap) {
            glXDestroyGLXPixmap(fDisplay, fGlxPixmap);
            fGlxPixmap = 0;
        }

        if (fPixmap) {
            XFreePixmap(fDisplay, fPixmap);
            fPixmap = 0;
        }

        XCloseDisplay(fDisplay);
        fDisplay = nullptr;
    }
}

void GLXGLContext::onPlatformMakeCurrent() const {
    if (!glXMakeCurrent(fDisplay, fGlxPixmap, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void GLXGLContext::onPlatformSwapBuffers() const {
    glXSwapBuffers(fDisplay, fGlxPixmap);
}

GrGLFuncPtr GLXGLContext::onPlatformGetProcAddress(const char* procName) const {
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(procName));
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    GLXGLContext *ctx = new GLXGLContext(forcedGpuAPI);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkOnce.h"
#include "tools/gpu/gl/GLTestContext.h"

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <vector>
#include <utility>

namespace {

/* Note: Skia requires glx 1.3 or newer */

/* This struct is taken from a mesa demo.  Please update as required */
static const std::vector<std::pair<int, int>> gl_versions = {
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
};

static const std::vector<std::pair<int, int>> gles_versions = {
    {2, 0},
    {3, 0},
};

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    ctxErrorOccurred = true;
    return 0;
}

class GLXGLTestContext : public sk_gpu_test::GLTestContext {
public:
    GLXGLTestContext(GrGLStandard forcedGpuAPI, GLXGLTestContext* shareList);
    ~GLXGLTestContext() override;

private:
    void destroyGLContext();
    static GLXContext CreateBestContext(bool isES, Display* display, GLXFBConfig bestFbc,
                                        GLXContext glxSharedContext);

    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    GLXContext fContext;
    Display* fDisplay;
    Pixmap fPixmap;
    GLXPixmap fGlxPixmap;
};

static Display* get_display() {
    class AutoDisplay {
    public:
        AutoDisplay() { fDisplay = XOpenDisplay(nullptr); }
        ~AutoDisplay() {
            if (fDisplay) {
                XCloseDisplay(fDisplay);
            }
        }
        Display* display() const { return fDisplay; }
    private:
        Display* fDisplay;
    };
    static std::unique_ptr<AutoDisplay> ad;
    static SkOnce once;
    once([] { ad = std::make_unique<AutoDisplay>(); });
    return ad->display();
}

std::function<void()> context_restorer() {
    auto display = glXGetCurrentDisplay();
    auto drawable = glXGetCurrentDrawable();
    auto context = glXGetCurrentContext();
    // On some systems calling glXMakeCurrent with a null display crashes.
    if (!display) {
        display = get_display();
    }
    return [display, drawable, context] { glXMakeCurrent(display, drawable, context); };
}

GLXGLTestContext::GLXGLTestContext(GrGLStandard forcedGpuAPI, GLXGLTestContext* shareContext)
    : fContext(nullptr)
    , fDisplay(nullptr)
    , fPixmap(0)
    , fGlxPixmap(0) {
    // We cross our fingers that this is the first X call in the program and that if the application
    // is actually threaded that this succeeds.
    static SkOnce gOnce;
    gOnce([] { XInitThreads(); });

    fDisplay = get_display();

    GLXContext glxShareContext = shareContext ? shareContext->fContext : nullptr;

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

            if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
                best_fbc = i;
                best_num_samp = samples;
            }
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

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString(
        fDisplay, DefaultScreen(fDisplay)
    );
    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if (!gluCheckExtension(reinterpret_cast<const GLubyte*>("GLX_ARB_create_context"),
                           reinterpret_cast<const GLubyte*>(glxExts))) {
        if (kGLES_GrGLStandard != forcedGpuAPI) {
            fContext = glXCreateNewContext(fDisplay, bestFbc, GLX_RGBA_TYPE, nullptr, True);
        }
    } else {
        if (kGLES_GrGLStandard == forcedGpuAPI) {
            if (gluCheckExtension(
                    reinterpret_cast<const GLubyte*>("GLX_EXT_create_context_es2_profile"),
                    reinterpret_cast<const GLubyte*>(glxExts))) {
                fContext = CreateBestContext(true, fDisplay, bestFbc, glxShareContext);
            }
        } else {
            fContext = CreateBestContext(false, fDisplay, bestFbc, glxShareContext);
        }
    }
    if (!fContext) {
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

    SkScopeExit restorer(context_restorer());
    //SkDebugf("Making context current.\n");
    if (!glXMakeCurrent(fDisplay, fGlxPixmap, fContext)) {
      SkDebugf("Could not set the context.\n");
        this->destroyGLContext();
        return;
    }

#ifdef SK_GL
    auto gl = GrGLMakeNativeInterface();
    if (!gl) {
        SkDebugf("Failed to create gl interface");
        this->destroyGLContext();
        return;
    }

    if (!gl->validate()) {
        SkDebugf("Failed to validate gl interface");
        this->destroyGLContext();
        return;
    }

    this->init(std::move(gl));
#else
    // Allow the GLTestContext creation to succeed without a GrGLInterface to support
    // GrContextFactory's persistent GL context workaround for Vulkan. We won't need the
    // GrGLInterface since we're not running the GL backend.
    this->init(nullptr);
#endif
}


GLXGLTestContext::~GLXGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void GLXGLTestContext::destroyGLContext() {
    if (fDisplay) {
        if (fContext) {
            if (glXGetCurrentContext() == fContext) {
                // This will ensure that the context is immediately deleted.
                glXMakeContextCurrent(fDisplay, None, None, nullptr);
            }
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

        fDisplay = nullptr;
    }
}

/* Create a context with the highest possible version.
 *
 * Disable Xlib errors for the duration of this function (by default they abort
 * the program) and try to get a context starting from the highest version
 * number - there is no way to just directly ask what the highest supported
 * version is.
 *
 * Returns the correct context or NULL on failure.
 */
GLXContext GLXGLTestContext::CreateBestContext(bool isES, Display* display, GLXFBConfig bestFbc,
                                               GLXContext glxShareContext) {
    auto glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddressARB((const GrGLubyte*)"glXCreateContextAttribsARB");
    if (!glXCreateContextAttribsARB) {
        SkDebugf("Failed to get address of glXCreateContextAttribsARB");
        return nullptr;
    }
    GLXContext context = nullptr;
    // Install Xlib error handler that will set ctxErrorOccurred.
    // WARNING: It is global for all threads.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    auto versions = isES ? gles_versions : gl_versions;
    // Well, unfortunately GLX will not just give us the highest context so
    // instead we have to do this nastiness
    for (int i = versions.size() - 1; i >= 0 ; i--) {
        // WARNING: Don't try to optimize this and make this array static. The
        // glXCreateContextAttribsARB call writes to it upon failure and the
        // next call would fail too.
        std::vector<int> flags = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, versions[i].first,
            GLX_CONTEXT_MINOR_VERSION_ARB, versions[i].second,
        };
        if (isES) {
            flags.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
            // the ES2 flag should work even for higher versions
            flags.push_back(GLX_CONTEXT_ES2_PROFILE_BIT_EXT);
        } else if (versions[i].first > 2) {
            flags.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
            flags.push_back(GLX_CONTEXT_CORE_PROFILE_BIT_ARB);
        }
        flags.push_back(0);
        context = glXCreateContextAttribsARB(display, bestFbc, glxShareContext, true,
                                             &flags[0]);
        // Sync to ensure any errors generated are processed.
        XSync(display, False);

        if (!ctxErrorOccurred && context) {
            break;
        }
        // try again
        ctxErrorOccurred = false;
    }
    // Restore the original error handler.
    XSetErrorHandler(oldHandler);
    return context;
}

void GLXGLTestContext::onPlatformMakeNotCurrent() const {
    if (!glXMakeCurrent(fDisplay, None , nullptr)) {
        SkDebugf("Could not reset the context.\n");
    }
}

void GLXGLTestContext::onPlatformMakeCurrent() const {
    if (!glXMakeCurrent(fDisplay, fGlxPixmap, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

std::function<void()> GLXGLTestContext::onPlatformGetAutoContextRestore() const {
    if (glXGetCurrentContext() == fContext) {
        return nullptr;
    }
    return context_restorer();
}

GrGLFuncPtr GLXGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(procName));
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext *CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    GLXGLTestContext *glxShareContext = reinterpret_cast<GLXGLTestContext *>(shareContext);
    GLXGLTestContext *ctx = new GLXGLTestContext(forcedGpuAPI, glxShareContext);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

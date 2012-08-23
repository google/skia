
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkNativeGLContext.h"

#include <GL/glu.h>

#define GLX_1_3 1

SkNativeGLContext::AutoContextRestore::AutoContextRestore() {
    fOldGLXContext = glXGetCurrentContext();
    fOldDisplay = glXGetCurrentDisplay();
    fOldDrawable = glXGetCurrentDrawable();
}

SkNativeGLContext::AutoContextRestore::~AutoContextRestore() {
    if (NULL != fOldDisplay) {
        glXMakeCurrent(fOldDisplay, fOldDrawable, fOldGLXContext);
    }
}

///////////////////////////////////////////////////////////////////////////////

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
    ctxErrorOccurred = true;
    return 0;
}

SkNativeGLContext::SkNativeGLContext()
    : fContext(NULL)
    , fDisplay(NULL)
    , fPixmap(0)
    , fGlxPixmap(0) {
}

SkNativeGLContext::~SkNativeGLContext() {
    this->destroyGLContext();
}

void SkNativeGLContext::destroyGLContext() {
    if (fDisplay) {
        glXMakeCurrent(fDisplay, 0, 0);

        if (fContext) {
            glXDestroyContext(fDisplay, fContext);
            fContext = NULL;
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
        fDisplay = NULL;
    }
}

const GrGLInterface* SkNativeGLContext::createGLContext() {
    fDisplay = XOpenDisplay(0);

    if (!fDisplay) {
        SkDebugf("Failed to open X display.\n");
        this->destroyGLContext();
        return NULL;
    }

    // Get a matching FB config
    static int visual_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_PIXMAP_BIT,
        None
    };

#ifdef GLX_1_3
    //SkDebugf("Getting matching framebuffer configs.\n");
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(fDisplay, DefaultScreen(fDisplay),
                                          visual_attribs, &fbcount);
    if (!fbc) {
        SkDebugf("Failed to retrieve a framebuffer config.\n");
        this->destroyGLContext();
        return NULL;
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
#else
    int numVisuals;
    XVisualInfo visTemplate, *visReturn;

    visReturn = XGetVisualInfo(fDisplay, VisualNoMask, &visTemplate, &numVisuals);
    if (NULL == visReturn)
    {
        SkDebugf("Failed to get visual information.\n");
        this->destroyGLContext();
        return NULL;
    }

    int best = -1, best_num_samp = -1;

    for (int i = 0; i < numVisuals; ++i)
    {
        int samp_buf, samples;

        glXGetConfig(fDisplay, &visReturn[i], GLX_SAMPLE_BUFFERS, &samp_buf);
        glXGetConfig(fDisplay, &visReturn[i], GLX_SAMPLES, &samples);

        if (best < 0 || (samp_buf && samples > best_num_samp))
            best = i, best_num_samp = samples;
    }

    XVisualInfo temp = visReturn[best];
    XVisualInfo *vi = &temp;

    XFree(visReturn);
#endif

    fPixmap = XCreatePixmap(fDisplay, RootWindow(fDisplay, vi->screen), 10, 10, vi->depth);

    if (!fPixmap) {
        SkDebugf("Failed to create pixmap.\n");
        this->destroyGLContext();
        return NULL;
    }

    fGlxPixmap = glXCreateGLXPixmap(fDisplay, vi, fPixmap);

#ifdef GLX_1_3
    // Done with the visual info data
    XFree(vi);
#endif

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
    if (!gluCheckExtension(
          reinterpret_cast<const GLubyte*>("GLX_ARB_create_context")
          , reinterpret_cast<const GLubyte*>(glxExts)))
    {
        //SkDebugf("GLX_ARB_create_context not found."
        //       " Using old-style GLX context.\n");
#ifdef GLX_1_3
        fContext = glXCreateNewContext(fDisplay, bestFbc, GLX_RGBA_TYPE, 0, True);
#else
        fContext = glXCreateContext(fDisplay, vi, 0, True);
#endif

    }
#ifdef GLX_1_3
    else {
        //SkDebugf("Creating context.\n");

        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
            (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddressARB((GrGLubyte*)"glXCreateContextAttribsARB");
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };
        fContext = glXCreateContextAttribsARB(
            fDisplay, bestFbc, 0, True, context_attribs
        );

        // Sync to ensure any errors generated are processed.
        XSync(fDisplay, False);
        if (!ctxErrorOccurred && fContext) {
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
            fContext = glXCreateContextAttribsARB(
                fDisplay, bestFbc, 0, True, context_attribs
            );
        }
    }
#endif

    // Sync to ensure any errors generated are processed.
    XSync(fDisplay, False);

    // Restore the original error handler
    XSetErrorHandler(oldHandler);

    if (ctxErrorOccurred || !fContext) {
        SkDebugf("Failed to create an OpenGL context.\n");
        this->destroyGLContext();
        return NULL;
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
        return NULL;
    }

    const GrGLInterface* interface = GrGLCreateNativeInterface();
    if (!interface) {
        SkDebugf("Failed to create gl interface");
        this->destroyGLContext();
        return NULL;
    }
    return interface;
}

void SkNativeGLContext::makeCurrent() const {
    if (!glXMakeCurrent(fDisplay, fGlxPixmap, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

/*
 Copyright 2010 Google Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "GrGLConfig.h"
#include "GrTypes.h"
#include <stdio.h>

bool has_gl_extension(const char* ext) {
    const char* glstr = (const char*) glGetString(GL_EXTENSIONS);

    int extLength = strlen(ext);

    while (true) {
        int n = strcspn(glstr, " ");
        if (n == extLength && 0 == strncmp(ext, glstr, n)) {
            return true;
        }
        if (0 == glstr[n]) {
            return false;
        }
        glstr += n+1;
    }
}

void gl_version(int* major, int* minor) {
    const char* v = (const char*) glGetString(GL_VERSION);
    if (NULL == v) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
#if GR_SUPPORT_GLDESKTOP
    int n = sscanf(v, "%d.%d", major, minor);
    if (n != 2) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
#else
    char profile[2];
    int n = sscanf(v, "OpenGL ES-%c%c %d.%d", profile, profile+1, major, minor);
    bool ok = 4 == n;
    if (!ok) {
        int n = sscanf(v, "OpenGL ES %d.%d", major, minor);
        ok = 2 == n;
    }
    if (!ok) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
#endif
}

#if defined(GR_GL_PROC_ADDRESS_HEADER)
    #include GR_GL_PROC_ADDRESS_HEADER
#endif

typedef void (*glProc)(void);

#define GET_PROC(EXT_STRUCT, PROC_NAME) \
    *(GrTCast<glProc*>(&(EXT_STRUCT-> PROC_NAME))) = (glProc)GR_GL_PROC_ADDRESS((gl ## PROC_NAME)); \
    GrAssert(NULL != EXT_STRUCT-> PROC_NAME)

#define GET_SUFFIX_PROC(EXT_STRUCT, PROC_NAME, SUFFIX) \
    *(GrTCast<glProc*>(&(EXT_STRUCT-> PROC_NAME))) = (glProc)GR_GL_PROC_ADDRESS((gl ## PROC_NAME ## SUFFIX)); \
    GrAssert(NULL != EXT_STRUCT-> PROC_NAME)

extern void GrGLInitExtensions(GrGLExts* exts) {
    exts->GenFramebuffers                   = NULL;
    exts->BindFramebuffer                   = NULL;
    exts->FramebufferTexture2D              = NULL;
    exts->CheckFramebufferStatus            = NULL;
    exts->DeleteFramebuffers                = NULL;
    exts->RenderbufferStorage               = NULL;
    exts->GenRenderbuffers                  = NULL;
    exts->DeleteRenderbuffers               = NULL;
    exts->FramebufferRenderbuffer           = NULL;
    exts->BindRenderbuffer                  = NULL;
    exts->RenderbufferStorageMultisample    = NULL;
    exts->BlitFramebuffer                   = NULL;
    exts->ResolveMultisampleFramebuffer     = NULL;
    exts->FramebufferTexture2DMultisample   = NULL;
    exts->MapBuffer                         = NULL;
    exts->UnmapBuffer                       = NULL;

    GLint major, minor;
    gl_version(&major, &minor);

    bool fboFound = false;
#if GR_SUPPORT_GLDESKTOP
    #if defined(GL_VERSION_3_0) && GL_VERSION_3_0
    if (!fboFound && major >= 3) { // all of ARB_fbo is in 3.x
        exts->GenFramebuffers                   = glGenFramebuffers;
        exts->BindFramebuffer                   = glBindFramebuffer;
        exts->FramebufferTexture2D              = glFramebufferTexture2D;
        exts->CheckFramebufferStatus            = glCheckFramebufferStatus;
        exts->DeleteFramebuffers                = glDeleteFramebuffers;
        exts->RenderbufferStorage               = glRenderbufferStorage;
        exts->GenRenderbuffers                  = glGenRenderbuffers;
        exts->DeleteRenderbuffers               = glDeleteRenderbuffers;
        exts->FramebufferRenderbuffer           = glFramebufferRenderbuffer;
        exts->BindRenderbuffer                  = glBindRenderbuffer;
        exts->RenderbufferStorageMultisample    = glRenderbufferStorageMultisample;
        exts->BlitFramebuffer                   = glBlitFramebuffer;
        fboFound = true;
    }
    #endif
    #if GL_ARB_framebuffer_object
    if (!fboFound && has_gl_extension("GL_ARB_framebuffer_object")) {
        // GL_ARB_framebuffer_object doesn't use ARB suffix.
        GET_PROC(exts, GenFramebuffers);
        GET_PROC(exts, BindFramebuffer);
        GET_PROC(exts, FramebufferTexture2D);
        GET_PROC(exts, CheckFramebufferStatus);
        GET_PROC(exts, DeleteFramebuffers);
        GET_PROC(exts, RenderbufferStorage);
        GET_PROC(exts, GenRenderbuffers);
        GET_PROC(exts, DeleteRenderbuffers);
        GET_PROC(exts, FramebufferRenderbuffer);
        GET_PROC(exts, BindRenderbuffer);
        GET_PROC(exts, RenderbufferStorageMultisample);
        GET_PROC(exts, BlitFramebuffer);
        fboFound = true;
    }
    #endif
    // Mac doesn't declare prototypes for EXT FBO extensions
    #if GL_EXT_framebuffer_object && !GR_MAC_BUILD
    if (!fboFound && has_gl_extension("GL_EXT_framebuffer_object")) {
        GET_SUFFIX_PROC(exts, GenFramebuffers, EXT);
        GET_SUFFIX_PROC(exts, BindFramebuffer, EXT);
        GET_SUFFIX_PROC(exts, FramebufferTexture2D, EXT);
        GET_SUFFIX_PROC(exts, CheckFramebufferStatus, EXT);
        GET_SUFFIX_PROC(exts, DeleteFramebuffers, EXT);
        GET_SUFFIX_PROC(exts, RenderbufferStorage, EXT);
        GET_SUFFIX_PROC(exts, GenRenderbuffers, EXT);
        GET_SUFFIX_PROC(exts, DeleteRenderbuffers, EXT);
        GET_SUFFIX_PROC(exts, FramebufferRenderbuffer, EXT);
        GET_SUFFIX_PROC(exts, BindRenderbuffer, EXT);
        fboFound = true;
        // check for fbo ms and fbo blit
        #if GL_EXT_framebuffer_multisample
        if (has_gl_extension("GL_EXT_framebuffer_multisample")) {
            GET_SUFFIX_PROC(exts, RenderbufferStorageMultisample, EXT);
        }
        #endif
        #if GL_EXT_framebuffer_blit
        if (has_gl_extension("GL_EXT_framebuffer_blit")) {
            GET_SUFFIX_PROC(exts, BlitFramebuffer, EXT);
        }
        #endif
    }
    #endif
    if (!fboFound) {
        // we require some form of FBO
        GrAssert(!"No FBOs supported?");
    }
    // we assume we have at least GL 1.5 or higher (VBOs introduced in 1.5)
    exts->MapBuffer     = glMapBuffer;
    exts->UnmapBuffer   = glUnmapBuffer;
#else // !GR_SUPPORT_GLDESKTOP
    #if GR_SUPPORT_GLES2
    if (!fboFound && major >= 2) {// ES 2.0 supports FBO
        exts->GenFramebuffers                   = glGenFramebuffers;
        exts->BindFramebuffer                   = glBindFramebuffer;
        exts->FramebufferTexture2D              = glFramebufferTexture2D;
        exts->CheckFramebufferStatus            = glCheckFramebufferStatus;
        exts->DeleteFramebuffers                = glDeleteFramebuffers;
        exts->RenderbufferStorage               = glRenderbufferStorage;
        exts->GenRenderbuffers                  = glGenRenderbuffers;
        exts->DeleteRenderbuffers               = glDeleteRenderbuffers;
        exts->FramebufferRenderbuffer           = glFramebufferRenderbuffer;
        exts->BindRenderbuffer                  = glBindRenderbuffer;
        fboFound = true;
    }
    #endif
    #if GL_OES_framebuffer_object
    if (!fboFound && has_gl_extension("GL_OES_framebuffer_object")) {
        GET_SUFFIX_PROC(exts, GenFramebuffers, OES);
        GET_SUFFIX_PROC(exts, BindFramebuffer, OES);
        GET_SUFFIX_PROC(exts, FramebufferTexture2D, OES);
        GET_SUFFIX_PROC(exts, CheckFramebufferStatus, OES);
        GET_SUFFIX_PROC(exts, DeleteFramebuffers, OES);
        GET_SUFFIX_PROC(exts, RenderbufferStorage, OES);
        GET_SUFFIX_PROC(exts, GenRenderbuffers, OES);
        GET_SUFFIX_PROC(exts, DeleteRenderbuffers, OES);
        GET_SUFFIX_PROC(exts, FramebufferRenderbuffer, OES);
        GET_SUFFIX_PROC(exts, BindRenderbuffer, OES);
    }
    #endif

    if (!fboFound) {
        // we require some form of FBO
        GrAssert(!"No FBOs supported?");
    }

    #if GL_APPLE_framebuffer_multisample
    if (has_gl_extension("GL_APPLE_framebuffer_multisample")) {
        GET_SUFFIX_PROC(exts, ResolveMultisampleFramebuffer, APPLE);
    }
    #endif

    #if GL_IMG_multisampled_render_to_texture
    if (has_gl_extension("GL_IMG_multisampled_render_to_texture")) {
        GET_SUFFIX_PROC(exts, FramebufferTexture2DMultisample, IMG);
    }
    #endif

    #if GL_OES_mapbuffer
    if (has_gl_extension("GL_OES_mapbuffer")) {
        GET_SUFFIX_PROC(exts, MapBuffer, OES);
        GET_SUFFIX_PROC(exts, UnmapBuffer, OES);
    }
    #endif
#endif // !GR_SUPPORT_GLDESKTOP
}


///////////////////////////////////////////////////////////////////////////////

void GrGLCheckErr(const char* location, const char* call) {
    uint32_t err =  glGetError();
    if (GL_NO_ERROR != err) {
        GrPrintf("---- glGetError %x", err);
        if (NULL != location) {
            GrPrintf(" at\n\t%s", location);
        }
        if (NULL != call) {
            GrPrintf("\n\t\t%s", call);
        }
        GrPrintf("\n");
    }
}

///////////////////////////////////////////////////////////////////////////////

bool gLogCallsGL = !!(GR_GL_LOG_CALLS_START);

bool gCheckErrorGL = !!(GR_GL_CHECK_ERROR_START);

// Modified from chromium/src/webkit/glue/gl_bindings_skia_cmd_buffer.cc

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLExtensions.h"
#include "gl/GrGLUtil.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

static GrGLInterface* create_es_interface(GrGLVersion version,
                                          GrGLExtensions* extensions) {
    if (version < GR_GL_VER(2,0)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    interface->fStandard = kGLES_GrGLStandard;
    GrGLInterface::Functions* functions = &interface->fFunctions;

    functions->fActiveTexture = glActiveTexture;
    functions->fAttachShader = glAttachShader;
    functions->fBindAttribLocation = glBindAttribLocation;
    functions->fBindBuffer = glBindBuffer;
    functions->fBindTexture = glBindTexture;
    functions->fBindVertexArray = glBindVertexArrayOES;
    functions->fBlendColor = glBlendColor;
    functions->fBlendFunc = glBlendFunc;
    functions->fBufferData = glBufferData;
    functions->fBufferSubData = glBufferSubData;
    functions->fClear = glClear;
    functions->fClearColor = glClearColor;
    functions->fClearStencil = glClearStencil;
    functions->fColorMask = glColorMask;
    functions->fCompileShader = glCompileShader;
    functions->fCompressedTexImage2D = glCompressedTexImage2D;
    functions->fCompressedTexSubImage2D = glCompressedTexSubImage2D;
    functions->fCopyTexSubImage2D = glCopyTexSubImage2D;
    functions->fCreateProgram = glCreateProgram;
    functions->fCreateShader = glCreateShader;
    functions->fCullFace = glCullFace;
    functions->fDeleteBuffers = glDeleteBuffers;
    functions->fDeleteProgram = glDeleteProgram;
    functions->fDeleteShader = glDeleteShader;
    functions->fDeleteTextures = glDeleteTextures;
    functions->fDeleteVertexArrays = glDeleteVertexArraysOES;
    functions->fDepthMask = glDepthMask;
    functions->fDisable = glDisable;
    functions->fDisableVertexAttribArray = glDisableVertexAttribArray;
    functions->fDrawArrays = glDrawArrays;
    functions->fDrawElements = glDrawElements;
    functions->fEnable = glEnable;
    functions->fEnableVertexAttribArray = glEnableVertexAttribArray;
    functions->fFinish = glFinish;
    functions->fFlush = glFlush;
    functions->fFrontFace = glFrontFace;
    functions->fGenBuffers = glGenBuffers;
    functions->fGenerateMipmap = glGenerateMipmap;
    functions->fGenTextures = glGenTextures;
    functions->fGenVertexArrays = glGenVertexArraysOES;
    functions->fGetBufferParameteriv = glGetBufferParameteriv;
    functions->fGetError = glGetError;
    functions->fGetIntegerv = glGetIntegerv;
    functions->fGetProgramInfoLog = glGetProgramInfoLog;
    functions->fGetProgramiv = glGetProgramiv;
    functions->fGetShaderInfoLog = glGetShaderInfoLog;
    functions->fGetShaderiv = glGetShaderiv;
    functions->fGetString = glGetString;
#if GL_ES_VERSION_3_0
    functions->fGetStringi = glGetStringi;
#else
    functions->fGetStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");
#endif
    functions->fGetUniformLocation = glGetUniformLocation;
    functions->fLineWidth = glLineWidth;
    functions->fLinkProgram = glLinkProgram;
    functions->fPixelStorei = glPixelStorei;
    functions->fReadPixels = glReadPixels;
    functions->fScissor = glScissor;
#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
    functions->fShaderSource = (GrGLShaderSourceProc) glShaderSource;
#else
    functions->fShaderSource = glShaderSource;
#endif
    functions->fStencilFunc = glStencilFunc;
    functions->fStencilFuncSeparate = glStencilFuncSeparate;
    functions->fStencilMask = glStencilMask;
    functions->fStencilMaskSeparate = glStencilMaskSeparate;
    functions->fStencilOp = glStencilOp;
    functions->fStencilOpSeparate = glStencilOpSeparate;
    functions->fTexImage2D = glTexImage2D;
    functions->fTexParameteri = glTexParameteri;
    functions->fTexParameteriv = glTexParameteriv;
    functions->fTexSubImage2D = glTexSubImage2D;

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        functions->fTexStorage2D = glTexStorage2D;
#else
        functions->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2D");
#endif
    } else {
#if GL_EXT_texture_storage
        functions->fTexStorage2D = glTexStorage2DEXT;
#else
        functions->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2DEXT");
#endif
    }

#if GL_EXT_discard_framebuffer
    functions->fDiscardFramebuffer = glDiscardFramebufferEXT;
#endif
    functions->fUniform1f = glUniform1f;
    functions->fUniform1i = glUniform1i;
    functions->fUniform1fv = glUniform1fv;
    functions->fUniform1iv = glUniform1iv;
    functions->fUniform2f = glUniform2f;
    functions->fUniform2i = glUniform2i;
    functions->fUniform2fv = glUniform2fv;
    functions->fUniform2iv = glUniform2iv;
    functions->fUniform3f = glUniform3f;
    functions->fUniform3i = glUniform3i;
    functions->fUniform3fv = glUniform3fv;
    functions->fUniform3iv = glUniform3iv;
    functions->fUniform4f = glUniform4f;
    functions->fUniform4i = glUniform4i;
    functions->fUniform4fv = glUniform4fv;
    functions->fUniform4iv = glUniform4iv;
    functions->fUniformMatrix2fv = glUniformMatrix2fv;
    functions->fUniformMatrix3fv = glUniformMatrix3fv;
    functions->fUniformMatrix4fv = glUniformMatrix4fv;
    functions->fUseProgram = glUseProgram;
    functions->fVertexAttrib4fv = glVertexAttrib4fv;
    functions->fVertexAttribPointer = glVertexAttribPointer;
    functions->fViewport = glViewport;
    functions->fBindFramebuffer = glBindFramebuffer;
    functions->fBindRenderbuffer = glBindRenderbuffer;
    functions->fCheckFramebufferStatus = glCheckFramebufferStatus;
    functions->fDeleteFramebuffers = glDeleteFramebuffers;
    functions->fDeleteRenderbuffers = glDeleteRenderbuffers;
    functions->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
    functions->fFramebufferTexture2D = glFramebufferTexture2D;

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        functions->fRenderbufferStorageMultisample = glRenderbufferStorageMultisample;
        functions->fBlitFramebuffer = glBlitFramebuffer;
#else
        functions->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisample");
        functions->fBlitFramebuffer = (GrGLBlitFramebufferProc) eglGetProcAddress("glBlitFramebuffer");
#endif
    }

    if (extensions->has("GL_EXT_multisampled_render_to_texture")) {
#if GL_EXT_multisampled_render_to_texture
        functions->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleEXT;
        functions->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleEXT;
#else
        functions->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
#endif
    } else if (extensions->has("GL_IMG_multisampled_render_to_texture")) {
#if GL_IMG_multisampled_render_to_texture
        functions->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleIMG;
        functions->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleIMG;
#else
        functions->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleIMG");
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleIMG");
#endif
    }

    functions->fGenFramebuffers = glGenFramebuffers;
    functions->fGenRenderbuffers = glGenRenderbuffers;
    functions->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
    functions->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
    functions->fRenderbufferStorage = glRenderbufferStorage;

#if GL_OES_mapbuffer
    functions->fMapBuffer = glMapBufferOES;
    functions->fUnmapBuffer = glUnmapBufferOES;
#else
    functions->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBufferOES");
    functions->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBufferOES");

#endif

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        functions->fMapBufferRange = glMapBufferRange;
        functions->fFlushMappedBufferRange = glFlushMappedBufferRange;
#else
        functions->fMapBufferRange = (GrGLMapBufferRangeProc) eglGetProcAddress("glMapBufferRange");
        functions->fFlushMappedBufferRange = (GrGLFlushMappedBufferRangeProc) eglGetProcAddress("glFlushMappedBufferRange");
#endif
    } else if (extensions->has("GL_EXT_map_buffer_range")) {
#if GL_EXT_map_buffer_range
        functions->fMapBufferRange = glMapBufferRangeEXT;
        functions->fFlushMappedBufferRange = glFlushMappedBufferRangeEXT;
#else
        functions->fMapBufferRange = (GrGLMapBufferRangeProc) eglGetProcAddress("glMapBufferRangeEXT");
        functions->fFlushMappedBufferRange = (GrGLFlushMappedBufferRangeProc) eglGetProcAddress("glFlushMappedBufferRangeEXT");
#endif
    }

    if (extensions->has("GL_EXT_debug_marker")) {
        functions->fInsertEventMarker = (GrGLInsertEventMarkerProc) eglGetProcAddress("glInsertEventMarker");
        functions->fPushGroupMarker = (GrGLInsertEventMarkerProc) eglGetProcAddress("glPushGroupMarker");
        functions->fPopGroupMarker = (GrGLPopGroupMarkerProc) eglGetProcAddress("glPopGroupMarker");
        // The below check is here because a device has been found that has the extension string but
        // returns NULL from the eglGetProcAddress for the functions
        if (NULL == functions->fInsertEventMarker ||
            NULL == functions->fPushGroupMarker ||
            NULL == functions->fPopGroupMarker) {
            extensions->remove("GL_EXT_debug_marker");
        }
    }

#if GL_ES_VERSION_3_0
    functions->fInvalidateFramebuffer = glInvalidateFramebuffer;
    functions->fInvalidateSubFramebuffer = glInvalidateSubFramebuffer;
#else
    functions->fInvalidateFramebuffer = (GrGLInvalidateFramebufferProc) eglGetProcAddress("glInvalidateFramebuffer");
    functions->fInvalidateSubFramebuffer = (GrGLInvalidateSubFramebufferProc) eglGetProcAddress("glInvalidateSubFramebuffer");
#endif
    functions->fInvalidateBufferData = (GrGLInvalidateBufferDataProc) eglGetProcAddress("glInvalidateBufferData");
    functions->fInvalidateBufferSubData = (GrGLInvalidateBufferSubDataProc) eglGetProcAddress("glInvalidateBufferSubData");
    functions->fInvalidateTexImage = (GrGLInvalidateTexImageProc) eglGetProcAddress("glInvalidateTexImage");
    functions->fInvalidateTexSubImage = (GrGLInvalidateTexSubImageProc) eglGetProcAddress("glInvalidateTexSubImage");

    return interface;
}

static GrGLFuncPtr android_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);
    return eglGetProcAddress(name);
}

static const GrGLInterface* create_desktop_interface() {
    return GrGLAssembleGLInterface(NULL, android_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() {

    const char* verStr = reinterpret_cast<const char*>(glGetString(GR_GL_VERSION));
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (kGLES_GrGLStandard == standard) {
        GrGLVersion version = GrGLGetVersionFromString(verStr);
        GrGLExtensions extensions;
        GrGLGetStringiProc getStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");
        if (!extensions.init(standard, glGetString, getStringi, glGetIntegerv)) {
            return NULL;
        }
        GrGLInterface* interface = create_es_interface(version, &extensions);

        if (NULL != interface) {
            interface->fExtensions.swap(&extensions);
        }

        return interface;
    } else if (kGL_GrGLStandard == standard) {
        return create_desktop_interface();
    }

    return NULL;
}

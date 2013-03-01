
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

const GrGLInterface* GrGLCreateNativeInterface() {
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = SkNEW(GrGLInterface);
        glInterface.reset(interface);

        interface->fActiveTexture = glActiveTexture;
        interface->fAttachShader = glAttachShader;
        interface->fBindAttribLocation = glBindAttribLocation;
        interface->fBindBuffer = glBindBuffer;
        interface->fBindTexture = glBindTexture;
        interface->fBlendColor = glBlendColor;
        interface->fBlendFunc = glBlendFunc;
        interface->fBufferData = (GrGLBufferDataProc)glBufferData;
        interface->fBufferSubData = (GrGLBufferSubDataProc)glBufferSubData;
        interface->fClear = glClear;
        interface->fClearColor = glClearColor;
        interface->fClearStencil = glClearStencil;
        interface->fColorMask = glColorMask;
        interface->fCompileShader = glCompileShader;
        interface->fCompressedTexImage2D = glCompressedTexImage2D;
        interface->fCreateProgram = glCreateProgram;
        interface->fCreateShader = glCreateShader;
        interface->fCullFace = glCullFace;
        interface->fDeleteBuffers = glDeleteBuffers;
        interface->fDeleteProgram = glDeleteProgram;
        interface->fDeleteShader = glDeleteShader;
        interface->fDeleteTextures = glDeleteTextures;
        interface->fDepthMask = glDepthMask;
        interface->fDisable = glDisable;
        interface->fDisableVertexAttribArray = glDisableVertexAttribArray;
        interface->fDrawArrays = glDrawArrays;
        interface->fDrawBuffer = NULL;
        interface->fDrawBuffers = NULL;
        interface->fDrawElements = glDrawElements;
        interface->fEnable = glEnable;
        interface->fEnableVertexAttribArray = glEnableVertexAttribArray;
        interface->fFinish = glFinish;
        interface->fFlush = glFlush;
        interface->fFrontFace = glFrontFace;
        interface->fGenBuffers = glGenBuffers;
        interface->fGetBufferParameteriv = glGetBufferParameteriv;
        interface->fGetError = glGetError;
        interface->fGetIntegerv = glGetIntegerv;
        interface->fGetProgramInfoLog = glGetProgramInfoLog;
        interface->fGetProgramiv = glGetProgramiv;
        interface->fGetShaderInfoLog = glGetShaderInfoLog;
        interface->fGetShaderiv = glGetShaderiv;
        interface->fGetString = glGetString;
        interface->fGenTextures = glGenTextures;
        interface->fGetUniformLocation = glGetUniformLocation;
        interface->fLineWidth = glLineWidth;
        interface->fLinkProgram = glLinkProgram;
        interface->fPixelStorei = glPixelStorei;
        interface->fReadBuffer = NULL;
        interface->fReadPixels = glReadPixels;
        interface->fScissor = glScissor;
        interface->fShaderSource = glShaderSource;
        interface->fStencilFunc = glStencilFunc;
        interface->fStencilFuncSeparate = glStencilFuncSeparate;
        interface->fStencilMask = glStencilMask;
        interface->fStencilMaskSeparate = glStencilMaskSeparate;
        interface->fStencilOp = glStencilOp;
        interface->fStencilOpSeparate = glStencilOpSeparate;
        // mac uses GLenum for internalFormat param (non-standard)
        // amounts to int vs. uint.
        interface->fTexImage2D = (GrGLTexImage2DProc)glTexImage2D;
#if GL_ARB_texture_storage
        interface->fTexStorage2D = glTexStorage2D;
#elif GL_EXT_texture_storage
        interface->fTexStorage2D = glTexStorage2DEXT;
#endif
        interface->fTexParameteri = glTexParameteri;
        interface->fTexParameteriv = glTexParameteriv;
        interface->fTexSubImage2D = glTexSubImage2D;
        interface->fUniform1f = glUniform1f;
        interface->fUniform1i = glUniform1i;
        interface->fUniform1fv = glUniform1fv;
        interface->fUniform1iv = glUniform1iv;
        interface->fUniform2f = glUniform2f;
        interface->fUniform2i = glUniform2i;
        interface->fUniform2fv = glUniform2fv;
        interface->fUniform2iv = glUniform2iv;
        interface->fUniform3f = glUniform3f;
        interface->fUniform3i = glUniform3i;
        interface->fUniform3fv = glUniform3fv;
        interface->fUniform3iv = glUniform3iv;
        interface->fUniform4f = glUniform4f;
        interface->fUniform4i = glUniform4i;
        interface->fUniform4fv = glUniform4fv;
        interface->fUniform4iv = glUniform4iv;
        interface->fUniform4fv = glUniform4fv;
        interface->fUniformMatrix2fv = glUniformMatrix2fv;
        interface->fUniformMatrix3fv = glUniformMatrix3fv;
        interface->fUniformMatrix4fv = glUniformMatrix4fv;
        interface->fUseProgram = glUseProgram;
        interface->fVertexAttrib4fv = glVertexAttrib4fv;
        interface->fVertexAttribPointer = glVertexAttribPointer;
        interface->fViewport = glViewport;
        interface->fGenFramebuffers = glGenFramebuffers;
        interface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
        interface->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
        interface->fBindFramebuffer = glBindFramebuffer;
        interface->fFramebufferTexture2D = glFramebufferTexture2D;
        interface->fCheckFramebufferStatus = glCheckFramebufferStatus;
        interface->fDeleteFramebuffers = glDeleteFramebuffers;
        interface->fRenderbufferStorage = glRenderbufferStorage;
        interface->fGenRenderbuffers = glGenRenderbuffers;
        interface->fDeleteRenderbuffers = glDeleteRenderbuffers;
        interface->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
        interface->fBindRenderbuffer = glBindRenderbuffer;

#if GL_OES_mapbuffer
        interface->fMapBuffer = glMapBufferOES;
        interface->fUnmapBuffer = glUnmapBufferOES;
#endif

#if GL_APPLE_framebuffer_multisample
        interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleAPPLE;
        interface->fResolveMultisampleFramebuffer = glResolveMultisampleFramebufferAPPLE;
#endif

#if GL_OES_vertex_array_object
        interface->fBindVertexArray = glBindVertexArrayOES;
        interface->fDeleteVertexArrays = glDeleteVertexArraysOES;
        interface->fGenVertexArrays = glGenVertexArraysOES;
#endif

        interface->fBindingsExported = kES2_GrGLBinding;
    }
    glInterface.get()->ref();
    return glInterface.get();
}

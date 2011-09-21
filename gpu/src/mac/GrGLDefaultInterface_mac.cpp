
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLInterface.h"

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

const GrGLInterface* GrGLDefaultInterface() {
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = new GrGLInterface;
        glInterface.reset(interface);
        interface->fBindingsExported = kDesktop_GrGLBinding;
        interface->fActiveTexture = glActiveTexture;
        interface->fAttachShader = glAttachShader;
        interface->fBindAttribLocation = glBindAttribLocation;
        interface->fBindBuffer = glBindBuffer;
#if GL_VERSION_3_0
        interface->fBindFragDataLocation = glBindFragDataLocation;
#endif
        interface->fBindTexture = glBindTexture;
        interface->fBlendColor = glBlendColor;
        interface->fBlendFunc = glBlendFunc;
        interface->fBufferData = glBufferData;
        interface->fBufferSubData = glBufferSubData;
        interface->fClear = glClear;
        interface->fClearColor = glClearColor;
        interface->fClearStencil = glClearStencil;
        interface->fClientActiveTexture = glClientActiveTexture;
        interface->fColorMask = glColorMask;
        interface->fColorPointer = glColorPointer;
        interface->fColor4ub = glColor4ub;
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
        interface->fDisableClientState = glDisableClientState;
        interface->fDisableVertexAttribArray = 
                                            glDisableVertexAttribArray;
        interface->fDrawArrays = glDrawArrays;
        interface->fDrawBuffer = glDrawBuffer;
        interface->fDrawBuffers = glDrawBuffers;
        interface->fDrawElements = glDrawElements;
        interface->fEnable = glEnable;
        interface->fEnableClientState = glEnableClientState;
        interface->fEnableVertexAttribArray = glEnableVertexAttribArray;
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
        interface->fGetTexLevelParameteriv = glGetTexLevelParameteriv;
        interface->fGenTextures = glGenTextures;
        interface->fGetUniformLocation = glGetUniformLocation;
        interface->fLineWidth = glLineWidth;
        interface->fLinkProgram = glLinkProgram;
        interface->fLoadMatrixf = glLoadMatrixf;
        interface->fMapBuffer = glMapBuffer;
        interface->fMatrixMode = glMatrixMode;
        interface->fPointSize = glPointSize;
        interface->fPixelStorei = glPixelStorei;
        interface->fReadBuffer = glReadBuffer;
        interface->fReadPixels = glReadPixels;
        interface->fScissor = glScissor;
        interface->fShadeModel = glShadeModel;
        interface->fShaderSource = glShaderSource;
        interface->fStencilFunc = glStencilFunc;
        interface->fStencilFuncSeparate = glStencilFuncSeparate;
        interface->fStencilMask = glStencilMask;
        interface->fStencilMaskSeparate = glStencilMaskSeparate;
        interface->fStencilOp = glStencilOp;
        interface->fStencilOpSeparate = glStencilOpSeparate;
        interface->fTexCoordPointer = glTexCoordPointer;
        interface->fTexEnvi = glTexEnvi;
        // mac uses GLenum for internalFormat param (non-standard)
        // amounts to int vs. uint.
        interface->fTexImage2D = (GrGLTexImage2DProc)glTexImage2D;
        interface->fTexParameteri = glTexParameteri;
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
        interface->fUnmapBuffer = glUnmapBuffer;
        interface->fUseProgram = glUseProgram;
        interface->fVertexAttrib4fv = glVertexAttrib4fv;
        interface->fVertexAttribPointer = glVertexAttribPointer;
        interface->fVertexPointer = glVertexPointer;
        interface->fViewport = glViewport;

    #if GL_ARB_framebuffer_object
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
        interface->fRenderbufferStorageMultisample = 
                                        glRenderbufferStorageMultisample;
        interface->fBlitFramebuffer = glBlitFramebuffer;
    #elif GL_EXT_framebuffer_object
        interface->fGenFramebuffers = glGenFramebuffersEXT;
        interface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
        interface->fGLGetRenderbufferParameteriv = glGetRenderbufferParameterivEXT;
        interface->fBindFramebuffer = glBindFramebufferEXT;
        interface->fFramebufferTexture2D = glFramebufferTexture2DEXT;
        interface->fCheckFramebufferStatus = glCheckFramebufferStatusEXT;
        interface->fDeleteFramebuffers = glDeleteFramebuffersEXT;
        interface->fRenderbufferStorage = glRenderbufferStorageEXT;
        interface->fGenRenderbuffers = glGenRenderbuffersEXT;
        interface->fDeleteRenderbuffers = glDeleteRenderbuffersEXT;
        interface->fFramebufferRenderbuffer = 
                                                glFramebufferRenderbufferEXT;
        interface->fBindRenderbuffer = glBindRenderbufferEXT;
    #if GL_EXT_framebuffer_multisample
        interface->fRenderbufferStorageMultisample = 
                                            glRenderbufferStorageMultisampleEXT;
    #endif
    #if GL_EXT_framebuffer_blit
        interface->fBlitFramebuffer = glBlitFramebufferEXT;
    #endif
    #endif
        interface->fBindFragDataLocationIndexed = NULL;

        interface->fBindingsExported = kDesktop_GrGLBinding;
    }
    glInterface.get()->ref();
    return glInterface.get();
}

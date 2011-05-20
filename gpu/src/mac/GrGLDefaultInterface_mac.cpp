/*
    Copyright 2011 Google Inc.

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

#include "GrGLInterface.h"

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

void GrGLSetDefaultGLInterface() {
    static GrGLInterface gDefaultInterface;
    static bool gDefaultInterfaceInit;
    if (!gDefaultInterfaceInit) {
        gDefaultInterface.fActiveTexture = glActiveTexture;
        gDefaultInterface.fAttachShader = glAttachShader;
        gDefaultInterface.fBindAttribLocation = glBindAttribLocation;
        gDefaultInterface.fBindBuffer = glBindBuffer;
        gDefaultInterface.fBindTexture = glBindTexture;
        gDefaultInterface.fBlendColor = glBlendColor;
        gDefaultInterface.fBlendFunc = glBlendFunc;
        gDefaultInterface.fBufferData = glBufferData;
        gDefaultInterface.fBufferSubData = glBufferSubData;
        gDefaultInterface.fClear = glClear;
        gDefaultInterface.fClearColor = glClearColor;
        gDefaultInterface.fClearStencil = glClearStencil;
        gDefaultInterface.fClientActiveTexture = glClientActiveTexture;
        gDefaultInterface.fColorMask = glColorMask;
        gDefaultInterface.fColorPointer = glColorPointer;
        gDefaultInterface.fColor4ub = glColor4ub;
        gDefaultInterface.fCompileShader = glCompileShader;
        gDefaultInterface.fCompressedTexImage2D = glCompressedTexImage2D;
        gDefaultInterface.fCreateProgram = glCreateProgram;
        gDefaultInterface.fCreateShader = glCreateShader;
        gDefaultInterface.fCullFace = glCullFace;
        gDefaultInterface.fDeleteBuffers = glDeleteBuffers;
        gDefaultInterface.fDeleteProgram = glDeleteProgram;
        gDefaultInterface.fDeleteShader = glDeleteShader;
        gDefaultInterface.fDeleteTextures = glDeleteTextures;
        gDefaultInterface.fDepthMask = glDepthMask;
        gDefaultInterface.fDisable = glDisable;
        gDefaultInterface.fDisableClientState = glDisableClientState;
        gDefaultInterface.fDisableVertexAttribArray = 
                                            glDisableVertexAttribArray;
        gDefaultInterface.fDrawArrays = glDrawArrays;
        gDefaultInterface.fDrawElements = glDrawElements;
        gDefaultInterface.fEnable = glEnable;
        gDefaultInterface.fEnableClientState = glEnableClientState;
        gDefaultInterface.fEnableVertexAttribArray = glEnableVertexAttribArray;
        gDefaultInterface.fFrontFace = glFrontFace;
        gDefaultInterface.fGenBuffers = glGenBuffers;
        gDefaultInterface.fGetBufferParameteriv = glGetBufferParameteriv;
        gDefaultInterface.fGetError = glGetError;
        gDefaultInterface.fGetIntegerv = glGetIntegerv;
        gDefaultInterface.fGetProgramInfoLog = glGetProgramInfoLog;
        gDefaultInterface.fGetProgramiv = glGetProgramiv;
        gDefaultInterface.fGetShaderInfoLog = glGetShaderInfoLog;
        gDefaultInterface.fGetShaderiv = glGetShaderiv;
        gDefaultInterface.fGetString = glGetString;
        gDefaultInterface.fGenTextures = glGenTextures;
        gDefaultInterface.fGetUniformLocation = glGetUniformLocation;
        gDefaultInterface.fLineWidth = glLineWidth;
        gDefaultInterface.fLinkProgram = glLinkProgram;
        gDefaultInterface.fLoadMatrixf = glLoadMatrixf;
        gDefaultInterface.fMapBuffer = glMapBuffer;
        gDefaultInterface.fMatrixMode = glMatrixMode;
        gDefaultInterface.fPointSize = glPointSize;
        gDefaultInterface.fPixelStorei = glPixelStorei;
        gDefaultInterface.fReadPixels = glReadPixels;
        gDefaultInterface.fScissor = glScissor;
        gDefaultInterface.fShadeModel = glShadeModel;
        gDefaultInterface.fShaderSource = glShaderSource;
        gDefaultInterface.fStencilFunc = glStencilFunc;
        gDefaultInterface.fStencilFuncSeparate = glStencilFuncSeparate;
        gDefaultInterface.fStencilMask = glStencilMask;
        gDefaultInterface.fStencilMaskSeparate = glStencilMaskSeparate;
        gDefaultInterface.fStencilOp = glStencilOp;
        gDefaultInterface.fStencilOpSeparate = glStencilOpSeparate;
        gDefaultInterface.fTexCoordPointer = glTexCoordPointer;
        gDefaultInterface.fTexEnvi = glTexEnvi;
        // mac uses GLenum for internalFormat param (non-standard)
        // amounts to int vs. uint.
        gDefaultInterface.fTexImage2D = (GrGLTexImage2DProc)glTexImage2D;
        gDefaultInterface.fTexParameteri = glTexParameteri;
        gDefaultInterface.fTexSubImage2D = glTexSubImage2D;
        gDefaultInterface.fUniform1f = glUniform1f;
        gDefaultInterface.fUniform1i = glUniform1i;
        gDefaultInterface.fUniform1fv = glUniform1fv;
        gDefaultInterface.fUniform1iv = glUniform1iv;
        gDefaultInterface.fUniform2f = glUniform2f;
        gDefaultInterface.fUniform2i = glUniform2i;
        gDefaultInterface.fUniform2fv = glUniform2fv;
        gDefaultInterface.fUniform2iv = glUniform2iv;
        gDefaultInterface.fUniform3f = glUniform3f;
        gDefaultInterface.fUniform3i = glUniform3i;
        gDefaultInterface.fUniform3fv = glUniform3fv;
        gDefaultInterface.fUniform3iv = glUniform3iv;
        gDefaultInterface.fUniform4f = glUniform4f;
        gDefaultInterface.fUniform4i = glUniform4i;
        gDefaultInterface.fUniform4fv = glUniform4fv;
        gDefaultInterface.fUniform4iv = glUniform4iv;
        gDefaultInterface.fUniform4fv = glUniform4fv;
        gDefaultInterface.fUniformMatrix2fv = glUniformMatrix2fv;
        gDefaultInterface.fUniformMatrix3fv = glUniformMatrix3fv;
        gDefaultInterface.fUniformMatrix4fv = glUniformMatrix4fv;
        gDefaultInterface.fUnmapBuffer = glUnmapBuffer;
        gDefaultInterface.fUseProgram = glUseProgram;
        gDefaultInterface.fVertexAttrib4fv = glVertexAttrib4fv;
        gDefaultInterface.fVertexAttribPointer = glVertexAttribPointer;
        gDefaultInterface.fVertexPointer = glVertexPointer;
        gDefaultInterface.fViewport = glViewport;

#if GL_ARB_framebuffer_object
        gDefaultInterface.fGenFramebuffers = glGenFramebuffers;
        gDefaultInterface.fBindFramebuffer = glBindFramebuffer;
        gDefaultInterface.fFramebufferTexture2D = glFramebufferTexture2D;
        gDefaultInterface.fCheckFramebufferStatus = glCheckFramebufferStatus;
        gDefaultInterface.fDeleteFramebuffers = glDeleteFramebuffers;
        gDefaultInterface.fRenderbufferStorage = glRenderbufferStorage;
        gDefaultInterface.fGenRenderbuffers = glGenRenderbuffers;
        gDefaultInterface.fDeleteRenderbuffers = glDeleteRenderbuffers;
        gDefaultInterface.fFramebufferRenderbuffer = glFramebufferRenderbuffer;
        gDefaultInterface.fBindRenderbuffer = glBindRenderbuffer;
        gDefaultInterface.fRenderbufferStorageMultisample = 
                                        glRenderbufferStorageMultisample;
        gDefaultInterface.fBlitFramebuffer = glBlitFramebuffer;
#elif GL_EXT_framebuffer_object
        gDefaultInterface.fGenFramebuffers = glGenFramebuffersEXT;
        gDefaultInterface.fBindFramebuffer = glBindFramebufferEXT;
        gDefaultInterface.fFramebufferTexture2D = glFramebufferTexture2DEXT;
        gDefaultInterface.fCheckFramebufferStatus = glCheckFramebufferStatusEXT;
        gDefaultInterface.fDeleteFramebuffers = glDeleteFramebuffersEXT;
        gDefaultInterface.fRenderbufferStorage = glRenderbufferStorageEXT;
        gDefaultInterface.fGenRenderbuffers = glGenRenderbuffersEXT;
        gDefaultInterface.fDeleteRenderbuffers = glDeleteRenderbuffers;
        gDefaultInterface.fFramebufferRenderbuffer = 
                                                glFramebufferRenderbufferEXT;
        gDefaultInterface.fBindRenderbuffer = glBindRenderbufferEXT;
    #if GL_EXT_framebuffer_multisample
        gDefaultInterface.fRenderbufferStorageMultisample = 
                                            glRenderbufferStorageMultisampleEXT;
    #endif
    #if GL_EXT_framebuffer_blit
        gDefaultInterface.fBlitFramebuffer = glBlitFramebufferEXT;
    #endif
#endif
        gDefaultInterface.fBindFragDataLocationIndexed = NULL;

        gDefaultInterface.fBindingsExported = kDesktop_GrGLBinding;

        gDefaultInterfaceInit = true;
    }
    GrGLSetGLInterface(&gDefaultInterface);
}

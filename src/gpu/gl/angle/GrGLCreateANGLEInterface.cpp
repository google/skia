
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "EGL/egl.h"

#define GET_PROC(name)             \
    interface->f ## name = (GrGL ## name ## Proc) GetProcAddress(ghANGLELib, "gl" #name);

const GrGLInterface* GrGLCreateANGLEInterface() {

    static SkAutoTUnref<GrGLInterface> glInterface;
    static HMODULE ghANGLELib = NULL;

    if (NULL == ghANGLELib) {
        // We load the ANGLE library and never let it go
        ghANGLELib = LoadLibrary("libGLESv2.dll");
    }
    if (NULL == ghANGLELib) {
        // We can't setup the interface correctly w/o the DLL
        return NULL;
    }

    if (!glInterface.get()) {
        GrGLInterface* interface = new GrGLInterface;
        glInterface.reset(interface);
        interface->fBindingsExported = kES_GrGLBinding;

        GET_PROC(ActiveTexture);
        GET_PROC(AttachShader);
        GET_PROC(BindAttribLocation);
        GET_PROC(BindBuffer);
        GET_PROC(BindTexture);
        interface->fBindVertexArray =
            (GrGLBindVertexArrayProc) eglGetProcAddress("glBindVertexArrayOES");
        GET_PROC(BlendColor);
        GET_PROC(BlendFunc);
        GET_PROC(BufferData);
        GET_PROC(BufferSubData);
        GET_PROC(Clear);
        GET_PROC(ClearColor);
        GET_PROC(ClearStencil);
        GET_PROC(ColorMask);
        GET_PROC(CompileShader);
        GET_PROC(CompressedTexImage2D);
        GET_PROC(CopyTexSubImage2D);
        GET_PROC(CreateProgram);
        GET_PROC(CreateShader);
        GET_PROC(CullFace);
        GET_PROC(DeleteBuffers);
        GET_PROC(DeleteProgram);
        GET_PROC(DeleteShader);
        GET_PROC(DeleteTextures);
        interface->fDeleteVertexArrays =
            (GrGLDeleteVertexArraysProc) eglGetProcAddress("glDeleteVertexArraysOES");
        GET_PROC(DepthMask);
        GET_PROC(Disable);
        GET_PROC(DisableVertexAttribArray);
        GET_PROC(DrawArrays);
        GET_PROC(DrawElements);
        GET_PROC(Enable);
        GET_PROC(EnableVertexAttribArray);
        GET_PROC(Finish);
        GET_PROC(Flush);
        GET_PROC(FrontFace);
        GET_PROC(GenBuffers);
        GET_PROC(GenerateMipmap);
        GET_PROC(GenTextures);
        interface->fGenVertexArrays =
            (GrGLGenVertexArraysProc) eglGetProcAddress("glGenVertexArraysOES");
        GET_PROC(GetBufferParameteriv);
        GET_PROC(GetError);
        GET_PROC(GetIntegerv);
        GET_PROC(GetProgramInfoLog);
        GET_PROC(GetProgramiv);
        GET_PROC(GetShaderInfoLog);
        GET_PROC(GetShaderiv);
        GET_PROC(GetString);
        GET_PROC(GetUniformLocation);
        GET_PROC(LineWidth);
        GET_PROC(LinkProgram);
        GET_PROC(PixelStorei);
        GET_PROC(ReadPixels);
        GET_PROC(Scissor);
        GET_PROC(ShaderSource);
        GET_PROC(StencilFunc);
        GET_PROC(StencilFuncSeparate);
        GET_PROC(StencilMask);
        GET_PROC(StencilMaskSeparate);
        GET_PROC(StencilOp);
        GET_PROC(StencilOpSeparate);
        GET_PROC(TexImage2D);
        GET_PROC(TexParameteri);
        GET_PROC(TexParameteriv);
        GET_PROC(TexSubImage2D);
#if GL_ARB_texture_storage
        GET_PROC(TexStorage2D);
#elif GL_EXT_texture_storage
        interface->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2DEXT");
#endif
        GET_PROC(Uniform1f);
        GET_PROC(Uniform1i);
        GET_PROC(Uniform1fv);
        GET_PROC(Uniform1iv);

        GET_PROC(Uniform2f);
        GET_PROC(Uniform2i);
        GET_PROC(Uniform2fv);
        GET_PROC(Uniform2iv);

        GET_PROC(Uniform3f);
        GET_PROC(Uniform3i);
        GET_PROC(Uniform3fv);
        GET_PROC(Uniform3iv);

        GET_PROC(Uniform4f);
        GET_PROC(Uniform4i);
        GET_PROC(Uniform4fv);
        GET_PROC(Uniform4iv);

        GET_PROC(UniformMatrix2fv);
        GET_PROC(UniformMatrix3fv);
        GET_PROC(UniformMatrix4fv);
        GET_PROC(UseProgram);
        GET_PROC(VertexAttrib4fv);
        GET_PROC(VertexAttribPointer);
        GET_PROC(Viewport);
        GET_PROC(BindFramebuffer);
        GET_PROC(BindRenderbuffer);
        GET_PROC(CheckFramebufferStatus);
        GET_PROC(DeleteFramebuffers);
        GET_PROC(DeleteRenderbuffers);
        GET_PROC(FramebufferRenderbuffer);
        GET_PROC(FramebufferTexture2D);
        GET_PROC(GenFramebuffers);
        GET_PROC(GenRenderbuffers);
        GET_PROC(GetFramebufferAttachmentParameteriv);
        GET_PROC(GetRenderbufferParameteriv);
        GET_PROC(RenderbufferStorage);

        interface->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBufferOES");
        interface->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBufferOES");
    }
    glInterface.get()->ref();
    return glInterface.get();
}

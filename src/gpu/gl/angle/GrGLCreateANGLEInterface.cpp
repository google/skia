
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

#define GR_GET_PROC(procType, baseName)             \
    interface->f ## baseName = (procType) GetProcAddress(ghANGLELib, "gl" #baseName);

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
        interface->fBindingsExported = kES2_GrGLBinding;

        GR_GET_PROC(GrGLActiveTextureProc,      ActiveTexture);
        GR_GET_PROC(GrGLAttachShaderProc,       AttachShader);
        GR_GET_PROC(GrGLBindAttribLocationProc, BindAttribLocation);
        GR_GET_PROC(GrGLBindBufferProc,         BindBuffer);
        GR_GET_PROC(GrGLBindTextureProc,        BindTexture);
        interface->fBindVertexArray =
            (GrGLBindVertexArrayProc) eglGetProcAddress("glBindVertexArrayOES");
        GR_GET_PROC(GrGLBlendColorProc,         BlendColor);
        GR_GET_PROC(GrGLBlendFuncProc,          BlendFunc);
        GR_GET_PROC(GrGLBufferDataProc,         BufferData);
        GR_GET_PROC(GrGLBufferSubDataProc,      BufferSubData);
        GR_GET_PROC(GrGLClearProc,              Clear);
        GR_GET_PROC(GrGLClearColorProc,         ClearColor);
        GR_GET_PROC(GrGLClearStencilProc,       ClearStencil);
        GR_GET_PROC(GrGLColorMaskProc,          ColorMask);
        GR_GET_PROC(GrGLCompileShaderProc,      CompileShader);
        GR_GET_PROC(GrGLCompressedTexImage2DProc, CompressedTexImage2D);
        GR_GET_PROC(GrGLCreateProgramProc,      CreateProgram);
        GR_GET_PROC(GrGLCreateShaderProc,       CreateShader);
        GR_GET_PROC(GrGLCullFaceProc,           CullFace);
        GR_GET_PROC(GrGLDeleteBuffersProc,      DeleteBuffers);
        GR_GET_PROC(GrGLDeleteProgramProc,      DeleteProgram);
        GR_GET_PROC(GrGLDeleteShaderProc,       DeleteShader);
        GR_GET_PROC(GrGLDeleteTexturesProc,     DeleteTextures);
        interface->fDeleteVertexArrays =
            (GrGLDeleteVertexArraysProc) eglGetProcAddress("glDeleteVertexArraysOES");
        GR_GET_PROC(GrGLDepthMaskProc,          DepthMask);
        GR_GET_PROC(GrGLDisableProc,            Disable);
        GR_GET_PROC(GrGLDisableVertexAttribArrayProc, DisableVertexAttribArray);
        GR_GET_PROC(GrGLDrawArraysProc,         DrawArrays);
        GR_GET_PROC(GrGLDrawElementsProc,       DrawElements);
        GR_GET_PROC(GrGLEnableProc,             Enable);
        GR_GET_PROC(GrGLEnableVertexAttribArrayProc, EnableVertexAttribArray);
        GR_GET_PROC(GrGLFinishProc,             Finish);
        GR_GET_PROC(GrGLFlushProc,              Flush);
        GR_GET_PROC(GrGLFrontFaceProc,          FrontFace);
        GR_GET_PROC(GrGLGenBuffersProc,         GenBuffers);
        GR_GET_PROC(GrGLGenTexturesProc,        GenTextures);
        interface->fGenVertexArrays =
            (GrGLGenVertexArraysProc) eglGetProcAddress("glGenVertexArraysOES");
        GR_GET_PROC(GrGLGetBufferParameterivProc, GetBufferParameteriv);
        GR_GET_PROC(GrGLGetErrorProc,           GetError);
        GR_GET_PROC(GrGLGetIntegervProc,        GetIntegerv);
        GR_GET_PROC(GrGLGetProgramInfoLogProc,  GetProgramInfoLog);
        GR_GET_PROC(GrGLGetProgramivProc,       GetProgramiv);
        GR_GET_PROC(GrGLGetShaderInfoLogProc,   GetShaderInfoLog);
        GR_GET_PROC(GrGLGetShaderivProc,        GetShaderiv);
        GR_GET_PROC(GrGLGetStringProc,          GetString);
        GR_GET_PROC(GrGLGetUniformLocationProc, GetUniformLocation);
        GR_GET_PROC(GrGLLineWidthProc,          LineWidth);
        GR_GET_PROC(GrGLLinkProgramProc,        LinkProgram);
        GR_GET_PROC(GrGLPixelStoreiProc,        PixelStorei);
        GR_GET_PROC(GrGLReadPixelsProc,         ReadPixels);
        GR_GET_PROC(GrGLScissorProc,            Scissor);
        GR_GET_PROC(GrGLShaderSourceProc,       ShaderSource);
        GR_GET_PROC(GrGLStencilFuncProc,        StencilFunc);
        GR_GET_PROC(GrGLStencilFuncSeparateProc, StencilFuncSeparate);
        GR_GET_PROC(GrGLStencilMaskProc,        StencilMask);
        GR_GET_PROC(GrGLStencilMaskSeparateProc, StencilMaskSeparate);
        GR_GET_PROC(GrGLStencilOpProc,          StencilOp);
        GR_GET_PROC(GrGLStencilOpSeparateProc,  StencilOpSeparate);
        GR_GET_PROC(GrGLTexImage2DProc,         TexImage2D);
        GR_GET_PROC(GrGLTexParameteriProc,      TexParameteri);
        GR_GET_PROC(GrGLTexParameterivProc,     TexParameteriv);
        GR_GET_PROC(GrGLTexSubImage2DProc,      TexSubImage2D);
#if GL_ARB_texture_storage
        GR_GET_PROC(GrGLTexStorage2DProc,       TexStorage2D);
#elif GL_EXT_texture_storage
        interface->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2DEXT");
#endif
        GR_GET_PROC(GrGLUniform1fProc,          Uniform1f);
        GR_GET_PROC(GrGLUniform1iProc,          Uniform1i);
        GR_GET_PROC(GrGLUniform1fvProc,         Uniform1fv);
        GR_GET_PROC(GrGLUniform1ivProc,         Uniform1iv);

        GR_GET_PROC(GrGLUniform2fProc,          Uniform2f);
        GR_GET_PROC(GrGLUniform2iProc,          Uniform2i);
        GR_GET_PROC(GrGLUniform2fvProc,         Uniform2fv);
        GR_GET_PROC(GrGLUniform2ivProc,         Uniform2iv);

        GR_GET_PROC(GrGLUniform3fProc,          Uniform3f);
        GR_GET_PROC(GrGLUniform3iProc,          Uniform3i);
        GR_GET_PROC(GrGLUniform3fvProc,         Uniform3fv);
        GR_GET_PROC(GrGLUniform3ivProc,         Uniform3iv);

        GR_GET_PROC(GrGLUniform4fProc,          Uniform4f);
        GR_GET_PROC(GrGLUniform4iProc,          Uniform4i);
        GR_GET_PROC(GrGLUniform4fvProc,         Uniform4fv);
        GR_GET_PROC(GrGLUniform4ivProc,         Uniform4iv);

        GR_GET_PROC(GrGLUniformMatrix2fvProc,   UniformMatrix2fv);
        GR_GET_PROC(GrGLUniformMatrix3fvProc,   UniformMatrix3fv);
        GR_GET_PROC(GrGLUniformMatrix4fvProc,   UniformMatrix4fv);
        GR_GET_PROC(GrGLUseProgramProc,         UseProgram);
        GR_GET_PROC(GrGLVertexAttrib4fvProc,    VertexAttrib4fv);
        GR_GET_PROC(GrGLVertexAttribPointerProc, VertexAttribPointer);
        GR_GET_PROC(GrGLViewportProc,           Viewport);
        GR_GET_PROC(GrGLBindFramebufferProc,    BindFramebuffer);
        GR_GET_PROC(GrGLBindRenderbufferProc,   BindRenderbuffer);
        GR_GET_PROC(GrGLCheckFramebufferStatusProc, CheckFramebufferStatus);
        GR_GET_PROC(GrGLDeleteFramebuffersProc, DeleteFramebuffers);
        GR_GET_PROC(GrGLDeleteRenderbuffersProc, DeleteRenderbuffers);
        GR_GET_PROC(GrGLFramebufferRenderbufferProc, FramebufferRenderbuffer);
        GR_GET_PROC(GrGLFramebufferTexture2DProc, FramebufferTexture2D);
        GR_GET_PROC(GrGLGenFramebuffersProc,    GenFramebuffers);
        GR_GET_PROC(GrGLGenRenderbuffersProc,   GenRenderbuffers);
        GR_GET_PROC(GrGLGetFramebufferAttachmentParameterivProc,
                                GetFramebufferAttachmentParameteriv);
        GR_GET_PROC(GrGLGetRenderbufferParameterivProc,
                                GetRenderbufferParameteriv);
        GR_GET_PROC(GrGLRenderbufferStorageProc, RenderbufferStorage);

        interface->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBufferOES");
        interface->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBufferOES");
    }
    glInterface.get()->ref();
    return glInterface.get();
}

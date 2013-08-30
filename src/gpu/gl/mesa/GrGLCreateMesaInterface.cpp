
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLExtensions.h"
#include "gl/GrGLInterface.h"
#include "../GrGLUtil.h"

#define GL_GLEXT_PROTOTYPES
#include "osmesa_wrapper.h"

#define GR_GL_GET_PROC(F) interface->f ## F = (GrGL ## F ## Proc) \
        OSMesaGetProcAddress("gl" #F);
#define GR_GL_GET_PROC_SUFFIX(F, S) interface->f ## F = (GrGL ## F ## Proc) \
        OSMesaGetProcAddress("gl" #F #S);

// We use OSMesaGetProcAddress for every gl function to avoid accidentally using
// non-Mesa gl functions.

const GrGLInterface* GrGLCreateMesaInterface() {
    if (NULL != OSMesaGetCurrentContext()) {

        GrGLGetStringProc getString = (GrGLGetStringProc) OSMesaGetProcAddress("glGetString");
        GrGLGetStringiProc getStringi = (GrGLGetStringiProc) OSMesaGetProcAddress("glGetStringi");
        GrGLGetIntegervProc getIntegerv =
            (GrGLGetIntegervProc) OSMesaGetProcAddress("glGetIntegerv");

        GrGLExtensions extensions;
        if (!extensions.init(kDesktop_GrGLBinding, getString, getStringi, getIntegerv)) {
            return NULL;
        }

        const char* versionString = (const char*) getString(GL_VERSION);
        GrGLVersion glVer = GrGLGetVersionFromString(versionString);

        if (glVer < GR_GL_VER(1,5)) {
            // We must have array and element_array buffer objects.
            return NULL;
        }
        GrGLInterface* interface = new GrGLInterface();

        GR_GL_GET_PROC(ActiveTexture);
        GR_GL_GET_PROC(BeginQuery);
        GR_GL_GET_PROC(AttachShader);
        GR_GL_GET_PROC(BindAttribLocation);
        GR_GL_GET_PROC(BindBuffer);
        GR_GL_GET_PROC(BindFragDataLocation);
        GR_GL_GET_PROC(BindTexture);
        GR_GL_GET_PROC(BlendFunc);

        if (glVer >= GR_GL_VER(1,4) ||
            extensions.has("GL_ARB_imaging") ||
            extensions.has("GL_EXT_blend_color")) {
            GR_GL_GET_PROC(BlendColor);
        }

        GR_GL_GET_PROC(BufferData);
        GR_GL_GET_PROC(BufferSubData);
        GR_GL_GET_PROC(Clear);
        GR_GL_GET_PROC(ClearColor);
        GR_GL_GET_PROC(ClearStencil);
        GR_GL_GET_PROC(ClientActiveTexture);
        GR_GL_GET_PROC(ColorMask);
        GR_GL_GET_PROC(CompileShader);
        GR_GL_GET_PROC(CompressedTexImage2D);
        GR_GL_GET_PROC(CopyTexSubImage2D);
        GR_GL_GET_PROC(CreateProgram);
        GR_GL_GET_PROC(CreateShader);
        GR_GL_GET_PROC(CullFace);
        GR_GL_GET_PROC(DeleteBuffers);
        GR_GL_GET_PROC(DeleteProgram);
        GR_GL_GET_PROC(DeleteQueries);
        GR_GL_GET_PROC(DeleteShader);
        GR_GL_GET_PROC(DeleteTextures);
        GR_GL_GET_PROC(DepthMask);
        GR_GL_GET_PROC(Disable);
        GR_GL_GET_PROC(DisableClientState);
        GR_GL_GET_PROC(DisableVertexAttribArray);
        GR_GL_GET_PROC(DrawArrays);
        GR_GL_GET_PROC(DrawBuffer);
        GR_GL_GET_PROC(DrawBuffers);
        GR_GL_GET_PROC(DrawElements);
        GR_GL_GET_PROC(Enable);
        GR_GL_GET_PROC(EnableClientState);
        GR_GL_GET_PROC(EnableVertexAttribArray);
        GR_GL_GET_PROC(EndQuery);
        GR_GL_GET_PROC(Finish);
        GR_GL_GET_PROC(Flush);
        GR_GL_GET_PROC(FrontFace);
        GR_GL_GET_PROC(GenBuffers);
        GR_GL_GET_PROC(GenerateMipmap);
        GR_GL_GET_PROC(GenQueries);
        GR_GL_GET_PROC(GetBufferParameteriv);
        GR_GL_GET_PROC(GetError);
        GR_GL_GET_PROC(GetIntegerv);
        GR_GL_GET_PROC(GetProgramInfoLog);
        GR_GL_GET_PROC(GetProgramiv);
        if (glVer >= GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
            GR_GL_GET_PROC(GetQueryObjecti64v);
            GR_GL_GET_PROC(GetQueryObjectui64v)
            GR_GL_GET_PROC(QueryCounter);
        } else if (extensions.has("GL_EXT_timer_query")) {
            GR_GL_GET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
            GR_GL_GET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
        }
        GR_GL_GET_PROC(GetQueryObjectiv);
        GR_GL_GET_PROC(GetQueryObjectuiv);
        GR_GL_GET_PROC(GetQueryiv);
        GR_GL_GET_PROC(GetShaderInfoLog);
        GR_GL_GET_PROC(GetShaderiv);
        GR_GL_GET_PROC(GetString);
        GR_GL_GET_PROC(GetStringi);
        GR_GL_GET_PROC(GetTexLevelParameteriv);
        GR_GL_GET_PROC(GenTextures);
        GR_GL_GET_PROC(GetUniformLocation);
        GR_GL_GET_PROC(LineWidth);
        GR_GL_GET_PROC(LinkProgram);
        GR_GL_GET_PROC(LoadIdentity);
        GR_GL_GET_PROC(LoadMatrixf);
        GR_GL_GET_PROC(MatrixMode);
        GR_GL_GET_PROC(MapBuffer);
        GR_GL_GET_PROC(PixelStorei);
        GR_GL_GET_PROC(ReadBuffer);
        GR_GL_GET_PROC(ReadPixels);
        GR_GL_GET_PROC(Scissor);
        GR_GL_GET_PROC(ShaderSource);
        GR_GL_GET_PROC(StencilFunc);
        GR_GL_GET_PROC(StencilFuncSeparate);
        GR_GL_GET_PROC(StencilMask);
        GR_GL_GET_PROC(StencilMaskSeparate);
        GR_GL_GET_PROC(StencilOp);
        GR_GL_GET_PROC(StencilOpSeparate);
        GR_GL_GET_PROC(TexGenf);
        GR_GL_GET_PROC(TexGenfv);
        GR_GL_GET_PROC(TexGeni);
        GR_GL_GET_PROC(TexImage2D)
        GR_GL_GET_PROC(TexParameteri);
        GR_GL_GET_PROC(TexParameteriv);
        GR_GL_GET_PROC(TexStorage2D);
        if (NULL == interface->fTexStorage2D) {
            GR_GL_GET_PROC_SUFFIX(TexStorage2D, EXT);
        }
        GR_GL_GET_PROC(TexSubImage2D);
        GR_GL_GET_PROC(Uniform1f);
        GR_GL_GET_PROC(Uniform1i);
        GR_GL_GET_PROC(Uniform1fv);
        GR_GL_GET_PROC(Uniform1iv);
        GR_GL_GET_PROC(Uniform2f);
        GR_GL_GET_PROC(Uniform2i);
        GR_GL_GET_PROC(Uniform2fv);
        GR_GL_GET_PROC(Uniform2iv);
        GR_GL_GET_PROC(Uniform3f);
        GR_GL_GET_PROC(Uniform3i);
        GR_GL_GET_PROC(Uniform3fv);
        GR_GL_GET_PROC(Uniform3iv);
        GR_GL_GET_PROC(Uniform4f);
        GR_GL_GET_PROC(Uniform4i);
        GR_GL_GET_PROC(Uniform4fv);
        GR_GL_GET_PROC(Uniform4iv);
        GR_GL_GET_PROC(UniformMatrix2fv);
        GR_GL_GET_PROC(UniformMatrix3fv);
        GR_GL_GET_PROC(UniformMatrix4fv);
        GR_GL_GET_PROC(UnmapBuffer);
        GR_GL_GET_PROC(UseProgram);
        GR_GL_GET_PROC(VertexAttrib4fv);
        GR_GL_GET_PROC(VertexAttribPointer);
        GR_GL_GET_PROC(VertexPointer);
        GR_GL_GET_PROC(Viewport);

        if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
            // no ARB suffix for GL_ARB_vertex_array_object
            GR_GL_GET_PROC(BindVertexArray);
            GR_GL_GET_PROC(DeleteVertexArrays);
            GR_GL_GET_PROC(GenVertexArrays);
        }

        // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
        // GL_ARB_framebuffer_object doesn't use ARB suffix.)
        if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
            GR_GL_GET_PROC(GenFramebuffers);
            GR_GL_GET_PROC(GetFramebufferAttachmentParameteriv);
            GR_GL_GET_PROC(GetRenderbufferParameteriv);
            GR_GL_GET_PROC(BindFramebuffer);
            GR_GL_GET_PROC(FramebufferTexture2D);
            GR_GL_GET_PROC(CheckFramebufferStatus);
            GR_GL_GET_PROC(DeleteFramebuffers);
            GR_GL_GET_PROC(RenderbufferStorage);
            GR_GL_GET_PROC(GenRenderbuffers);
            GR_GL_GET_PROC(DeleteRenderbuffers);
            GR_GL_GET_PROC(FramebufferRenderbuffer);
            GR_GL_GET_PROC(BindRenderbuffer);
            GR_GL_GET_PROC(RenderbufferStorageMultisample);
            GR_GL_GET_PROC(BlitFramebuffer);
        } else if (extensions.has("GL_EXT_framebuffer_object")) {
            GR_GL_GET_PROC_SUFFIX(GenFramebuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(GetFramebufferAttachmentParameteriv, EXT);
            GR_GL_GET_PROC_SUFFIX(GetRenderbufferParameteriv, EXT);
            GR_GL_GET_PROC_SUFFIX(BindFramebuffer, EXT);
            GR_GL_GET_PROC_SUFFIX(FramebufferTexture2D, EXT);
            GR_GL_GET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
            GR_GL_GET_PROC_SUFFIX(DeleteFramebuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(RenderbufferStorage, EXT);
            GR_GL_GET_PROC_SUFFIX(GenRenderbuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
            GR_GL_GET_PROC_SUFFIX(BindRenderbuffer, EXT);
            if (extensions.has("GL_EXT_framebuffer_multisample")) {
                GR_GL_GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
            }
            if (extensions.has("GL_EXT_framebuffer_blit")) {
                GR_GL_GET_PROC_SUFFIX(BlitFramebuffer, EXT);
            }
        } else {
            // we must have FBOs
            delete interface;
            return NULL;
        }
        GR_GL_GET_PROC(BindFragDataLocationIndexed);
        interface->fBindingsExported = kDesktop_GrGLBinding;
        return interface;
    } else {
        return NULL;
    }
}

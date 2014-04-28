
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

#define GET_PROC(F) interface->fFunctions.f ## F = (GrGL ## F ## Proc) \
        OSMesaGetProcAddress("gl" #F);
#define GET_PROC_SUFFIX(F, S) interface->fFunctions.f ## F = (GrGL ## F ## Proc) \
        OSMesaGetProcAddress("gl" #F #S);

// We use OSMesaGetProcAddress for every gl function to avoid accidentally using
// non-Mesa gl functions.

const GrGLInterface* GrGLCreateMesaInterface() {
    if (NULL == OSMesaGetCurrentContext()) {
        return NULL;
    }

    GrGLGetStringProc getString = (GrGLGetStringProc) OSMesaGetProcAddress("glGetString");
    GrGLGetStringiProc getStringi = (GrGLGetStringiProc) OSMesaGetProcAddress("glGetStringi");
    GrGLGetIntegervProc getIntegerv =
        (GrGLGetIntegervProc) OSMesaGetProcAddress("glGetIntegerv");

    GrGLExtensions extensions;
    if (!extensions.init(kGL_GrGLStandard, getString, getStringi, getIntegerv)) {
        return NULL;
    }

    const char* versionString = (const char*) getString(GL_VERSION);
    GrGLVersion glVer = GrGLGetVersionFromString(versionString);

    if (glVer < GR_GL_VER(1,5)) {
        // We must have array and element_array buffer objects.
        return NULL;
    }
    GrGLInterface* interface = SkNEW(GrGLInterface());

    GET_PROC(ActiveTexture);
    GET_PROC(BeginQuery);
    GET_PROC(AttachShader);
    GET_PROC(BindAttribLocation);
    GET_PROC(BindBuffer);
    GET_PROC(BindFragDataLocation);
    GET_PROC(BindTexture);
    GET_PROC(BlendFunc);

    if (glVer >= GR_GL_VER(1,4) ||
        extensions.has("GL_ARB_imaging") ||
        extensions.has("GL_EXT_blend_color")) {
        GET_PROC(BlendColor);
    }

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
    GET_PROC(DeleteQueries);
    GET_PROC(DeleteShader);
    GET_PROC(DeleteTextures);
    GET_PROC(DepthMask);
    GET_PROC(Disable);
    GET_PROC(DisableVertexAttribArray);
    GET_PROC(DrawArrays);
    GET_PROC(DrawBuffer);
    GET_PROC(DrawBuffers);
    GET_PROC(DrawElements);
    GET_PROC(Enable);
    GET_PROC(EnableVertexAttribArray);
    GET_PROC(EndQuery);
    GET_PROC(Finish);
    GET_PROC(Flush);
    GET_PROC(FrontFace);
    GET_PROC(GenBuffers);
    GET_PROC(GenerateMipmap);
    GET_PROC(GenQueries);
    GET_PROC(GetBufferParameteriv);
    GET_PROC(GetError);
    GET_PROC(GetIntegerv);
    GET_PROC(GetProgramInfoLog);
    GET_PROC(GetProgramiv);
    if (glVer >= GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
        GET_PROC(GetQueryObjecti64v);
        GET_PROC(GetQueryObjectui64v)
        GET_PROC(QueryCounter);
    } else if (extensions.has("GL_EXT_timer_query")) {
        GET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
        GET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
    }
    GET_PROC(GetQueryObjectiv);
    GET_PROC(GetQueryObjectuiv);
    GET_PROC(GetQueryiv);
    GET_PROC(GetShaderInfoLog);
    GET_PROC(GetShaderiv);
    GET_PROC(GetString);
    GET_PROC(GetStringi);
    GET_PROC(GetTexLevelParameteriv);
    GET_PROC(GenTextures);
    GET_PROC(GetUniformLocation);
    GET_PROC(LineWidth);
    GET_PROC(LinkProgram);
    GET_PROC(MapBuffer);
    if (extensions.has("GL_EXT_direct_state_access")) {
        GET_PROC_SUFFIX(MatrixLoadf, EXT);
        GET_PROC_SUFFIX(MatrixLoadIdentity, EXT);
    }
    GET_PROC(PixelStorei);
    GET_PROC(ReadBuffer);
    GET_PROC(ReadPixels);
    GET_PROC(Scissor);
    GET_PROC(ShaderSource);
    GET_PROC(StencilFunc);
    GET_PROC(StencilFuncSeparate);
    GET_PROC(StencilMask);
    GET_PROC(StencilMaskSeparate);
    GET_PROC(StencilOp);
    GET_PROC(StencilOpSeparate);
    GET_PROC(TexImage2D)
    GET_PROC(TexParameteri);
    GET_PROC(TexParameteriv);
    GET_PROC(TexStorage2D);
    if (NULL == interface->fFunctions.fTexStorage2D) {
        GET_PROC_SUFFIX(TexStorage2D, EXT);
    }
    GET_PROC(TexSubImage2D);
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
    GET_PROC(UnmapBuffer);
    GET_PROC(UseProgram);
    GET_PROC(VertexAttrib4fv);
    GET_PROC(VertexAttribPointer);
    GET_PROC(Viewport);

    if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
        // no ARB suffix for GL_ARB_vertex_array_object
        GET_PROC(BindVertexArray);
        GET_PROC(DeleteVertexArrays);
        GET_PROC(GenVertexArrays);
    }

    // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
    // GL_ARB_framebuffer_object doesn't use ARB suffix.)
    if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
        GET_PROC(GenFramebuffers);
        GET_PROC(GetFramebufferAttachmentParameteriv);
        GET_PROC(GetRenderbufferParameteriv);
        GET_PROC(BindFramebuffer);
        GET_PROC(FramebufferTexture2D);
        GET_PROC(CheckFramebufferStatus);
        GET_PROC(DeleteFramebuffers);
        GET_PROC(RenderbufferStorage);
        GET_PROC(GenRenderbuffers);
        GET_PROC(DeleteRenderbuffers);
        GET_PROC(FramebufferRenderbuffer);
        GET_PROC(BindRenderbuffer);
        GET_PROC(RenderbufferStorageMultisample);
        GET_PROC(BlitFramebuffer);
    } else if (extensions.has("GL_EXT_framebuffer_object")) {
        GET_PROC_SUFFIX(GenFramebuffers, EXT);
        GET_PROC_SUFFIX(GetFramebufferAttachmentParameteriv, EXT);
        GET_PROC_SUFFIX(GetRenderbufferParameteriv, EXT);
        GET_PROC_SUFFIX(BindFramebuffer, EXT);
        GET_PROC_SUFFIX(FramebufferTexture2D, EXT);
        GET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
        GET_PROC_SUFFIX(DeleteFramebuffers, EXT);
        GET_PROC_SUFFIX(RenderbufferStorage, EXT);
        GET_PROC_SUFFIX(GenRenderbuffers, EXT);
        GET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
        GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
        GET_PROC_SUFFIX(BindRenderbuffer, EXT);
        if (extensions.has("GL_EXT_framebuffer_multisample")) {
            GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
        }
        if (extensions.has("GL_EXT_framebuffer_blit")) {
            GET_PROC_SUFFIX(BlitFramebuffer, EXT);
        }
    } else {
        // we must have FBOs
        delete interface;
        return NULL;
    }
    GET_PROC(BindFragDataLocationIndexed);

    if (extensions.has("GL_EXT_debug_marker")) {
        GET_PROC_SUFFIX(InsertEventMarker, EXT);
        GET_PROC_SUFFIX(PopGroupMarker, EXT);
        GET_PROC_SUFFIX(PushGroupMarker, EXT);
    }

    if (glVer >= GR_GL_VER(4,3) || extensions.has("GL_ARB_invalidate_subdata")) {
        GET_PROC(InvalidateBufferData);
        GET_PROC(InvalidateBufferSubData);
        GET_PROC(InvalidateFramebuffer);
        GET_PROC(InvalidateSubFramebuffer);
        GET_PROC(InvalidateTexImage);
        GET_PROC(InvalidateTexSubImage);
    }

    interface->fStandard = kGL_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return interface;
}

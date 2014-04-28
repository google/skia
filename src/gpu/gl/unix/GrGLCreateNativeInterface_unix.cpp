
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "../GrGLUtil.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#define GET_PROC(F) interface->fFunctions.f ## F = (GrGL ## F ## Proc) \
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("gl" #F));
#define GET_PROC_SUFFIX(F, S) interface->fFunctions.f ## F = (GrGL ## F ## Proc) \
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("gl" #F #S));

const GrGLInterface* GrGLCreateNativeInterface() {
    if (NULL == glXGetCurrentContext()) {
        return NULL;
    }

    const char* versionString = (const char*) glGetString(GL_VERSION);
    GrGLVersion glVer = GrGLGetVersionFromString(versionString);

    // This may or may not succeed depending on the gl version.
    GrGLGetStringiProc glGetStringi =
        (GrGLGetStringiProc) glXGetProcAddress(reinterpret_cast<const GLubyte*>("glGetStringi"));

    GrGLExtensions extensions;
    if (!extensions.init(kGL_GrGLStandard, glGetString, glGetStringi, glGetIntegerv)) {
        return NULL;
    }

    if (glVer < GR_GL_VER(1,5)) {
        // We must have array and element_array buffer objects.
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface());
    GrGLInterface::Functions* functions = &interface->fFunctions;

    functions->fActiveTexture = glActiveTexture;
    GET_PROC(AttachShader);
    GET_PROC(BindAttribLocation);
    GET_PROC(BindBuffer);
    GET_PROC(BindFragDataLocation);
    GET_PROC(BeginQuery);
    functions->fBindTexture = glBindTexture;
    functions->fBlendFunc = glBlendFunc;

    if (glVer >= GR_GL_VER(1,4) ||
        extensions.has("GL_ARB_imaging") ||
        extensions.has("GL_EXT_blend_color")) {
        GET_PROC(BlendColor);
    }

    GET_PROC(BufferData);
    GET_PROC(BufferSubData);
    functions->fClear = glClear;
    functions->fClearColor = glClearColor;
    functions->fClearStencil = glClearStencil;
    functions->fColorMask = glColorMask;
    GET_PROC(CompileShader);
    functions->fCompressedTexImage2D = glCompressedTexImage2D;
    functions->fCopyTexSubImage2D = glCopyTexSubImage2D;
    GET_PROC(CreateProgram);
    GET_PROC(CreateShader);
    functions->fCullFace = glCullFace;
    GET_PROC(DeleteBuffers);
    GET_PROC(DeleteProgram);
    GET_PROC(DeleteQueries);
    GET_PROC(DeleteShader);
    functions->fDeleteTextures = glDeleteTextures;
    functions->fDepthMask = glDepthMask;
    functions->fDisable = glDisable;
    GET_PROC(DisableVertexAttribArray);
    functions->fDrawArrays = glDrawArrays;
    functions->fDrawBuffer = glDrawBuffer;
    GET_PROC(DrawBuffers);
    functions->fDrawElements = glDrawElements;
    functions->fEnable = glEnable;
    GET_PROC(EnableVertexAttribArray);
    GET_PROC(EndQuery);
    functions->fFinish = glFinish;
    functions->fFlush = glFlush;
    functions->fFrontFace = glFrontFace;
    GET_PROC(GenBuffers);
    GET_PROC(GenerateMipmap);
    GET_PROC(GetBufferParameteriv);
    functions->fGetError = glGetError;
    functions->fGetIntegerv = glGetIntegerv;
    GET_PROC(GetQueryObjectiv);
    GET_PROC(GetQueryObjectuiv);
    if (glVer >= GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
        GET_PROC(GetQueryObjecti64v);
        GET_PROC(GetQueryObjectui64v);
        GET_PROC(QueryCounter);
    } else if (extensions.has("GL_EXT_timer_query")) {
        GET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
        GET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
    }
    GET_PROC(GetQueryiv);
    GET_PROC(GetProgramInfoLog);
    GET_PROC(GetProgramiv);
    GET_PROC(GetShaderInfoLog);
    GET_PROC(GetShaderiv);
    functions->fGetString = glGetString;
    GET_PROC(GetStringi);
    functions->fGetTexLevelParameteriv = glGetTexLevelParameteriv;
    GET_PROC(GenQueries);
    functions->fGenTextures = glGenTextures;
    GET_PROC(GetUniformLocation);
    functions->fLineWidth = glLineWidth;
    GET_PROC(LinkProgram);
    GET_PROC(MapBuffer);
    if (extensions.has("GL_EXT_direct_state_access")) {
        GET_PROC_SUFFIX(MatrixLoadf, EXT);
        GET_PROC_SUFFIX(MatrixLoadIdentity, EXT);
    }
    functions->fPixelStorei = glPixelStorei;
    functions->fReadBuffer = glReadBuffer;
    functions->fReadPixels = glReadPixels;
    functions->fScissor = glScissor;
    GET_PROC(ShaderSource);
    functions->fStencilFunc = glStencilFunc;
    GET_PROC(StencilFuncSeparate);
    functions->fStencilMask = glStencilMask;
    GET_PROC(StencilMaskSeparate);
    functions->fStencilOp = glStencilOp;
    GET_PROC(StencilOpSeparate);
    functions->fTexImage2D = glTexImage2D;
    functions->fTexParameteri = glTexParameteri;
    functions->fTexParameteriv = glTexParameteriv;
    if (glVer >= GR_GL_VER(4,2) || extensions.has("GL_ARB_texture_storage")) {
        GET_PROC(TexStorage2D);
    } else if (extensions.has("GL_EXT_texture_storage")) {
        GET_PROC_SUFFIX(TexStorage2D, EXT);
    }
    functions->fTexSubImage2D = glTexSubImage2D;
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
    functions->fViewport = glViewport;
    GET_PROC(BindFragDataLocationIndexed);

    if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
        // no ARB suffix for GL_ARB_vertex_array_object
        GET_PROC(BindVertexArray);
        GET_PROC(GenVertexArrays);
        GET_PROC(DeleteVertexArrays);
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

    if (extensions.has("GL_NV_path_rendering")) {
        GET_PROC_SUFFIX(PathCommands, NV);
        GET_PROC_SUFFIX(PathCoords, NV);
        GET_PROC_SUFFIX(PathSubCommands, NV);
        GET_PROC_SUFFIX(PathSubCoords, NV);
        GET_PROC_SUFFIX(PathString, NV);
        GET_PROC_SUFFIX(PathGlyphs, NV);
        GET_PROC_SUFFIX(PathGlyphRange, NV);
        GET_PROC_SUFFIX(WeightPaths, NV);
        GET_PROC_SUFFIX(CopyPath, NV);
        GET_PROC_SUFFIX(InterpolatePaths, NV);
        GET_PROC_SUFFIX(TransformPath, NV);
        GET_PROC_SUFFIX(PathParameteriv, NV);
        GET_PROC_SUFFIX(PathParameteri, NV);
        GET_PROC_SUFFIX(PathParameterfv, NV);
        GET_PROC_SUFFIX(PathParameterf, NV);
        GET_PROC_SUFFIX(PathDashArray, NV);
        GET_PROC_SUFFIX(GenPaths, NV);
        GET_PROC_SUFFIX(DeletePaths, NV);
        GET_PROC_SUFFIX(IsPath, NV);
        GET_PROC_SUFFIX(PathStencilFunc, NV);
        GET_PROC_SUFFIX(PathStencilDepthOffset, NV);
        GET_PROC_SUFFIX(StencilFillPath, NV);
        GET_PROC_SUFFIX(StencilStrokePath, NV);
        GET_PROC_SUFFIX(StencilFillPathInstanced, NV);
        GET_PROC_SUFFIX(StencilStrokePathInstanced, NV);
        GET_PROC_SUFFIX(PathCoverDepthFunc, NV);
        GET_PROC_SUFFIX(PathColorGen, NV);
        GET_PROC_SUFFIX(PathTexGen, NV);
        GET_PROC_SUFFIX(PathFogGen, NV);
        GET_PROC_SUFFIX(CoverFillPath, NV);
        GET_PROC_SUFFIX(CoverStrokePath, NV);
        GET_PROC_SUFFIX(CoverFillPathInstanced, NV);
        GET_PROC_SUFFIX(CoverStrokePathInstanced, NV);
        GET_PROC_SUFFIX(GetPathParameteriv, NV);
        GET_PROC_SUFFIX(GetPathParameterfv, NV);
        GET_PROC_SUFFIX(GetPathCommands, NV);
        GET_PROC_SUFFIX(GetPathCoords, NV);
        GET_PROC_SUFFIX(GetPathDashArray, NV);
        GET_PROC_SUFFIX(GetPathMetrics, NV);
        GET_PROC_SUFFIX(GetPathMetricRange, NV);
        GET_PROC_SUFFIX(GetPathSpacing, NV);
        GET_PROC_SUFFIX(GetPathColorGeniv, NV);
        GET_PROC_SUFFIX(GetPathColorGenfv, NV);
        GET_PROC_SUFFIX(GetPathTexGeniv, NV);
        GET_PROC_SUFFIX(GetPathTexGenfv, NV);
        GET_PROC_SUFFIX(IsPointInFillPath, NV);
        GET_PROC_SUFFIX(IsPointInStrokePath, NV);
        GET_PROC_SUFFIX(GetPathLength, NV);
        GET_PROC_SUFFIX(PointAlongPath, NV);
    }

    if (extensions.has("GL_EXT_debug_marker")) {
        GET_PROC_SUFFIX(InsertEventMarker, EXT);
        GET_PROC_SUFFIX(PushGroupMarker, EXT);
        GET_PROC_SUFFIX(PopGroupMarker, EXT);
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

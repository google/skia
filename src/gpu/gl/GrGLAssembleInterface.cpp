/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLAssembleInterface.h"
#include "GrGLUtil.h"

#define GET_PROC(F) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

#define GET_EGL_PROC_SUFFIX(F, S) functions->fEGL##F = (GrEGL##F##Fn*)get(ctx, "egl" #F #S)

sk_sp<const GrGLInterface> GrGLMakeAssembledInterface(void *ctx, GrGLGetProc get) {
#if IS_WEBGL==1
    return GrGLMakeAssembledGLESInterface(ctx, get);
#else
    GET_PROC_LOCAL(GetString);
    if (nullptr == GetString) {
        return nullptr;
    }

    const char* verStr = reinterpret_cast<const char*>(GetString(GR_GL_VERSION));
    if (nullptr == verStr) {
        return nullptr;
    }

    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (kGLES_GrGLStandard == standard) {
        return GrGLMakeAssembledGLESInterface(ctx, get);
    } else if (kGL_GrGLStandard == standard) {
        return GrGLMakeAssembledGLInterface(ctx, get);
    }
    return nullptr;
#endif
}

static void get_egl_query_and_display(GrEGLQueryStringFn** queryString, GrEGLDisplay* display,
                                      void* ctx, GrGLGetProc get) {
    *queryString = (GrEGLQueryStringFn*)get(ctx, "eglQueryString");
    *display = GR_EGL_NO_DISPLAY;
    if (*queryString) {
        GrEGLGetCurrentDisplayFn* getCurrentDisplay =
                (GrEGLGetCurrentDisplayFn*)get(ctx, "eglGetCurrentDisplay");
        if (getCurrentDisplay) {
            *display = getCurrentDisplay();
        } else {
            *queryString = nullptr;
        }
    }
}

sk_sp<const GrGLInterface> GrGLMakeAssembledGLInterface(void *ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    GET_PROC_LOCAL(GetStringi);
    GET_PROC_LOCAL(GetIntegerv);

    // GetStringi may be nullptr depending on the GL version.
    if (nullptr == GetString || nullptr == GetIntegerv) {
        return nullptr;
    }

    const char* versionString = (const char*) GetString(GR_GL_VERSION);
    GrGLVersion glVer = GrGLGetVersionFromString(versionString);

    if (glVer < GR_GL_VER(2,0) || GR_GL_INVALID_VER == glVer) {
        // This is our minimum for non-ES GL.
        return nullptr;
    }

    GrEGLQueryStringFn* queryString;
    GrEGLDisplay display;
    get_egl_query_and_display(&queryString, &display, ctx, get);
    GrGLExtensions extensions;
    if (!extensions.init(kGL_GrGLStandard, GetString, GetStringi, GetIntegerv, queryString,
                         display)) {
        return nullptr;
    }

    sk_sp<GrGLInterface> interface(new GrGLInterface());
    GrGLInterface::Functions* functions = &interface->fFunctions;

    GET_PROC(ActiveTexture);
    GET_PROC(AttachShader);
    GET_PROC(BindAttribLocation);
    GET_PROC(BindBuffer);
    if (glVer >= GR_GL_VER(3,0)) {
        GET_PROC(BindFragDataLocation);
    }
    GET_PROC(BeginQuery);
    GET_PROC(BindTexture);

    if (extensions.has("GL_KHR_blend_equation_advanced")) {
        GET_PROC_SUFFIX(BlendBarrier, KHR);
    } else if (extensions.has("GL_NV_blend_equation_advanced")) {
        GET_PROC_SUFFIX(BlendBarrier, NV);
    }

    GET_PROC(BlendColor);
    GET_PROC(BlendEquation);
    GET_PROC(BlendFunc);
    GET_PROC(BufferData);
    GET_PROC(BufferSubData);
    GET_PROC(Clear);
    GET_PROC(ClearColor);
    GET_PROC(ClearStencil);
    if (glVer >= GR_GL_VER(4,4) || extensions.has("GL_ARB_clear_texture")) {
        GET_PROC(ClearTexImage);
        GET_PROC(ClearTexSubImage);
    }
    GET_PROC(ColorMask);
    GET_PROC(CompileShader);
    GET_PROC(CompressedTexImage2D);
    GET_PROC(CompressedTexSubImage2D);
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

    if (glVer >= GR_GL_VER(3,1) || extensions.has("GL_ARB_draw_instanced") ||
        extensions.has("GL_EXT_draw_instanced")) {
        GET_PROC(DrawArraysInstanced);
        GET_PROC(DrawElementsInstanced);
    }

    if (glVer >= GR_GL_VER(4,0) || extensions.has("GL_ARB_draw_indirect")) {
        GET_PROC(DrawArraysIndirect);
        GET_PROC(DrawElementsIndirect);
    }
    GET_PROC(DrawRangeElements);
    GET_PROC(Enable);
    GET_PROC(EnableVertexAttribArray);
    GET_PROC(EndQuery);
    GET_PROC(Finish);
    GET_PROC(Flush);
    GET_PROC(FrontFace);
    GET_PROC(GenBuffers);
    GET_PROC(GetBufferParameteriv);
    GET_PROC(GetError);
    GET_PROC(GetIntegerv);
    if (glVer >= GR_GL_VER(3,2) || extensions.has("GL_ARB_texture_multisample")) {
        GET_PROC(GetMultisamplefv);
    }
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
    GET_PROC(GetString);
    GET_PROC(GetStringi);
    GET_PROC(GetShaderPrecisionFormat);
    GET_PROC(GetTexLevelParameteriv);
    GET_PROC(GenQueries);
    GET_PROC(GenTextures);
    GET_PROC(GetUniformLocation);
    GET_PROC(IsTexture);
    GET_PROC(LineWidth);
    GET_PROC(LinkProgram);
    GET_PROC(MapBuffer);

    if (glVer >= GR_GL_VER(4,3) || extensions.has("GL_ARB_multi_draw_indirect")) {
        GET_PROC(MultiDrawArraysIndirect);
        GET_PROC(MultiDrawElementsIndirect);
    }

    GET_PROC(PixelStorei);
    GET_PROC(PolygonMode);
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
    if (glVer >= GR_GL_VER(3,1)) {
        GET_PROC(TexBuffer);
    }
    if (glVer >= GR_GL_VER(4,3)) {
        GET_PROC(TexBufferRange);
    }
    GET_PROC(TexImage2D);
    GET_PROC(TexParameterf);
    GET_PROC(TexParameterfv);
    GET_PROC(TexParameteri);
    GET_PROC(TexParameteriv);
    if (glVer >= GR_GL_VER(4,2) || extensions.has("GL_ARB_texture_storage")) {
        GET_PROC(TexStorage2D);
    } else if (extensions.has("GL_EXT_texture_storage")) {
        GET_PROC_SUFFIX(TexStorage2D, EXT);
    }
    GET_PROC(TexSubImage2D);
    if (glVer >= GR_GL_VER(4,5) || extensions.has("GL_ARB_texture_barrier")) {
        GET_PROC(TextureBarrier);
    } else if (extensions.has("GL_NV_texture_barrier")) {
        GET_PROC_SUFFIX(TextureBarrier, NV);
    }
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
    GET_PROC(VertexAttrib1f);
    GET_PROC(VertexAttrib2fv);
    GET_PROC(VertexAttrib3fv);
    GET_PROC(VertexAttrib4fv);

    if (glVer >= GR_GL_VER(3,2) || extensions.has("GL_ARB_instanced_arrays")) {
        GET_PROC(VertexAttribDivisor);
    }

    if (glVer >= GR_GL_VER(3,0)) {
        GET_PROC(VertexAttribIPointer);
    }

    GET_PROC(VertexAttribPointer);
    GET_PROC(Viewport);
    GET_PROC(BindFragDataLocationIndexed);

    if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
        // no ARB suffix for GL_ARB_vertex_array_object
        GET_PROC(BindVertexArray);
        GET_PROC(GenVertexArrays);
        GET_PROC(DeleteVertexArrays);
    } else if (extensions.has("GL_APPLE_vertex_array_object")) {
        GET_PROC_SUFFIX(BindVertexArray, APPLE);
        GET_PROC_SUFFIX(GenVertexArrays, APPLE);
        GET_PROC_SUFFIX(DeleteVertexArrays, APPLE);
    }

    if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_map_buffer_range")) {
        GET_PROC(MapBufferRange);
        GET_PROC(FlushMappedBufferRange);
    }

    // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
    // GL_ARB_framebuffer_object doesn't use ARB suffix.)
    if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
        GET_PROC(GenerateMipmap);
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
        GET_PROC_SUFFIX(GenerateMipmap, EXT);
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
        return nullptr;
    }

    if (extensions.has("GL_NV_path_rendering")) {
        GET_PROC_SUFFIX(MatrixLoadf, EXT);
        GET_PROC_SUFFIX(MatrixLoadIdentity, EXT);
        GET_PROC_SUFFIX(PathCommands, NV);
        GET_PROC_SUFFIX(PathParameteri, NV);
        GET_PROC_SUFFIX(PathParameterf, NV);
        GET_PROC_SUFFIX(GenPaths, NV);
        GET_PROC_SUFFIX(DeletePaths, NV);
        GET_PROC_SUFFIX(IsPath, NV);
        GET_PROC_SUFFIX(PathStencilFunc, NV);
        GET_PROC_SUFFIX(StencilFillPath, NV);
        GET_PROC_SUFFIX(StencilStrokePath, NV);
        GET_PROC_SUFFIX(StencilFillPathInstanced, NV);
        GET_PROC_SUFFIX(StencilStrokePathInstanced, NV);
        GET_PROC_SUFFIX(CoverFillPath, NV);
        GET_PROC_SUFFIX(CoverStrokePath, NV);
        GET_PROC_SUFFIX(CoverFillPathInstanced, NV);
        GET_PROC_SUFFIX(CoverStrokePathInstanced, NV);
        GET_PROC_SUFFIX(StencilThenCoverFillPath, NV);
        GET_PROC_SUFFIX(StencilThenCoverStrokePath, NV);
        GET_PROC_SUFFIX(StencilThenCoverFillPathInstanced, NV);
        GET_PROC_SUFFIX(StencilThenCoverStrokePathInstanced, NV);
        GET_PROC_SUFFIX(ProgramPathFragmentInputGen, NV);
    }

    if (extensions.has("GL_NV_framebuffer_mixed_samples")) {
        GET_PROC_SUFFIX(CoverageModulation, NV);
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

    if (glVer >= GR_GL_VER(4,3) || extensions.has("GL_ARB_program_interface_query")) {
        GET_PROC(GetProgramResourceLocation);
    }

    if (glVer >= GR_GL_VER(4,3) || extensions.has("GL_KHR_debug")) {
        // KHR_debug defines these methods to have no suffix in an OpenGL (not ES) context.
        GET_PROC(DebugMessageControl);
        GET_PROC(DebugMessageInsert);
        GET_PROC(DebugMessageCallback);
        GET_PROC(GetDebugMessageLog);
        GET_PROC(PushDebugGroup);
        GET_PROC(PopDebugGroup);
        GET_PROC(ObjectLabel);
    }

    if (extensions.has("GL_EXT_window_rectangles")) {
        GET_PROC_SUFFIX(WindowRectangles, EXT);
    }

    if (extensions.has("EGL_KHR_image") || extensions.has("EGL_KHR_image_base")) {
        GET_EGL_PROC_SUFFIX(CreateImage, KHR);
        GET_EGL_PROC_SUFFIX(DestroyImage, KHR);
    }

    if (glVer >= GR_GL_VER(3, 2) || extensions.has("GL_ARB_sync")) {
        GET_PROC(FenceSync);
        GET_PROC(IsSync);
        GET_PROC(ClientWaitSync);
        GET_PROC(WaitSync);
        GET_PROC(DeleteSync);
    }

    if (glVer >= GR_GL_VER(4,2) || extensions.has("GL_ARB_internalformat_query")) {
        GET_PROC(GetInternalformativ);
    }

    if (glVer >= GR_GL_VER(4, 1)) {
        GET_PROC(GetProgramBinary);
        GET_PROC(ProgramBinary);
        GET_PROC(ProgramParameteri);
    }

    if (glVer >= GR_GL_VER(3,2) || extensions.has("GL_ARB_sampler_objects")) {
        GET_PROC(BindSampler);
        GET_PROC(DeleteSamplers);
        GET_PROC(GenSamplers);
        GET_PROC(SamplerParameteri);
        GET_PROC(SamplerParameteriv);
    }

    interface->fStandard = kGL_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return std::move(interface);
}

sk_sp<const GrGLInterface> GrGLMakeAssembledGLESInterface(void *ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    if (nullptr == GetString) {
        return nullptr;
    }

    const char* verStr = reinterpret_cast<const char*>(GetString(GR_GL_VERSION));
    GrGLVersion version = GrGLGetVersionFromString(verStr);

    if (version < GR_GL_VER(2,0)) {
        return nullptr;
    }

    GET_PROC_LOCAL(GetIntegerv);
    GET_PROC_LOCAL(GetStringi);
    GrEGLQueryStringFn* queryString;
    GrEGLDisplay display;
    get_egl_query_and_display(&queryString, &display, ctx, get);
    GrGLExtensions extensions;
    if (!extensions.init(kGLES_GrGLStandard, GetString, GetStringi, GetIntegerv, queryString,
                         display)) {
        return nullptr;
    }

    sk_sp<GrGLInterface> interface(new GrGLInterface);
    GrGLInterface::Functions* functions = &interface->fFunctions;

    GET_PROC(ActiveTexture);
    GET_PROC(AttachShader);
    GET_PROC(BindAttribLocation);
    GET_PROC(BindBuffer);
    GET_PROC(BindTexture);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(BindVertexArray);
        GET_PROC(DeleteVertexArrays);
        GET_PROC(GenVertexArrays);
    } else if (extensions.has("GL_OES_vertex_array_object")) {
        GET_PROC_SUFFIX(BindVertexArray, OES);
        GET_PROC_SUFFIX(DeleteVertexArrays, OES);
        GET_PROC_SUFFIX(GenVertexArrays, OES);
    }

    if (version >= GR_GL_VER(3,0) && extensions.has("GL_EXT_blend_func_extended")) {
        GET_PROC_SUFFIX(BindFragDataLocation, EXT);
        GET_PROC_SUFFIX(BindFragDataLocationIndexed, EXT);
    }

    if (extensions.has("GL_KHR_blend_equation_advanced")) {
        GET_PROC_SUFFIX(BlendBarrier, KHR);
    } else if (extensions.has("GL_NV_blend_equation_advanced")) {
        GET_PROC_SUFFIX(BlendBarrier, NV);
    }

    GET_PROC(BlendColor);
    GET_PROC(BlendEquation);
    GET_PROC(BlendFunc);
    GET_PROC(BufferData);
    GET_PROC(BufferSubData);
    GET_PROC(Clear);
    GET_PROC(ClearColor);
    GET_PROC(ClearStencil);
    if (extensions.has("GL_EXT_clear_texture")) {
        GET_PROC_SUFFIX(ClearTexImage, EXT);
        GET_PROC_SUFFIX(ClearTexSubImage, EXT);
    }
    GET_PROC(ColorMask);
    GET_PROC(CompileShader);
    GET_PROC(CompressedTexImage2D);
    GET_PROC(CompressedTexSubImage2D);
    GET_PROC(CopyTexSubImage2D);
    GET_PROC(CreateProgram);
    GET_PROC(CreateShader);
    GET_PROC(CullFace);
    GET_PROC(DeleteBuffers);
    GET_PROC(DeleteProgram);
    GET_PROC(DeleteShader);
    GET_PROC(DeleteTextures);
    GET_PROC(DepthMask);
    GET_PROC(Disable);
    GET_PROC(DisableVertexAttribArray);
    GET_PROC(DrawArrays);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(DrawArraysInstanced);
        GET_PROC(DrawBuffers);
        GET_PROC(DrawElementsInstanced);
    } else if (extensions.has("GL_EXT_draw_instanced")) {
        GET_PROC_SUFFIX(DrawArraysInstanced, EXT);
        GET_PROC_SUFFIX(DrawElementsInstanced, EXT);
    }

    if (version >= GR_GL_VER(3,1)) {
        GET_PROC(DrawArraysIndirect);
        GET_PROC(DrawElementsIndirect);
    }

    GET_PROC(DrawElements);
    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(DrawRangeElements);
    }
    GET_PROC(Enable);
    GET_PROC(EnableVertexAttribArray);
    GET_PROC(Finish);
    GET_PROC(Flush);
    GET_PROC(FrontFace);
    GET_PROC(GenBuffers);
    GET_PROC(GenerateMipmap);
    GET_PROC(GenTextures);
    GET_PROC(GetBufferParameteriv);
    GET_PROC(GetError);
    GET_PROC(GetIntegerv);

    if (version >= GR_GL_VER(3,1)) {
        GET_PROC(GetMultisamplefv);
    }

    GET_PROC(GetProgramInfoLog);
    GET_PROC(GetProgramiv);
    GET_PROC(GetShaderInfoLog);
    GET_PROC(GetShaderPrecisionFormat);
    GET_PROC(GetShaderiv);
    GET_PROC(GetString);
    GET_PROC(GetStringi);
    if (version >= GR_GL_VER(3,1)) {
        GET_PROC(GetTexLevelParameteriv);
    }
    GET_PROC(GetUniformLocation);
    GET_PROC(IsTexture);
    GET_PROC(LineWidth);
    GET_PROC(LinkProgram);

    if (extensions.has("GL_EXT_multi_draw_indirect")) {
        GET_PROC_SUFFIX(MultiDrawArraysIndirect, EXT);
        GET_PROC_SUFFIX(MultiDrawElementsIndirect, EXT);
    }

    GET_PROC(PixelStorei);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(ReadBuffer);
    }
    GET_PROC(ReadPixels);
    GET_PROC(Scissor);
    GET_PROC(ShaderSource);
    GET_PROC(StencilFunc);
    GET_PROC(StencilFuncSeparate);
    GET_PROC(StencilMask);
    GET_PROC(StencilMaskSeparate);
    GET_PROC(StencilOp);
    GET_PROC(StencilOpSeparate);

    if (version >= GR_GL_VER(3,2)) {
        GET_PROC(TexBuffer);
        GET_PROC(TexBufferRange);
    } else if (extensions.has("GL_OES_texture_buffer")) {
        GET_PROC_SUFFIX(TexBuffer, OES);
        GET_PROC_SUFFIX(TexBufferRange, OES);
    } else if (extensions.has("GL_EXT_texture_buffer")) {
        GET_PROC_SUFFIX(TexBuffer, EXT);
        GET_PROC_SUFFIX(TexBufferRange, EXT);
    }

    GET_PROC(TexImage2D);
    GET_PROC(TexParameterf);
    GET_PROC(TexParameterfv);
    GET_PROC(TexParameteri);
    GET_PROC(TexParameteriv);
    GET_PROC(TexSubImage2D);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(TexStorage2D);
    } else {
        GET_PROC_SUFFIX(TexStorage2D, EXT);
    }

    if (extensions.has("GL_NV_texture_barrier")) {
        GET_PROC_SUFFIX(TextureBarrier, NV);
    }

    GET_PROC_SUFFIX(DiscardFramebuffer, EXT);
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
    GET_PROC(VertexAttrib1f);
    GET_PROC(VertexAttrib2fv);
    GET_PROC(VertexAttrib3fv);
    GET_PROC(VertexAttrib4fv);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(VertexAttribDivisor);
    } else if (extensions.has("GL_EXT_instanced_arrays")) {
        GET_PROC_SUFFIX(VertexAttribDivisor, EXT);
    }

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(VertexAttribIPointer);
    }

    GET_PROC(VertexAttribPointer);
    GET_PROC(Viewport);
    GET_PROC(BindFramebuffer);
    GET_PROC(BindRenderbuffer);
    GET_PROC(CheckFramebufferStatus);
    GET_PROC(DeleteFramebuffers);
    GET_PROC(DeleteRenderbuffers);
    GET_PROC(FramebufferRenderbuffer);
    GET_PROC(FramebufferTexture2D);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(RenderbufferStorageMultisample);
        GET_PROC(BlitFramebuffer);
    } else if (extensions.has("GL_CHROMIUM_framebuffer_multisample")) {
        GET_PROC_SUFFIX(RenderbufferStorageMultisample, CHROMIUM);
        GET_PROC_SUFFIX(BlitFramebuffer, CHROMIUM);
    } else {
        if (extensions.has("GL_ANGLE_framebuffer_multisample")) {
            GET_PROC_SUFFIX(RenderbufferStorageMultisample, ANGLE);
        }
        if (extensions.has("GL_ANGLE_framebuffer_blit")) {
            GET_PROC_SUFFIX(BlitFramebuffer, ANGLE);
        }
    }

    if (extensions.has("GL_CHROMIUM_map_sub")) {
        GET_PROC_SUFFIX(MapBufferSubData, CHROMIUM);
        GET_PROC_SUFFIX(MapTexSubImage2D, CHROMIUM);
        GET_PROC_SUFFIX(UnmapBufferSubData, CHROMIUM);
        GET_PROC_SUFFIX(UnmapTexSubImage2D, CHROMIUM);
    }

    if (extensions.has("GL_EXT_multisampled_render_to_texture")) {
        GET_PROC_SUFFIX(FramebufferTexture2DMultisample, EXT);
        functions->fRenderbufferStorageMultisampleES2EXT =
                (GrGLRenderbufferStorageMultisampleFn*)get(ctx,
                                                           "glRenderbufferStorageMultisampleEXT");
    } else if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
        GET_PROC_SUFFIX(FramebufferTexture2DMultisample, IMG);
        functions->fRenderbufferStorageMultisampleES2EXT =
                (GrGLRenderbufferStorageMultisampleFn*)get(ctx,
                                                           "glRenderbufferStorageMultisampleIMG");
    } else if (extensions.has("GL_APPLE_framebuffer_multisample")) {
        functions->fRenderbufferStorageMultisampleES2APPLE =
                (GrGLRenderbufferStorageMultisampleFn*)get(ctx,
                                                           "glRenderbufferStorageMultisampleAPPLE");
        GET_PROC_SUFFIX(ResolveMultisampleFramebuffer, APPLE);
    }

    GET_PROC(GenFramebuffers);
    GET_PROC(GenRenderbuffers);
    GET_PROC(GetFramebufferAttachmentParameteriv);
    GET_PROC(GetRenderbufferParameteriv);
    GET_PROC(RenderbufferStorage);

    // There are several APIs for buffer mapping:
    // ES2 + GL_OES_mapbuffer: MapBufferOES and UnmapBufferOES
    // ES2 + GL_EXT_map_buffer_range: Adds MapBufferRangeEXT and FlushMappedBufferRangeEXT
    // ES3: MapBufferRange, FlushMappedBufferRange, and UnmapBuffer are core (so no suffix).
    //
    // MapBuffer is not part of ES3, but implementations may still report the OES versions of
    // MapBuffer and UnmapBuffer, per the older GL_OES_mapbuffer extension. Some implementations
    // let us mix the newer MapBufferRange with the older UnmapBufferOES, but we've hit others that
    // don't permit it. Note that in GrGLBuffer, we choose which API to use based on version and
    // extensions. This code is written so that we never mix OES and non-OES functions.
    GET_PROC_SUFFIX(MapBuffer, OES);
    if (version >= GR_GL_VER(3, 0)) {
        GET_PROC(UnmapBuffer);
    } else {
        GET_PROC_SUFFIX(UnmapBuffer, OES);
    }

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(MapBufferRange);
        GET_PROC(FlushMappedBufferRange);
    } else if (extensions.has("GL_EXT_map_buffer_range")) {
        GET_PROC_SUFFIX(MapBufferRange, EXT);
        GET_PROC_SUFFIX(FlushMappedBufferRange, EXT);
    }

    if (extensions.has("GL_EXT_debug_marker")) {
        GET_PROC_SUFFIX(InsertEventMarker, EXT);
        GET_PROC_SUFFIX(PushGroupMarker, EXT);
        GET_PROC_SUFFIX(PopGroupMarker, EXT);
    }

    GET_PROC(InvalidateFramebuffer);
    GET_PROC(InvalidateSubFramebuffer);
    GET_PROC(InvalidateBufferData);
    GET_PROC(InvalidateBufferSubData);
    GET_PROC(InvalidateTexImage);
    GET_PROC(InvalidateTexSubImage);

    if (version >= GR_GL_VER(3,1)) {
        GET_PROC(GetProgramResourceLocation);
    }

    if (extensions.has("GL_NV_path_rendering")) {
        GET_PROC_SUFFIX(MatrixLoadf, EXT);
        GET_PROC_SUFFIX(MatrixLoadIdentity, EXT);
        GET_PROC_SUFFIX(PathCommands, NV);
        GET_PROC_SUFFIX(PathParameteri, NV);
        GET_PROC_SUFFIX(PathParameterf, NV);
        GET_PROC_SUFFIX(GenPaths, NV);
        GET_PROC_SUFFIX(DeletePaths, NV);
        GET_PROC_SUFFIX(IsPath, NV);
        GET_PROC_SUFFIX(PathStencilFunc, NV);
        GET_PROC_SUFFIX(StencilFillPath, NV);
        GET_PROC_SUFFIX(StencilStrokePath, NV);
        GET_PROC_SUFFIX(StencilFillPathInstanced, NV);
        GET_PROC_SUFFIX(StencilStrokePathInstanced, NV);
        GET_PROC_SUFFIX(CoverFillPath, NV);
        GET_PROC_SUFFIX(CoverStrokePath, NV);
        GET_PROC_SUFFIX(CoverFillPathInstanced, NV);
        GET_PROC_SUFFIX(CoverStrokePathInstanced, NV);
        GET_PROC_SUFFIX(StencilThenCoverFillPath, NV);
        GET_PROC_SUFFIX(StencilThenCoverStrokePath, NV);
        GET_PROC_SUFFIX(StencilThenCoverFillPathInstanced, NV);
        GET_PROC_SUFFIX(StencilThenCoverStrokePathInstanced, NV);
        GET_PROC_SUFFIX(ProgramPathFragmentInputGen, NV);
    }

    if (extensions.has("GL_CHROMIUM_path_rendering")) {
        GET_PROC_SUFFIX(MatrixLoadf, CHROMIUM);
        GET_PROC_SUFFIX(MatrixLoadIdentity, CHROMIUM);
        GET_PROC_SUFFIX(PathCommands, CHROMIUM);
        GET_PROC_SUFFIX(PathParameteri, CHROMIUM);
        GET_PROC_SUFFIX(PathParameterf, CHROMIUM);
        GET_PROC_SUFFIX(GenPaths, CHROMIUM);
        GET_PROC_SUFFIX(DeletePaths, CHROMIUM);
        GET_PROC_SUFFIX(IsPath, CHROMIUM);
        GET_PROC_SUFFIX(PathStencilFunc, CHROMIUM);
        GET_PROC_SUFFIX(StencilFillPath, CHROMIUM);
        GET_PROC_SUFFIX(StencilStrokePath, CHROMIUM);
        GET_PROC_SUFFIX(StencilFillPathInstanced, CHROMIUM);
        GET_PROC_SUFFIX(StencilStrokePathInstanced, CHROMIUM);
        GET_PROC_SUFFIX(CoverFillPath, CHROMIUM);
        GET_PROC_SUFFIX(CoverStrokePath, CHROMIUM);
        GET_PROC_SUFFIX(CoverFillPathInstanced, CHROMIUM);
        GET_PROC_SUFFIX(CoverStrokePathInstanced, CHROMIUM);
        GET_PROC_SUFFIX(StencilThenCoverFillPath, CHROMIUM);
        GET_PROC_SUFFIX(StencilThenCoverStrokePath, CHROMIUM);
        GET_PROC_SUFFIX(StencilThenCoverFillPathInstanced, CHROMIUM);
        GET_PROC_SUFFIX(StencilThenCoverStrokePathInstanced, CHROMIUM);
        GET_PROC_SUFFIX(ProgramPathFragmentInputGen, CHROMIUM);
        // GL_CHROMIUM_path_rendering additions:
        GET_PROC_SUFFIX(BindFragmentInputLocation, CHROMIUM);
    }

    if (extensions.has("GL_NV_framebuffer_mixed_samples")) {
        GET_PROC_SUFFIX(CoverageModulation, NV);
    }
    if (extensions.has("GL_CHROMIUM_framebuffer_mixed_samples")) {
        GET_PROC_SUFFIX(CoverageModulation, CHROMIUM);
    }

    if (extensions.has("GL_KHR_debug")) {
        GET_PROC_SUFFIX(DebugMessageControl, KHR);
        GET_PROC_SUFFIX(DebugMessageInsert, KHR);
        GET_PROC_SUFFIX(DebugMessageCallback, KHR);
        GET_PROC_SUFFIX(GetDebugMessageLog, KHR);
        GET_PROC_SUFFIX(PushDebugGroup, KHR);
        GET_PROC_SUFFIX(PopDebugGroup, KHR);
        GET_PROC_SUFFIX(ObjectLabel, KHR);
        // In general we have a policy against removing extension strings when the driver does
        // not provide function pointers for an advertised extension. However, because there is a
        // known device that advertises GL_KHR_debug but fails to provide the functions and this is
        // a debugging- only extension we've made an exception. This also can happen when using
        // APITRACE.
        if (!interface->fFunctions.fDebugMessageControl) {
            extensions.remove("GL_KHR_debug");
        }
    }

    if (extensions.has("GL_CHROMIUM_bind_uniform_location")) {
        GET_PROC_SUFFIX(BindUniformLocation, CHROMIUM);
    }

    if (extensions.has("GL_EXT_window_rectangles")) {
        GET_PROC_SUFFIX(WindowRectangles, EXT);
    }

    if (extensions.has("EGL_KHR_image") || extensions.has("EGL_KHR_image_base")) {
        GET_EGL_PROC_SUFFIX(CreateImage, KHR);
        GET_EGL_PROC_SUFFIX(DestroyImage, KHR);
    }

    if (version >= GR_GL_VER(3, 0)) {
        GET_PROC(FenceSync);
        GET_PROC(IsSync);
        GET_PROC(ClientWaitSync);
        GET_PROC(WaitSync);
        GET_PROC(DeleteSync);
    } else if (extensions.has("GL_APPLE_sync")) {
        GET_PROC_SUFFIX(FenceSync, APPLE);
        GET_PROC_SUFFIX(IsSync, APPLE);
        GET_PROC_SUFFIX(ClientWaitSync, APPLE);
        GET_PROC_SUFFIX(WaitSync, APPLE);
        GET_PROC_SUFFIX(DeleteSync, APPLE);
    }

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(GetInternalformativ);
    }

    if (version >= GR_GL_VER(3, 0)) {
        GET_PROC(GetProgramBinary);
        GET_PROC(ProgramBinary);
        GET_PROC(ProgramParameteri);
    }

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(BindSampler);
        GET_PROC(DeleteSamplers);
        GET_PROC(GenSamplers);
        GET_PROC(SamplerParameteri);
        GET_PROC(SamplerParameteriv);
    }

    interface->fStandard = kGLES_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return std::move(interface);
}

SK_API const GrGLInterface* GrGLAssembleInterface(void *ctx, GrGLGetProc get) {
    return GrGLMakeAssembledInterface(ctx, get).release();
}

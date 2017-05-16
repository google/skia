
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLAssembleInterface.h"
#include "GrGLUtil.h"

#define GET_PROC(F) functions->f ## F = (GrGL ## F ## Proc) get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f ## F = (GrGL ## F ## Proc) get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL ## F ## Proc F = (GrGL ## F ## Proc) get(ctx, "gl" #F)

const GrGLInterface* GrGLAssembleInterface(void* ctx, GrGLGetProc get) {
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
        return GrGLAssembleGLESInterface(ctx, get);
    } else if (kGL_GrGLStandard == standard) {
        return GrGLAssembleGLInterface(ctx, get);
    }
    return nullptr;
}

const GrGLInterface* GrGLAssembleGLInterface(void* ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    GET_PROC_LOCAL(GetStringi);
    GET_PROC_LOCAL(GetIntegerv);

    // GetStringi may be nullptr depending on the GL version.
    if (nullptr == GetString || nullptr == GetIntegerv) {
        return nullptr;
    }

    const char* versionString = (const char*) GetString(GR_GL_VERSION);
    GrGLVersion glVer = GrGLGetVersionFromString(versionString);

    if (glVer < GR_GL_VER(1,5) || GR_GL_INVALID_VER == glVer) {
        // We must have array and element_array buffer objects.
        return nullptr;
    }

    GrGLExtensions extensions;
    if (!extensions.init(kGL_GrGLStandard, GetString, GetStringi, GetIntegerv)) {
        return nullptr;
    }

    GrGLInterface* interface = new GrGLInterface();
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

    if (glVer >= GR_GL_VER(1,4) ||
        extensions.has("GL_ARB_imaging")) {
        GET_PROC(BlendColor);
    } else if (extensions.has("GL_EXT_blend_color")) {
        GET_PROC_SUFFIX(BlendColor, EXT);
    }

    if (glVer >= GR_GL_VER(1,4) ||
        extensions.has("GL_ARB_imaging")) {
        GET_PROC(BlendEquation);
    } else if (extensions.has("GL_EXT_blend_subtract")) {
        GET_PROC_SUFFIX(BlendEquation, EXT);
    }

    GET_PROC(BlendFunc);
    GET_PROC(BufferData);
    GET_PROC(BufferSubData);
    GET_PROC(Clear);
    GET_PROC(ClearColor);
    GET_PROC(ClearStencil);
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
    GET_PROC(Enable);
    GET_PROC(EnableVertexAttribArray);
    GET_PROC(EndQuery);
    GET_PROC(Finish);
    GET_PROC(Flush);
    GET_PROC(FrontFace);
    GET_PROC(GenBuffers);
    GET_PROC(GenerateMipmap);
    GET_PROC(GetBufferParameteriv);
    GET_PROC(GetError);
    GET_PROC(GetIntegerv);
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
    GET_PROC(PixelStorei);
    if (extensions.has("GL_EXT_raster_multisample")) {
        GET_PROC_SUFFIX(RasterSamples, EXT);
    }
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
    GET_PROC(TexImage2D);
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

    if (glVer >= GR_GL_VER(3,1) || extensions.has("GL_ARB_draw_instanced")) {
        GET_PROC(DrawArraysInstanced);
        GET_PROC(DrawElementsInstanced);
    }

    if (glVer >= GR_GL_VER(3,2) || extensions.has("GL_ARB_instanced_arrays")) {
        GET_PROC(VertexAttribDivisor);
    }

    if (extensions.has("GL_NV_bindless_texture")) {
        GET_PROC_SUFFIX(GetTextureHandle, NV);
        GET_PROC_SUFFIX(GetTextureSamplerHandle, NV);
        GET_PROC_SUFFIX(MakeTextureHandleResident, NV);
        GET_PROC_SUFFIX(MakeTextureHandleNonResident, NV);
        GET_PROC_SUFFIX(GetImageHandle, NV);
        GET_PROC_SUFFIX(MakeImageHandleResident, NV);
        GET_PROC_SUFFIX(MakeImageHandleNonResident, NV);
        GET_PROC_SUFFIX(IsTextureHandleResident, NV);
        GET_PROC_SUFFIX(IsImageHandleResident, NV);
        GET_PROC_SUFFIX(UniformHandleui64, NV);
        GET_PROC_SUFFIX(UniformHandleui64v, NV);
        GET_PROC_SUFFIX(ProgramUniformHandleui64, NV);
        GET_PROC_SUFFIX(ProgramUniformHandleui64v, NV);
    }

    if (extensions.has("GL_EXT_direct_state_access")) {
        GET_PROC_SUFFIX(TextureParameteri, EXT);
        GET_PROC_SUFFIX(TextureParameteriv, EXT);
        GET_PROC_SUFFIX(TextureParameterf, EXT);
        GET_PROC_SUFFIX(TextureParameterfv, EXT);
        GET_PROC_SUFFIX(TextureImage1D, EXT);
        GET_PROC_SUFFIX(TextureImage2D, EXT);
        GET_PROC_SUFFIX(TextureSubImage1D, EXT);
        GET_PROC_SUFFIX(TextureSubImage2D, EXT);
        GET_PROC_SUFFIX(CopyTextureImage1D, EXT);
        GET_PROC_SUFFIX(CopyTextureImage2D, EXT);
        GET_PROC_SUFFIX(CopyTextureSubImage1D, EXT);
        GET_PROC_SUFFIX(CopyTextureSubImage2D, EXT);
        GET_PROC_SUFFIX(GetTextureImage, EXT);
        GET_PROC_SUFFIX(GetTextureParameterfv, EXT);
        GET_PROC_SUFFIX(GetTextureParameteriv, EXT);
        GET_PROC_SUFFIX(GetTextureLevelParameterfv, EXT);
        GET_PROC_SUFFIX(GetTextureLevelParameteriv, EXT);
        if (glVer >= GR_GL_VER(1,2)) {
            GET_PROC_SUFFIX(TextureImage3D, EXT);
            GET_PROC_SUFFIX(TextureSubImage3D, EXT);
            GET_PROC_SUFFIX(CopyTextureSubImage3D, EXT);
            GET_PROC_SUFFIX(CompressedTextureImage3D, EXT);
            GET_PROC_SUFFIX(CompressedTextureImage2D, EXT);
            GET_PROC_SUFFIX(CompressedTextureImage1D, EXT);
            GET_PROC_SUFFIX(CompressedTextureSubImage3D, EXT);
            GET_PROC_SUFFIX(CompressedTextureSubImage2D, EXT);
            GET_PROC_SUFFIX(CompressedTextureSubImage1D, EXT);
            GET_PROC_SUFFIX(GetCompressedTextureImage, EXT);
        }
        if (glVer >= GR_GL_VER(1,5)) {
            GET_PROC_SUFFIX(NamedBufferData, EXT);
            GET_PROC_SUFFIX(NamedBufferSubData, EXT);
            GET_PROC_SUFFIX(MapNamedBuffer, EXT);
            GET_PROC_SUFFIX(UnmapNamedBuffer, EXT);
            GET_PROC_SUFFIX(GetNamedBufferParameteriv, EXT);
            GET_PROC_SUFFIX(GetNamedBufferPointerv, EXT);
            GET_PROC_SUFFIX(GetNamedBufferSubData, EXT);
        }
        if (glVer >= GR_GL_VER(2,0)) {
            GET_PROC_SUFFIX(ProgramUniform1f, EXT);
            GET_PROC_SUFFIX(ProgramUniform2f, EXT);
            GET_PROC_SUFFIX(ProgramUniform3f, EXT);
            GET_PROC_SUFFIX(ProgramUniform4f, EXT);
            GET_PROC_SUFFIX(ProgramUniform1i, EXT);
            GET_PROC_SUFFIX(ProgramUniform2i, EXT);
            GET_PROC_SUFFIX(ProgramUniform3i, EXT);
            GET_PROC_SUFFIX(ProgramUniform4i, EXT);
            GET_PROC_SUFFIX(ProgramUniform1fv, EXT);
            GET_PROC_SUFFIX(ProgramUniform2fv, EXT);
            GET_PROC_SUFFIX(ProgramUniform3fv, EXT);
            GET_PROC_SUFFIX(ProgramUniform4fv, EXT);
            GET_PROC_SUFFIX(ProgramUniform1iv, EXT);
            GET_PROC_SUFFIX(ProgramUniform2iv, EXT);
            GET_PROC_SUFFIX(ProgramUniform3iv, EXT);
            GET_PROC_SUFFIX(ProgramUniform4iv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix2fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix3fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix4fv, EXT);
        }
        if (glVer >= GR_GL_VER(2,1)) {
            GET_PROC_SUFFIX(ProgramUniformMatrix2x3fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix3x2fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix2x4fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix4x2fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix3x4fv, EXT);
            GET_PROC_SUFFIX(ProgramUniformMatrix4x3fv, EXT);
        }
        if (glVer >= GR_GL_VER(3,0)) {
            GET_PROC_SUFFIX(NamedRenderbufferStorage, EXT);
            GET_PROC_SUFFIX(GetNamedRenderbufferParameteriv, EXT);
            GET_PROC_SUFFIX(NamedRenderbufferStorageMultisample, EXT);
            GET_PROC_SUFFIX(CheckNamedFramebufferStatus, EXT);
            GET_PROC_SUFFIX(NamedFramebufferTexture1D, EXT);
            GET_PROC_SUFFIX(NamedFramebufferTexture2D, EXT);
            GET_PROC_SUFFIX(NamedFramebufferTexture3D, EXT);
            GET_PROC_SUFFIX(NamedFramebufferRenderbuffer, EXT);
            GET_PROC_SUFFIX(GetNamedFramebufferAttachmentParameteriv, EXT);
            GET_PROC_SUFFIX(GenerateTextureMipmap, EXT);
            GET_PROC_SUFFIX(FramebufferDrawBuffer, EXT);
            GET_PROC_SUFFIX(FramebufferDrawBuffers, EXT);
            GET_PROC_SUFFIX(FramebufferReadBuffer, EXT);
            GET_PROC_SUFFIX(GetFramebufferParameteriv, EXT);
            GET_PROC_SUFFIX(NamedCopyBufferSubData, EXT);
            GET_PROC_SUFFIX(VertexArrayVertexOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayColorOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayEdgeFlagOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayIndexOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayNormalOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayTexCoordOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayMultiTexCoordOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayFogCoordOffset, EXT);
            GET_PROC_SUFFIX(VertexArraySecondaryColorOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayVertexAttribOffset, EXT);
            GET_PROC_SUFFIX(VertexArrayVertexAttribIOffset, EXT);
            GET_PROC_SUFFIX(EnableVertexArray, EXT);
            GET_PROC_SUFFIX(DisableVertexArray, EXT);
            GET_PROC_SUFFIX(EnableVertexArrayAttrib, EXT);
            GET_PROC_SUFFIX(DisableVertexArrayAttrib, EXT);
            GET_PROC_SUFFIX(GetVertexArrayIntegerv, EXT);
            GET_PROC_SUFFIX(GetVertexArrayPointerv, EXT);
            GET_PROC_SUFFIX(GetVertexArrayIntegeri_v, EXT);
            GET_PROC_SUFFIX(GetVertexArrayPointeri_v, EXT);
            GET_PROC_SUFFIX(MapNamedBufferRange, EXT);
            GET_PROC_SUFFIX(FlushMappedNamedBufferRange, EXT);
        }
    }

    if (glVer >= GR_GL_VER(4,5)) {
        GET_PROC(NamedFramebufferParameteri);
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

    interface->fStandard = kGL_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return interface;
}

const GrGLInterface* GrGLAssembleGLESInterface(void* ctx, GrGLGetProc get) {
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
    GrGLExtensions extensions;
    if (!extensions.init(kGLES_GrGLStandard, GetString, GetStringi, GetIntegerv)) {
        return nullptr;
    }

    GrGLInterface* interface = new GrGLInterface;
    GrGLInterface::Functions* functions = &interface->fFunctions;

    GET_PROC(ActiveTexture);
    GET_PROC(AttachShader);
    GET_PROC(BindAttribLocation);
    GET_PROC(BindBuffer);
    GET_PROC(BindTexture);
    GET_PROC_SUFFIX(BindVertexArray, OES);

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
    GET_PROC_SUFFIX(DeleteVertexArrays, OES);
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
    GET_PROC_SUFFIX(GenVertexArrays, OES);
    GET_PROC(GetBufferParameteriv);
    GET_PROC(GetError);
    GET_PROC(GetIntegerv);
    GET_PROC(GetProgramInfoLog);
    GET_PROC(GetProgramiv);
    GET_PROC(GetShaderInfoLog);
    GET_PROC(GetShaderPrecisionFormat);
    GET_PROC(GetShaderiv);
    GET_PROC(GetString);
    GET_PROC(GetStringi);
    GET_PROC(GetUniformLocation);
    GET_PROC(IsTexture);
    GET_PROC(LineWidth);
    GET_PROC(LinkProgram);
    GET_PROC(PixelStorei);

    if (extensions.has("GL_EXT_raster_multisample")) {
        GET_PROC_SUFFIX(RasterSamples, EXT);
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
    GET_PROC(TexImage2D);
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
    GET_PROC(VertexAttribPointer);
    GET_PROC(Viewport);
    GET_PROC(BindFramebuffer);
    GET_PROC(BindRenderbuffer);
    GET_PROC(CheckFramebufferStatus);
    GET_PROC(DeleteFramebuffers);
    GET_PROC(DeleteRenderbuffers);
    GET_PROC(FramebufferRenderbuffer);
    GET_PROC(FramebufferTexture2D);

    if (extensions.has("GL_CHROMIUM_framebuffer_multisample")) {
        GET_PROC_SUFFIX(RenderbufferStorageMultisample, CHROMIUM);
        GET_PROC_SUFFIX(BlitFramebuffer, CHROMIUM);
    } else if (version >= GR_GL_VER(3,0)) {
        GET_PROC(RenderbufferStorageMultisample);
        GET_PROC(BlitFramebuffer);
    }

    if (extensions.has("GL_CHROMIUM_map_sub")) {
        GET_PROC_SUFFIX(MapBufferSubData, CHROMIUM);
        GET_PROC_SUFFIX(MapTexSubImage2D, CHROMIUM);
        GET_PROC_SUFFIX(UnmapBufferSubData, CHROMIUM);
        GET_PROC_SUFFIX(UnmapTexSubImage2D, CHROMIUM);
    }

    if (extensions.has("GL_EXT_multisampled_render_to_texture")) {
        GET_PROC_SUFFIX(FramebufferTexture2DMultisample, EXT);
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) get(ctx, "glRenderbufferStorageMultisampleEXT");
    } else if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
        GET_PROC_SUFFIX(FramebufferTexture2DMultisample, IMG);
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) get(ctx, "glRenderbufferStorageMultisampleIMG");
    } else if (extensions.has("GL_APPLE_framebuffer_multisample")) {
        functions->fRenderbufferStorageMultisampleES2APPLE = (GrGLRenderbufferStorageMultisampleProc) get(ctx, "glRenderbufferStorageMultisampleAPPLE");
        GET_PROC_SUFFIX(ResolveMultisampleFramebuffer, APPLE);
    }

    GET_PROC(GenFramebuffers);
    GET_PROC(GenRenderbuffers);
    GET_PROC(GetFramebufferAttachmentParameteriv);
    GET_PROC(GetRenderbufferParameteriv);
    GET_PROC(RenderbufferStorage);

    GET_PROC_SUFFIX(MapBuffer, OES);
    GET_PROC_SUFFIX(UnmapBuffer, OES);

    if (version >= GR_GL_VER(3,0)) {
        GET_PROC(MapBufferRange);
        GET_PROC(FlushMappedBufferRange);
        GET_PROC(UnmapBuffer);
    } else if (extensions.has("GL_EXT_map_buffer_range")) {
        GET_PROC_SUFFIX(MapBufferRange, EXT);
        GET_PROC_SUFFIX(FlushMappedBufferRange, EXT);
    }

    if (extensions.has("GL_EXT_debug_marker")) {
        GET_PROC(InsertEventMarker);
        GET_PROC(PushGroupMarker);
        GET_PROC(PopGroupMarker);
        // The below check is here because a device has been found that has the extension string but
        // returns nullptr from the eglGetProcAddress for the functions
        if (nullptr == functions->fInsertEventMarker ||
            nullptr == functions->fPushGroupMarker ||
            nullptr == functions->fPopGroupMarker) {
            extensions.remove("GL_EXT_debug_marker");
        }
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

    if (extensions.has("GL_NV_framebuffer_mixed_samples")) {
        GET_PROC_SUFFIX(CoverageModulation, NV);
    }

    if (version >= GR_GL_VER(3,0) || extensions.has("GL_EXT_draw_instanced")) {
        GET_PROC(DrawArraysInstanced);
        GET_PROC(DrawElementsInstanced);
    }

    if (version >= GR_GL_VER(3,0) || extensions.has("GL_EXT_instanced_arrays")) {
        GET_PROC(VertexAttribDivisor);
    }

    if (extensions.has("GL_NV_bindless_texture")) {
        GET_PROC_SUFFIX(GetTextureHandle, NV);
        GET_PROC_SUFFIX(GetTextureSamplerHandle, NV);
        GET_PROC_SUFFIX(MakeTextureHandleResident, NV);
        GET_PROC_SUFFIX(MakeTextureHandleNonResident, NV);
        GET_PROC_SUFFIX(GetImageHandle, NV);
        GET_PROC_SUFFIX(MakeImageHandleResident, NV);
        GET_PROC_SUFFIX(MakeImageHandleNonResident, NV);
        GET_PROC_SUFFIX(IsTextureHandleResident, NV);
        GET_PROC_SUFFIX(IsImageHandleResident, NV);
        GET_PROC_SUFFIX(UniformHandleui64, NV);
        GET_PROC_SUFFIX(UniformHandleui64v, NV);
        GET_PROC_SUFFIX(ProgramUniformHandleui64, NV);
        GET_PROC_SUFFIX(ProgramUniformHandleui64v, NV);
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

    interface->fStandard = kGLES_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return interface;
}

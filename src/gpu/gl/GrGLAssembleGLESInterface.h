
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* This file is meant to be included "inline" the implementation that is using the function.
 * The platform native GL implementation header file should be included before this file.
 * Following macros should be defined before this file is included:
 * GET_PROC and GET_PROC_SUFFIX
 *   Call the get function and assign to the interface instance
 * GET_PROC_LOCAL
 *   Call the get function and assign to a local variable
 * GET_LINKED and GET_LINKED_SUFFIX
 *   Get the link-time address of the function and assign it to the interface instance. If
 *   using the linked addresses is not intended, can be the same as GET_PROC.
 */

/**
 * Generic function for creating a GrGLInterface for an OpenGL ES (but not Open GL) context. It
 * calls get() to get each function address. ctx is a generic ptr passed to and interpreted by
 * get().
 */
static const GrGLInterface* GrGLAssembleGLESInterface(void* ctx, GrGLGetProc get) {
    const char* verStr = reinterpret_cast<const char*>(glGetString(GR_GL_VERSION));
    GrGLVersion version = GrGLGetVersionFromString(verStr);

    if (version < GR_GL_VER(2,0)) {
        return NULL;
    }

    GET_PROC_LOCAL(GetIntegerv);
    GET_PROC_LOCAL(GetStringi);
    GrGLExtensions extensions;
    if (!extensions.init(kGLES_GrGLStandard, glGetString, GetStringi, GetIntegerv)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    GrGLInterface::Functions* functions = &interface->fFunctions;

    GET_LINKED(ActiveTexture);
    GET_LINKED(AttachShader);
    GET_LINKED(BindAttribLocation);
    GET_LINKED(BindBuffer);
    GET_LINKED(BindTexture);
    GET_LINKED_SUFFIX(BindVertexArray, OES);
    GET_LINKED(BlendColor);
    GET_LINKED(BlendFunc);
    GET_LINKED(BufferData);
    GET_LINKED(BufferSubData);
    GET_LINKED(Clear);
    GET_LINKED(ClearColor);
    GET_LINKED(ClearStencil);
    GET_LINKED(ColorMask);
    GET_LINKED(CompileShader);
    GET_LINKED(CompressedTexImage2D);
    GET_LINKED(CompressedTexSubImage2D);
    GET_LINKED(CopyTexSubImage2D);
    GET_LINKED(CreateProgram);
    GET_LINKED(CreateShader);
    GET_LINKED(CullFace);
    GET_LINKED(DeleteBuffers);
    GET_LINKED(DeleteProgram);
    GET_LINKED(DeleteShader);
    GET_LINKED(DeleteTextures);
    GET_LINKED_SUFFIX(DeleteVertexArrays, OES);
    GET_LINKED(DepthMask);
    GET_LINKED(Disable);
    GET_LINKED(DisableVertexAttribArray);
    GET_LINKED(DrawArrays);
    GET_LINKED(DrawElements);
    GET_LINKED(Enable);
    GET_LINKED(EnableVertexAttribArray);
    GET_LINKED(Finish);
    GET_LINKED(Flush);
    GET_LINKED(FrontFace);
    GET_LINKED(GenBuffers);
    GET_LINKED(GenerateMipmap);
    GET_LINKED(GenTextures);
    GET_LINKED_SUFFIX(GenVertexArrays, OES);
    GET_LINKED(GetBufferParameteriv);
    GET_LINKED(GetError);
    GET_LINKED(GetIntegerv);
    GET_LINKED(GetProgramInfoLog);
    GET_LINKED(GetProgramiv);
    GET_LINKED(GetShaderInfoLog);
    GET_LINKED(GetShaderiv);
    GET_LINKED(GetString);
#if GL_ES_VERSION_3_0
    GET_LINKED(GetStringi);
#else
    GET_PROC(GetStringi);
#endif
    GET_LINKED(GetUniformLocation);
    GET_LINKED(LineWidth);
    GET_LINKED(LinkProgram);
    GET_LINKED(PixelStorei);
    GET_LINKED(ReadPixels);
    GET_LINKED(Scissor);
#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
    functions->fShaderSource = (GrGLShaderSourceProc) glShaderSource;
#else
    GET_LINKED(ShaderSource);
#endif
    GET_LINKED(StencilFunc);
    GET_LINKED(StencilFuncSeparate);
    GET_LINKED(StencilMask);
    GET_LINKED(StencilMaskSeparate);
    GET_LINKED(StencilOp);
    GET_LINKED(StencilOpSeparate);
    GET_LINKED(TexImage2D);
    GET_LINKED(TexParameteri);
    GET_LINKED(TexParameteriv);
    GET_LINKED(TexSubImage2D);

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        GET_LINKED(TexStorage2D);
#else
        GET_PROC(TexStorage2D);
#endif
    } else {
#if GL_EXT_texture_storage
        GET_LINKED_SUFFIX(TexStorage2D, EXT);
#else
        GET_PROC_SUFFIX(TexStorage2D, EXT);
#endif
    }

#if GL_EXT_discard_framebuffer
    GET_LINKED_SUFFIX(DiscardFramebuffer, EXT);
#endif
    GET_LINKED(Uniform1f);
    GET_LINKED(Uniform1i);
    GET_LINKED(Uniform1fv);
    GET_LINKED(Uniform1iv);
    GET_LINKED(Uniform2f);
    GET_LINKED(Uniform2i);
    GET_LINKED(Uniform2fv);
    GET_LINKED(Uniform2iv);
    GET_LINKED(Uniform3f);
    GET_LINKED(Uniform3i);
    GET_LINKED(Uniform3fv);
    GET_LINKED(Uniform3iv);
    GET_LINKED(Uniform4f);
    GET_LINKED(Uniform4i);
    GET_LINKED(Uniform4fv);
    GET_LINKED(Uniform4iv);
    GET_LINKED(UniformMatrix2fv);
    GET_LINKED(UniformMatrix3fv);
    GET_LINKED(UniformMatrix4fv);
    GET_LINKED(UseProgram);
    GET_LINKED(VertexAttrib4fv);
    GET_LINKED(VertexAttribPointer);
    GET_LINKED(Viewport);
    GET_LINKED(BindFramebuffer);
    GET_LINKED(BindRenderbuffer);
    GET_LINKED(CheckFramebufferStatus);
    GET_LINKED(DeleteFramebuffers);
    GET_LINKED(DeleteRenderbuffers);
    GET_LINKED(FramebufferRenderbuffer);
    GET_LINKED(FramebufferTexture2D);

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        GET_LINKED(RenderbufferStorageMultisample);
        GET_LINKED(BlitFramebuffer);
#else
        GET_PROC(RenderbufferStorageMultisample);
        GET_PROC(BlitFramebuffer);
#endif
    }

    if (extensions.has("GL_EXT_multisampled_render_to_texture")) {
#if GL_EXT_multisampled_render_to_texture
        GET_LINKED_SUFFIX(FramebufferTexture2DMultisample, EXT);
        functions->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleEXT;
#else
        GET_PROC_SUFFIX(FramebufferTexture2DMultisample, EXT);
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) get(ctx, "glRenderbufferStorageMultisampleEXT");
#endif
    } else if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
#if GL_IMG_multisampled_render_to_texture
        GET_LINKED_SUFFIX(FramebufferTexture2DMultisample, IMG);
        functions->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleIMG;
#else
        GET_PROC_SUFFIX(FramebufferTexture2DMultisample, IMG);
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) get(ctx, "glRenderbufferStorageMultisampleIMG");
#endif
    }

    GET_LINKED(GenFramebuffers);
    GET_LINKED(GenRenderbuffers);
    GET_LINKED(GetFramebufferAttachmentParameteriv);
    GET_LINKED(GetRenderbufferParameteriv);
    GET_LINKED(RenderbufferStorage);

#if GL_OES_mapbuffer
    GET_LINKED_SUFFIX(MapBuffer, OES);
    GET_LINKED_SUFFIX(UnmapBuffer, OES);
#else
    GET_PROC_SUFFIX(MapBuffer, OES);
    GET_PROC_SUFFIX(UnmapBuffer, OES);
#endif

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        GET_LINKED(MapBufferRange);
        GET_LINKED(FlushMappedBufferRange);
#else
        GET_PROC(MapBufferRange);
        GET_PROC(FlushMappedBufferRange);
#endif
    } else if (extensions.has("GL_EXT_map_buffer_range")) {
#if GL_EXT_map_buffer_range
        GET_LINKED_SUFFIX(MapBufferRange, EXT);
        GET_LINKED_SUFFIX(FlushMappedBufferRange, EXT);
#else
        GET_PROC_SUFFIX(MapBufferRange, EXT);
        GET_PROC_SUFFIX(FlushMappedBufferRange, EXT);
#endif
    }

    if (extensions.has("GL_EXT_debug_marker")) {
        GET_PROC(InsertEventMarker);
        GET_PROC(PushGroupMarker);
        GET_PROC(PopGroupMarker);
        // The below check is here because a device has been found that has the extension string but
        // returns NULL from the eglGetProcAddress for the functions
        if (NULL == functions->fInsertEventMarker ||
            NULL == functions->fPushGroupMarker ||
            NULL == functions->fPopGroupMarker) {
            extensions.remove("GL_EXT_debug_marker");
        }
    }

#if GL_ES_VERSION_3_0
    GET_LINKED(InvalidateFramebuffer);
    GET_LINKED(InvalidateSubFramebuffer);
#else
    GET_PROC(InvalidateFramebuffer);
    GET_PROC(InvalidateSubFramebuffer);
#endif
    GET_PROC(InvalidateBufferData);
    GET_PROC(InvalidateBufferSubData);
    GET_PROC(InvalidateTexImage);
    GET_PROC(InvalidateTexSubImage);

#if GL_ES_VERSION_3_1
    GET_LINKED(GetProgramResourceLocation);
#else
    if (version >= GR_GL_VER(3,1)) {
        GET_PROC(GetProgramResourceLocation);
    }
#endif

    if (extensions.has("GL_NV_path_rendering")) {
        GET_PROC_SUFFIX(MatrixLoadf, EXT);
        GET_PROC_SUFFIX(MatrixLoadIdentity, EXT);
        GET_PROC_SUFFIX(PathCommands, NV);
        GET_PROC_SUFFIX(PathCoords, NV);
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
        GET_PROC_SUFFIX(ProgramPathFragmentInputGen, NV);
    }

    interface->fStandard = kGLES_GrGLStandard;
    interface->fExtensions.swap(&extensions);

    return interface;
}

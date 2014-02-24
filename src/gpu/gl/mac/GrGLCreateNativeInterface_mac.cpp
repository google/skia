
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "../GrGLUtil.h"

#include <dlfcn.h>

// We get the proc addresss of all GL functions dynamically because we sometimes link against
// alternative GL implementations (e.g. MESA) in addition to the native GL implementation.
class GLLoader {
public:
    GLLoader() {
        fLibrary = dlopen(
                    "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib",
                    RTLD_LAZY);
    }
    ~GLLoader() {
        if (NULL != fLibrary) {
            dlclose(fLibrary);
        }
    }
    void* handle() {
        return NULL == fLibrary ? RTLD_DEFAULT : fLibrary;
    }
private:
    void* fLibrary;
};

static void* GetProcAddress(const char* name) {
    static GLLoader gLoader;
    return dlsym(gLoader.handle(), name);
}

#define GET_PROC(name) (interface->fFunctions.f ## name = ((GrGL ## name ## Proc) GetProcAddress("gl" #name)))
#define GET_PROC_SUFFIX(name, suffix) (interface->fFunctions.f ## name = ((GrGL ## name ## Proc) GetProcAddress("gl" #name #suffix)))

const GrGLInterface* GrGLCreateNativeInterface() {

    GrGLGetStringProc glGetString = (GrGLGetStringProc) GetProcAddress("glGetString");
    GrGLGetStringiProc glGetStringi = (GrGLGetStringiProc) GetProcAddress("glGetStringi");
    GrGLGetIntegervProc glGetIntegerv = (GrGLGetIntegervProc) GetProcAddress("glGetIntegerv");

    const char* verStr = (const char*) glGetString(GR_GL_VERSION);
    GrGLVersion ver = GrGLGetVersionFromString(verStr);
    GrGLExtensions extensions;
    if (!extensions.init(kGL_GrGLStandard, glGetString, glGetStringi, glGetIntegerv)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    interface->fStandard = kGL_GrGLStandard;

    GET_PROC(ActiveTexture);
    GET_PROC(AttachShader);
    GET_PROC(BeginQuery);
    GET_PROC(BindAttribLocation);
    GET_PROC(BindBuffer);
    if (ver >= GR_GL_VER(3,0)) {
        GET_PROC(BindFragDataLocation);
    }
    GET_PROC(BindTexture);
    GET_PROC(BlendFunc);

    if (ver >= GR_GL_VER(1,4) ||
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
    GET_PROC(GetQueryiv);
    GET_PROC(GetQueryObjectiv);
    GET_PROC(GetQueryObjectuiv);
    GET_PROC(GetShaderInfoLog);
    GET_PROC(GetShaderiv);
    GET_PROC(GetString);
    GET_PROC(GetStringi);
    GET_PROC(GetTexLevelParameteriv);
    GET_PROC(GenTextures);
    GET_PROC(GetUniformLocation);
    GET_PROC(LineWidth);
    GET_PROC(LinkProgram);
    GET_PROC(LoadIdentity);
    GET_PROC(LoadMatrixf);
    GET_PROC(MapBuffer);
    GET_PROC(MatrixMode);
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
    GET_PROC(TexGenfv);
    GET_PROC(TexGeni);
    GET_PROC(TexImage2D);
    GET_PROC(TexParameteri);
    GET_PROC(TexParameteriv);
    if (ver >= GR_GL_VER(4,2) || extensions.has("GL_ARB_texture_storage")) {
        GET_PROC(TexStorage2D);
    } else if (extensions.has("GL_EXT_texture_storage")) {
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
    GET_PROC(Uniform4fv);
    GET_PROC(UniformMatrix2fv);
    GET_PROC(UniformMatrix3fv);
    GET_PROC(UniformMatrix4fv);
    GET_PROC(UnmapBuffer);
    GET_PROC(UseProgram);
    GET_PROC(VertexAttrib4fv);
    GET_PROC(VertexAttribPointer);
    GET_PROC(Viewport);

    if (ver >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
        // no ARB suffix for GL_ARB_vertex_array_object
        GET_PROC(BindVertexArray);
        GET_PROC(DeleteVertexArrays);
        GET_PROC(GenVertexArrays);
    }

    if (ver >= GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
        // ARB extension doesn't use the ARB suffix on the function name
        GET_PROC(QueryCounter);
        GET_PROC(GetQueryObjecti64v);
        GET_PROC(GetQueryObjectui64v);
    } else if (extensions.has("GL_EXT_timer_query")) {
        GET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
        GET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
    }

    if (ver >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
        // ARB extension doesn't use the ARB suffix on the function names
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
    } else {
        if (extensions.has("GL_EXT_framebuffer_object")) {
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
        }
        if (extensions.has("GL_EXT_framebuffer_multisample")) {
            GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
        }
        if (extensions.has("GL_EXT_framebuffer_blit")) {
            GET_PROC_SUFFIX(BlitFramebuffer, EXT);
        }
    }
    if (ver >= GR_GL_VER(3,3) || extensions.has("GL_ARB_blend_func_extended")) {
        // ARB extension doesn't use the ARB suffix on the function name
        GET_PROC(BindFragDataLocationIndexed);
    }

    if (extensions.has("GL_EXT_debug_marker")) {
        GET_PROC_SUFFIX(InsertEventMarker, EXT);
        GET_PROC_SUFFIX(PushGroupMarker, EXT);
        GET_PROC_SUFFIX(PopGroupMarker, EXT);
    }

    interface->fExtensions.swap(&extensions);
    return interface;
}

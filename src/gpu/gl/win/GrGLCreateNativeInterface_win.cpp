
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLExtensions.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 * Windows makes the GL funcs all be __stdcall instead of __cdecl :(
 * This implementation will only work if GR_GL_FUNCTION_TYPE is __stdcall.
 * Otherwise, a springboard would be needed that hides the calling convention.
 */

#define SET_PROC(F) interface->f ## F = (GrGL ## F ## Proc) GetProcAddress(alu.get(), "gl" #F);
#define WGL_SET_PROC(F) interface->f ## F = (GrGL ## F ## Proc) wglGetProcAddress("gl" #F);
#define WGL_SET_PROC_SUFFIX(F, S) interface->f ## F = \
                                  (GrGL ## F ## Proc) wglGetProcAddress("gl" #F #S);

class AutoLibraryUnload {
public:
    AutoLibraryUnload(const char* moduleName) {
        fModule = LoadLibrary(moduleName);
    }
    ~AutoLibraryUnload() {
        if (NULL != fModule) {
            FreeLibrary(fModule);
        }
    }
    HMODULE get() const { return fModule; }

private:
    HMODULE fModule;
};

const GrGLInterface* GrGLCreateNativeInterface() {
    // wglGetProcAddress requires a context.
    // GL Function pointers retrieved in one context may not be valid in another
    // context. For that reason we create a new GrGLInterface each time we're
    // called.
    AutoLibraryUnload alu("opengl32.dll");
    if (NULL == alu.get()) {
        return NULL;
    }

    if (NULL != wglGetCurrentContext()) {

        // These should always be present and don't require wglGetProcAddress
        GrGLGetStringProc glGetString =
            (GrGLGetStringProc) GetProcAddress(alu.get(), "glGetString");
        GrGLGetIntegervProc glGetIntegerv =
            (GrGLGetIntegervProc) GetProcAddress(alu.get(), "glGetIntegerv");
        if (NULL == glGetString || NULL == glGetIntegerv) {
            return NULL;
        }

        // This may or may not succeed depending on the gl version.
        GrGLGetStringiProc glGetStringi = (GrGLGetStringiProc)  wglGetProcAddress("glGetStringi");

        GrGLExtensions extensions;
        if (!extensions.init(kDesktop_GrGLBinding, glGetString, glGetStringi, glGetIntegerv)) {
            return NULL;
        }
        const char* versionString = (const char*) glGetString(GR_GL_VERSION);
        GrGLVersion glVer = GrGLGetVersionFromString(versionString);

        if (glVer < GR_GL_VER(1,5)) {
            // We must have array and element_array buffer objects.
            return NULL;
        }
        GrGLInterface* interface = new GrGLInterface();

        // Functions that are part of GL 1.1 will return NULL in
        // wglGetProcAddress
        SET_PROC(BindTexture)
        SET_PROC(BlendFunc)

        if (glVer >= GR_GL_VER(1,4) ||
            extensions.has("GL_ARB_imaging") ||
            extensions.has("GL_EXT_blend_color")) {
            WGL_SET_PROC(BlendColor);
        }

        SET_PROC(Clear)
        SET_PROC(ClearColor)
        SET_PROC(ClearStencil)
        SET_PROC(ColorMask)
        SET_PROC(CopyTexSubImage2D)
        SET_PROC(CullFace)
        SET_PROC(DeleteTextures)
        SET_PROC(DepthMask)
        SET_PROC(Disable)
        SET_PROC(DisableClientState)
        SET_PROC(DrawArrays)
        SET_PROC(DrawElements)
        SET_PROC(DrawBuffer)
        SET_PROC(Enable)
        SET_PROC(EnableClientState)
        SET_PROC(FrontFace)
        SET_PROC(Finish)
        SET_PROC(Flush)
        SET_PROC(GenTextures)
        SET_PROC(GetError)
        SET_PROC(GetIntegerv)
        SET_PROC(GetString)
        SET_PROC(GetTexLevelParameteriv)
        SET_PROC(LineWidth)
        SET_PROC(LoadIdentity)
        SET_PROC(LoadMatrixf)
        SET_PROC(MatrixMode)
        SET_PROC(PixelStorei)
        SET_PROC(ReadBuffer)
        SET_PROC(ReadPixels)
        SET_PROC(Scissor)
        SET_PROC(StencilFunc)
        SET_PROC(StencilMask)
        SET_PROC(StencilOp)
        SET_PROC(TexGenf)
        SET_PROC(TexGenfv)
        SET_PROC(TexGeni)
        SET_PROC(TexImage2D)
        SET_PROC(TexParameteri)
        SET_PROC(TexParameteriv)
        if (glVer >= GR_GL_VER(4,2) || extensions.has("GL_ARB_texture_storage")) {
            WGL_SET_PROC(TexStorage2D);
        } else if (extensions.has("GL_EXT_texture_storage")) {
            WGL_SET_PROC_SUFFIX(TexStorage2D, EXT);
        }
        SET_PROC(TexSubImage2D)
        SET_PROC(Viewport)
        SET_PROC(VertexPointer)

        WGL_SET_PROC(ActiveTexture);
        WGL_SET_PROC(AttachShader);
        WGL_SET_PROC(BeginQuery);
        WGL_SET_PROC(BindAttribLocation);
        WGL_SET_PROC(BindBuffer);
        WGL_SET_PROC(BindFragDataLocation);
        WGL_SET_PROC(BufferData);
        WGL_SET_PROC(BufferSubData);
        WGL_SET_PROC(ClientActiveTexture);
        WGL_SET_PROC(CompileShader);
        WGL_SET_PROC(CompressedTexImage2D);
        WGL_SET_PROC(CreateProgram);
        WGL_SET_PROC(CreateShader);
        WGL_SET_PROC(DeleteBuffers);
        WGL_SET_PROC(DeleteQueries);
        WGL_SET_PROC(DeleteProgram);
        WGL_SET_PROC(DeleteShader);
        WGL_SET_PROC(DisableVertexAttribArray);
        WGL_SET_PROC(DrawBuffers);
        WGL_SET_PROC(EnableVertexAttribArray);
        WGL_SET_PROC(EndQuery);
        WGL_SET_PROC(GenBuffers);
        WGL_SET_PROC(GenerateMipmap);
        WGL_SET_PROC(GenQueries);
        WGL_SET_PROC(GetBufferParameteriv);
        WGL_SET_PROC(GetQueryiv);
        WGL_SET_PROC(GetQueryObjectiv);
        WGL_SET_PROC(GetQueryObjectuiv);
        if (glVer > GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
            WGL_SET_PROC(GetQueryObjecti64v);
            WGL_SET_PROC(GetQueryObjectui64v);
            WGL_SET_PROC(QueryCounter);
        } else if (extensions.has("GL_EXT_timer_query")) {
            WGL_SET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
            WGL_SET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
        }
        WGL_SET_PROC(GetProgramInfoLog);
        WGL_SET_PROC(GetProgramiv);
        WGL_SET_PROC(GetShaderInfoLog);
        WGL_SET_PROC(GetShaderiv);
        WGL_SET_PROC(GetStringi)
        WGL_SET_PROC(GetUniformLocation);
        WGL_SET_PROC(LinkProgram);
        WGL_SET_PROC(ShaderSource);
        WGL_SET_PROC(StencilFuncSeparate);
        WGL_SET_PROC(StencilMaskSeparate);
        WGL_SET_PROC(StencilOpSeparate);
        WGL_SET_PROC(Uniform1f);
        WGL_SET_PROC(Uniform1i);
        WGL_SET_PROC(Uniform1fv);
        WGL_SET_PROC(Uniform1iv);
        WGL_SET_PROC(Uniform2f);
        WGL_SET_PROC(Uniform2i);
        WGL_SET_PROC(Uniform2fv);
        WGL_SET_PROC(Uniform2iv);
        WGL_SET_PROC(Uniform3f);
        WGL_SET_PROC(Uniform3i);
        WGL_SET_PROC(Uniform3fv);
        WGL_SET_PROC(Uniform3iv);
        WGL_SET_PROC(Uniform4f);
        WGL_SET_PROC(Uniform4i);
        WGL_SET_PROC(Uniform4fv);
        WGL_SET_PROC(Uniform4iv);
        WGL_SET_PROC(UniformMatrix2fv);
        WGL_SET_PROC(UniformMatrix3fv);
        WGL_SET_PROC(UniformMatrix4fv);
        WGL_SET_PROC(UseProgram);
        WGL_SET_PROC(VertexAttrib4fv);
        WGL_SET_PROC(VertexAttribPointer);
        WGL_SET_PROC(BindFragDataLocationIndexed);

        if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
            // no ARB suffix for GL_ARB_vertex_array_object
            WGL_SET_PROC(BindVertexArray);
            WGL_SET_PROC(DeleteVertexArrays);
            WGL_SET_PROC(GenVertexArrays);
        }

        // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
        // GL_ARB_framebuffer_object doesn't use ARB suffix.)
        if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
            WGL_SET_PROC(GenFramebuffers);
            WGL_SET_PROC(GetFramebufferAttachmentParameteriv);
            WGL_SET_PROC(GetRenderbufferParameteriv);
            WGL_SET_PROC(BindFramebuffer);
            WGL_SET_PROC(FramebufferTexture2D);
            WGL_SET_PROC(CheckFramebufferStatus);
            WGL_SET_PROC(DeleteFramebuffers);
            WGL_SET_PROC(RenderbufferStorage);
            WGL_SET_PROC(GenRenderbuffers);
            WGL_SET_PROC(DeleteRenderbuffers);
            WGL_SET_PROC(FramebufferRenderbuffer);
            WGL_SET_PROC(BindRenderbuffer);
            WGL_SET_PROC(RenderbufferStorageMultisample);
            WGL_SET_PROC(BlitFramebuffer);
        } else if (extensions.has("GL_EXT_framebuffer_object")) {
            WGL_SET_PROC_SUFFIX(GenFramebuffers, EXT);
            WGL_SET_PROC_SUFFIX(GetFramebufferAttachmentParameteriv, EXT);
            WGL_SET_PROC_SUFFIX(GetRenderbufferParameteriv, EXT);
            WGL_SET_PROC_SUFFIX(BindFramebuffer, EXT);
            WGL_SET_PROC_SUFFIX(FramebufferTexture2D, EXT);
            WGL_SET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
            WGL_SET_PROC_SUFFIX(DeleteFramebuffers, EXT);
            WGL_SET_PROC_SUFFIX(RenderbufferStorage, EXT);
            WGL_SET_PROC_SUFFIX(GenRenderbuffers, EXT);
            WGL_SET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
            WGL_SET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
            WGL_SET_PROC_SUFFIX(BindRenderbuffer, EXT);
            if (extensions.has("GL_EXT_framebuffer_multisample")) {
                WGL_SET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
            }
            if (extensions.has("GL_EXT_framebuffer_blit")) {
                WGL_SET_PROC_SUFFIX(BlitFramebuffer, EXT);
            }
        } else {
            // we must have FBOs
            delete interface;
            return NULL;
        }
        WGL_SET_PROC(MapBuffer);
        WGL_SET_PROC(UnmapBuffer);

        if (extensions.has("GL_NV_path_rendering")) {
            WGL_SET_PROC_SUFFIX(PathCommands, NV);
            WGL_SET_PROC_SUFFIX(PathCoords, NV);
            WGL_SET_PROC_SUFFIX(PathSubCommands, NV);
            WGL_SET_PROC_SUFFIX(PathSubCoords, NV);
            WGL_SET_PROC_SUFFIX(PathString, NV);
            WGL_SET_PROC_SUFFIX(PathGlyphs, NV);
            WGL_SET_PROC_SUFFIX(PathGlyphRange, NV);
            WGL_SET_PROC_SUFFIX(WeightPaths, NV);
            WGL_SET_PROC_SUFFIX(CopyPath, NV);
            WGL_SET_PROC_SUFFIX(InterpolatePaths, NV);
            WGL_SET_PROC_SUFFIX(TransformPath, NV);
            WGL_SET_PROC_SUFFIX(PathParameteriv, NV);
            WGL_SET_PROC_SUFFIX(PathParameteri, NV);
            WGL_SET_PROC_SUFFIX(PathParameterfv, NV);
            WGL_SET_PROC_SUFFIX(PathParameterf, NV);
            WGL_SET_PROC_SUFFIX(PathDashArray, NV);
            WGL_SET_PROC_SUFFIX(GenPaths, NV);
            WGL_SET_PROC_SUFFIX(DeletePaths, NV);
            WGL_SET_PROC_SUFFIX(IsPath, NV);
            WGL_SET_PROC_SUFFIX(PathStencilFunc, NV);
            WGL_SET_PROC_SUFFIX(PathStencilDepthOffset, NV);
            WGL_SET_PROC_SUFFIX(StencilFillPath, NV);
            WGL_SET_PROC_SUFFIX(StencilStrokePath, NV);
            WGL_SET_PROC_SUFFIX(StencilFillPathInstanced, NV);
            WGL_SET_PROC_SUFFIX(StencilStrokePathInstanced, NV);
            WGL_SET_PROC_SUFFIX(PathCoverDepthFunc, NV);
            WGL_SET_PROC_SUFFIX(PathColorGen, NV);
            WGL_SET_PROC_SUFFIX(PathTexGen, NV);
            WGL_SET_PROC_SUFFIX(PathFogGen, NV);
            WGL_SET_PROC_SUFFIX(CoverFillPath, NV);
            WGL_SET_PROC_SUFFIX(CoverStrokePath, NV);
            WGL_SET_PROC_SUFFIX(CoverFillPathInstanced, NV);
            WGL_SET_PROC_SUFFIX(CoverStrokePathInstanced, NV);
            WGL_SET_PROC_SUFFIX(GetPathParameteriv, NV);
            WGL_SET_PROC_SUFFIX(GetPathParameterfv, NV);
            WGL_SET_PROC_SUFFIX(GetPathCommands, NV);
            WGL_SET_PROC_SUFFIX(GetPathCoords, NV);
            WGL_SET_PROC_SUFFIX(GetPathDashArray, NV);
            WGL_SET_PROC_SUFFIX(GetPathMetrics, NV);
            WGL_SET_PROC_SUFFIX(GetPathMetricRange, NV);
            WGL_SET_PROC_SUFFIX(GetPathSpacing, NV);
            WGL_SET_PROC_SUFFIX(GetPathColorGeniv, NV);
            WGL_SET_PROC_SUFFIX(GetPathColorGenfv, NV);
            WGL_SET_PROC_SUFFIX(GetPathTexGeniv, NV);
            WGL_SET_PROC_SUFFIX(GetPathTexGenfv, NV);
            WGL_SET_PROC_SUFFIX(IsPointInFillPath, NV);
            WGL_SET_PROC_SUFFIX(IsPointInStrokePath, NV);
            WGL_SET_PROC_SUFFIX(GetPathLength, NV);
            WGL_SET_PROC_SUFFIX(PointAlongPath, NV);
        }

        interface->fBindingsExported = kDesktop_GrGLBinding;

        return interface;
    } else {
        return NULL;
    }
}

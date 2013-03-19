/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLExtensions.h"
#include "gl/GrGLUtil.h"

#include <stdio.h>

SK_DEFINE_INST_COUNT(GrGLInterface)

#if GR_GL_PER_GL_FUNC_CALLBACK
namespace {
void GrGLDefaultInterfaceCallback(const GrGLInterface*) {}
}
#endif

GrGLInterface::GrGLInterface() {
    fBindingsExported = kNone_GrGLBinding;

#if GR_GL_PER_GL_FUNC_CALLBACK
    fCallback = GrGLDefaultInterfaceCallback;
    fCallbackData = 0;
#endif
}

bool GrGLInterface::validate(GrGLBinding binding) const {

    // kNone must be 0 so that the check we're about to do can never succeed if
    // binding == kNone.
    GR_STATIC_ASSERT(kNone_GrGLBinding == 0);

    if (0 == (binding & fBindingsExported)) {
        return false;
    }

    GrGLExtensions extensions;
    if (!extensions.init(binding, this)) {
        return false;
    }

    // functions that are always required
    if (NULL == fActiveTexture ||
        NULL == fAttachShader ||
        NULL == fBindAttribLocation ||
        NULL == fBindBuffer ||
        NULL == fBindTexture ||
        NULL == fBlendFunc ||
        NULL == fBlendColor ||      // -> GL >= 1.4, ES >= 2.0 or extension
        NULL == fBufferData ||
        NULL == fBufferSubData ||
        NULL == fClear ||
        NULL == fClearColor ||
        NULL == fClearStencil ||
        NULL == fColorMask ||
        NULL == fCompileShader ||
        NULL == fCreateProgram ||
        NULL == fCreateShader ||
        NULL == fCullFace ||
        NULL == fDeleteBuffers ||
        NULL == fDeleteProgram ||
        NULL == fDeleteShader ||
        NULL == fDeleteTextures ||
        NULL == fDepthMask ||
        NULL == fDisable ||
        NULL == fDisableVertexAttribArray ||
        NULL == fDrawArrays ||
        NULL == fDrawElements ||
        NULL == fEnable ||
        NULL == fEnableVertexAttribArray ||
        NULL == fFrontFace ||
        NULL == fGenBuffers ||
        NULL == fGenTextures ||
        NULL == fGetBufferParameteriv ||
        NULL == fGetError ||
        NULL == fGetIntegerv ||
        NULL == fGetProgramInfoLog ||
        NULL == fGetProgramiv ||
        NULL == fGetShaderInfoLog ||
        NULL == fGetShaderiv ||
        NULL == fGetString ||
        NULL == fGetUniformLocation ||
        NULL == fLinkProgram ||
        NULL == fPixelStorei ||
        NULL == fReadPixels ||
        NULL == fScissor ||
        NULL == fShaderSource ||
        NULL == fStencilFunc ||
        NULL == fStencilMask ||
        NULL == fStencilOp ||
        NULL == fTexImage2D ||
        NULL == fTexParameteri ||
        NULL == fTexParameteriv ||
        NULL == fTexSubImage2D ||
        NULL == fUniform1f ||
        NULL == fUniform1i ||
        NULL == fUniform1fv ||
        NULL == fUniform1iv ||
        NULL == fUniform2f ||
        NULL == fUniform2i ||
        NULL == fUniform2fv ||
        NULL == fUniform2iv ||
        NULL == fUniform3f ||
        NULL == fUniform3i ||
        NULL == fUniform3fv ||
        NULL == fUniform3iv ||
        NULL == fUniform4f ||
        NULL == fUniform4i ||
        NULL == fUniform4fv ||
        NULL == fUniform4iv ||
        NULL == fUniformMatrix2fv ||
        NULL == fUniformMatrix3fv ||
        NULL == fUniformMatrix4fv ||
        NULL == fUseProgram ||
        NULL == fVertexAttrib4fv ||
        NULL == fVertexAttribPointer ||
        NULL == fViewport ||
        NULL == fBindFramebuffer ||
        NULL == fBindRenderbuffer ||
        NULL == fCheckFramebufferStatus ||
        NULL == fDeleteFramebuffers ||
        NULL == fDeleteRenderbuffers ||
        NULL == fFinish ||
        NULL == fFlush ||
        NULL == fFramebufferRenderbuffer ||
        NULL == fFramebufferTexture2D ||
        NULL == fGetFramebufferAttachmentParameteriv ||
        NULL == fGetRenderbufferParameteriv ||
        NULL == fGenFramebuffers ||
        NULL == fGenRenderbuffers ||
        NULL == fRenderbufferStorage) {
        return false;
    }

    GrGLVersion glVer = GrGLGetVersion(this);

    // Now check that baseline ES/Desktop fns not covered above are present
    // and that we have fn pointers for any advertised extensions that we will
    // try to use.

    // these functions are part of ES2, we assume they are available
    // On the desktop we assume they are available if the extension
    // is present or GL version is high enough.
    if (kES2_GrGLBinding == binding) {
        if (NULL == fStencilFuncSeparate ||
            NULL == fStencilMaskSeparate ||
            NULL == fStencilOpSeparate) {
            return false;
        }
    } else if (kDesktop_GrGLBinding == binding) {

        if (glVer >= GR_GL_VER(2,0)) {
            if (NULL == fStencilFuncSeparate ||
                NULL == fStencilMaskSeparate ||
                NULL == fStencilOpSeparate) {
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,0) && NULL == fBindFragDataLocation) {
            return false;
        }
        if (glVer >= GR_GL_VER(2,0) || extensions.has("GL_ARB_draw_buffers")) {
            if (NULL == fDrawBuffers) {
                return false;
            }
        }

        if (glVer >= GR_GL_VER(1,5) || extensions.has("GL_ARB_occlusion_query")) {
            if (NULL == fGenQueries ||
                NULL == fDeleteQueries ||
                NULL == fBeginQuery ||
                NULL == fEndQuery ||
                NULL == fGetQueryiv ||
                NULL == fGetQueryObjectiv ||
                NULL == fGetQueryObjectuiv) {
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,3) ||
            extensions.has("GL_ARB_timer_query") ||
            extensions.has("GL_EXT_timer_query")) {
            if (NULL == fGetQueryObjecti64v ||
                NULL == fGetQueryObjectui64v) {
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
            if (NULL == fQueryCounter) {
                return false;
            }
        }
        // The below two blocks are checks for functions used with
        // GL_NV_path_rendering. We're not enforcing that they be non-NULL
        // because they aren't actually called at this time.
        if (false &&
            (NULL == fMatrixMode ||
             NULL == fLoadIdentity ||
             NULL == fLoadMatrixf)) {
            return false;
        }
        if (false && extensions.has("GL_NV_path_rendering")) {
            if (NULL == fPathCommands ||
                NULL == fPathCoords ||
                NULL == fPathSubCommands ||
                NULL == fPathSubCoords ||
                NULL == fPathString ||
                NULL == fPathGlyphs ||
                NULL == fPathGlyphRange ||
                NULL == fWeightPaths ||
                NULL == fCopyPath ||
                NULL == fInterpolatePaths ||
                NULL == fTransformPath ||
                NULL == fPathParameteriv ||
                NULL == fPathParameteri ||
                NULL == fPathParameterfv ||
                NULL == fPathParameterf ||
                NULL == fPathDashArray ||
                NULL == fGenPaths ||
                NULL == fDeletePaths ||
                NULL == fIsPath ||
                NULL == fPathStencilFunc ||
                NULL == fPathStencilDepthOffset ||
                NULL == fStencilFillPath ||
                NULL == fStencilStrokePath ||
                NULL == fStencilFillPathInstanced ||
                NULL == fStencilStrokePathInstanced ||
                NULL == fPathCoverDepthFunc ||
                NULL == fPathColorGen ||
                NULL == fPathTexGen ||
                NULL == fPathFogGen ||
                NULL == fCoverFillPath ||
                NULL == fCoverStrokePath ||
                NULL == fCoverFillPathInstanced ||
                NULL == fCoverStrokePathInstanced ||
                NULL == fGetPathParameteriv ||
                NULL == fGetPathParameterfv ||
                NULL == fGetPathCommands ||
                NULL == fGetPathCoords ||
                NULL == fGetPathDashArray ||
                NULL == fGetPathMetrics ||
                NULL == fGetPathMetricRange ||
                NULL == fGetPathSpacing ||
                NULL == fGetPathColorGeniv ||
                NULL == fGetPathColorGenfv ||
                NULL == fGetPathTexGeniv ||
                NULL == fGetPathTexGenfv ||
                NULL == fIsPointInFillPath ||
                NULL == fIsPointInStrokePath ||
                NULL == fGetPathLength ||
                NULL == fPointAlongPath) {
                return false;
            }
        }
    }

    // optional function on desktop before 1.3
    if (kDesktop_GrGLBinding != binding ||
        (glVer >= GR_GL_VER(1,3)) ||
        extensions.has("GL_ARB_texture_compression")) {
        if (NULL == fCompressedTexImage2D) {
            return false;
        }
    }

    // part of desktop GL, but not ES
    if (kDesktop_GrGLBinding == binding &&
        (NULL == fLineWidth ||
         NULL == fGetTexLevelParameteriv ||
         NULL == fDrawBuffer ||
         NULL == fReadBuffer)) {
        return false;
    }

    // GL_EXT_texture_storage is part of desktop 4.2
    // There is a desktop ARB extension and an ES+desktop EXT extension
    if (kDesktop_GrGLBinding == binding) {
        if (glVer >= GR_GL_VER(4,2) ||
            extensions.has("GL_ARB_texture_storage") ||
            extensions.has("GL_EXT_texture_storage")) {
            if (NULL == fTexStorage2D) {
                return false;
            }
        }
    } else if (extensions.has("GL_EXT_texture_storage")) {
        if (NULL == fTexStorage2D) {
            return false;
        }
    }

    // FBO MSAA
    if (kDesktop_GrGLBinding == binding) {
        // GL 3.0 and the ARB extension have multisample + blit
        if (glVer >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fBlitFramebuffer) {
                return false;
            }
        } else {
            if (extensions.has("GL_EXT_framebuffer_blit") &&
                NULL == fBlitFramebuffer) {
                return false;
            }
            if (extensions.has("GL_EXT_framebuffer_multisample") &&
                NULL == fRenderbufferStorageMultisample) {
                return false;
            }
        }
    } else {
        if (extensions.has("GL_CHROMIUM_framebuffer_multisample")) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fBlitFramebuffer) {
                return false;
            }
        }
        if (extensions.has("GL_APPLE_framebuffer_multisample")) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fResolveMultisampleFramebuffer) {
                return false;
            }
        }
        if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fFramebufferTexture2DMultisample) {
                return false;
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (kDesktop_GrGLBinding == binding || extensions.has("GL_OES_mapbuffer")) {
        if (NULL == fMapBuffer ||
            NULL == fUnmapBuffer) {
            return false;
        }
    }

    // Dual source blending
    if (kDesktop_GrGLBinding == binding &&
        (glVer >= GR_GL_VER(3,3) || extensions.has("GL_ARB_blend_func_extended"))) {
        if (NULL == fBindFragDataLocationIndexed) {
            return false;
        }
    }

    if (kDesktop_GrGLBinding == binding && glVer >= GR_GL_VER(3, 0)) {
        if (NULL == fGetStringi) {
            return false;
        }
    }

    if (kDesktop_GrGLBinding == binding) {
        if (glVer >= GR_GL_VER(3, 0) || extensions.has("GL_ARB_vertex_array_object")) {
            if (NULL == fBindVertexArray ||
                NULL == fDeleteVertexArrays ||
                NULL == fGenVertexArrays) {
                return false;
            }
        }
    } else {
        if (extensions.has("GL_OES_vertex_array_object")) {
            if (NULL == fBindVertexArray ||
                NULL == fDeleteVertexArrays ||
                NULL == fGenVertexArrays) {
                return false;
            }
        }
    }

    return true;
}

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

GrGLInterface::GrGLInterface() {
    fStandard = kNone_GrGLStandard;
}

#ifdef SK_DEBUG
    static int kIsDebug = 1;
#else
    static int kIsDebug = 0;
#endif

#define RETURN_FALSE_INTERFACE                                                                   \
    if (kIsDebug) { SkDebugf("%s:%d GrGLInterface::validate() failed.\n", __FILE__, __LINE__); } \
    return false

bool GrGLInterface::validate() const {

    if (kNone_GrGLStandard == fStandard) {
        RETURN_FALSE_INTERFACE;
    }

    if (!fExtensions.isInitialized()) {
        RETURN_FALSE_INTERFACE;
    }

    // functions that are always required
    if (!fFunctions.fActiveTexture ||
        !fFunctions.fAttachShader ||
        !fFunctions.fBindAttribLocation ||
        !fFunctions.fBindBuffer ||
        !fFunctions.fBindTexture ||
        !fFunctions.fBlendColor ||      // -> GL >= 1.4 or extension, ES >= 2.0
        !fFunctions.fBlendEquation ||   // -> GL >= 1.4 or extension, ES >= 2.0
        !fFunctions.fBlendFunc ||
        !fFunctions.fBufferData ||
        !fFunctions.fBufferSubData ||
        !fFunctions.fClear ||
        !fFunctions.fClearColor ||
        !fFunctions.fClearStencil ||
        !fFunctions.fColorMask ||
        !fFunctions.fCompileShader ||
        !fFunctions.fCompressedTexImage2D ||
        !fFunctions.fCompressedTexSubImage2D ||
        !fFunctions.fCopyTexSubImage2D ||
        !fFunctions.fCreateProgram ||
        !fFunctions.fCreateShader ||
        !fFunctions.fCullFace ||
        !fFunctions.fDeleteBuffers ||
        !fFunctions.fDeleteProgram ||
        !fFunctions.fDeleteShader ||
        !fFunctions.fDeleteTextures ||
        !fFunctions.fDepthMask ||
        !fFunctions.fDisable ||
        !fFunctions.fDisableVertexAttribArray ||
        !fFunctions.fDrawArrays ||
        !fFunctions.fDrawElements ||
        !fFunctions.fEnable ||
        !fFunctions.fEnableVertexAttribArray ||
        !fFunctions.fFrontFace ||
        !fFunctions.fGenBuffers ||
        !fFunctions.fGenTextures ||
        !fFunctions.fGetBufferParameteriv ||
        !fFunctions.fGenerateMipmap ||
        !fFunctions.fGetError ||
        !fFunctions.fGetIntegerv ||
        !fFunctions.fGetProgramInfoLog ||
        !fFunctions.fGetProgramiv ||
        !fFunctions.fGetShaderInfoLog ||
        !fFunctions.fGetShaderiv ||
        !fFunctions.fGetString ||
        !fFunctions.fGetUniformLocation ||
        !fFunctions.fIsTexture ||
        !fFunctions.fLinkProgram ||
        !fFunctions.fLineWidth ||
        !fFunctions.fPixelStorei ||
        !fFunctions.fReadPixels ||
        !fFunctions.fScissor ||
        !fFunctions.fShaderSource ||
        !fFunctions.fStencilFunc ||
        !fFunctions.fStencilFuncSeparate ||
        !fFunctions.fStencilMask ||
        !fFunctions.fStencilMaskSeparate ||
        !fFunctions.fStencilOp ||
        !fFunctions.fStencilOpSeparate ||
        !fFunctions.fTexImage2D ||
        !fFunctions.fTexParameterf ||
        !fFunctions.fTexParameterfv ||
        !fFunctions.fTexParameteri ||
        !fFunctions.fTexParameteriv ||
        !fFunctions.fTexSubImage2D ||
        !fFunctions.fUniform1f ||
        !fFunctions.fUniform1i ||
        !fFunctions.fUniform1fv ||
        !fFunctions.fUniform1iv ||
        !fFunctions.fUniform2f ||
        !fFunctions.fUniform2i ||
        !fFunctions.fUniform2fv ||
        !fFunctions.fUniform2iv ||
        !fFunctions.fUniform3f ||
        !fFunctions.fUniform3i ||
        !fFunctions.fUniform3fv ||
        !fFunctions.fUniform3iv ||
        !fFunctions.fUniform4f ||
        !fFunctions.fUniform4i ||
        !fFunctions.fUniform4fv ||
        !fFunctions.fUniform4iv ||
        !fFunctions.fUniformMatrix2fv ||
        !fFunctions.fUniformMatrix3fv ||
        !fFunctions.fUniformMatrix4fv ||
        !fFunctions.fUseProgram ||
        !fFunctions.fVertexAttrib1f ||
        !fFunctions.fVertexAttrib2fv ||
        !fFunctions.fVertexAttrib3fv ||
        !fFunctions.fVertexAttrib4fv ||
        !fFunctions.fVertexAttribPointer ||
        !fFunctions.fViewport ||
        !fFunctions.fBindFramebuffer ||
        !fFunctions.fBindRenderbuffer ||
        !fFunctions.fCheckFramebufferStatus ||
        !fFunctions.fDeleteFramebuffers ||
        !fFunctions.fDeleteRenderbuffers ||
        !fFunctions.fFinish ||
        !fFunctions.fFlush ||
        !fFunctions.fFramebufferRenderbuffer ||
        !fFunctions.fFramebufferTexture2D ||
        !fFunctions.fGetFramebufferAttachmentParameteriv ||
        !fFunctions.fGetRenderbufferParameteriv ||
        !fFunctions.fGenFramebuffers ||
        !fFunctions.fGenRenderbuffers ||
        !fFunctions.fRenderbufferStorage) {
        RETURN_FALSE_INTERFACE;
    }

    GrGLVersion glVer = GrGLGetVersion(this);
    if (GR_GL_INVALID_VER == glVer) {
        RETURN_FALSE_INTERFACE;
    }

    // Now check that baseline ES/Desktop fns not covered above are present
    // and that we have fn pointers for any advertised fExtensions that we will
    // try to use.

    // these functions are part of ES2, we assume they are available
    // On the desktop we assume they are available if the extension
    // is present or GL version is high enough.
    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3,0) && !fFunctions.fBindFragDataLocation) {
            RETURN_FALSE_INTERFACE;
        }

        if (glVer >= GR_GL_VER(3,3) ||
            fExtensions.has("GL_ARB_timer_query") ||
            fExtensions.has("GL_EXT_timer_query")) {
            if (!fFunctions.fGetQueryObjecti64v ||
                !fFunctions.fGetQueryObjectui64v) {
                RETURN_FALSE_INTERFACE;
            }
        }
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_timer_query")) {
            if (!fFunctions.fQueryCounter) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    // part of desktop GL, but not ES
    if (GR_IS_GR_GL(fStandard) &&
        (!fFunctions.fDrawBuffer ||
         !fFunctions.fPolygonMode)) {
        RETURN_FALSE_INTERFACE;
    }

    // ES 3.0 (or ES 2.0 extended) has glDrawBuffers but not glDrawBuffer
    if (GR_IS_GR_GL(fStandard) ||
       (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,0))) {
        if (!fFunctions.fDrawBuffers) {
            RETURN_FALSE_INTERFACE;
        }
        if (!fFunctions.fReadBuffer) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // glGetTexLevelParameteriv was added to ES in 3.1.
    if (GR_IS_GR_GL(fStandard) ||
       (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,1))) {
        if (!fFunctions.fGetTexLevelParameteriv) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // GL_EXT_texture_storage is part of desktop 4.2
    // There is a desktop ARB extension and an ES+desktop EXT extension
    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(4,2) ||
            fExtensions.has("GL_ARB_texture_storage") ||
            fExtensions.has("GL_EXT_texture_storage")) {
            if (!fFunctions.fTexStorage2D) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
         if (fExtensions.has("GL_EXT_texture_storage")) {
            if (!fFunctions.fTexStorage2D) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    // glTextureBarrier is part of desktop 4.5. There are also ARB and NV extensions.
    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(4,5) ||
            fExtensions.has("GL_ARB_texture_barrier") ||
            fExtensions.has("GL_NV_texture_barrier")) {
            if (!fFunctions.fTextureBarrier) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (fExtensions.has("GL_NV_texture_barrier")) {
            if (!fFunctions.fTextureBarrier) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (fExtensions.has("GL_KHR_blend_equation_advanced") ||
        fExtensions.has("GL_NV_blend_equation_advanced")) {
        if (!fFunctions.fBlendBarrier) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (fExtensions.has("GL_EXT_discard_framebuffer")) {
        if (!fFunctions.fDiscardFramebuffer) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // Required since OpenGL 1.5 and ES 3.0 or with GL_EXT_occlusion_query_boolean
    if (GR_IS_GR_GL(fStandard) ||
       (GR_IS_GR_GL_ES(fStandard) && (glVer >= GR_GL_VER(3,0) ||
                                  fExtensions.has("GL_EXT_occlusion_query_boolean")))) {
#if 0 // Not yet added to chrome's bindings.
        if (!fFunctions.fGenQueries ||
            !fFunctions.fDeleteQueries ||
            !fFunctions.fBeginQuery ||
            !fFunctions.fEndQuery ||
            !fFunctions.fGetQueryiv ||
            !fFunctions.fGetQueryObjectuiv) {
            RETURN_FALSE_INTERFACE;
        }
#endif
    }
    // glGetQueryObjectiv doesn't exist in ES.
    if (GR_IS_GR_GL(fStandard) && !fFunctions.fGetQueryObjectiv) {
        RETURN_FALSE_INTERFACE;
    }

    // FBO MSAA
    if (GR_IS_GR_GL(fStandard)) {
        // GL 3.0 and the ARB extension have multisample + blit
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_ARB_framebuffer_object")) {
            if (!fFunctions.fRenderbufferStorageMultisample ||
                !fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE;
            }
        } else {
            if (fExtensions.has("GL_EXT_framebuffer_blit") &&
                !fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE;
            }
            if (fExtensions.has("GL_EXT_framebuffer_multisample") &&
                !fFunctions.fRenderbufferStorageMultisample) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_CHROMIUM_framebuffer_multisample")) {
            if (!fFunctions.fRenderbufferStorageMultisample ||
                !fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE;
            }
        } else {
            if (fExtensions.has("GL_ANGLE_framebuffer_multisample") &&
                !fFunctions.fRenderbufferStorageMultisample) {
                RETURN_FALSE_INTERFACE;
            }
            if (fExtensions.has("GL_ANGLE_framebuffer_blit") &&
                !fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE;
            }
        }
        if (fExtensions.has("GL_APPLE_framebuffer_multisample")) {
            if (!fFunctions.fRenderbufferStorageMultisampleES2APPLE ||
                !fFunctions.fResolveMultisampleFramebuffer) {
                RETURN_FALSE_INTERFACE;
            }
        }
        if (fExtensions.has("GL_IMG_multisampled_render_to_texture") ||
            fExtensions.has("GL_EXT_multisampled_render_to_texture")) {
            if (!fFunctions.fRenderbufferStorageMultisampleES2EXT ||
                !fFunctions.fFramebufferTexture2DMultisample) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (GR_IS_GR_GL(fStandard) ||
       (GR_IS_GR_GL_ES(fStandard) && fExtensions.has("GL_OES_mapbuffer"))) {
        if (!fFunctions.fMapBuffer ||
            !fFunctions.fUnmapBuffer) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // Dual source blending
    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_blend_func_extended")) {
            if (!fFunctions.fBindFragDataLocationIndexed) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3,0) && fExtensions.has("GL_EXT_blend_func_extended")) {
            if (!fFunctions.fBindFragDataLocation ||
                !fFunctions.fBindFragDataLocationIndexed) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (GR_IS_GR_GL(fStandard) || GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3, 0)) {
            // glGetStringi was added in version 3.0 of both desktop and ES.
            if (!fFunctions.fGetStringi) {
                RETURN_FALSE_INTERFACE;
            }
            // glVertexAttribIPointer was added in version 3.0 of both desktop and ES.
            if (!fFunctions.fVertexAttribIPointer) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3,1)) {
            if (!fFunctions.fTexBuffer) {
                RETURN_FALSE_INTERFACE;
            }
        }
        if (glVer >= GR_GL_VER(4,3)) {
            if (!fFunctions.fTexBufferRange) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3,2) || fExtensions.has("GL_OES_texture_buffer") ||
            fExtensions.has("GL_EXT_texture_buffer")) {
            if (!fFunctions.fTexBuffer ||
                !fFunctions.fTexBufferRange) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3, 0) || fExtensions.has("GL_ARB_vertex_array_object")) {
            if (!fFunctions.fBindVertexArray ||
                !fFunctions.fDeleteVertexArrays ||
                !fFunctions.fGenVertexArrays) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_OES_vertex_array_object")) {
            if (!fFunctions.fBindVertexArray ||
                !fFunctions.fDeleteVertexArrays ||
                !fFunctions.fGenVertexArrays) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (fExtensions.has("GL_EXT_debug_marker")) {
        if (!fFunctions.fInsertEventMarker ||
            !fFunctions.fPushGroupMarker ||
            !fFunctions.fPopGroupMarker) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) && glVer >= GR_GL_VER(4,3)) ||
        fExtensions.has("GL_ARB_invalidate_subdata")) {
        if (!fFunctions.fInvalidateBufferData ||
            !fFunctions.fInvalidateBufferSubData ||
            !fFunctions.fInvalidateFramebuffer ||
            !fFunctions.fInvalidateSubFramebuffer ||
            !fFunctions.fInvalidateTexImage ||
            !fFunctions.fInvalidateTexSubImage) {
            RETURN_FALSE_INTERFACE;
        }
    } else if (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,0)) {
        // ES 3.0 adds the framebuffer functions but not the others.
        if (!fFunctions.fInvalidateFramebuffer ||
            !fFunctions.fInvalidateSubFramebuffer) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (GR_IS_GR_GL_ES(fStandard) && fExtensions.has("GL_CHROMIUM_map_sub")) {
        if (!fFunctions.fMapBufferSubData ||
            !fFunctions.fMapTexSubImage2D ||
            !fFunctions.fUnmapBufferSubData ||
            !fFunctions.fUnmapTexSubImage2D) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) && fExtensions.has("GL_ARB_map_buffer_range")) ||
        (GR_IS_GR_GL_ES(fStandard) && fExtensions.has("GL_EXT_map_buffer_range")) ||
        // These functions are added to the 3.0 version of both GLES and GL.
        ((GR_IS_GR_GL(fStandard) || GR_IS_GR_GL_ES(fStandard)) && glVer >= GR_GL_VER(3,0))) {
        if (!fFunctions.fMapBufferRange ||
            !fFunctions.fFlushMappedBufferRange) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) &&
            (glVer >= GR_GL_VER(3,2) || fExtensions.has("GL_ARB_texture_multisample"))) ||
        (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,1))) {
        if (!fFunctions.fGetMultisamplefv) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) &&
            (glVer >= GR_GL_VER(4,3) || fExtensions.has("GL_ARB_program_interface_query"))) ||
        (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,1))) {
        if (!fFunctions.fGetProgramResourceLocation) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (GR_IS_GR_GL_ES(fStandard) || glVer >= GR_GL_VER(4,1) ||
        fExtensions.has("GL_ARB_ES2_compatibility")) {
        if (!fFunctions.fGetShaderPrecisionFormat) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (fExtensions.has("GL_NV_path_rendering") || fExtensions.has("GL_CHROMIUM_path_rendering")) {
        if (!fFunctions.fMatrixLoadf ||
            !fFunctions.fMatrixLoadIdentity ||
            !fFunctions.fPathCommands ||
            !fFunctions.fPathParameteri ||
            !fFunctions.fPathParameterf ||
            !fFunctions.fGenPaths ||
            !fFunctions.fDeletePaths ||
            !fFunctions.fIsPath ||
            !fFunctions.fPathStencilFunc ||
            !fFunctions.fStencilFillPath ||
            !fFunctions.fStencilStrokePath ||
            !fFunctions.fStencilFillPathInstanced ||
            !fFunctions.fStencilStrokePathInstanced ||
            !fFunctions.fCoverFillPath ||
            !fFunctions.fCoverStrokePath ||
            !fFunctions.fCoverFillPathInstanced ||
            !fFunctions.fCoverStrokePathInstanced
#if 0
            // List of functions that Skia uses, but which have been added since the initial release
            // of NV_path_rendering driver. We do not want to fail interface validation due to
            // missing features, we will just not use the extension.
            // Update this list -> update GrGLCaps::hasPathRenderingSupport too.
            || !fFunctions.fStencilThenCoverFillPath ||
            !fFunctions.fStencilThenCoverStrokePath ||
            !fFunctions.fStencilThenCoverFillPathInstanced ||
            !fFunctions.fStencilThenCoverStrokePathInstanced ||
            !fFunctions.fProgramPathFragmentInputGen
#endif
            ) {
            RETURN_FALSE_INTERFACE;
        }
        if (fExtensions.has("GL_CHROMIUM_path_rendering")) {
            if (!fFunctions.fBindFragmentInputLocation) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (fExtensions.has("GL_NV_framebuffer_mixed_samples") ||
        fExtensions.has("GL_CHROMIUM_framebuffer_mixed_samples")) {
        if (!fFunctions.fCoverageModulation) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3,1) ||
            fExtensions.has("GL_EXT_draw_instanced") || fExtensions.has("GL_ARB_draw_instanced")) {
            if (!fFunctions.fDrawArraysInstanced ||
                !fFunctions.fDrawElementsInstanced) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_draw_instanced")) {
            if (!fFunctions.fDrawArraysInstanced ||
                !fFunctions.fDrawElementsInstanced) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3,2) || fExtensions.has("GL_ARB_instanced_arrays")) {
            if (!fFunctions.fVertexAttribDivisor) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_instanced_arrays")) {
            if (!fFunctions.fVertexAttribDivisor) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if ((GR_IS_GR_GL(fStandard) &&
             (glVer >= GR_GL_VER(4,0) || fExtensions.has("GL_ARB_draw_indirect"))) ||
        (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,1))) {
        if (!fFunctions.fDrawArraysIndirect ||
            !fFunctions.fDrawElementsIndirect) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) &&
            (glVer >= GR_GL_VER(4,3) || fExtensions.has("GL_ARB_multi_draw_indirect"))) ||
        (GR_IS_GR_GL_ES(fStandard) && fExtensions.has("GL_EXT_multi_draw_indirect"))) {
        if (!fFunctions.fMultiDrawArraysIndirect ||
            !fFunctions.fMultiDrawElementsIndirect) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) && glVer >= GR_GL_VER(4,3)) ||
        fExtensions.has("GL_KHR_debug")) {
        if (!fFunctions.fDebugMessageControl ||
            !fFunctions.fDebugMessageInsert ||
            !fFunctions.fDebugMessageCallback ||
            !fFunctions.fGetDebugMessageLog ||
            !fFunctions.fPushDebugGroup ||
            !fFunctions.fPopDebugGroup ||
            !fFunctions.fObjectLabel) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (fExtensions.has("GL_EXT_window_rectangles")) {
        if (!fFunctions.fWindowRectangles) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (GR_IS_GR_GL(fStandard)) {
        if (glVer >= GR_GL_VER(3, 2) || fExtensions.has("GL_ARB_sync")) {
            if (!fFunctions.fFenceSync ||
                !fFunctions.fIsSync ||
                !fFunctions.fClientWaitSync ||
                !fFunctions.fWaitSync ||
                !fFunctions.fDeleteSync) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else if (GR_IS_GR_GL_ES(fStandard)) {
        if (glVer >= GR_GL_VER(3, 0) || fExtensions.has("GL_APPLE_sync")) {
            if (!fFunctions.fFenceSync ||
                !fFunctions.fIsSync ||
                !fFunctions.fClientWaitSync ||
                !fFunctions.fWaitSync ||
                !fFunctions.fDeleteSync) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (fExtensions.has("EGL_KHR_image") || fExtensions.has("EGL_KHR_image_base")) {
        if (!fFunctions.fEGLCreateImage ||
            !fFunctions.fEGLDestroyImage) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // glDrawRangeElements was added to ES in 3.0.
    if (GR_IS_GR_GL(fStandard) ||
       (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,0))) {
        if (!fFunctions.fDrawRangeElements) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // getInternalformativ was added in GL 4.2, ES 3.0, and with extension ARB_internalformat_query
    if ((GR_IS_GR_GL(fStandard) && (glVer >= GR_GL_VER(4,2) ||
        (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,0)) ||
         fExtensions.has("GL_ARB_internalformat_query")))) {
        if (!fFunctions.fGetInternalformativ) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) && glVer >= GR_GL_VER(4,1)) ||
        (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,0))) {
        if (!fFunctions.fGetProgramBinary ||
            !fFunctions.fProgramBinary ||
            !fFunctions.fProgramParameteri) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((GR_IS_GR_GL(fStandard) && glVer >= GR_GL_VER(4,1)) ||
        (GR_IS_GR_GL_ES(fStandard) && glVer >= GR_GL_VER(3,0))) {
        if (!fFunctions.fBindSampler ||
            !fFunctions.fDeleteSamplers  ||
            !fFunctions.fGenSamplers ||
            !fFunctions.fSamplerParameteri ||
            !fFunctions.fSamplerParameteriv) {
            RETURN_FALSE_INTERFACE;
        }
    }

    return true;
}

#if GR_TEST_UTILS

void GrGLInterface::abandon() const {
    const_cast<GrGLInterface*>(this)->fFunctions = GrGLInterface::Functions();
}

#endif // GR_TEST_UTILS


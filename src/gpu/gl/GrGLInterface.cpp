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

const GrGLInterface* GrGLInterfaceAddTestDebugMarker(const GrGLInterface* interface,
                                                     GrGLInsertEventMarkerProc insertEventMarkerFn,
                                                     GrGLPushGroupMarkerProc pushGroupMarkerFn,
                                                     GrGLPopGroupMarkerProc popGroupMarkerFn) {
    GrGLInterface* newInterface = GrGLInterface::NewClone(interface);

    if (!newInterface->fExtensions.has("GL_EXT_debug_marker")) {
        newInterface->fExtensions.add("GL_EXT_debug_marker");
    }

    newInterface->fFunctions.fInsertEventMarker = insertEventMarkerFn;
    newInterface->fFunctions.fPushGroupMarker = pushGroupMarkerFn;
    newInterface->fFunctions.fPopGroupMarker = popGroupMarkerFn;

    return newInterface;
}

const GrGLInterface* GrGLInterfaceRemoveNVPR(const GrGLInterface* interface) {
    GrGLInterface* newInterface = GrGLInterface::NewClone(interface);

    newInterface->fExtensions.remove("GL_NV_path_rendering");
    newInterface->fExtensions.remove("GL_CHROMIUM_path_rendering");
    newInterface->fFunctions.fMatrixLoadf = nullptr;
    newInterface->fFunctions.fMatrixLoadIdentity = nullptr;
    newInterface->fFunctions.fPathCommands = nullptr;
    newInterface->fFunctions.fPathParameteri = nullptr;
    newInterface->fFunctions.fPathParameterf = nullptr;
    newInterface->fFunctions.fGenPaths = nullptr;
    newInterface->fFunctions.fDeletePaths = nullptr;
    newInterface->fFunctions.fIsPath = nullptr;
    newInterface->fFunctions.fPathStencilFunc = nullptr;
    newInterface->fFunctions.fStencilFillPath = nullptr;
    newInterface->fFunctions.fStencilStrokePath = nullptr;
    newInterface->fFunctions.fStencilFillPathInstanced = nullptr;
    newInterface->fFunctions.fStencilStrokePathInstanced = nullptr;
    newInterface->fFunctions.fCoverFillPath = nullptr;
    newInterface->fFunctions.fCoverStrokePath = nullptr;
    newInterface->fFunctions.fCoverFillPathInstanced = nullptr;
    newInterface->fFunctions.fCoverStrokePathInstanced = nullptr;
    newInterface->fFunctions.fStencilThenCoverFillPath = nullptr;
    newInterface->fFunctions.fStencilThenCoverStrokePath = nullptr;
    newInterface->fFunctions.fStencilThenCoverFillPathInstanced = nullptr;
    newInterface->fFunctions.fStencilThenCoverStrokePathInstanced = nullptr;
    newInterface->fFunctions.fProgramPathFragmentInputGen = nullptr;
    newInterface->fFunctions.fBindFragmentInputLocation = nullptr;
    return newInterface;
}

GrGLInterface::GrGLInterface() {
    fStandard = kNone_GrGLStandard;
}

GrGLInterface* GrGLInterface::NewClone(const GrGLInterface* interface) {
    SkASSERT(interface);

    GrGLInterface* clone = new GrGLInterface;
    clone->fStandard = interface->fStandard;
    clone->fExtensions = interface->fExtensions;
    clone->fFunctions = interface->fFunctions;
    return clone;
}

#ifdef SK_DEBUG
    static int kIsDebug = 1;
#else
    static int kIsDebug = 0;
#endif

#define RETURN_FALSE_INTERFACE                                                                   \
    if (kIsDebug) { SkDebugf("%s:%d GrGLInterface::validate() failed.\n", __FILE__, __LINE__); } \
    return false;

bool GrGLInterface::validate() const {

    if (kNone_GrGLStandard == fStandard) {
        RETURN_FALSE_INTERFACE
    }

    if (!fExtensions.isInitialized()) {
        RETURN_FALSE_INTERFACE
    }

    // functions that are always required
    if (nullptr == fFunctions.fActiveTexture ||
        nullptr == fFunctions.fAttachShader ||
        nullptr == fFunctions.fBindAttribLocation ||
        nullptr == fFunctions.fBindBuffer ||
        nullptr == fFunctions.fBindTexture ||
        nullptr == fFunctions.fBlendColor ||      // -> GL >= 1.4 or extension, ES >= 2.0
        nullptr == fFunctions.fBlendEquation ||   // -> GL >= 1.4 or extension, ES >= 2.0
        nullptr == fFunctions.fBlendFunc ||
        nullptr == fFunctions.fBufferData ||
        nullptr == fFunctions.fBufferSubData ||
        nullptr == fFunctions.fClear ||
        nullptr == fFunctions.fClearColor ||
        nullptr == fFunctions.fClearStencil ||
        nullptr == fFunctions.fColorMask ||
        nullptr == fFunctions.fCompileShader ||
        nullptr == fFunctions.fCopyTexSubImage2D ||
        nullptr == fFunctions.fCreateProgram ||
        nullptr == fFunctions.fCreateShader ||
        nullptr == fFunctions.fCullFace ||
        nullptr == fFunctions.fDeleteBuffers ||
        nullptr == fFunctions.fDeleteProgram ||
        nullptr == fFunctions.fDeleteShader ||
        nullptr == fFunctions.fDeleteTextures ||
        nullptr == fFunctions.fDepthMask ||
        nullptr == fFunctions.fDisable ||
        nullptr == fFunctions.fDisableVertexAttribArray ||
        nullptr == fFunctions.fDrawArrays ||
        nullptr == fFunctions.fDrawElements ||
        nullptr == fFunctions.fEnable ||
        nullptr == fFunctions.fEnableVertexAttribArray ||
        nullptr == fFunctions.fFrontFace ||
        nullptr == fFunctions.fGenBuffers ||
        nullptr == fFunctions.fGenTextures ||
        nullptr == fFunctions.fGetBufferParameteriv ||
        nullptr == fFunctions.fGenerateMipmap ||
        nullptr == fFunctions.fGetError ||
        nullptr == fFunctions.fGetIntegerv ||
        nullptr == fFunctions.fGetProgramInfoLog ||
        nullptr == fFunctions.fGetProgramiv ||
        nullptr == fFunctions.fGetShaderInfoLog ||
        nullptr == fFunctions.fGetShaderiv ||
        nullptr == fFunctions.fGetString ||
        nullptr == fFunctions.fGetUniformLocation ||
#if 0 //  Not included in Chrome yet
        nullptr == fFunctions.fIsTexture ||
#endif
        nullptr == fFunctions.fLinkProgram ||
        nullptr == fFunctions.fLineWidth ||
        nullptr == fFunctions.fPixelStorei ||
        nullptr == fFunctions.fReadPixels ||
        nullptr == fFunctions.fScissor ||
        nullptr == fFunctions.fShaderSource ||
        nullptr == fFunctions.fStencilFunc ||
        nullptr == fFunctions.fStencilMask ||
        nullptr == fFunctions.fStencilOp ||
        nullptr == fFunctions.fTexImage2D ||
        nullptr == fFunctions.fTexParameteri ||
        nullptr == fFunctions.fTexParameteriv ||
        nullptr == fFunctions.fTexSubImage2D ||
        nullptr == fFunctions.fUniform1f ||
        nullptr == fFunctions.fUniform1i ||
        nullptr == fFunctions.fUniform1fv ||
        nullptr == fFunctions.fUniform1iv ||
        nullptr == fFunctions.fUniform2f ||
        nullptr == fFunctions.fUniform2i ||
        nullptr == fFunctions.fUniform2fv ||
        nullptr == fFunctions.fUniform2iv ||
        nullptr == fFunctions.fUniform3f ||
        nullptr == fFunctions.fUniform3i ||
        nullptr == fFunctions.fUniform3fv ||
        nullptr == fFunctions.fUniform3iv ||
        nullptr == fFunctions.fUniform4f ||
        nullptr == fFunctions.fUniform4i ||
        nullptr == fFunctions.fUniform4fv ||
        nullptr == fFunctions.fUniform4iv ||
        nullptr == fFunctions.fUniformMatrix2fv ||
        nullptr == fFunctions.fUniformMatrix3fv ||
        nullptr == fFunctions.fUniformMatrix4fv ||
        nullptr == fFunctions.fUseProgram ||
        nullptr == fFunctions.fVertexAttrib1f ||
        nullptr == fFunctions.fVertexAttrib2fv ||
        nullptr == fFunctions.fVertexAttrib3fv ||
        nullptr == fFunctions.fVertexAttrib4fv ||
        nullptr == fFunctions.fVertexAttribPointer ||
        nullptr == fFunctions.fViewport ||
        nullptr == fFunctions.fBindFramebuffer ||
        nullptr == fFunctions.fBindRenderbuffer ||
        nullptr == fFunctions.fCheckFramebufferStatus ||
        nullptr == fFunctions.fDeleteFramebuffers ||
        nullptr == fFunctions.fDeleteRenderbuffers ||
        nullptr == fFunctions.fFinish ||
        nullptr == fFunctions.fFlush ||
        nullptr == fFunctions.fFramebufferRenderbuffer ||
        nullptr == fFunctions.fFramebufferTexture2D ||
        nullptr == fFunctions.fGetFramebufferAttachmentParameteriv ||
        nullptr == fFunctions.fGetRenderbufferParameteriv ||
        nullptr == fFunctions.fGenFramebuffers ||
        nullptr == fFunctions.fGenRenderbuffers ||
        nullptr == fFunctions.fRenderbufferStorage) {
        RETURN_FALSE_INTERFACE
    }

    GrGLVersion glVer = GrGLGetVersion(this);
    if (GR_GL_INVALID_VER == glVer) {
        RETURN_FALSE_INTERFACE
    }

    // Now check that baseline ES/Desktop fns not covered above are present
    // and that we have fn pointers for any advertised fExtensions that we will
    // try to use.

    // these functions are part of ES2, we assume they are available
    // On the desktop we assume they are available if the extension
    // is present or GL version is high enough.
    if (kGLES_GrGLStandard == fStandard) {
        if (nullptr == fFunctions.fStencilFuncSeparate ||
            nullptr == fFunctions.fStencilMaskSeparate ||
            nullptr == fFunctions.fStencilOpSeparate) {
            RETURN_FALSE_INTERFACE
        }
    } else if (kGL_GrGLStandard == fStandard) {

        if (glVer >= GR_GL_VER(2,0)) {
            if (nullptr == fFunctions.fStencilFuncSeparate ||
                nullptr == fFunctions.fStencilMaskSeparate ||
                nullptr == fFunctions.fStencilOpSeparate) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,0) && nullptr == fFunctions.fBindFragDataLocation) {
            RETURN_FALSE_INTERFACE
        }
        if (glVer >= GR_GL_VER(2,0) || fExtensions.has("GL_ARB_draw_buffers")) {
            if (nullptr == fFunctions.fDrawBuffers) {
                RETURN_FALSE_INTERFACE
            }
        }

        if (glVer >= GR_GL_VER(1,5) || fExtensions.has("GL_ARB_occlusion_query")) {
            if (nullptr == fFunctions.fGenQueries ||
                nullptr == fFunctions.fDeleteQueries ||
                nullptr == fFunctions.fBeginQuery ||
                nullptr == fFunctions.fEndQuery ||
                nullptr == fFunctions.fGetQueryiv ||
                nullptr == fFunctions.fGetQueryObjectiv ||
                nullptr == fFunctions.fGetQueryObjectuiv) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,3) ||
            fExtensions.has("GL_ARB_timer_query") ||
            fExtensions.has("GL_EXT_timer_query")) {
            if (nullptr == fFunctions.fGetQueryObjecti64v ||
                nullptr == fFunctions.fGetQueryObjectui64v) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_timer_query")) {
            if (nullptr == fFunctions.fQueryCounter) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    // optional function on desktop before 1.3
    if (kGL_GrGLStandard != fStandard ||
        (glVer >= GR_GL_VER(1,3)) ||
        fExtensions.has("GL_ARB_texture_compression")) {
        if (nullptr == fFunctions.fCompressedTexImage2D
#if 0
            || nullptr == fFunctions.fCompressedTexSubImage2D
#endif
            ) {
            RETURN_FALSE_INTERFACE
        }
    }

    // part of desktop GL, but not ES
    if (kGL_GrGLStandard == fStandard &&
        (nullptr == fFunctions.fGetTexLevelParameteriv ||
         nullptr == fFunctions.fDrawBuffer ||
         nullptr == fFunctions.fReadBuffer)) {
        RETURN_FALSE_INTERFACE
    }

    // GL_EXT_texture_storage is part of desktop 4.2
    // There is a desktop ARB extension and an ES+desktop EXT extension
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(4,2) ||
            fExtensions.has("GL_ARB_texture_storage") ||
            fExtensions.has("GL_EXT_texture_storage")) {
            if (nullptr == fFunctions.fTexStorage2D) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_texture_storage")) {
        if (nullptr == fFunctions.fTexStorage2D) {
            RETURN_FALSE_INTERFACE
        }
    }

    // glTextureBarrier is part of desktop 4.5. There are also ARB and NV extensions.
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(4,5) ||
            fExtensions.has("GL_ARB_texture_barrier") ||
            fExtensions.has("GL_NV_texture_barrier")) {
            if (nullptr == fFunctions.fTextureBarrier) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else if (fExtensions.has("GL_NV_texture_barrier")) {
        if (nullptr == fFunctions.fTextureBarrier) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_KHR_blend_equation_advanced") ||
        fExtensions.has("GL_NV_blend_equation_advanced")) {
        if (nullptr == fFunctions.fBlendBarrier) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_EXT_discard_framebuffer")) {
// FIXME: Remove this once Chromium is updated to provide this function
#if 0
        if (nullptr == fFunctions.fDiscardFramebuffer) {
            RETURN_FALSE_INTERFACE
        }
#endif
    }

    // FBO MSAA
    if (kGL_GrGLStandard == fStandard) {
        // GL 3.0 and the ARB extension have multisample + blit
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_ARB_framebuffer_object")) {
            if (nullptr == fFunctions.fRenderbufferStorageMultisample ||
                nullptr == fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
        } else {
            if (fExtensions.has("GL_EXT_framebuffer_blit") &&
                nullptr == fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
            if (fExtensions.has("GL_EXT_framebuffer_multisample") &&
                nullptr == fFunctions.fRenderbufferStorageMultisample) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_CHROMIUM_framebuffer_multisample")) {
            if (nullptr == fFunctions.fRenderbufferStorageMultisample ||
                nullptr == fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (fExtensions.has("GL_APPLE_framebuffer_multisample")) {
            if (nullptr == fFunctions.fRenderbufferStorageMultisampleES2APPLE ||
                nullptr == fFunctions.fResolveMultisampleFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (fExtensions.has("GL_IMG_multisampled_render_to_texture") ||
            fExtensions.has("GL_EXT_multisampled_render_to_texture")) {
            if (nullptr == fFunctions.fRenderbufferStorageMultisampleES2EXT ||
                nullptr == fFunctions.fFramebufferTexture2DMultisample) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (kGL_GrGLStandard == fStandard || fExtensions.has("GL_OES_mapbuffer")) {
        if (nullptr == fFunctions.fMapBuffer ||
            nullptr == fFunctions.fUnmapBuffer) {
            RETURN_FALSE_INTERFACE
        }
    }

    // Dual source blending
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_blend_func_extended")) {
            if (nullptr == fFunctions.fBindFragDataLocationIndexed) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) && fExtensions.has("GL_EXT_blend_func_extended")) {
            if (nullptr == fFunctions.fBindFragDataLocation ||
                nullptr == fFunctions.fBindFragDataLocationIndexed) {
                RETURN_FALSE_INTERFACE
            }
        }
    }


    // glGetStringi was added in version 3.0 of both desktop and ES.
    if (glVer >= GR_GL_VER(3, 0)) {
        if (nullptr == fFunctions.fGetStringi) {
            RETURN_FALSE_INTERFACE
        }
    }

    // glVertexAttribIPointer was added in version 3.0 of both desktop and ES.
    if (glVer >= GR_GL_VER(3, 0)) {
        if (NULL == fFunctions.fVertexAttribIPointer) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,1)) {
            if (nullptr == fFunctions.fTexBuffer) {
                RETURN_FALSE_INTERFACE;
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,2) || fExtensions.has("GL_OES_texture_buffer") ||
            fExtensions.has("GL_EXT_texture_buffer")) {
            if (nullptr == fFunctions.fTexBuffer) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3, 0) || fExtensions.has("GL_ARB_vertex_array_object")) {
            if (nullptr == fFunctions.fBindVertexArray ||
                nullptr == fFunctions.fDeleteVertexArrays ||
                nullptr == fFunctions.fGenVertexArrays) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_OES_vertex_array_object")) {
            if (nullptr == fFunctions.fBindVertexArray ||
                nullptr == fFunctions.fDeleteVertexArrays ||
                nullptr == fFunctions.fGenVertexArrays) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if (fExtensions.has("GL_EXT_debug_marker")) {
        if (nullptr == fFunctions.fInsertEventMarker ||
            nullptr == fFunctions.fPushGroupMarker ||
            nullptr == fFunctions.fPopGroupMarker) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,3)) ||
        fExtensions.has("GL_ARB_invalidate_subdata")) {
        if (nullptr == fFunctions.fInvalidateBufferData ||
            nullptr == fFunctions.fInvalidateBufferSubData ||
            nullptr == fFunctions.fInvalidateFramebuffer ||
            nullptr == fFunctions.fInvalidateSubFramebuffer ||
            nullptr == fFunctions.fInvalidateTexImage ||
            nullptr == fFunctions.fInvalidateTexSubImage) {
            RETURN_FALSE_INTERFACE;
        }
    } else if (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,0)) {
        // ES 3.0 adds the framebuffer functions but not the others.
        if (nullptr == fFunctions.fInvalidateFramebuffer ||
            nullptr == fFunctions.fInvalidateSubFramebuffer) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (kGLES_GrGLStandard == fStandard && fExtensions.has("GL_CHROMIUM_map_sub")) {
        if (nullptr == fFunctions.fMapBufferSubData ||
            nullptr == fFunctions.fMapTexSubImage2D ||
            nullptr == fFunctions.fUnmapBufferSubData ||
            nullptr == fFunctions.fUnmapTexSubImage2D) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // These functions are added to the 3.0 version of both GLES and GL.
    if (glVer >= GR_GL_VER(3,0) ||
        (kGLES_GrGLStandard == fStandard && fExtensions.has("GL_EXT_map_buffer_range")) ||
        (kGL_GrGLStandard == fStandard && fExtensions.has("GL_ARB_map_buffer_range"))) {
        if (nullptr == fFunctions.fMapBufferRange ||
            nullptr == fFunctions.fFlushMappedBufferRange) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((kGL_GrGLStandard == fStandard &&
         (glVer >= GR_GL_VER(3,2) || fExtensions.has("GL_ARB_texture_multisample"))) ||
        (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,1))) {
        if (NULL == fFunctions.fGetMultisamplefv) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard &&
         (glVer >= GR_GL_VER(4,3) || fExtensions.has("GL_ARB_program_interface_query"))) ||
        (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,1))) {
        if (nullptr == fFunctions.fGetProgramResourceLocation) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGLES_GrGLStandard == fStandard || glVer >= GR_GL_VER(4,1) ||
        fExtensions.has("GL_ARB_ES2_compatibility")) {
#if 0 // Enable this once Chrome gives us the function ptr
        if (nullptr == fFunctions.fGetShaderPrecisionFormat) {
            RETURN_FALSE_INTERFACE
        }
#endif
    }

    if (fExtensions.has("GL_NV_path_rendering") || fExtensions.has("GL_CHROMIUM_path_rendering")) {
        if (nullptr == fFunctions.fMatrixLoadf ||
            nullptr == fFunctions.fMatrixLoadIdentity ||
            nullptr == fFunctions.fPathCommands ||
            nullptr == fFunctions.fPathParameteri ||
            nullptr == fFunctions.fPathParameterf ||
            nullptr == fFunctions.fGenPaths ||
            nullptr == fFunctions.fDeletePaths ||
            nullptr == fFunctions.fIsPath ||
            nullptr == fFunctions.fPathStencilFunc ||
            nullptr == fFunctions.fStencilFillPath ||
            nullptr == fFunctions.fStencilStrokePath ||
            nullptr == fFunctions.fStencilFillPathInstanced ||
            nullptr == fFunctions.fStencilStrokePathInstanced ||
            nullptr == fFunctions.fCoverFillPath ||
            nullptr == fFunctions.fCoverStrokePath ||
            nullptr == fFunctions.fCoverFillPathInstanced ||
            nullptr == fFunctions.fCoverStrokePathInstanced
#if 0
            // List of functions that Skia uses, but which have been added since the initial release
            // of NV_path_rendering driver. We do not want to fail interface validation due to
            // missing features, we will just not use the extension.
            // Update this list -> update GrGLCaps::hasPathRenderingSupport too.
            || nullptr == fFunctions.fStencilThenCoverFillPath ||
            nullptr == fFunctions.fStencilThenCoverStrokePath ||
            nullptr == fFunctions.fStencilThenCoverFillPathInstanced ||
            nullptr == fFunctions.fStencilThenCoverStrokePathInstanced ||
            nullptr == fFunctions.fProgramPathFragmentInputGen
#endif
            ) {
            RETURN_FALSE_INTERFACE
        }
        if (fExtensions.has("GL_CHROMIUM_path_rendering")) {
            if (nullptr == fFunctions.fBindFragmentInputLocation) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if (fExtensions.has("GL_EXT_raster_multisample")) {
        if (nullptr == fFunctions.fRasterSamples) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_NV_framebuffer_mixed_samples") ||
        fExtensions.has("GL_CHROMIUM_framebuffer_mixed_samples")) {
        if (nullptr == fFunctions.fCoverageModulation) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,1) ||
            fExtensions.has("GL_EXT_draw_instanced") || fExtensions.has("GL_ARB_draw_instanced")) {
            if (nullptr == fFunctions.fDrawArraysInstanced ||
                nullptr == fFunctions.fDrawElementsInstanced) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else if (kGLES_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_draw_instanced")) {
            if (nullptr == fFunctions.fDrawArraysInstanced ||
                nullptr == fFunctions.fDrawElementsInstanced) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,2) || fExtensions.has("GL_ARB_instanced_arrays")) {
            if (nullptr == fFunctions.fVertexAttribDivisor) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else if (kGLES_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_instanced_arrays")) {
            if (nullptr == fFunctions.fVertexAttribDivisor) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,3)) ||
        (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,1))) {
        if (NULL == fFunctions.fDrawArraysIndirect ||
            NULL == fFunctions.fDrawElementsIndirect) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,3)) ||
        (kGLES_GrGLStandard == fStandard && fExtensions.has("GL_EXT_multi_draw_indirect"))) {
        if (NULL == fFunctions.fMultiDrawArraysIndirect ||
            NULL == fFunctions.fMultiDrawElementsIndirect) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_NV_bindless_texture")) {
        if (nullptr == fFunctions.fGetTextureHandle ||
            nullptr == fFunctions.fGetTextureSamplerHandle ||
            nullptr == fFunctions.fMakeTextureHandleResident ||
            nullptr == fFunctions.fMakeTextureHandleNonResident ||
            nullptr == fFunctions.fGetImageHandle ||
            nullptr == fFunctions.fMakeImageHandleResident ||
            nullptr == fFunctions.fMakeImageHandleNonResident ||
            nullptr == fFunctions.fIsTextureHandleResident ||
            nullptr == fFunctions.fIsImageHandleResident ||
            nullptr == fFunctions.fUniformHandleui64 ||
            nullptr == fFunctions.fUniformHandleui64v ||
            nullptr == fFunctions.fProgramUniformHandleui64 ||
            nullptr == fFunctions.fProgramUniformHandleui64v) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGL_GrGLStandard == fStandard && fExtensions.has("GL_EXT_direct_state_access")) {
        if (nullptr == fFunctions.fTextureParameteri ||
            nullptr == fFunctions.fTextureParameteriv ||
            nullptr == fFunctions.fTextureParameterf ||
            nullptr == fFunctions.fTextureParameterfv ||
            nullptr == fFunctions.fTextureImage1D ||
            nullptr == fFunctions.fTextureImage2D ||
            nullptr == fFunctions.fTextureSubImage1D ||
            nullptr == fFunctions.fTextureSubImage2D ||
            nullptr == fFunctions.fCopyTextureImage1D ||
            nullptr == fFunctions.fCopyTextureImage2D ||
            nullptr == fFunctions.fCopyTextureSubImage1D ||
            nullptr == fFunctions.fCopyTextureSubImage2D ||
            nullptr == fFunctions.fGetTextureImage ||
            nullptr == fFunctions.fGetTextureParameterfv ||
            nullptr == fFunctions.fGetTextureParameteriv ||
            nullptr == fFunctions.fGetTextureLevelParameterfv ||
            nullptr == fFunctions.fGetTextureLevelParameteriv) {
            RETURN_FALSE_INTERFACE
        }
        if (glVer >= GR_GL_VER(1,2)) {
            if (nullptr == fFunctions.fTextureImage3D ||
                nullptr == fFunctions.fTextureSubImage3D ||
                nullptr == fFunctions.fCopyTextureSubImage3D ||
                nullptr == fFunctions.fCompressedTextureImage3D ||
                nullptr == fFunctions.fCompressedTextureImage2D ||
                nullptr == fFunctions.fCompressedTextureImage1D ||
                nullptr == fFunctions.fCompressedTextureSubImage3D ||
                nullptr == fFunctions.fCompressedTextureSubImage2D ||
                nullptr == fFunctions.fCompressedTextureSubImage1D ||
                nullptr == fFunctions.fGetCompressedTextureImage) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(1,5)) {
            if (nullptr == fFunctions.fNamedBufferData ||
                nullptr == fFunctions.fNamedBufferSubData ||
                nullptr == fFunctions.fMapNamedBuffer ||
                nullptr == fFunctions.fUnmapNamedBuffer ||
                nullptr == fFunctions.fGetNamedBufferParameteriv ||
                nullptr == fFunctions.fGetNamedBufferPointerv ||
                nullptr == fFunctions.fGetNamedBufferSubData) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(2,0)) {
            if (nullptr == fFunctions.fProgramUniform1f ||
                nullptr == fFunctions.fProgramUniform2f ||
                nullptr == fFunctions.fProgramUniform3f ||
                nullptr == fFunctions.fProgramUniform4f ||
                nullptr == fFunctions.fProgramUniform1i ||
                nullptr == fFunctions.fProgramUniform2i ||
                nullptr == fFunctions.fProgramUniform3i ||
                nullptr == fFunctions.fProgramUniform4i ||
                nullptr == fFunctions.fProgramUniform1fv ||
                nullptr == fFunctions.fProgramUniform2fv ||
                nullptr == fFunctions.fProgramUniform3fv ||
                nullptr == fFunctions.fProgramUniform4fv ||
                nullptr == fFunctions.fProgramUniform1iv ||
                nullptr == fFunctions.fProgramUniform2iv ||
                nullptr == fFunctions.fProgramUniform3iv ||
                nullptr == fFunctions.fProgramUniform4iv ||
                nullptr == fFunctions.fProgramUniformMatrix2fv ||
                nullptr == fFunctions.fProgramUniformMatrix3fv ||
                nullptr == fFunctions.fProgramUniformMatrix4fv) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(2,1)) {
            if (nullptr == fFunctions.fProgramUniformMatrix2x3fv ||
                nullptr == fFunctions.fProgramUniformMatrix3x2fv ||
                nullptr == fFunctions.fProgramUniformMatrix2x4fv ||
                nullptr == fFunctions.fProgramUniformMatrix4x2fv ||
                nullptr == fFunctions.fProgramUniformMatrix3x4fv ||
                nullptr == fFunctions.fProgramUniformMatrix4x3fv) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,0)) {
            if (nullptr == fFunctions.fNamedRenderbufferStorage ||
                nullptr == fFunctions.fGetNamedRenderbufferParameteriv ||
                nullptr == fFunctions.fNamedRenderbufferStorageMultisample ||
                nullptr == fFunctions.fCheckNamedFramebufferStatus ||
                nullptr == fFunctions.fNamedFramebufferTexture1D ||
                nullptr == fFunctions.fNamedFramebufferTexture2D ||
                nullptr == fFunctions.fNamedFramebufferTexture3D ||
                nullptr == fFunctions.fNamedFramebufferRenderbuffer ||
                nullptr == fFunctions.fGetNamedFramebufferAttachmentParameteriv ||
                nullptr == fFunctions.fGenerateTextureMipmap ||
                nullptr == fFunctions.fFramebufferDrawBuffer ||
                nullptr == fFunctions.fFramebufferDrawBuffers ||
                nullptr == fFunctions.fFramebufferReadBuffer ||
                nullptr == fFunctions.fGetFramebufferParameteriv ||
                nullptr == fFunctions.fNamedCopyBufferSubData ||
                nullptr == fFunctions.fVertexArrayVertexOffset ||
                nullptr == fFunctions.fVertexArrayColorOffset ||
                nullptr == fFunctions.fVertexArrayEdgeFlagOffset ||
                nullptr == fFunctions.fVertexArrayIndexOffset ||
                nullptr == fFunctions.fVertexArrayNormalOffset ||
                nullptr == fFunctions.fVertexArrayTexCoordOffset ||
                nullptr == fFunctions.fVertexArrayMultiTexCoordOffset ||
                nullptr == fFunctions.fVertexArrayFogCoordOffset ||
                nullptr == fFunctions.fVertexArraySecondaryColorOffset ||
                nullptr == fFunctions.fVertexArrayVertexAttribOffset ||
                nullptr == fFunctions.fVertexArrayVertexAttribIOffset ||
                nullptr == fFunctions.fEnableVertexArray ||
                nullptr == fFunctions.fDisableVertexArray ||
                nullptr == fFunctions.fEnableVertexArrayAttrib ||
                nullptr == fFunctions.fDisableVertexArrayAttrib ||
                nullptr == fFunctions.fGetVertexArrayIntegerv ||
                nullptr == fFunctions.fGetVertexArrayPointerv ||
                nullptr == fFunctions.fGetVertexArrayIntegeri_v ||
                nullptr == fFunctions.fGetVertexArrayPointeri_v ||
                nullptr == fFunctions.fMapNamedBufferRange ||
                nullptr == fFunctions.fFlushMappedNamedBufferRange) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,1)) {
            if (nullptr == fFunctions.fTextureBuffer) {
                RETURN_FALSE_INTERFACE;
            }
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,3)) ||
        fExtensions.has("GL_KHR_debug")) {
        if (nullptr == fFunctions.fDebugMessageControl ||
            nullptr == fFunctions.fDebugMessageInsert ||
            nullptr == fFunctions.fDebugMessageCallback ||
            nullptr == fFunctions.fGetDebugMessageLog ||
            nullptr == fFunctions.fPushDebugGroup ||
            nullptr == fFunctions.fPopDebugGroup ||
            nullptr == fFunctions.fObjectLabel) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,0)) ||
        fExtensions.has("GL_ARB_sample_shading")) {
        if (nullptr == fFunctions.fMinSampleShading) {
            RETURN_FALSE_INTERFACE
        }
    } else if (kGL_GrGLStandard == fStandard && fExtensions.has("GL_OES_sample_shading")) {
        if (nullptr == fFunctions.fMinSampleShading) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("EGL_KHR_image") || fExtensions.has("EGL_KHR_image_base")) {
        if (nullptr == fFunctions.fEGLCreateImage ||
            nullptr == fFunctions.fEGLDestroyImage) {
            RETURN_FALSE_INTERFACE
        }
    }

    return true;
}

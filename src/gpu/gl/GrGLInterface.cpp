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

#if GR_GL_PER_GL_FUNC_CALLBACK
namespace {
void GrGLDefaultInterfaceCallback(const GrGLInterface*) {}
}
#endif

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
    newInterface->fFunctions.fPathCommands = NULL;
    newInterface->fFunctions.fPathParameteri = NULL;
    newInterface->fFunctions.fPathParameterf = NULL;
    newInterface->fFunctions.fGenPaths = NULL;
    newInterface->fFunctions.fDeletePaths = NULL;
    newInterface->fFunctions.fIsPath = NULL;
    newInterface->fFunctions.fPathStencilFunc = NULL;
    newInterface->fFunctions.fStencilFillPath = NULL;
    newInterface->fFunctions.fStencilStrokePath = NULL;
    newInterface->fFunctions.fStencilFillPathInstanced = NULL;
    newInterface->fFunctions.fStencilStrokePathInstanced = NULL;
    newInterface->fFunctions.fCoverFillPath = NULL;
    newInterface->fFunctions.fCoverStrokePath = NULL;
    newInterface->fFunctions.fCoverFillPathInstanced = NULL;
    newInterface->fFunctions.fCoverStrokePathInstanced = NULL;
    newInterface->fFunctions.fStencilThenCoverFillPath = NULL;
    newInterface->fFunctions.fStencilThenCoverStrokePath = NULL;
    newInterface->fFunctions.fStencilThenCoverFillPathInstanced = NULL;
    newInterface->fFunctions.fStencilThenCoverStrokePathInstanced = NULL;
    newInterface->fFunctions.fProgramPathFragmentInputGen = NULL;
    newInterface->fFunctions.fBindFragmentInputLocation = NULL;
    return newInterface;
}

GrGLInterface::GrGLInterface() {
    fStandard = kNone_GrGLStandard;

#if GR_GL_PER_GL_FUNC_CALLBACK
    fCallback = GrGLDefaultInterfaceCallback;
    fCallbackData = 0;
#endif
}

GrGLInterface* GrGLInterface::NewClone(const GrGLInterface* interface) {
    SkASSERT(interface);

    GrGLInterface* clone = SkNEW(GrGLInterface);
    clone->fStandard = interface->fStandard;
    clone->fExtensions = interface->fExtensions;
    clone->fFunctions = interface->fFunctions;
#if GR_GL_PER_GL_FUNC_CALLBACK
    clone->fCallback = interface->fCallback;
    clone->fCallbackData = interface->fCallbackData;
#endif
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
    if (NULL == fFunctions.fActiveTexture ||
        NULL == fFunctions.fAttachShader ||
        NULL == fFunctions.fBindAttribLocation ||
        NULL == fFunctions.fBindBuffer ||
        NULL == fFunctions.fBindTexture ||
        NULL == fFunctions.fBlendColor ||      // -> GL >= 1.4 or extension, ES >= 2.0
        NULL == fFunctions.fBlendEquation ||   // -> GL >= 1.4 or extension, ES >= 2.0
        NULL == fFunctions.fBlendFunc ||
        NULL == fFunctions.fBufferData ||
        NULL == fFunctions.fBufferSubData ||
        NULL == fFunctions.fClear ||
        NULL == fFunctions.fClearColor ||
        NULL == fFunctions.fClearStencil ||
        NULL == fFunctions.fColorMask ||
        NULL == fFunctions.fCompileShader ||
        NULL == fFunctions.fCopyTexSubImage2D ||
        NULL == fFunctions.fCreateProgram ||
        NULL == fFunctions.fCreateShader ||
        NULL == fFunctions.fCullFace ||
        NULL == fFunctions.fDeleteBuffers ||
        NULL == fFunctions.fDeleteProgram ||
        NULL == fFunctions.fDeleteShader ||
        NULL == fFunctions.fDeleteTextures ||
        NULL == fFunctions.fDepthMask ||
        NULL == fFunctions.fDisable ||
        NULL == fFunctions.fDisableVertexAttribArray ||
        NULL == fFunctions.fDrawArrays ||
        NULL == fFunctions.fDrawElements ||
        NULL == fFunctions.fEnable ||
        NULL == fFunctions.fEnableVertexAttribArray ||
        NULL == fFunctions.fFrontFace ||
        NULL == fFunctions.fGenBuffers ||
        NULL == fFunctions.fGenTextures ||
        NULL == fFunctions.fGetBufferParameteriv ||
        NULL == fFunctions.fGenerateMipmap ||
        NULL == fFunctions.fGetError ||
        NULL == fFunctions.fGetIntegerv ||
        NULL == fFunctions.fGetProgramInfoLog ||
        NULL == fFunctions.fGetProgramiv ||
        NULL == fFunctions.fGetShaderInfoLog ||
        NULL == fFunctions.fGetShaderiv ||
        NULL == fFunctions.fGetString ||
        NULL == fFunctions.fGetUniformLocation ||
#if 0 //  Not included in Chrome yet
        NULL == fFunctions.fIsTexture ||
#endif
        NULL == fFunctions.fLinkProgram ||
        NULL == fFunctions.fLineWidth ||
        NULL == fFunctions.fPixelStorei ||
        NULL == fFunctions.fReadPixels ||
        NULL == fFunctions.fScissor ||
        NULL == fFunctions.fShaderSource ||
        NULL == fFunctions.fStencilFunc ||
        NULL == fFunctions.fStencilMask ||
        NULL == fFunctions.fStencilOp ||
        NULL == fFunctions.fTexImage2D ||
        NULL == fFunctions.fTexParameteri ||
        NULL == fFunctions.fTexParameteriv ||
        NULL == fFunctions.fTexSubImage2D ||
        NULL == fFunctions.fUniform1f ||
        NULL == fFunctions.fUniform1i ||
        NULL == fFunctions.fUniform1fv ||
        NULL == fFunctions.fUniform1iv ||
        NULL == fFunctions.fUniform2f ||
        NULL == fFunctions.fUniform2i ||
        NULL == fFunctions.fUniform2fv ||
        NULL == fFunctions.fUniform2iv ||
        NULL == fFunctions.fUniform3f ||
        NULL == fFunctions.fUniform3i ||
        NULL == fFunctions.fUniform3fv ||
        NULL == fFunctions.fUniform3iv ||
        NULL == fFunctions.fUniform4f ||
        NULL == fFunctions.fUniform4i ||
        NULL == fFunctions.fUniform4fv ||
        NULL == fFunctions.fUniform4iv ||
        NULL == fFunctions.fUniformMatrix2fv ||
        NULL == fFunctions.fUniformMatrix3fv ||
        NULL == fFunctions.fUniformMatrix4fv ||
        NULL == fFunctions.fUseProgram ||
        NULL == fFunctions.fVertexAttrib1f ||
        NULL == fFunctions.fVertexAttrib2fv ||
        NULL == fFunctions.fVertexAttrib3fv ||
        NULL == fFunctions.fVertexAttrib4fv ||
        NULL == fFunctions.fVertexAttribPointer ||
        NULL == fFunctions.fViewport ||
        NULL == fFunctions.fBindFramebuffer ||
        NULL == fFunctions.fBindRenderbuffer ||
        NULL == fFunctions.fCheckFramebufferStatus ||
        NULL == fFunctions.fDeleteFramebuffers ||
        NULL == fFunctions.fDeleteRenderbuffers ||
        NULL == fFunctions.fFinish ||
        NULL == fFunctions.fFlush ||
        NULL == fFunctions.fFramebufferRenderbuffer ||
        NULL == fFunctions.fFramebufferTexture2D ||
        NULL == fFunctions.fGetFramebufferAttachmentParameteriv ||
        NULL == fFunctions.fGetRenderbufferParameteriv ||
        NULL == fFunctions.fGenFramebuffers ||
        NULL == fFunctions.fGenRenderbuffers ||
        NULL == fFunctions.fRenderbufferStorage) {
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
        if (NULL == fFunctions.fStencilFuncSeparate ||
            NULL == fFunctions.fStencilMaskSeparate ||
            NULL == fFunctions.fStencilOpSeparate) {
            RETURN_FALSE_INTERFACE
        }
    } else if (kGL_GrGLStandard == fStandard) {

        if (glVer >= GR_GL_VER(2,0)) {
            if (NULL == fFunctions.fStencilFuncSeparate ||
                NULL == fFunctions.fStencilMaskSeparate ||
                NULL == fFunctions.fStencilOpSeparate) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,0) && NULL == fFunctions.fBindFragDataLocation) {
            RETURN_FALSE_INTERFACE
        }
        if (glVer >= GR_GL_VER(2,0) || fExtensions.has("GL_ARB_draw_buffers")) {
            if (NULL == fFunctions.fDrawBuffers) {
                RETURN_FALSE_INTERFACE
            }
        }

        if (glVer >= GR_GL_VER(1,5) || fExtensions.has("GL_ARB_occlusion_query")) {
            if (NULL == fFunctions.fGenQueries ||
                NULL == fFunctions.fDeleteQueries ||
                NULL == fFunctions.fBeginQuery ||
                NULL == fFunctions.fEndQuery ||
                NULL == fFunctions.fGetQueryiv ||
                NULL == fFunctions.fGetQueryObjectiv ||
                NULL == fFunctions.fGetQueryObjectuiv) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,3) ||
            fExtensions.has("GL_ARB_timer_query") ||
            fExtensions.has("GL_EXT_timer_query")) {
            if (NULL == fFunctions.fGetQueryObjecti64v ||
                NULL == fFunctions.fGetQueryObjectui64v) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_timer_query")) {
            if (NULL == fFunctions.fQueryCounter) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    // optional function on desktop before 1.3
    if (kGL_GrGLStandard != fStandard ||
        (glVer >= GR_GL_VER(1,3)) ||
        fExtensions.has("GL_ARB_texture_compression")) {
        if (NULL == fFunctions.fCompressedTexImage2D
#if 0
            || NULL == fFunctions.fCompressedTexSubImage2D
#endif
            ) {
            RETURN_FALSE_INTERFACE
        }
    }

    // part of desktop GL, but not ES
    if (kGL_GrGLStandard == fStandard &&
        (NULL == fFunctions.fGetTexLevelParameteriv ||
         NULL == fFunctions.fDrawBuffer ||
         NULL == fFunctions.fReadBuffer)) {
        RETURN_FALSE_INTERFACE
    }

    // GL_EXT_texture_storage is part of desktop 4.2
    // There is a desktop ARB extension and an ES+desktop EXT extension
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(4,2) ||
            fExtensions.has("GL_ARB_texture_storage") ||
            fExtensions.has("GL_EXT_texture_storage")) {
            if (NULL == fFunctions.fTexStorage2D) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_texture_storage")) {
        if (NULL == fFunctions.fTexStorage2D) {
            RETURN_FALSE_INTERFACE
        }
    }

    // glTextureBarrier is part of desktop 4.5. There are also ARB and NV extensions.
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(4,5) ||
            fExtensions.has("GL_ARB_texture_barrier") ||
            fExtensions.has("GL_NV_texture_barrier")) {
            if (NULL == fFunctions.fTextureBarrier) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else if (fExtensions.has("GL_NV_texture_barrier")) {
        if (NULL == fFunctions.fTextureBarrier) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_KHR_blend_equation_advanced") ||
        fExtensions.has("GL_NV_blend_equation_advanced")) {
        if (NULL == fFunctions.fBlendBarrier) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_EXT_discard_framebuffer")) {
// FIXME: Remove this once Chromium is updated to provide this function
#if 0
        if (NULL == fFunctions.fDiscardFramebuffer) {
            RETURN_FALSE_INTERFACE
        }
#endif
    }

    // FBO MSAA
    if (kGL_GrGLStandard == fStandard) {
        // GL 3.0 and the ARB extension have multisample + blit
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_ARB_framebuffer_object")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisample ||
                NULL == fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
        } else {
            if (fExtensions.has("GL_EXT_framebuffer_blit") &&
                NULL == fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
            if (fExtensions.has("GL_EXT_framebuffer_multisample") &&
                NULL == fFunctions.fRenderbufferStorageMultisample) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_CHROMIUM_framebuffer_multisample")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisample ||
                NULL == fFunctions.fBlitFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (fExtensions.has("GL_APPLE_framebuffer_multisample")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisampleES2APPLE ||
                NULL == fFunctions.fResolveMultisampleFramebuffer) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (fExtensions.has("GL_IMG_multisampled_render_to_texture") ||
            fExtensions.has("GL_EXT_multisampled_render_to_texture")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisampleES2EXT ||
                NULL == fFunctions.fFramebufferTexture2DMultisample) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (kGL_GrGLStandard == fStandard || fExtensions.has("GL_OES_mapbuffer")) {
        if (NULL == fFunctions.fMapBuffer ||
            NULL == fFunctions.fUnmapBuffer) {
            RETURN_FALSE_INTERFACE
        }
    }

    // Dual source blending
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_blend_func_extended")) {
            if (NULL == fFunctions.fBindFragDataLocationIndexed) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) && fExtensions.has("GL_EXT_blend_func_extended")) {
            if (NULL == fFunctions.fBindFragDataLocation ||
                NULL == fFunctions.fBindFragDataLocationIndexed) {
                RETURN_FALSE_INTERFACE
            }
        }
    }


    // glGetStringi was added in version 3.0 of both desktop and ES.
    if (glVer >= GR_GL_VER(3, 0)) {
        if (NULL == fFunctions.fGetStringi) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3, 0) || fExtensions.has("GL_ARB_vertex_array_object")) {
            if (NULL == fFunctions.fBindVertexArray ||
                NULL == fFunctions.fDeleteVertexArrays ||
                NULL == fFunctions.fGenVertexArrays) {
                RETURN_FALSE_INTERFACE
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_OES_vertex_array_object")) {
            if (NULL == fFunctions.fBindVertexArray ||
                NULL == fFunctions.fDeleteVertexArrays ||
                NULL == fFunctions.fGenVertexArrays) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if (fExtensions.has("GL_EXT_debug_marker")) {
        if (NULL == fFunctions.fInsertEventMarker ||
            NULL == fFunctions.fPushGroupMarker ||
            NULL == fFunctions.fPopGroupMarker) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,3)) ||
        fExtensions.has("GL_ARB_invalidate_subdata")) {
        if (NULL == fFunctions.fInvalidateBufferData ||
            NULL == fFunctions.fInvalidateBufferSubData ||
            NULL == fFunctions.fInvalidateFramebuffer ||
            NULL == fFunctions.fInvalidateSubFramebuffer ||
            NULL == fFunctions.fInvalidateTexImage ||
            NULL == fFunctions.fInvalidateTexSubImage) {
            RETURN_FALSE_INTERFACE;
        }
    } else if (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,0)) {
        // ES 3.0 adds the framebuffer functions but not the others.
        if (NULL == fFunctions.fInvalidateFramebuffer ||
            NULL == fFunctions.fInvalidateSubFramebuffer) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if (kGLES_GrGLStandard == fStandard && fExtensions.has("GL_CHROMIUM_map_sub")) {
        if (NULL == fFunctions.fMapBufferSubData ||
            NULL == fFunctions.fMapTexSubImage2D ||
            NULL == fFunctions.fUnmapBufferSubData ||
            NULL == fFunctions.fUnmapTexSubImage2D) {
            RETURN_FALSE_INTERFACE;
        }
    }

    // These functions are added to the 3.0 version of both GLES and GL.
    if (glVer >= GR_GL_VER(3,0) ||
        (kGLES_GrGLStandard == fStandard && fExtensions.has("GL_EXT_map_buffer_range")) ||
        (kGL_GrGLStandard == fStandard && fExtensions.has("GL_ARB_map_buffer_range"))) {
        if (NULL == fFunctions.fMapBufferRange ||
            NULL == fFunctions.fFlushMappedBufferRange) {
            RETURN_FALSE_INTERFACE;
        }
    }

    if ((kGL_GrGLStandard == fStandard &&
         (glVer >= GR_GL_VER(4,3) || fExtensions.has("GL_ARB_program_interface_query"))) ||
        (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,1))) {
        if (NULL == fFunctions.fGetProgramResourceLocation) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGLES_GrGLStandard == fStandard || glVer >= GR_GL_VER(4,1) ||
        fExtensions.has("GL_ARB_ES2_compatibility")) {
#if 0 // Enable this once Chrome gives us the function ptr
        if (NULL == fFunctions.fGetShaderPrecisionFormat) {
            RETURN_FALSE_INTERFACE
        }
#endif
    }

    if (fExtensions.has("GL_NV_path_rendering") || fExtensions.has("GL_CHROMIUM_path_rendering")) {
        if (NULL == fFunctions.fMatrixLoadf ||
            NULL == fFunctions.fMatrixLoadIdentity ||
            NULL == fFunctions.fPathCommands ||
            NULL == fFunctions.fPathParameteri ||
            NULL == fFunctions.fPathParameterf ||
            NULL == fFunctions.fGenPaths ||
            NULL == fFunctions.fDeletePaths ||
            NULL == fFunctions.fIsPath ||
            NULL == fFunctions.fPathStencilFunc ||
            NULL == fFunctions.fStencilFillPath ||
            NULL == fFunctions.fStencilStrokePath ||
            NULL == fFunctions.fStencilFillPathInstanced ||
            NULL == fFunctions.fStencilStrokePathInstanced ||
            NULL == fFunctions.fCoverFillPath ||
            NULL == fFunctions.fCoverStrokePath ||
            NULL == fFunctions.fCoverFillPathInstanced ||
            NULL == fFunctions.fCoverStrokePathInstanced
#if 0
            // List of functions that Skia uses, but which have been added since the initial release
            // of NV_path_rendering driver. We do not want to fail interface validation due to
            // missing features, we will just not use the extension.
            // Update this list -> update GrGLCaps::hasPathRenderingSupport too.
            || NULL == fFunctions.fStencilThenCoverFillPath ||
            NULL == fFunctions.fStencilThenCoverStrokePath ||
            NULL == fFunctions.fStencilThenCoverFillPathInstanced ||
            NULL == fFunctions.fStencilThenCoverStrokePathInstanced ||
            NULL == fFunctions.fProgramPathFragmentInputGen
#endif
            ) {
            RETURN_FALSE_INTERFACE
        }
        if (fExtensions.has("GL_CHROMIUM_path_rendering")) {
            if (NULL == fFunctions.fBindFragmentInputLocation) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if (fExtensions.has("GL_EXT_raster_multisample")) {
        if (NULL == fFunctions.fRasterSamples) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_NV_framebuffer_mixed_samples")) {
        if (NULL == fFunctions.fCoverageModulation) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,1)) ||
        (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,0)) ||
        fExtensions.has("GL_ARB_draw_instanced") ||
        fExtensions.has("GL_EXT_draw_instanced")) {
        if (NULL == fFunctions.fDrawArraysInstanced ||
            NULL == fFunctions.fDrawElementsInstanced) {
            RETURN_FALSE_INTERFACE
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,2)) ||
        (kGLES_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,0)) ||
        fExtensions.has("GL_ARB_instanced_arrays") ||
        fExtensions.has("GL_EXT_instanced_arrays")) {
        if (NULL == fFunctions.fVertexAttribDivisor) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (fExtensions.has("GL_NV_bindless_texture")) {
        if (NULL == fFunctions.fGetTextureHandle ||
            NULL == fFunctions.fGetTextureSamplerHandle ||
            NULL == fFunctions.fMakeTextureHandleResident ||
            NULL == fFunctions.fMakeTextureHandleNonResident ||
            NULL == fFunctions.fGetImageHandle ||
            NULL == fFunctions.fMakeImageHandleResident ||
            NULL == fFunctions.fMakeImageHandleNonResident ||
            NULL == fFunctions.fIsTextureHandleResident ||
            NULL == fFunctions.fIsImageHandleResident ||
            NULL == fFunctions.fUniformHandleui64 ||
            NULL == fFunctions.fUniformHandleui64v ||
            NULL == fFunctions.fProgramUniformHandleui64 ||
            NULL == fFunctions.fProgramUniformHandleui64v) {
            RETURN_FALSE_INTERFACE
        }
    }

    if (kGL_GrGLStandard == fStandard && fExtensions.has("GL_EXT_direct_state_access")) {
        if (NULL == fFunctions.fTextureParameteri ||
            NULL == fFunctions.fTextureParameteriv ||
            NULL == fFunctions.fTextureParameterf ||
            NULL == fFunctions.fTextureParameterfv ||
            NULL == fFunctions.fTextureImage1D ||
            NULL == fFunctions.fTextureImage2D ||
            NULL == fFunctions.fTextureSubImage1D ||
            NULL == fFunctions.fTextureSubImage2D ||
            NULL == fFunctions.fCopyTextureImage1D ||
            NULL == fFunctions.fCopyTextureImage2D ||
            NULL == fFunctions.fCopyTextureSubImage1D ||
            NULL == fFunctions.fCopyTextureSubImage2D ||
            NULL == fFunctions.fGetTextureImage ||
            NULL == fFunctions.fGetTextureParameterfv ||
            NULL == fFunctions.fGetTextureParameteriv ||
            NULL == fFunctions.fGetTextureLevelParameterfv ||
            NULL == fFunctions.fGetTextureLevelParameteriv) {
            RETURN_FALSE_INTERFACE
        }
        if (glVer >= GR_GL_VER(1,2)) {
            if (NULL == fFunctions.fTextureImage3D ||
                NULL == fFunctions.fTextureSubImage3D ||
                NULL == fFunctions.fCopyTextureSubImage3D ||
                NULL == fFunctions.fCompressedTextureImage3D ||
                NULL == fFunctions.fCompressedTextureImage2D ||
                NULL == fFunctions.fCompressedTextureImage1D ||
                NULL == fFunctions.fCompressedTextureSubImage3D ||
                NULL == fFunctions.fCompressedTextureSubImage2D ||
                NULL == fFunctions.fCompressedTextureSubImage1D ||
                NULL == fFunctions.fGetCompressedTextureImage) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(1,5)) {
            if (NULL == fFunctions.fNamedBufferData ||
                NULL == fFunctions.fNamedBufferSubData ||
                NULL == fFunctions.fMapNamedBuffer ||
                NULL == fFunctions.fUnmapNamedBuffer ||
                NULL == fFunctions.fGetNamedBufferParameteriv ||
                NULL == fFunctions.fGetNamedBufferPointerv ||
                NULL == fFunctions.fGetNamedBufferSubData) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(2,0)) {
            if (NULL == fFunctions.fProgramUniform1f ||
                NULL == fFunctions.fProgramUniform2f ||
                NULL == fFunctions.fProgramUniform3f ||
                NULL == fFunctions.fProgramUniform4f ||
                NULL == fFunctions.fProgramUniform1i ||
                NULL == fFunctions.fProgramUniform2i ||
                NULL == fFunctions.fProgramUniform3i ||
                NULL == fFunctions.fProgramUniform4i ||
                NULL == fFunctions.fProgramUniform1fv ||
                NULL == fFunctions.fProgramUniform2fv ||
                NULL == fFunctions.fProgramUniform3fv ||
                NULL == fFunctions.fProgramUniform4fv ||
                NULL == fFunctions.fProgramUniform1iv ||
                NULL == fFunctions.fProgramUniform2iv ||
                NULL == fFunctions.fProgramUniform3iv ||
                NULL == fFunctions.fProgramUniform4iv ||
                NULL == fFunctions.fProgramUniformMatrix2fv ||
                NULL == fFunctions.fProgramUniformMatrix3fv ||
                NULL == fFunctions.fProgramUniformMatrix4fv) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(2,1)) {
            if (NULL == fFunctions.fProgramUniformMatrix2x3fv ||
                NULL == fFunctions.fProgramUniformMatrix3x2fv ||
                NULL == fFunctions.fProgramUniformMatrix2x4fv ||
                NULL == fFunctions.fProgramUniformMatrix4x2fv ||
                NULL == fFunctions.fProgramUniformMatrix3x4fv ||
                NULL == fFunctions.fProgramUniformMatrix4x3fv) {
                RETURN_FALSE_INTERFACE
            }
        }
        if (glVer >= GR_GL_VER(3,0)) {
            if (NULL == fFunctions.fNamedRenderbufferStorage ||
                NULL == fFunctions.fGetNamedRenderbufferParameteriv ||
                NULL == fFunctions.fNamedRenderbufferStorageMultisample ||
                NULL == fFunctions.fCheckNamedFramebufferStatus ||
                NULL == fFunctions.fNamedFramebufferTexture1D ||
                NULL == fFunctions.fNamedFramebufferTexture2D ||
                NULL == fFunctions.fNamedFramebufferTexture3D ||
                NULL == fFunctions.fNamedFramebufferRenderbuffer ||
                NULL == fFunctions.fGetNamedFramebufferAttachmentParameteriv ||
                NULL == fFunctions.fGenerateTextureMipmap ||
                NULL == fFunctions.fFramebufferDrawBuffer ||
                NULL == fFunctions.fFramebufferDrawBuffers ||
                NULL == fFunctions.fFramebufferReadBuffer ||
                NULL == fFunctions.fGetFramebufferParameteriv ||
                NULL == fFunctions.fNamedCopyBufferSubData ||
                NULL == fFunctions.fVertexArrayVertexOffset ||
                NULL == fFunctions.fVertexArrayColorOffset ||
                NULL == fFunctions.fVertexArrayEdgeFlagOffset ||
                NULL == fFunctions.fVertexArrayIndexOffset ||
                NULL == fFunctions.fVertexArrayNormalOffset ||
                NULL == fFunctions.fVertexArrayTexCoordOffset ||
                NULL == fFunctions.fVertexArrayMultiTexCoordOffset ||
                NULL == fFunctions.fVertexArrayFogCoordOffset ||
                NULL == fFunctions.fVertexArraySecondaryColorOffset ||
                NULL == fFunctions.fVertexArrayVertexAttribOffset ||
                NULL == fFunctions.fVertexArrayVertexAttribIOffset ||
                NULL == fFunctions.fEnableVertexArray ||
                NULL == fFunctions.fDisableVertexArray ||
                NULL == fFunctions.fEnableVertexArrayAttrib ||
                NULL == fFunctions.fDisableVertexArrayAttrib ||
                NULL == fFunctions.fGetVertexArrayIntegerv ||
                NULL == fFunctions.fGetVertexArrayPointerv ||
                NULL == fFunctions.fGetVertexArrayIntegeri_v ||
                NULL == fFunctions.fGetVertexArrayPointeri_v ||
                NULL == fFunctions.fMapNamedBufferRange ||
                NULL == fFunctions.fFlushMappedNamedBufferRange) {
                RETURN_FALSE_INTERFACE
            }
        }
    }

    if ((kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(4,3)) ||
        fExtensions.has("GL_KHR_debug")) {
        if (NULL == fFunctions.fDebugMessageControl ||
            NULL == fFunctions.fDebugMessageInsert ||
            NULL == fFunctions.fDebugMessageCallback ||
            NULL == fFunctions.fGetDebugMessageLog ||
            NULL == fFunctions.fPushDebugGroup ||
            NULL == fFunctions.fPopDebugGroup ||
            NULL == fFunctions.fObjectLabel) {
            RETURN_FALSE_INTERFACE
        }
    }

    return true;
}

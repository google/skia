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
    newInterface->fFunctions.fPathCoords = NULL;
    newInterface->fFunctions.fPathSubCommands = NULL;
    newInterface->fFunctions.fPathSubCoords = NULL;
    newInterface->fFunctions.fPathString = NULL;
    newInterface->fFunctions.fPathGlyphs = NULL;
    newInterface->fFunctions.fPathGlyphRange = NULL;
    newInterface->fFunctions.fWeightPaths = NULL;
    newInterface->fFunctions.fCopyPath = NULL;
    newInterface->fFunctions.fInterpolatePaths = NULL;
    newInterface->fFunctions.fTransformPath = NULL;
    newInterface->fFunctions.fPathParameteriv = NULL;
    newInterface->fFunctions.fPathParameteri = NULL;
    newInterface->fFunctions.fPathParameterfv = NULL;
    newInterface->fFunctions.fPathParameterf = NULL;
    newInterface->fFunctions.fPathDashArray = NULL;
    newInterface->fFunctions.fGenPaths = NULL;
    newInterface->fFunctions.fDeletePaths = NULL;
    newInterface->fFunctions.fIsPath = NULL;
    newInterface->fFunctions.fPathStencilFunc = NULL;
    newInterface->fFunctions.fPathStencilDepthOffset = NULL;
    newInterface->fFunctions.fStencilFillPath = NULL;
    newInterface->fFunctions.fStencilStrokePath = NULL;
    newInterface->fFunctions.fStencilFillPathInstanced = NULL;
    newInterface->fFunctions.fStencilStrokePathInstanced = NULL;
    newInterface->fFunctions.fPathCoverDepthFunc = NULL;
    newInterface->fFunctions.fPathColorGen = NULL;
    newInterface->fFunctions.fPathTexGen = NULL;
    newInterface->fFunctions.fPathFogGen = NULL;
    newInterface->fFunctions.fCoverFillPath = NULL;
    newInterface->fFunctions.fCoverStrokePath = NULL;
    newInterface->fFunctions.fCoverFillPathInstanced = NULL;
    newInterface->fFunctions.fCoverStrokePathInstanced = NULL;
    newInterface->fFunctions.fGetPathParameteriv = NULL;
    newInterface->fFunctions.fGetPathParameterfv = NULL;
    newInterface->fFunctions.fGetPathCommands = NULL;
    newInterface->fFunctions.fGetPathCoords = NULL;
    newInterface->fFunctions.fGetPathDashArray = NULL;
    newInterface->fFunctions.fGetPathMetrics = NULL;
    newInterface->fFunctions.fGetPathMetricRange = NULL;
    newInterface->fFunctions.fGetPathSpacing = NULL;
    newInterface->fFunctions.fGetPathColorGeniv = NULL;
    newInterface->fFunctions.fGetPathColorGenfv = NULL;
    newInterface->fFunctions.fGetPathTexGeniv = NULL;
    newInterface->fFunctions.fGetPathTexGenfv = NULL;
    newInterface->fFunctions.fIsPointInFillPath = NULL;
    newInterface->fFunctions.fIsPointInStrokePath = NULL;
    newInterface->fFunctions.fGetPathLength = NULL;
    newInterface->fFunctions.fPointAlongPath = NULL;

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
    SkASSERT(NULL != interface);

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

bool GrGLInterface::validate() const {

    if (kNone_GrGLStandard == fStandard) {
        return false;
    }

    if (!fExtensions.isInitialized()) {
        return false;
    }

    // functions that are always required
    if (NULL == fFunctions.fActiveTexture ||
        NULL == fFunctions.fAttachShader ||
        NULL == fFunctions.fBindAttribLocation ||
        NULL == fFunctions.fBindBuffer ||
        NULL == fFunctions.fBindTexture ||
        NULL == fFunctions.fBlendFunc ||
        NULL == fFunctions.fBlendColor ||      // -> GL >= 1.4, ES >= 2.0 or extension
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
        return false;
    }

    GrGLVersion glVer = GrGLGetVersion(this);

    bool isCoreProfile = false;
    if (kGL_GrGLStandard == fStandard && glVer >= GR_GL_VER(3,2)) {
        GrGLint profileMask;
        GR_GL_GetIntegerv(this, GR_GL_CONTEXT_PROFILE_MASK, &profileMask);
        isCoreProfile = SkToBool(profileMask & GR_GL_CONTEXT_CORE_PROFILE_BIT);
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
            return false;
        }
    } else if (kGL_GrGLStandard == fStandard) {

        if (glVer >= GR_GL_VER(2,0)) {
            if (NULL == fFunctions.fStencilFuncSeparate ||
                NULL == fFunctions.fStencilMaskSeparate ||
                NULL == fFunctions.fStencilOpSeparate) {
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,0) && NULL == fFunctions.fBindFragDataLocation) {
            return false;
        }
        if (glVer >= GR_GL_VER(2,0) || fExtensions.has("GL_ARB_draw_buffers")) {
            if (NULL == fFunctions.fDrawBuffers) {
                return false;
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
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,3) ||
            fExtensions.has("GL_ARB_timer_query") ||
            fExtensions.has("GL_EXT_timer_query")) {
            if (NULL == fFunctions.fGetQueryObjecti64v ||
                NULL == fFunctions.fGetQueryObjectui64v) {
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_timer_query")) {
            if (NULL == fFunctions.fQueryCounter) {
                return false;
            }
        }
        if (!isCoreProfile) {
            if (NULL == fFunctions.fLoadIdentity ||
                NULL == fFunctions.fLoadMatrixf ||
                NULL == fFunctions.fMatrixMode ||
                NULL == fFunctions.fTexGenfv ||
                NULL == fFunctions.fTexGeni) {
                return false;
            }
        }
        if (fExtensions.has("GL_NV_path_rendering")) {
            if (NULL == fFunctions.fPathCommands ||
                NULL == fFunctions.fPathCoords ||
                NULL == fFunctions.fPathSubCommands ||
                NULL == fFunctions.fPathSubCoords ||
                NULL == fFunctions.fPathString ||
                NULL == fFunctions.fPathGlyphs ||
                NULL == fFunctions.fPathGlyphRange ||
                NULL == fFunctions.fWeightPaths ||
                NULL == fFunctions.fCopyPath ||
                NULL == fFunctions.fInterpolatePaths ||
                NULL == fFunctions.fTransformPath ||
                NULL == fFunctions.fPathParameteriv ||
                NULL == fFunctions.fPathParameteri ||
                NULL == fFunctions.fPathParameterfv ||
                NULL == fFunctions.fPathParameterf ||
                NULL == fFunctions.fPathDashArray ||
                NULL == fFunctions.fGenPaths ||
                NULL == fFunctions.fDeletePaths ||
                NULL == fFunctions.fIsPath ||
                NULL == fFunctions.fPathStencilFunc ||
                NULL == fFunctions.fPathStencilDepthOffset ||
                NULL == fFunctions.fStencilFillPath ||
                NULL == fFunctions.fStencilStrokePath ||
                NULL == fFunctions.fStencilFillPathInstanced ||
                NULL == fFunctions.fStencilStrokePathInstanced ||
                NULL == fFunctions.fPathCoverDepthFunc ||
                NULL == fFunctions.fPathColorGen ||
                NULL == fFunctions.fPathTexGen ||
                NULL == fFunctions.fPathFogGen ||
                NULL == fFunctions.fCoverFillPath ||
                NULL == fFunctions.fCoverStrokePath ||
                NULL == fFunctions.fCoverFillPathInstanced ||
                NULL == fFunctions.fCoverStrokePathInstanced ||
                NULL == fFunctions.fGetPathParameteriv ||
                NULL == fFunctions.fGetPathParameterfv ||
                NULL == fFunctions.fGetPathCommands ||
                NULL == fFunctions.fGetPathCoords ||
                NULL == fFunctions.fGetPathDashArray ||
                NULL == fFunctions.fGetPathMetrics ||
                NULL == fFunctions.fGetPathMetricRange ||
                NULL == fFunctions.fGetPathSpacing ||
                NULL == fFunctions.fGetPathColorGeniv ||
                NULL == fFunctions.fGetPathColorGenfv ||
                NULL == fFunctions.fGetPathTexGeniv ||
                NULL == fFunctions.fGetPathTexGenfv ||
                NULL == fFunctions.fIsPointInFillPath ||
                NULL == fFunctions.fIsPointInStrokePath ||
                NULL == fFunctions.fGetPathLength ||
                NULL == fFunctions.fPointAlongPath) {
                return false;
            }
        }
    }

    // optional function on desktop before 1.3
    if (kGL_GrGLStandard != fStandard ||
        (glVer >= GR_GL_VER(1,3)) ||
        fExtensions.has("GL_ARB_texture_compression")) {
        if (NULL == fFunctions.fCompressedTexImage2D) {
            return false;
        }
    }

    // part of desktop GL, but not ES
    if (kGL_GrGLStandard == fStandard &&
        (NULL == fFunctions.fGetTexLevelParameteriv ||
         NULL == fFunctions.fDrawBuffer ||
         NULL == fFunctions.fReadBuffer)) {
        return false;
    }

    // GL_EXT_texture_storage is part of desktop 4.2
    // There is a desktop ARB extension and an ES+desktop EXT extension
    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(4,2) ||
            fExtensions.has("GL_ARB_texture_storage") ||
            fExtensions.has("GL_EXT_texture_storage")) {
            if (NULL == fFunctions.fTexStorage2D) {
                return false;
            }
        }
    } else if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_EXT_texture_storage")) {
        if (NULL == fFunctions.fTexStorage2D) {
            return false;
        }
    }

    if (fExtensions.has("GL_EXT_discard_framebuffer")) {
// FIXME: Remove this once Chromium is updated to provide this function
#if 0
        if (NULL == fFunctions.fDiscardFramebuffer) {
            return false;
        }
#endif
    }

    // FBO MSAA
    if (kGL_GrGLStandard == fStandard) {
        // GL 3.0 and the ARB extension have multisample + blit
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_ARB_framebuffer_object")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisample ||
                NULL == fFunctions.fBlitFramebuffer) {
                return false;
            }
        } else {
            if (fExtensions.has("GL_EXT_framebuffer_blit") &&
                NULL == fFunctions.fBlitFramebuffer) {
                return false;
            }
            if (fExtensions.has("GL_EXT_framebuffer_multisample") &&
                NULL == fFunctions.fRenderbufferStorageMultisample) {
                return false;
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_CHROMIUM_framebuffer_multisample")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisample ||
                NULL == fFunctions.fBlitFramebuffer) {
                return false;
            }
        }
        if (fExtensions.has("GL_APPLE_framebuffer_multisample")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisampleES2APPLE ||
                NULL == fFunctions.fResolveMultisampleFramebuffer) {
                return false;
            }
        }
        if (fExtensions.has("GL_IMG_multisampled_render_to_texture") ||
            fExtensions.has("GL_EXT_multisampled_render_to_texture")) {
            if (NULL == fFunctions.fRenderbufferStorageMultisampleES2EXT ||
                NULL == fFunctions.fFramebufferTexture2DMultisample) {
                return false;
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (kGL_GrGLStandard == fStandard || fExtensions.has("GL_OES_mapbuffer")) {
        if (NULL == fFunctions.fMapBuffer ||
            NULL == fFunctions.fUnmapBuffer) {
            return false;
        }
    }

    // Dual source blending
    if (kGL_GrGLStandard == fStandard &&
        (glVer >= GR_GL_VER(3,3) || fExtensions.has("GL_ARB_blend_func_extended"))) {
        if (NULL == fFunctions.fBindFragDataLocationIndexed) {
            return false;
        }
    }

    // glGetStringi was added in version 3.0 of both desktop and ES.
    if (glVer >= GR_GL_VER(3, 0)) {
        if (NULL == fFunctions.fGetStringi) {
            return false;
        }
    }

    if (kGL_GrGLStandard == fStandard) {
        if (glVer >= GR_GL_VER(3, 0) || fExtensions.has("GL_ARB_vertex_array_object")) {
            if (NULL == fFunctions.fBindVertexArray ||
                NULL == fFunctions.fDeleteVertexArrays ||
                NULL == fFunctions.fGenVertexArrays) {
                return false;
            }
        }
    } else {
        if (glVer >= GR_GL_VER(3,0) || fExtensions.has("GL_OES_vertex_array_object")) {
            if (NULL == fFunctions.fBindVertexArray ||
                NULL == fFunctions.fDeleteVertexArrays ||
                NULL == fFunctions.fGenVertexArrays) {
                return false;
            }
        }
    }

#if 0
    if (fExtensions.has("GL_EXT_debug_marker")) {
        if (NULL == fFunctions.fInsertEventMarker ||
            NULL == fFunctions.fPushGroupMarker ||
            NULL == fFunctions.fPopGroupMarker) {
            return false;
        }
    }
#endif
    return true;
}

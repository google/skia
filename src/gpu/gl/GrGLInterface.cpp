/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "GrGLUtil.h"

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
        SkDebugf("%d: %i\n", __LINE__, (binding & fBindingsExported));
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
        SkDebugf("%d: %i\n", __LINE__, fActiveTexture );
        SkDebugf("%d: %i\n", __LINE__, fAttachShader );
        SkDebugf("%d: %i\n", __LINE__, fBindAttribLocation );
        SkDebugf("%d: %i\n", __LINE__, fBindBuffer );
        SkDebugf("%d: %i\n", __LINE__, fBindTexture );
        SkDebugf("%d: %i\n", __LINE__, fBlendFunc );
        SkDebugf("%d: %i\n", __LINE__, fBlendColor );      // -> GL >= 1.4, ES >= 2.0 or extension
        SkDebugf("%d: %i\n", __LINE__, fBufferData );
        SkDebugf("%d: %i\n", __LINE__, fBufferSubData );
        SkDebugf("%d: %i\n", __LINE__, fClear );
        SkDebugf("%d: %i\n", __LINE__, fClearColor );
        SkDebugf("%d: %i\n", __LINE__, fClearStencil );
        SkDebugf("%d: %i\n", __LINE__, fColorMask );
        SkDebugf("%d: %i\n", __LINE__, fCompileShader );
        SkDebugf("%d: %i\n", __LINE__, fCreateProgram );
        SkDebugf("%d: %i\n", __LINE__, fCreateShader );
        SkDebugf("%d: %i\n", __LINE__, fCullFace );
        SkDebugf("%d: %i\n", __LINE__, fDeleteBuffers );
        SkDebugf("%d: %i\n", __LINE__, fDeleteProgram );
        SkDebugf("%d: %i\n", __LINE__, fDeleteShader );
        SkDebugf("%d: %i\n", __LINE__, fDeleteTextures );
        SkDebugf("%d: %i\n", __LINE__, fDepthMask );
        SkDebugf("%d: %i\n", __LINE__, fDisable );
        SkDebugf("%d: %i\n", __LINE__, fDisableVertexAttribArray );
        SkDebugf("%d: %i\n", __LINE__, fDrawArrays );
        SkDebugf("%d: %i\n", __LINE__, fDrawElements );
        SkDebugf("%d: %i\n", __LINE__, fEnable );
        SkDebugf("%d: %i\n", __LINE__, fEnableVertexAttribArray );
        SkDebugf("%d: %i\n", __LINE__, fFrontFace );
        SkDebugf("%d: %i\n", __LINE__, fGenBuffers );
        SkDebugf("%d: %i\n", __LINE__, fGenTextures );
        SkDebugf("%d: %i\n", __LINE__, fGetBufferParameteriv );
        SkDebugf("%d: %i\n", __LINE__, fGetError );
        SkDebugf("%d: %i\n", __LINE__, fGetIntegerv );
        SkDebugf("%d: %i\n", __LINE__, fGetProgramInfoLog );
        SkDebugf("%d: %i\n", __LINE__, fGetProgramiv );
        SkDebugf("%d: %i\n", __LINE__, fGetShaderInfoLog );
        SkDebugf("%d: %i\n", __LINE__, fGetShaderiv );
        SkDebugf("%d: %i\n", __LINE__, fGetString );
        SkDebugf("%d: %i\n", __LINE__, fGetUniformLocation );
        SkDebugf("%d: %i\n", __LINE__, fLinkProgram );
        SkDebugf("%d: %i\n", __LINE__, fPixelStorei );
        SkDebugf("%d: %i\n", __LINE__, fReadPixels );
        SkDebugf("%d: %i\n", __LINE__, fScissor );
        SkDebugf("%d: %i\n", __LINE__, fShaderSource );
        SkDebugf("%d: %i\n", __LINE__, fStencilFunc );
        SkDebugf("%d: %i\n", __LINE__, fStencilMask );
        SkDebugf("%d: %i\n", __LINE__, fStencilOp );
        SkDebugf("%d: %i\n", __LINE__, fTexImage2D );
        SkDebugf("%d: %i\n", __LINE__, fTexParameteri );
        SkDebugf("%d: %i\n", __LINE__, fTexParameteriv );
        SkDebugf("%d: %i\n", __LINE__, fTexSubImage2D );
        SkDebugf("%d: %i\n", __LINE__, fUniform1f );
        SkDebugf("%d: %i\n", __LINE__, fUniform1i );
        SkDebugf("%d: %i\n", __LINE__, fUniform1fv );
        SkDebugf("%d: %i\n", __LINE__, fUniform1iv );
        SkDebugf("%d: %i\n", __LINE__, fUniform2f );
        SkDebugf("%d: %i\n", __LINE__, fUniform2i );
        SkDebugf("%d: %i\n", __LINE__, fUniform2fv );
        SkDebugf("%d: %i\n", __LINE__, fUniform2iv );
        SkDebugf("%d: %i\n", __LINE__, fUniform3f );
        SkDebugf("%d: %i\n", __LINE__, fUniform3i );
        SkDebugf("%d: %i\n", __LINE__, fUniform3fv );
        SkDebugf("%d: %i\n", __LINE__, fUniform3iv );
        SkDebugf("%d: %i\n", __LINE__, fUniform4f );
        SkDebugf("%d: %i\n", __LINE__, fUniform4i );
        SkDebugf("%d: %i\n", __LINE__, fUniform4fv );
        SkDebugf("%d: %i\n", __LINE__, fUniform4iv );
        SkDebugf("%d: %i\n", __LINE__, fUniformMatrix2fv );
        SkDebugf("%d: %i\n", __LINE__, fUniformMatrix3fv );
        SkDebugf("%d: %i\n", __LINE__, fUniformMatrix4fv );
        SkDebugf("%d: %i\n", __LINE__, fUseProgram );
        SkDebugf("%d: %i\n", __LINE__, fVertexAttrib4fv );
        SkDebugf("%d: %i\n", __LINE__, fVertexAttribPointer );
        SkDebugf("%d: %i\n", __LINE__, fViewport );
        SkDebugf("%d: %i\n", __LINE__, fBindFramebuffer );
        SkDebugf("%d: %i\n", __LINE__, fBindRenderbuffer );
        SkDebugf("%d: %i\n", __LINE__, fCheckFramebufferStatus );
        SkDebugf("%d: %i\n", __LINE__, fDeleteFramebuffers );
        SkDebugf("%d: %i\n", __LINE__, fDeleteRenderbuffers );
        SkDebugf("%d: %i\n", __LINE__, fFinish );
        SkDebugf("%d: %i\n", __LINE__, fFlush );
        SkDebugf("%d: %i\n", __LINE__, fFramebufferRenderbuffer );
        SkDebugf("%d: %i\n", __LINE__, fFramebufferTexture2D );
        SkDebugf("%d: %i\n", __LINE__, fGetFramebufferAttachmentParameteriv );
        SkDebugf("%d: %i\n", __LINE__, fGetRenderbufferParameteriv );
        SkDebugf("%d: %i\n", __LINE__, fGenFramebuffers );
        SkDebugf("%d: %i\n", __LINE__, fGenRenderbuffers );
        SkDebugf("%d: %i\n", __LINE__, fRenderbufferStorage);
        return false;
    }

    const char* ext;
    GrGLVersion glVer = GrGLGetVersion(this);
    ext = (const char*)fGetString(GR_GL_EXTENSIONS);

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
            SkDebugf("%d: %i\n", __LINE__, fStencilFuncSeparate );
            SkDebugf("%d: %i\n", __LINE__, fStencilMaskSeparate );
            SkDebugf("%d: %i\n", __LINE__, fStencilOpSeparate);
            return false;
        }
    } else if (kDesktop_GrGLBinding == binding) {

        if (glVer >= GR_GL_VER(2,0)) {
            if (NULL == fStencilFuncSeparate ||
                NULL == fStencilMaskSeparate ||
                NULL == fStencilOpSeparate) {
                SkDebugf("%d: %i\n", __LINE__, fStencilFuncSeparate );
                SkDebugf("%d: %i\n", __LINE__, fStencilMaskSeparate );
                SkDebugf("%d: %i\n", __LINE__, fStencilOpSeparate);
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,0) && NULL == fBindFragDataLocation) {
            SkDebugf("%d: %i\n", __LINE__, fBindFragDataLocation );
            return false;
        }
        if (glVer >= GR_GL_VER(2,0) ||
            GrGLHasExtensionFromString("GL_ARB_draw_buffers", ext)) {
            if (NULL == fDrawBuffers) {
                SkDebugf("%d: %i\n", __LINE__, fDrawBuffers );
                return false;
            }
        }

        if (glVer >= GR_GL_VER(1,5) ||
            GrGLHasExtensionFromString("GL_ARB_occlusion_query", ext)) {
            if (NULL == fGenQueries ||
                NULL == fDeleteQueries ||
                NULL == fBeginQuery ||
                NULL == fEndQuery ||
                NULL == fGetQueryiv ||
                NULL == fGetQueryObjectiv ||
                NULL == fGetQueryObjectuiv) {
                SkDebugf("%d: %i\n", __LINE__, fGenQueries );
                SkDebugf("%d: %i\n", __LINE__, fDeleteQueries );
                SkDebugf("%d: %i\n", __LINE__, fBeginQuery);
                SkDebugf("%d: %i\n", __LINE__, fEndQuery );
                SkDebugf("%d: %i\n", __LINE__, fGetQueryiv );
                SkDebugf("%d: %i\n", __LINE__, fGetQueryObjectiv);
                SkDebugf("%d: %i\n", __LINE__, fGetQueryObjectuiv);
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,3) ||
            GrGLHasExtensionFromString("GL_ARB_timer_query", ext) ||
            GrGLHasExtensionFromString("GL_EXT_timer_query", ext)) {
            if (NULL == fGetQueryObjecti64v ||
                NULL == fGetQueryObjectui64v) {
                SkDebugf("%d: %i\n", __LINE__, fGetQueryObjecti64v);
                SkDebugf("%d: %i\n", __LINE__, fGetQueryObjectui64v);
                return false;
            }
        }
        if (glVer >= GR_GL_VER(3,3) ||
            GrGLHasExtensionFromString("GL_ARB_timer_query", ext)) {
            if (NULL == fQueryCounter) {
                SkDebugf("%d: %i\n", __LINE__, fQueryCounter);
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
            SkDebugf("%d: %i\n", __LINE__, fMatrixMode );
            SkDebugf("%d: %i\n", __LINE__, fLoadIdentity);
            SkDebugf("%d: %i\n", __LINE__, fLoadMatrixf);
            return false;
        }
        if (false && GrGLHasExtensionFromString("GL_NV_path_rendering", ext)) {
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
                SkDebugf("%d: %i\n", __LINE__, fPathCommands );
                SkDebugf("%d: %i\n", __LINE__, fPathCoords );
                SkDebugf("%d: %i\n", __LINE__, fPathSubCommands );
                SkDebugf("%d: %i\n", __LINE__, fPathSubCoords );
                SkDebugf("%d: %i\n", __LINE__, fPathString );
                SkDebugf("%d: %i\n", __LINE__, fPathGlyphs );
                SkDebugf("%d: %i\n", __LINE__, fPathGlyphRange );
                SkDebugf("%d: %i\n", __LINE__, fWeightPaths );
                SkDebugf("%d: %i\n", __LINE__, fCopyPath );
                SkDebugf("%d: %i\n", __LINE__, fInterpolatePaths );
                SkDebugf("%d: %i\n", __LINE__, fTransformPath );
                SkDebugf("%d: %i\n", __LINE__, fPathParameteriv );
                SkDebugf("%d: %i\n", __LINE__, fPathParameteri );
                SkDebugf("%d: %i\n", __LINE__, fPathParameterfv );
                SkDebugf("%d: %i\n", __LINE__, fPathParameterf );
                SkDebugf("%d: %i\n", __LINE__, fPathDashArray );
                SkDebugf("%d: %i\n", __LINE__, fGenPaths );
                SkDebugf("%d: %i\n", __LINE__, fDeletePaths );
                SkDebugf("%d: %i\n", __LINE__, fIsPath );
                SkDebugf("%d: %i\n", __LINE__, fPathStencilFunc );
                SkDebugf("%d: %i\n", __LINE__, fPathStencilDepthOffset );
                SkDebugf("%d: %i\n", __LINE__, fStencilFillPath );
                SkDebugf("%d: %i\n", __LINE__, fStencilStrokePath );
                SkDebugf("%d: %i\n", __LINE__, fStencilFillPathInstanced );
                SkDebugf("%d: %i\n", __LINE__, fStencilStrokePathInstanced );
                SkDebugf("%d: %i\n", __LINE__, fPathCoverDepthFunc );
                SkDebugf("%d: %i\n", __LINE__, fPathColorGen );
                SkDebugf("%d: %i\n", __LINE__, fPathTexGen );
                SkDebugf("%d: %i\n", __LINE__, fPathFogGen );
                SkDebugf("%d: %i\n", __LINE__, fCoverFillPath );
                SkDebugf("%d: %i\n", __LINE__, fCoverStrokePath );
                SkDebugf("%d: %i\n", __LINE__, fCoverFillPathInstanced );
                SkDebugf("%d: %i\n", __LINE__, fCoverStrokePathInstanced );
                SkDebugf("%d: %i\n", __LINE__, fGetPathParameteriv );
                SkDebugf("%d: %i\n", __LINE__, fGetPathParameterfv );
                SkDebugf("%d: %i\n", __LINE__, fGetPathCommands );
                SkDebugf("%d: %i\n", __LINE__, fGetPathCoords );
                SkDebugf("%d: %i\n", __LINE__, fGetPathDashArray );
                SkDebugf("%d: %i\n", __LINE__, fGetPathMetrics );
                SkDebugf("%d: %i\n", __LINE__, fGetPathMetricRange );
                SkDebugf("%d: %i\n", __LINE__, fGetPathSpacing );
                SkDebugf("%d: %i\n", __LINE__, fGetPathColorGeniv );
                SkDebugf("%d: %i\n", __LINE__, fGetPathColorGenfv );
                SkDebugf("%d: %i\n", __LINE__, fGetPathTexGeniv );
                SkDebugf("%d: %i\n", __LINE__, fGetPathTexGenfv );
                SkDebugf("%d: %i\n", __LINE__, fIsPointInFillPath );
                SkDebugf("%d: %i\n", __LINE__, fIsPointInStrokePath );
                SkDebugf("%d: %i\n", __LINE__, fGetPathLength );
                SkDebugf("%d: %i\n", __LINE__, fPointAlongPath );
                return false;
            }
        }
    }

    // optional function on desktop before 1.3
    if (kDesktop_GrGLBinding != binding ||
        (glVer >= GR_GL_VER(1,3) ||
        GrGLHasExtensionFromString("GL_ARB_texture_compression", ext))) {
        if (NULL == fCompressedTexImage2D) {
            SkDebugf("%d: %i\n", __LINE__, fCompressedTexImage2D);
            return false;
        }
    }

    // part of desktop GL, but not ES
    if (kDesktop_GrGLBinding == binding &&
        (NULL == fLineWidth ||
         NULL == fGetTexLevelParameteriv ||
         NULL == fDrawBuffer ||
         NULL == fReadBuffer)) {
        SkDebugf("%d: %i\n", __LINE__, fLineWidth);
        SkDebugf("%d: %i\n", __LINE__, fGetTexLevelParameteriv);
        SkDebugf("%d: %i\n", __LINE__, fDrawBuffer);
        SkDebugf("%d: %i\n", __LINE__, fReadBuffer);
        return false;
    }

    // GL_EXT_texture_storage is part of desktop 4.2
    // There is a desktop ARB extension and an ES+desktop EXT extension
    if (kDesktop_GrGLBinding == binding) {
        if (glVer >= GR_GL_VER(4,2) ||
            GrGLHasExtensionFromString("GL_ARB_texture_storage", ext) ||
            GrGLHasExtensionFromString("GL_EXT_texture_storage", ext)) {
            if (NULL == fTexStorage2D) {
                SkDebugf("%d: %i\n", __LINE__, fTexStorage2D);
                return false;
            }
        }
    } else if (GrGLHasExtensionFromString("GL_EXT_texture_storage", ext)) {
        if (NULL == fTexStorage2D) {
            SkDebugf("%d: %i\n", __LINE__, fTexStorage2D);
            return false;
        }
    }

    // FBO MSAA
    if (kDesktop_GrGLBinding == binding) {
        // GL 3.0 and the ARB extension have multisample + blit
        if (glVer >= GR_GL_VER(3,0) || GrGLHasExtensionFromString("GL_ARB_framebuffer_object", ext)) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fBlitFramebuffer) {
                SkDebugf("%d: %i\n", __LINE__, fRenderbufferStorageMultisample);
                SkDebugf("%d: %i\n", __LINE__, fBlitFramebuffer);
                return false;
            }
        } else {
            if (GrGLHasExtensionFromString("GL_EXT_framebuffer_blit", ext) &&
                NULL == fBlitFramebuffer) {
                SkDebugf("%d: %i\n", __LINE__, fBlitFramebuffer);
                return false;
            }
            if (GrGLHasExtensionFromString("GL_EXT_framebuffer_multisample", ext) &&
                NULL == fRenderbufferStorageMultisample) {
                SkDebugf("%d: %i\n", __LINE__, fRenderbufferStorageMultisample);
                return false;
            }
        }
    } else {
        if (GrGLHasExtensionFromString("GL_CHROMIUM_framebuffer_multisample", ext)) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fBlitFramebuffer) {
                SkDebugf("%d: %i\n", __LINE__, fRenderbufferStorageMultisample);
                SkDebugf("%d: %i\n", __LINE__, fBlitFramebuffer);
                return false;
            }
        }
        if (GrGLHasExtensionFromString("GL_APPLE_framebuffer_multisample", ext)) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fResolveMultisampleFramebuffer) {
                SkDebugf("%d: %i\n", __LINE__, fRenderbufferStorageMultisample);
                SkDebugf("%d: %i\n", __LINE__, fResolveMultisampleFramebuffer);
                return false;
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (kDesktop_GrGLBinding == binding ||
        GrGLHasExtensionFromString("GL_OES_mapbuffer", ext)) {
        if (NULL == fMapBuffer ||
            NULL == fUnmapBuffer) {
            SkDebugf("%d: %i\n", __LINE__, fMapBuffer);
            SkDebugf("%d: %i\n", __LINE__, fUnmapBuffer);
            return false;
        }
    }

    // Dual source blending
    if (kDesktop_GrGLBinding == binding &&
        (glVer >= GR_GL_VER(3,3) ||
         GrGLHasExtensionFromString("GL_ARB_blend_func_extended", ext))) {
        if (NULL == fBindFragDataLocationIndexed) {
            SkDebugf("%d: %i\n", __LINE__, fBindFragDataLocationIndexed);
            return false;
        }
    }

    return true;
}


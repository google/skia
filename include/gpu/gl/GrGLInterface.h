/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLInterface_DEFINED
#define GrGLInterface_DEFINED

#include "GrGLFunctions.h"
#include "GrGLExtensions.h"
#include "SkRefCnt.h"

////////////////////////////////////////////////////////////////////////////////

typedef void(*GrGLFuncPtr)();
struct GrGLInterface;


/**
 * Rather than depend on platform-specific GL headers and libraries, we require
 * the client to provide a struct of GL function pointers. This struct can be
 * specified per-GrContext as a parameter to GrContext::MakeGL. If NULL is
 * passed to MakeGL then a "native" GL interface is created. If the native is
 * also NULL GrContext creation will fail.
 *
 * The default interface is returned by GrGLMakeNativeInterface. This function's
 * implementation is platform-specific. Several have been provided
 * (for GLX, WGL, EGL, etc), along with an implementation that simply returns
 * NULL.
 */
SK_API sk_sp<const GrGLInterface> GrGLMakeNativeInterface();
// Deprecated alternative to GrGLMakeNativeInterface().
SK_API const GrGLInterface* GrGLCreateNativeInterface();

/**
 * Creates a null GrGLInterface that doesn't draw anything. Used for measuring
 * CPU overhead. TODO: We would like to move this to tools/gpu/gl/null but currently
 * Chromium is using it in its unit tests.
 */
const SK_API GrGLInterface* GrGLCreateNullInterface(bool enableNVPR = false);

/**
 * GrContext uses the following interface to make all calls into OpenGL. When a
 * GrContext is created it is given a GrGLInterface. The interface's function
 * pointers must be valid for the OpenGL context associated with the GrContext.
 * On some platforms, such as Windows, function pointers for OpenGL extensions
 * may vary between OpenGL contexts. So the caller must be careful to use a
 * GrGLInterface initialized for the correct context. All functions that should
 * be available based on the OpenGL's version and extension string must be
 * non-NULL or GrContext creation will fail. This can be tested with the
 * validate() method when the OpenGL context has been made current.
 */
struct SK_API GrGLInterface : public SkRefCnt {
private:
    typedef SkRefCnt INHERITED;

public:
    GrGLInterface();

    // Validates that the GrGLInterface supports its advertised standard. This means the necessary
    // function pointers have been initialized for both the GL version and any advertised
    // extensions.
    bool validate() const;

    // Indicates the type of GL implementation
    union {
        GrGLStandard fStandard;
        GrGLStandard fBindingsExported; // Legacy name, will be remove when Chromium is updated.
    };

    GrGLExtensions fExtensions;

    bool hasExtension(const char ext[]) const { return fExtensions.has(ext); }

    /**
     * The function pointers are in a struct so that we can have a compiler generated assignment
     * operator.
     */
    struct Functions {
        GrGLFunction<GrGLActiveTextureProc> fActiveTexture;
        GrGLFunction<GrGLAttachShaderProc> fAttachShader;
        GrGLFunction<GrGLBeginQueryProc> fBeginQuery;
        GrGLFunction<GrGLBindAttribLocationProc> fBindAttribLocation;
        GrGLFunction<GrGLBindBufferProc> fBindBuffer;
        GrGLFunction<GrGLBindFragDataLocationProc> fBindFragDataLocation;
        GrGLFunction<GrGLBindFragDataLocationIndexedProc> fBindFragDataLocationIndexed;
        GrGLFunction<GrGLBindFramebufferProc> fBindFramebuffer;
        GrGLFunction<GrGLBindRenderbufferProc> fBindRenderbuffer;
        GrGLFunction<GrGLBindTextureProc> fBindTexture;
        GrGLFunction<GrGLBindVertexArrayProc> fBindVertexArray;
        GrGLFunction<GrGLBlendBarrierProc> fBlendBarrier;
        GrGLFunction<GrGLBlendColorProc> fBlendColor;
        GrGLFunction<GrGLBlendEquationProc> fBlendEquation;
        GrGLFunction<GrGLBlendFuncProc> fBlendFunc;
        GrGLFunction<GrGLBlitFramebufferProc> fBlitFramebuffer;
        GrGLFunction<GrGLBufferDataProc> fBufferData;
        GrGLFunction<GrGLBufferSubDataProc> fBufferSubData;
        GrGLFunction<GrGLCheckFramebufferStatusProc> fCheckFramebufferStatus;
        GrGLFunction<GrGLClearProc> fClear;
        GrGLFunction<GrGLClearColorProc> fClearColor;
        GrGLFunction<GrGLClearStencilProc> fClearStencil;
        GrGLFunction<GrGLClearTexImageProc> fClearTexImage;
        GrGLFunction<GrGLClearTexSubImageProc> fClearTexSubImage;
        GrGLFunction<GrGLColorMaskProc> fColorMask;
        GrGLFunction<GrGLCompileShaderProc> fCompileShader;
        GrGLFunction<GrGLCompressedTexImage2DProc> fCompressedTexImage2D;
        GrGLFunction<GrGLCompressedTexSubImage2DProc> fCompressedTexSubImage2D;
        GrGLFunction<GrGLCopyTexSubImage2DProc> fCopyTexSubImage2D;
        GrGLFunction<GrGLCreateProgramProc> fCreateProgram;
        GrGLFunction<GrGLCreateShaderProc> fCreateShader;
        GrGLFunction<GrGLCullFaceProc> fCullFace;
        GrGLFunction<GrGLDeleteBuffersProc> fDeleteBuffers;
        GrGLFunction<GrGLDeleteFramebuffersProc> fDeleteFramebuffers;
        GrGLFunction<GrGLDeleteProgramProc> fDeleteProgram;
        GrGLFunction<GrGLDeleteQueriesProc> fDeleteQueries;
        GrGLFunction<GrGLDeleteRenderbuffersProc> fDeleteRenderbuffers;
        GrGLFunction<GrGLDeleteShaderProc> fDeleteShader;
        GrGLFunction<GrGLDeleteTexturesProc> fDeleteTextures;
        GrGLFunction<GrGLDeleteVertexArraysProc> fDeleteVertexArrays;
        GrGLFunction<GrGLDepthMaskProc> fDepthMask;
        GrGLFunction<GrGLDisableProc> fDisable;
        GrGLFunction<GrGLDisableVertexAttribArrayProc> fDisableVertexAttribArray;
        GrGLFunction<GrGLDrawArraysProc> fDrawArrays;
        GrGLFunction<GrGLDrawArraysIndirectProc> fDrawArraysIndirect;
        GrGLFunction<GrGLDrawArraysInstancedProc> fDrawArraysInstanced;
        GrGLFunction<GrGLDrawBufferProc> fDrawBuffer;
        GrGLFunction<GrGLDrawBuffersProc> fDrawBuffers;
        GrGLFunction<GrGLDrawElementsProc> fDrawElements;
        GrGLFunction<GrGLDrawElementsIndirectProc> fDrawElementsIndirect;
        GrGLFunction<GrGLDrawElementsInstancedProc> fDrawElementsInstanced;
        GrGLFunction<GrGLDrawRangeElementsProc> fDrawRangeElements;
        GrGLFunction<GrGLEnableProc> fEnable;
        GrGLFunction<GrGLEnableVertexAttribArrayProc> fEnableVertexAttribArray;
        GrGLFunction<GrGLEndQueryProc> fEndQuery;
        GrGLFunction<GrGLFinishProc> fFinish;
        GrGLFunction<GrGLFlushProc> fFlush;
        GrGLFunction<GrGLFlushMappedBufferRangeProc> fFlushMappedBufferRange;
        GrGLFunction<GrGLFramebufferRenderbufferProc> fFramebufferRenderbuffer;
        GrGLFunction<GrGLFramebufferTexture2DProc> fFramebufferTexture2D;
        GrGLFunction<GrGLFramebufferTexture2DMultisampleProc> fFramebufferTexture2DMultisample;
        GrGLFunction<GrGLFrontFaceProc> fFrontFace;
        GrGLFunction<GrGLGenBuffersProc> fGenBuffers;
        GrGLFunction<GrGLGenFramebuffersProc> fGenFramebuffers;
        GrGLFunction<GrGLGenerateMipmapProc> fGenerateMipmap;
        GrGLFunction<GrGLGenQueriesProc> fGenQueries;
        GrGLFunction<GrGLGenRenderbuffersProc> fGenRenderbuffers;
        GrGLFunction<GrGLGenTexturesProc> fGenTextures;
        GrGLFunction<GrGLGenVertexArraysProc> fGenVertexArrays;
        GrGLFunction<GrGLGetBufferParameterivProc> fGetBufferParameteriv;
        GrGLFunction<GrGLGetErrorProc> fGetError;
        GrGLFunction<GrGLGetFramebufferAttachmentParameterivProc> fGetFramebufferAttachmentParameteriv;
        GrGLFunction<GrGLGetIntegervProc> fGetIntegerv;
        GrGLFunction<GrGLGetMultisamplefvProc> fGetMultisamplefv;
        GrGLFunction<GrGLGetProgramBinaryProc> fGetProgramBinary;
        GrGLFunction<GrGLGetProgramInfoLogProc> fGetProgramInfoLog;
        GrGLFunction<GrGLGetProgramivProc> fGetProgramiv;
        GrGLFunction<GrGLGetQueryObjecti64vProc> fGetQueryObjecti64v;
        GrGLFunction<GrGLGetQueryObjectivProc> fGetQueryObjectiv;
        GrGLFunction<GrGLGetQueryObjectui64vProc> fGetQueryObjectui64v;
        GrGLFunction<GrGLGetQueryObjectuivProc> fGetQueryObjectuiv;
        GrGLFunction<GrGLGetQueryivProc> fGetQueryiv;
        GrGLFunction<GrGLGetRenderbufferParameterivProc> fGetRenderbufferParameteriv;
        GrGLFunction<GrGLGetShaderInfoLogProc> fGetShaderInfoLog;
        GrGLFunction<GrGLGetShaderivProc> fGetShaderiv;
        GrGLFunction<GrGLGetShaderPrecisionFormatProc> fGetShaderPrecisionFormat;
        GrGLFunction<GrGLGetStringProc> fGetString;
        GrGLFunction<GrGLGetStringiProc> fGetStringi;
        GrGLFunction<GrGLGetTexLevelParameterivProc> fGetTexLevelParameteriv;
        GrGLFunction<GrGLGetUniformLocationProc> fGetUniformLocation;
        GrGLFunction<GrGLInsertEventMarkerProc> fInsertEventMarker;
        GrGLFunction<GrGLInvalidateBufferDataProc> fInvalidateBufferData;
        GrGLFunction<GrGLInvalidateBufferSubDataProc> fInvalidateBufferSubData;
        GrGLFunction<GrGLInvalidateFramebufferProc> fInvalidateFramebuffer;
        GrGLFunction<GrGLInvalidateSubFramebufferProc> fInvalidateSubFramebuffer;
        GrGLFunction<GrGLInvalidateTexImageProc> fInvalidateTexImage;
        GrGLFunction<GrGLInvalidateTexSubImageProc> fInvalidateTexSubImage;
        GrGLFunction<GrGLIsTextureProc> fIsTexture;
        GrGLFunction<GrGLLineWidthProc> fLineWidth;
        GrGLFunction<GrGLLinkProgramProc> fLinkProgram;
        GrGLFunction<GrGLProgramBinaryProc> fProgramBinary;
        GrGLFunction<GrGLProgramParameteriProc> fProgramParameteri;
        GrGLFunction<GrGLMapBufferProc> fMapBuffer;
        GrGLFunction<GrGLMapBufferRangeProc> fMapBufferRange;
        GrGLFunction<GrGLMapBufferSubDataProc> fMapBufferSubData;
        GrGLFunction<GrGLMapTexSubImage2DProc> fMapTexSubImage2D;
        GrGLFunction<GrGLMultiDrawArraysIndirectProc> fMultiDrawArraysIndirect;
        GrGLFunction<GrGLMultiDrawElementsIndirectProc> fMultiDrawElementsIndirect;
        GrGLFunction<GrGLPixelStoreiProc> fPixelStorei;
        GrGLFunction<GrGLPolygonModeProc> fPolygonMode;
        GrGLFunction<GrGLPopGroupMarkerProc> fPopGroupMarker;
        GrGLFunction<GrGLPushGroupMarkerProc> fPushGroupMarker;
        GrGLFunction<GrGLQueryCounterProc> fQueryCounter;
        GrGLFunction<GrGLRasterSamplesProc> fRasterSamples;
        GrGLFunction<GrGLReadBufferProc> fReadBuffer;
        GrGLFunction<GrGLReadPixelsProc> fReadPixels;
        GrGLFunction<GrGLRenderbufferStorageProc> fRenderbufferStorage;

        //  On OpenGL ES there are multiple incompatible extensions that add support for MSAA
        //  and ES3 adds MSAA support to the standard. On an ES3 driver we may still use the
        //  older extensions for performance reasons or due to ES3 driver bugs. We want the function
        //  that creates the GrGLInterface to provide all available functions and internally
        //  we will select among them. They all have a method called glRenderbufferStorageMultisample*.
        //  So we have separate function pointers for GL_IMG/EXT_multisampled_to_texture,
        //  GL_CHROMIUM/ANGLE_framebuffer_multisample/ES3, and GL_APPLE_framebuffer_multisample
        //  variations.
        //
        //  If a driver supports multiple GL_ARB_framebuffer_multisample-style extensions then we will
        //  assume the function pointers for the standard (or equivalent GL_ARB) version have
        //  been preferred over GL_EXT, GL_CHROMIUM, or GL_ANGLE variations that have reduced
        //  functionality.

        //  GL_EXT_multisampled_render_to_texture (preferred) or GL_IMG_multisampled_render_to_texture
        GrGLFunction<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisampleES2EXT;
        //  GL_APPLE_framebuffer_multisample
        GrGLFunction<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisampleES2APPLE;

        //  This is used to store the pointer for GL_ARB/EXT/ANGLE/CHROMIUM_framebuffer_multisample or
        //  the standard function in ES3+ or GL 3.0+.
        GrGLFunction<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisample;

        // Pointer to BindUniformLocationCHROMIUM from the GL_CHROMIUM_bind_uniform_location extension.
        GrGLFunction<GrGLBindUniformLocationProc> fBindUniformLocation;

        GrGLFunction<GrGLResolveMultisampleFramebufferProc> fResolveMultisampleFramebuffer;
        GrGLFunction<GrGLScissorProc> fScissor;
        GrGLFunction<GrGLShaderSourceProc> fShaderSource;
        GrGLFunction<GrGLStencilFuncProc> fStencilFunc;
        GrGLFunction<GrGLStencilFuncSeparateProc> fStencilFuncSeparate;
        GrGLFunction<GrGLStencilMaskProc> fStencilMask;
        GrGLFunction<GrGLStencilMaskSeparateProc> fStencilMaskSeparate;
        GrGLFunction<GrGLStencilOpProc> fStencilOp;
        GrGLFunction<GrGLStencilOpSeparateProc> fStencilOpSeparate;
        GrGLFunction<GrGLTexBufferProc> fTexBuffer;
        GrGLFunction<GrGLTexBufferRangeProc> fTexBufferRange;
        GrGLFunction<GrGLTexImage2DProc> fTexImage2D;
        GrGLFunction<GrGLTexParameteriProc> fTexParameteri;
        GrGLFunction<GrGLTexParameterivProc> fTexParameteriv;
        GrGLFunction<GrGLTexSubImage2DProc> fTexSubImage2D;
        GrGLFunction<GrGLTexStorage2DProc> fTexStorage2D;
        GrGLFunction<GrGLTextureBarrierProc> fTextureBarrier;
        GrGLFunction<GrGLDiscardFramebufferProc> fDiscardFramebuffer;
        GrGLFunction<GrGLUniform1fProc> fUniform1f;
        GrGLFunction<GrGLUniform1iProc> fUniform1i;
        GrGLFunction<GrGLUniform1fvProc> fUniform1fv;
        GrGLFunction<GrGLUniform1ivProc> fUniform1iv;
        GrGLFunction<GrGLUniform2fProc> fUniform2f;
        GrGLFunction<GrGLUniform2iProc> fUniform2i;
        GrGLFunction<GrGLUniform2fvProc> fUniform2fv;
        GrGLFunction<GrGLUniform2ivProc> fUniform2iv;
        GrGLFunction<GrGLUniform3fProc> fUniform3f;
        GrGLFunction<GrGLUniform3iProc> fUniform3i;
        GrGLFunction<GrGLUniform3fvProc> fUniform3fv;
        GrGLFunction<GrGLUniform3ivProc> fUniform3iv;
        GrGLFunction<GrGLUniform4fProc> fUniform4f;
        GrGLFunction<GrGLUniform4iProc> fUniform4i;
        GrGLFunction<GrGLUniform4fvProc> fUniform4fv;
        GrGLFunction<GrGLUniform4ivProc> fUniform4iv;
        GrGLFunction<GrGLUniformMatrix2fvProc> fUniformMatrix2fv;
        GrGLFunction<GrGLUniformMatrix3fvProc> fUniformMatrix3fv;
        GrGLFunction<GrGLUniformMatrix4fvProc> fUniformMatrix4fv;
        GrGLFunction<GrGLUnmapBufferProc> fUnmapBuffer;
        GrGLFunction<GrGLUnmapBufferSubDataProc> fUnmapBufferSubData;
        GrGLFunction<GrGLUnmapTexSubImage2DProc> fUnmapTexSubImage2D;
        GrGLFunction<GrGLUseProgramProc> fUseProgram;
        GrGLFunction<GrGLVertexAttrib1fProc> fVertexAttrib1f;
        GrGLFunction<GrGLVertexAttrib2fvProc> fVertexAttrib2fv;
        GrGLFunction<GrGLVertexAttrib3fvProc> fVertexAttrib3fv;
        GrGLFunction<GrGLVertexAttrib4fvProc> fVertexAttrib4fv;
        GrGLFunction<GrGLVertexAttribDivisorProc> fVertexAttribDivisor;
        GrGLFunction<GrGLVertexAttribIPointerProc> fVertexAttribIPointer;
        GrGLFunction<GrGLVertexAttribPointerProc> fVertexAttribPointer;
        GrGLFunction<GrGLViewportProc> fViewport;

        /* GL_NV_path_rendering */
        GrGLFunction<GrGLMatrixLoadfProc> fMatrixLoadf;
        GrGLFunction<GrGLMatrixLoadIdentityProc> fMatrixLoadIdentity;
        GrGLFunction<GrGLGetProgramResourceLocationProc> fGetProgramResourceLocation;
        GrGLFunction<GrGLPathCommandsProc> fPathCommands;
        GrGLFunction<GrGLPathParameteriProc> fPathParameteri;
        GrGLFunction<GrGLPathParameterfProc> fPathParameterf;
        GrGLFunction<GrGLGenPathsProc> fGenPaths;
        GrGLFunction<GrGLDeletePathsProc> fDeletePaths;
        GrGLFunction<GrGLIsPathProc> fIsPath;
        GrGLFunction<GrGLPathStencilFuncProc> fPathStencilFunc;
        GrGLFunction<GrGLStencilFillPathProc> fStencilFillPath;
        GrGLFunction<GrGLStencilStrokePathProc> fStencilStrokePath;
        GrGLFunction<GrGLStencilFillPathInstancedProc> fStencilFillPathInstanced;
        GrGLFunction<GrGLStencilStrokePathInstancedProc> fStencilStrokePathInstanced;
        GrGLFunction<GrGLCoverFillPathProc> fCoverFillPath;
        GrGLFunction<GrGLCoverStrokePathProc> fCoverStrokePath;
        GrGLFunction<GrGLCoverFillPathInstancedProc> fCoverFillPathInstanced;
        GrGLFunction<GrGLCoverStrokePathInstancedProc> fCoverStrokePathInstanced;
        // NV_path_rendering v1.2
        GrGLFunction<GrGLStencilThenCoverFillPathProc> fStencilThenCoverFillPath;
        GrGLFunction<GrGLStencilThenCoverStrokePathProc> fStencilThenCoverStrokePath;
        GrGLFunction<GrGLStencilThenCoverFillPathInstancedProc> fStencilThenCoverFillPathInstanced;
        GrGLFunction<GrGLStencilThenCoverStrokePathInstancedProc> fStencilThenCoverStrokePathInstanced;
        // NV_path_rendering v1.3
        GrGLFunction<GrGLProgramPathFragmentInputGenProc> fProgramPathFragmentInputGen;
        // CHROMIUM_path_rendering
        GrGLFunction<GrGLBindFragmentInputLocationProc> fBindFragmentInputLocation;

        /* NV_framebuffer_mixed_samples */
        GrGLFunction<GrGLCoverageModulationProc> fCoverageModulation;

        /* ARB_sample_shading */
        GrGLFunction<GrGLMinSampleShadingProc> fMinSampleShading;

        /* ARB_sync */
        GrGLFunction<GrGLFenceSyncProc> fFenceSync;
        GrGLFunction<GrGLIsSyncProc> fIsSync;
        GrGLFunction<GrGLClientWaitSyncProc> fClientWaitSync;
        GrGLFunction<GrGLWaitSyncProc> fWaitSync;
        GrGLFunction<GrGLDeleteSyncProc> fDeleteSync;

        /* ARB_internalforamt_query */
        GrGLFunction<GrGLGetInternalformativProc> fGetInternalformativ;

        /* KHR_debug */
        GrGLFunction<GrGLDebugMessageControlProc> fDebugMessageControl;
        GrGLFunction<GrGLDebugMessageInsertProc> fDebugMessageInsert;
        GrGLFunction<GrGLDebugMessageCallbackProc> fDebugMessageCallback;
        GrGLFunction<GrGLGetDebugMessageLogProc> fGetDebugMessageLog;
        GrGLFunction<GrGLPushDebugGroupProc> fPushDebugGroup;
        GrGLFunction<GrGLPopDebugGroupProc> fPopDebugGroup;
        GrGLFunction<GrGLObjectLabelProc> fObjectLabel;

        /* EXT_window_rectangles */
        GrGLFunction<GrGLWindowRectanglesProc> fWindowRectangles;

        /* EGL functions */
        GrGLFunction<GrEGLCreateImageProc> fEGLCreateImage;
        GrGLFunction<GrEGLDestroyImageProc> fEGLDestroyImage;
    } fFunctions;

    // This exists for internal testing.
    virtual void abandon() const {}
};

#endif

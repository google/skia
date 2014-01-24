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

/**
 * Rather than depend on platform-specific GL headers and libraries, we require
 * the client to provide a struct of GL function pointers. This struct can be
 * specified per-GrContext as a parameter to GrContext::Create. If NULL is
 * passed to Create then a "default" GL interface is created. If the default is
 * also NULL GrContext creation will fail.
 *
 * The default interface is returned by GrGLDefaultInterface. This function's
 * implementation is platform-specific. Several have been provided, along with
 * an implementation that simply returns NULL.
 *
 * By defining GR_GL_PER_GL_CALL_IFACE_CALLBACK to 1 the client can specify a
 * callback function that will be called prior to each GL function call. See
 * comments in GrGLConfig.h
 */

struct GrGLInterface;

const GrGLInterface* GrGLDefaultInterface();

/**
 * Creates a GrGLInterface for a "native" GL context (e.g. WGL on windows,
 * GLX on linux, AGL on Mac). The interface is only valid for the GL context
 * that is current when the interface is created.
 */
const GrGLInterface* GrGLCreateNativeInterface();

#if SK_MESA
/**
 * Creates a GrGLInterface for an OSMesa context.
 */
const GrGLInterface* GrGLCreateMesaInterface();
#endif

#if SK_ANGLE
/**
 * Creates a GrGLInterface for an ANGLE context.
 */
const GrGLInterface* GrGLCreateANGLEInterface();
#endif

/**
 * Creates a null GrGLInterface that doesn't draw anything. Used for measuring
 * CPU overhead.
 */
const SK_API GrGLInterface* GrGLCreateNullInterface();

/**
 * Creates a debugging GrGLInterface that doesn't draw anything. Used for
 * finding memory leaks and invalid memory accesses.
 */
const GrGLInterface* GrGLCreateDebugInterface();

#if GR_GL_PER_GL_FUNC_CALLBACK
typedef void (*GrGLInterfaceCallbackProc)(const GrGLInterface*);
typedef intptr_t GrGLInterfaceCallbackData;
#endif

/** Function that returns a new interface identical to "interface" but without support for
    GL_NV_path_rendering. */
const GrGLInterface* GrGLInterfaceRemoveNVPR(const GrGLInterface*);

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
    // simple wrapper class that exists only to initialize a pointer to NULL
    template <typename FNPTR_TYPE> class GLPtr {
    public:
        GLPtr() : fPtr(NULL) {}
        GLPtr operator=(FNPTR_TYPE ptr) { fPtr = ptr; return *this; }
        operator FNPTR_TYPE() const { return fPtr; }
    private:
        FNPTR_TYPE fPtr;
    };

    // This is a temporary workaround to keep Chromium's GrGLInterface factories compiling until
    // they're updated to use the Functions struct.
    template <typename FNPTR_TYPE> class GLPtrAlias {
    public:
        GLPtrAlias(GLPtr<FNPTR_TYPE>* base) : fBase(base) {}
        void operator=(FNPTR_TYPE ptr) { *fBase = ptr; }
    private:
        GLPtr<FNPTR_TYPE>* fBase;
    };

    typedef SkRefCnt INHERITED;

public:
    SK_DECLARE_INST_COUNT(GrGLInterface)

    GrGLInterface();

    static GrGLInterface* NewClone(const GrGLInterface*);

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

    // This wrapper and const hackery is necessary because the factories in Chromium do not yet
    // initialize fExtensions.
    bool hasExtension(const char ext[]) const {
        if (!fExtensions.isInitialized()) {
            GrGLExtensions* extensions = const_cast<GrGLExtensions*>(&fExtensions);
            if (!extensions->init(fStandard, fFunctions.fGetString,
                                  fFunctions.fGetStringi, fFunctions.fGetIntegerv)) {
                return false;
            }
        }
        return fExtensions.has(ext);
    }

    /**
     * The function pointers are in a struct so that we can have a compiler generated assignment
     * operator.
     */
    struct Functions {
        GLPtr<GrGLActiveTextureProc> fActiveTexture;
        GLPtr<GrGLAttachShaderProc> fAttachShader;
        GLPtr<GrGLBeginQueryProc> fBeginQuery;
        GLPtr<GrGLBindAttribLocationProc> fBindAttribLocation;
        GLPtr<GrGLBindBufferProc> fBindBuffer;
        GLPtr<GrGLBindFragDataLocationProc> fBindFragDataLocation;
        GLPtr<GrGLBindFragDataLocationIndexedProc> fBindFragDataLocationIndexed;
        GLPtr<GrGLBindFramebufferProc> fBindFramebuffer;
        GLPtr<GrGLBindRenderbufferProc> fBindRenderbuffer;
        GLPtr<GrGLBindTextureProc> fBindTexture;
        GLPtr<GrGLBindVertexArrayProc> fBindVertexArray;
        GLPtr<GrGLBlendColorProc> fBlendColor;
        GLPtr<GrGLBlendFuncProc> fBlendFunc;
        GLPtr<GrGLBlitFramebufferProc> fBlitFramebuffer;
        GLPtr<GrGLBufferDataProc> fBufferData;
        GLPtr<GrGLBufferSubDataProc> fBufferSubData;
        GLPtr<GrGLCheckFramebufferStatusProc> fCheckFramebufferStatus;
        GLPtr<GrGLClearProc> fClear;
        GLPtr<GrGLClearColorProc> fClearColor;
        GLPtr<GrGLClearStencilProc> fClearStencil;
        GLPtr<GrGLClientActiveTextureProc> fClientActiveTexture;
        GLPtr<GrGLColorMaskProc> fColorMask;
        GLPtr<GrGLCompileShaderProc> fCompileShader;
        GLPtr<GrGLCompressedTexImage2DProc> fCompressedTexImage2D;
        GLPtr<GrGLCopyTexSubImage2DProc> fCopyTexSubImage2D;
        GLPtr<GrGLCreateProgramProc> fCreateProgram;
        GLPtr<GrGLCreateShaderProc> fCreateShader;
        GLPtr<GrGLCullFaceProc> fCullFace;
        GLPtr<GrGLDeleteBuffersProc> fDeleteBuffers;
        GLPtr<GrGLDeleteFramebuffersProc> fDeleteFramebuffers;
        GLPtr<GrGLDeleteProgramProc> fDeleteProgram;
        GLPtr<GrGLDeleteQueriesProc> fDeleteQueries;
        GLPtr<GrGLDeleteRenderbuffersProc> fDeleteRenderbuffers;
        GLPtr<GrGLDeleteShaderProc> fDeleteShader;
        GLPtr<GrGLDeleteTexturesProc> fDeleteTextures;
        GLPtr<GrGLDeleteVertexArraysProc> fDeleteVertexArrays;
        GLPtr<GrGLDepthMaskProc> fDepthMask;
        GLPtr<GrGLDisableProc> fDisable;
        GLPtr<GrGLDisableClientStateProc> fDisableClientState;
        GLPtr<GrGLDisableVertexAttribArrayProc> fDisableVertexAttribArray;
        GLPtr<GrGLDrawArraysProc> fDrawArrays;
        GLPtr<GrGLDrawBufferProc> fDrawBuffer;
        GLPtr<GrGLDrawBuffersProc> fDrawBuffers;
        GLPtr<GrGLDrawElementsProc> fDrawElements;
        GLPtr<GrGLEnableProc> fEnable;
        GLPtr<GrGLEnableClientStateProc> fEnableClientState;
        GLPtr<GrGLEnableVertexAttribArrayProc> fEnableVertexAttribArray;
        GLPtr<GrGLEndQueryProc> fEndQuery;
        GLPtr<GrGLFinishProc> fFinish;
        GLPtr<GrGLFlushProc> fFlush;
        GLPtr<GrGLFramebufferRenderbufferProc> fFramebufferRenderbuffer;
        GLPtr<GrGLFramebufferTexture2DProc> fFramebufferTexture2D;
        GLPtr<GrGLFramebufferTexture2DMultisampleProc> fFramebufferTexture2DMultisample;
        GLPtr<GrGLFrontFaceProc> fFrontFace;
        GLPtr<GrGLGenBuffersProc> fGenBuffers;
        GLPtr<GrGLGenFramebuffersProc> fGenFramebuffers;
        GLPtr<GrGLGenerateMipmapProc> fGenerateMipmap;
        GLPtr<GrGLGenQueriesProc> fGenQueries;
        GLPtr<GrGLGenRenderbuffersProc> fGenRenderbuffers;
        GLPtr<GrGLGenTexturesProc> fGenTextures;
        GLPtr<GrGLGenVertexArraysProc> fGenVertexArrays;
        GLPtr<GrGLGetBufferParameterivProc> fGetBufferParameteriv;
        GLPtr<GrGLGetErrorProc> fGetError;
        GLPtr<GrGLGetFramebufferAttachmentParameterivProc> fGetFramebufferAttachmentParameteriv;
        GLPtr<GrGLGetIntegervProc> fGetIntegerv;
        GLPtr<GrGLGetQueryObjecti64vProc> fGetQueryObjecti64v;
        GLPtr<GrGLGetQueryObjectivProc> fGetQueryObjectiv;
        GLPtr<GrGLGetQueryObjectui64vProc> fGetQueryObjectui64v;
        GLPtr<GrGLGetQueryObjectuivProc> fGetQueryObjectuiv;
        GLPtr<GrGLGetQueryivProc> fGetQueryiv;
        GLPtr<GrGLGetProgramInfoLogProc> fGetProgramInfoLog;
        GLPtr<GrGLGetProgramivProc> fGetProgramiv;
        GLPtr<GrGLGetRenderbufferParameterivProc> fGetRenderbufferParameteriv;
        GLPtr<GrGLGetShaderInfoLogProc> fGetShaderInfoLog;
        GLPtr<GrGLGetShaderivProc> fGetShaderiv;
        GLPtr<GrGLGetStringProc> fGetString;
        GLPtr<GrGLGetStringiProc> fGetStringi;
        GLPtr<GrGLGetTexLevelParameterivProc> fGetTexLevelParameteriv;
        GLPtr<GrGLGetUniformLocationProc> fGetUniformLocation;
        GLPtr<GrGLLineWidthProc> fLineWidth;
        GLPtr<GrGLLinkProgramProc> fLinkProgram;
        GLPtr<GrGLLoadIdentityProc> fLoadIdentity;
        GLPtr<GrGLLoadMatrixfProc> fLoadMatrixf;
        GLPtr<GrGLMapBufferProc> fMapBuffer;
        GLPtr<GrGLMatrixModeProc> fMatrixMode;
        GLPtr<GrGLPixelStoreiProc> fPixelStorei;
        GLPtr<GrGLQueryCounterProc> fQueryCounter;
        GLPtr<GrGLReadBufferProc> fReadBuffer;
        GLPtr<GrGLReadPixelsProc> fReadPixels;
        GLPtr<GrGLRenderbufferStorageProc> fRenderbufferStorage;

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
        GLPtr<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisampleES2EXT;
        //  GL_APPLE_framebuffer_multisample
        GLPtr<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisampleES2APPLE;

        //  This is used to store the pointer for GL_ARB/EXT/ANGLE/CHROMIUM_framebuffer_multisample or
        //  the standard function in ES3+ or GL 3.0+.
        GLPtr<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisample;

        // Pointer to BindUniformLocationCHROMIUM from the GL_CHROMIUM_bind_uniform_location extension.
        GLPtr<GrGLBindUniformLocation> fBindUniformLocation;

        GLPtr<GrGLResolveMultisampleFramebufferProc> fResolveMultisampleFramebuffer;
        GLPtr<GrGLScissorProc> fScissor;
        GLPtr<GrGLShaderSourceProc> fShaderSource;
        GLPtr<GrGLStencilFuncProc> fStencilFunc;
        GLPtr<GrGLStencilFuncSeparateProc> fStencilFuncSeparate;
        GLPtr<GrGLStencilMaskProc> fStencilMask;
        GLPtr<GrGLStencilMaskSeparateProc> fStencilMaskSeparate;
        GLPtr<GrGLStencilOpProc> fStencilOp;
        GLPtr<GrGLStencilOpSeparateProc> fStencilOpSeparate;
        GLPtr<GrGLTexGenfProc> fTexGenf;
        GLPtr<GrGLTexGenfvProc> fTexGenfv;
        GLPtr<GrGLTexGeniProc> fTexGeni;
        GLPtr<GrGLTexImage2DProc> fTexImage2D;
        GLPtr<GrGLTexParameteriProc> fTexParameteri;
        GLPtr<GrGLTexParameterivProc> fTexParameteriv;
        GLPtr<GrGLTexSubImage2DProc> fTexSubImage2D;
        GLPtr<GrGLTexStorage2DProc> fTexStorage2D;
        GLPtr<GrGLDiscardFramebufferProc> fDiscardFramebuffer;
        GLPtr<GrGLUniform1fProc> fUniform1f;
        GLPtr<GrGLUniform1iProc> fUniform1i;
        GLPtr<GrGLUniform1fvProc> fUniform1fv;
        GLPtr<GrGLUniform1ivProc> fUniform1iv;
        GLPtr<GrGLUniform2fProc> fUniform2f;
        GLPtr<GrGLUniform2iProc> fUniform2i;
        GLPtr<GrGLUniform2fvProc> fUniform2fv;
        GLPtr<GrGLUniform2ivProc> fUniform2iv;
        GLPtr<GrGLUniform3fProc> fUniform3f;
        GLPtr<GrGLUniform3iProc> fUniform3i;
        GLPtr<GrGLUniform3fvProc> fUniform3fv;
        GLPtr<GrGLUniform3ivProc> fUniform3iv;
        GLPtr<GrGLUniform4fProc> fUniform4f;
        GLPtr<GrGLUniform4iProc> fUniform4i;
        GLPtr<GrGLUniform4fvProc> fUniform4fv;
        GLPtr<GrGLUniform4ivProc> fUniform4iv;
        GLPtr<GrGLUniformMatrix2fvProc> fUniformMatrix2fv;
        GLPtr<GrGLUniformMatrix3fvProc> fUniformMatrix3fv;
        GLPtr<GrGLUniformMatrix4fvProc> fUniformMatrix4fv;
        GLPtr<GrGLUnmapBufferProc> fUnmapBuffer;
        GLPtr<GrGLUseProgramProc> fUseProgram;
        GLPtr<GrGLVertexAttrib4fvProc> fVertexAttrib4fv;
        GLPtr<GrGLVertexAttribPointerProc> fVertexAttribPointer;
        GLPtr<GrGLVertexPointerProc> fVertexPointer;
        GLPtr<GrGLViewportProc> fViewport;

        // Experimental: Functions for GL_NV_path_rendering. These will be
        // alphabetized with the above functions once this is fully supported
        // (and functions we are unlikely to use will possibly be omitted).
        GLPtr<GrGLPathCommandsProc> fPathCommands;
        GLPtr<GrGLPathCoordsProc> fPathCoords;
        GLPtr<GrGLPathSubCommandsProc> fPathSubCommands;
        GLPtr<GrGLPathSubCoordsProc> fPathSubCoords;
        GLPtr<GrGLPathStringProc> fPathString;
        GLPtr<GrGLPathGlyphsProc> fPathGlyphs;
        GLPtr<GrGLPathGlyphRangeProc> fPathGlyphRange;
        GLPtr<GrGLWeightPathsProc> fWeightPaths;
        GLPtr<GrGLCopyPathProc> fCopyPath;
        GLPtr<GrGLInterpolatePathsProc> fInterpolatePaths;
        GLPtr<GrGLTransformPathProc> fTransformPath;
        GLPtr<GrGLPathParameterivProc> fPathParameteriv;
        GLPtr<GrGLPathParameteriProc> fPathParameteri;
        GLPtr<GrGLPathParameterfvProc> fPathParameterfv;
        GLPtr<GrGLPathParameterfProc> fPathParameterf;
        GLPtr<GrGLPathDashArrayProc> fPathDashArray;
        GLPtr<GrGLGenPathsProc> fGenPaths;
        GLPtr<GrGLDeletePathsProc> fDeletePaths;
        GLPtr<GrGLIsPathProc> fIsPath;
        GLPtr<GrGLPathStencilFuncProc> fPathStencilFunc;
        GLPtr<GrGLPathStencilDepthOffsetProc> fPathStencilDepthOffset;
        GLPtr<GrGLStencilFillPathProc> fStencilFillPath;
        GLPtr<GrGLStencilStrokePathProc> fStencilStrokePath;
        GLPtr<GrGLStencilFillPathInstancedProc> fStencilFillPathInstanced;
        GLPtr<GrGLStencilStrokePathInstancedProc> fStencilStrokePathInstanced;
        GLPtr<GrGLPathCoverDepthFuncProc> fPathCoverDepthFunc;
        GLPtr<GrGLPathColorGenProc> fPathColorGen;
        GLPtr<GrGLPathTexGenProc> fPathTexGen;
        GLPtr<GrGLPathFogGenProc> fPathFogGen;
        GLPtr<GrGLCoverFillPathProc> fCoverFillPath;
        GLPtr<GrGLCoverStrokePathProc> fCoverStrokePath;
        GLPtr<GrGLCoverFillPathInstancedProc> fCoverFillPathInstanced;
        GLPtr<GrGLCoverStrokePathInstancedProc> fCoverStrokePathInstanced;
        GLPtr<GrGLGetPathParameterivProc> fGetPathParameteriv;
        GLPtr<GrGLGetPathParameterfvProc> fGetPathParameterfv;
        GLPtr<GrGLGetPathCommandsProc> fGetPathCommands;
        GLPtr<GrGLGetPathCoordsProc> fGetPathCoords;
        GLPtr<GrGLGetPathDashArrayProc> fGetPathDashArray;
        GLPtr<GrGLGetPathMetricsProc> fGetPathMetrics;
        GLPtr<GrGLGetPathMetricRangeProc> fGetPathMetricRange;
        GLPtr<GrGLGetPathSpacingProc> fGetPathSpacing;
        GLPtr<GrGLGetPathColorGenivProc> fGetPathColorGeniv;
        GLPtr<GrGLGetPathColorGenfvProc> fGetPathColorGenfv;
        GLPtr<GrGLGetPathTexGenivProc> fGetPathTexGeniv;
        GLPtr<GrGLGetPathTexGenfvProc> fGetPathTexGenfv;
        GLPtr<GrGLIsPointInFillPathProc> fIsPointInFillPath;
        GLPtr<GrGLIsPointInStrokePathProc> fIsPointInStrokePath;
        GLPtr<GrGLGetPathLengthProc> fGetPathLength;
        GLPtr<GrGLPointAlongPathProc> fPointAlongPath;
    } fFunctions;

    // Temporary workaround aliases to keep Chromium GrGLInterface factories compiling until they
    // assign the members of fFunctions.
    GLPtrAlias<GrGLActiveTextureProc> fActiveTexture;
    GLPtrAlias<GrGLAttachShaderProc> fAttachShader;
    GLPtrAlias<GrGLBeginQueryProc> fBeginQuery;
    GLPtrAlias<GrGLBindAttribLocationProc> fBindAttribLocation;
    GLPtrAlias<GrGLBindBufferProc> fBindBuffer;
    GLPtrAlias<GrGLBindFragDataLocationProc> fBindFragDataLocation;
    GLPtrAlias<GrGLBindFragDataLocationIndexedProc> fBindFragDataLocationIndexed;
    GLPtrAlias<GrGLBindFramebufferProc> fBindFramebuffer;
    GLPtrAlias<GrGLBindRenderbufferProc> fBindRenderbuffer;
    GLPtrAlias<GrGLBindTextureProc> fBindTexture;
    GLPtrAlias<GrGLBindVertexArrayProc> fBindVertexArray;
    GLPtrAlias<GrGLBlendColorProc> fBlendColor;
    GLPtrAlias<GrGLBlendFuncProc> fBlendFunc;
    GLPtrAlias<GrGLBlitFramebufferProc> fBlitFramebuffer;
    GLPtrAlias<GrGLBufferDataProc> fBufferData;
    GLPtrAlias<GrGLBufferSubDataProc> fBufferSubData;
    GLPtrAlias<GrGLCheckFramebufferStatusProc> fCheckFramebufferStatus;
    GLPtrAlias<GrGLClearProc> fClear;
    GLPtrAlias<GrGLClearColorProc> fClearColor;
    GLPtrAlias<GrGLClearStencilProc> fClearStencil;
    GLPtrAlias<GrGLClientActiveTextureProc> fClientActiveTexture;
    GLPtrAlias<GrGLColorMaskProc> fColorMask;
    GLPtrAlias<GrGLCompileShaderProc> fCompileShader;
    GLPtrAlias<GrGLCompressedTexImage2DProc> fCompressedTexImage2D;
    GLPtrAlias<GrGLCopyTexSubImage2DProc> fCopyTexSubImage2D;
    GLPtrAlias<GrGLCreateProgramProc> fCreateProgram;
    GLPtrAlias<GrGLCreateShaderProc> fCreateShader;
    GLPtrAlias<GrGLCullFaceProc> fCullFace;
    GLPtrAlias<GrGLDeleteBuffersProc> fDeleteBuffers;
    GLPtrAlias<GrGLDeleteFramebuffersProc> fDeleteFramebuffers;
    GLPtrAlias<GrGLDeleteProgramProc> fDeleteProgram;
    GLPtrAlias<GrGLDeleteQueriesProc> fDeleteQueries;
    GLPtrAlias<GrGLDeleteRenderbuffersProc> fDeleteRenderbuffers;
    GLPtrAlias<GrGLDeleteShaderProc> fDeleteShader;
    GLPtrAlias<GrGLDeleteTexturesProc> fDeleteTextures;
    GLPtrAlias<GrGLDeleteVertexArraysProc> fDeleteVertexArrays;
    GLPtrAlias<GrGLDepthMaskProc> fDepthMask;
    GLPtrAlias<GrGLDisableProc> fDisable;
    GLPtrAlias<GrGLDisableClientStateProc> fDisableClientState;
    GLPtrAlias<GrGLDisableVertexAttribArrayProc> fDisableVertexAttribArray;
    GLPtrAlias<GrGLDrawArraysProc> fDrawArrays;
    GLPtrAlias<GrGLDrawBufferProc> fDrawBuffer;
    GLPtrAlias<GrGLDrawBuffersProc> fDrawBuffers;
    GLPtrAlias<GrGLDrawElementsProc> fDrawElements;
    GLPtrAlias<GrGLEnableProc> fEnable;
    GLPtrAlias<GrGLEnableClientStateProc> fEnableClientState;
    GLPtrAlias<GrGLEnableVertexAttribArrayProc> fEnableVertexAttribArray;
    GLPtrAlias<GrGLEndQueryProc> fEndQuery;
    GLPtrAlias<GrGLFinishProc> fFinish;
    GLPtrAlias<GrGLFlushProc> fFlush;
    GLPtrAlias<GrGLFramebufferRenderbufferProc> fFramebufferRenderbuffer;
    GLPtrAlias<GrGLFramebufferTexture2DProc> fFramebufferTexture2D;
    GLPtrAlias<GrGLFramebufferTexture2DMultisampleProc> fFramebufferTexture2DMultisample;
    GLPtrAlias<GrGLFrontFaceProc> fFrontFace;
    GLPtrAlias<GrGLGenBuffersProc> fGenBuffers;
    GLPtrAlias<GrGLGenFramebuffersProc> fGenFramebuffers;
    GLPtrAlias<GrGLGenerateMipmapProc> fGenerateMipmap;
    GLPtrAlias<GrGLGenQueriesProc> fGenQueries;
    GLPtrAlias<GrGLGenRenderbuffersProc> fGenRenderbuffers;
    GLPtrAlias<GrGLGenTexturesProc> fGenTextures;
    GLPtrAlias<GrGLGenVertexArraysProc> fGenVertexArrays;
    GLPtrAlias<GrGLGetBufferParameterivProc> fGetBufferParameteriv;
    GLPtrAlias<GrGLGetErrorProc> fGetError;
    GLPtrAlias<GrGLGetFramebufferAttachmentParameterivProc> fGetFramebufferAttachmentParameteriv;
    GLPtrAlias<GrGLGetIntegervProc> fGetIntegerv;
    GLPtrAlias<GrGLGetQueryObjecti64vProc> fGetQueryObjecti64v;
    GLPtrAlias<GrGLGetQueryObjectivProc> fGetQueryObjectiv;
    GLPtrAlias<GrGLGetQueryObjectui64vProc> fGetQueryObjectui64v;
    GLPtrAlias<GrGLGetQueryObjectuivProc> fGetQueryObjectuiv;
    GLPtrAlias<GrGLGetQueryivProc> fGetQueryiv;
    GLPtrAlias<GrGLGetProgramInfoLogProc> fGetProgramInfoLog;
    GLPtrAlias<GrGLGetProgramivProc> fGetProgramiv;
    GLPtrAlias<GrGLGetRenderbufferParameterivProc> fGetRenderbufferParameteriv;
    GLPtrAlias<GrGLGetShaderInfoLogProc> fGetShaderInfoLog;
    GLPtrAlias<GrGLGetShaderivProc> fGetShaderiv;
    GLPtrAlias<GrGLGetStringProc> fGetString;
    GLPtrAlias<GrGLGetStringiProc> fGetStringi;
    GLPtrAlias<GrGLGetTexLevelParameterivProc> fGetTexLevelParameteriv;
    GLPtrAlias<GrGLGetUniformLocationProc> fGetUniformLocation;
    GLPtrAlias<GrGLLineWidthProc> fLineWidth;
    GLPtrAlias<GrGLLinkProgramProc> fLinkProgram;
    GLPtrAlias<GrGLLoadIdentityProc> fLoadIdentity;
    GLPtrAlias<GrGLLoadMatrixfProc> fLoadMatrixf;
    GLPtrAlias<GrGLMapBufferProc> fMapBuffer;
    GLPtrAlias<GrGLMatrixModeProc> fMatrixMode;
    GLPtrAlias<GrGLPixelStoreiProc> fPixelStorei;
    GLPtrAlias<GrGLQueryCounterProc> fQueryCounter;
    GLPtrAlias<GrGLReadBufferProc> fReadBuffer;
    GLPtrAlias<GrGLReadPixelsProc> fReadPixels;
    GLPtrAlias<GrGLRenderbufferStorageProc> fRenderbufferStorage;
    GLPtrAlias<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisampleES2EXT;
    GLPtrAlias<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisampleES2APPLE;
    GLPtrAlias<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisample;
    GLPtrAlias<GrGLBindUniformLocation> fBindUniformLocation;
    GLPtrAlias<GrGLResolveMultisampleFramebufferProc> fResolveMultisampleFramebuffer;
    GLPtrAlias<GrGLScissorProc> fScissor;
    GLPtrAlias<GrGLShaderSourceProc> fShaderSource;
    GLPtrAlias<GrGLStencilFuncProc> fStencilFunc;
    GLPtrAlias<GrGLStencilFuncSeparateProc> fStencilFuncSeparate;
    GLPtrAlias<GrGLStencilMaskProc> fStencilMask;
    GLPtrAlias<GrGLStencilMaskSeparateProc> fStencilMaskSeparate;
    GLPtrAlias<GrGLStencilOpProc> fStencilOp;
    GLPtrAlias<GrGLStencilOpSeparateProc> fStencilOpSeparate;
    GLPtrAlias<GrGLTexGenfProc> fTexGenf;
    GLPtrAlias<GrGLTexGenfvProc> fTexGenfv;
    GLPtrAlias<GrGLTexGeniProc> fTexGeni;
    GLPtrAlias<GrGLTexImage2DProc> fTexImage2D;
    GLPtrAlias<GrGLTexParameteriProc> fTexParameteri;
    GLPtrAlias<GrGLTexParameterivProc> fTexParameteriv;
    GLPtrAlias<GrGLTexSubImage2DProc> fTexSubImage2D;
    GLPtrAlias<GrGLTexStorage2DProc> fTexStorage2D;
    GLPtrAlias<GrGLDiscardFramebufferProc> fDiscardFramebuffer;
    GLPtrAlias<GrGLUniform1fProc> fUniform1f;
    GLPtrAlias<GrGLUniform1iProc> fUniform1i;
    GLPtrAlias<GrGLUniform1fvProc> fUniform1fv;
    GLPtrAlias<GrGLUniform1ivProc> fUniform1iv;
    GLPtrAlias<GrGLUniform2fProc> fUniform2f;
    GLPtrAlias<GrGLUniform2iProc> fUniform2i;
    GLPtrAlias<GrGLUniform2fvProc> fUniform2fv;
    GLPtrAlias<GrGLUniform2ivProc> fUniform2iv;
    GLPtrAlias<GrGLUniform3fProc> fUniform3f;
    GLPtrAlias<GrGLUniform3iProc> fUniform3i;
    GLPtrAlias<GrGLUniform3fvProc> fUniform3fv;
    GLPtrAlias<GrGLUniform3ivProc> fUniform3iv;
    GLPtrAlias<GrGLUniform4fProc> fUniform4f;
    GLPtrAlias<GrGLUniform4iProc> fUniform4i;
    GLPtrAlias<GrGLUniform4fvProc> fUniform4fv;
    GLPtrAlias<GrGLUniform4ivProc> fUniform4iv;
    GLPtrAlias<GrGLUniformMatrix2fvProc> fUniformMatrix2fv;
    GLPtrAlias<GrGLUniformMatrix3fvProc> fUniformMatrix3fv;
    GLPtrAlias<GrGLUniformMatrix4fvProc> fUniformMatrix4fv;
    GLPtrAlias<GrGLUnmapBufferProc> fUnmapBuffer;
    GLPtrAlias<GrGLUseProgramProc> fUseProgram;
    GLPtrAlias<GrGLVertexAttrib4fvProc> fVertexAttrib4fv;
    GLPtrAlias<GrGLVertexAttribPointerProc> fVertexAttribPointer;
    GLPtrAlias<GrGLVertexPointerProc> fVertexPointer;
    GLPtrAlias<GrGLViewportProc> fViewport;
    GLPtrAlias<GrGLPathCommandsProc> fPathCommands;
    GLPtrAlias<GrGLPathCoordsProc> fPathCoords;
    GLPtrAlias<GrGLPathSubCommandsProc> fPathSubCommands;
    GLPtrAlias<GrGLPathSubCoordsProc> fPathSubCoords;
    GLPtrAlias<GrGLPathStringProc> fPathString;
    GLPtrAlias<GrGLPathGlyphsProc> fPathGlyphs;
    GLPtrAlias<GrGLPathGlyphRangeProc> fPathGlyphRange;
    GLPtrAlias<GrGLWeightPathsProc> fWeightPaths;
    GLPtrAlias<GrGLCopyPathProc> fCopyPath;
    GLPtrAlias<GrGLInterpolatePathsProc> fInterpolatePaths;
    GLPtrAlias<GrGLTransformPathProc> fTransformPath;
    GLPtrAlias<GrGLPathParameterivProc> fPathParameteriv;
    GLPtrAlias<GrGLPathParameteriProc> fPathParameteri;
    GLPtrAlias<GrGLPathParameterfvProc> fPathParameterfv;
    GLPtrAlias<GrGLPathParameterfProc> fPathParameterf;
    GLPtrAlias<GrGLPathDashArrayProc> fPathDashArray;
    GLPtrAlias<GrGLGenPathsProc> fGenPaths;
    GLPtrAlias<GrGLDeletePathsProc> fDeletePaths;
    GLPtrAlias<GrGLIsPathProc> fIsPath;
    GLPtrAlias<GrGLPathStencilFuncProc> fPathStencilFunc;
    GLPtrAlias<GrGLPathStencilDepthOffsetProc> fPathStencilDepthOffset;
    GLPtrAlias<GrGLStencilFillPathProc> fStencilFillPath;
    GLPtrAlias<GrGLStencilStrokePathProc> fStencilStrokePath;
    GLPtrAlias<GrGLStencilFillPathInstancedProc> fStencilFillPathInstanced;
    GLPtrAlias<GrGLStencilStrokePathInstancedProc> fStencilStrokePathInstanced;
    GLPtrAlias<GrGLPathCoverDepthFuncProc> fPathCoverDepthFunc;
    GLPtrAlias<GrGLPathColorGenProc> fPathColorGen;
    GLPtrAlias<GrGLPathTexGenProc> fPathTexGen;
    GLPtrAlias<GrGLPathFogGenProc> fPathFogGen;
    GLPtrAlias<GrGLCoverFillPathProc> fCoverFillPath;
    GLPtrAlias<GrGLCoverStrokePathProc> fCoverStrokePath;
    GLPtrAlias<GrGLCoverFillPathInstancedProc> fCoverFillPathInstanced;
    GLPtrAlias<GrGLCoverStrokePathInstancedProc> fCoverStrokePathInstanced;
    GLPtrAlias<GrGLGetPathParameterivProc> fGetPathParameteriv;
    GLPtrAlias<GrGLGetPathParameterfvProc> fGetPathParameterfv;
    GLPtrAlias<GrGLGetPathCommandsProc> fGetPathCommands;
    GLPtrAlias<GrGLGetPathCoordsProc> fGetPathCoords;
    GLPtrAlias<GrGLGetPathDashArrayProc> fGetPathDashArray;
    GLPtrAlias<GrGLGetPathMetricsProc> fGetPathMetrics;
    GLPtrAlias<GrGLGetPathMetricRangeProc> fGetPathMetricRange;
    GLPtrAlias<GrGLGetPathSpacingProc> fGetPathSpacing;
    GLPtrAlias<GrGLGetPathColorGenivProc> fGetPathColorGeniv;
    GLPtrAlias<GrGLGetPathColorGenfvProc> fGetPathColorGenfv;
    GLPtrAlias<GrGLGetPathTexGenivProc> fGetPathTexGeniv;
    GLPtrAlias<GrGLGetPathTexGenfvProc> fGetPathTexGenfv;
    GLPtrAlias<GrGLIsPointInFillPathProc> fIsPointInFillPath;
    GLPtrAlias<GrGLIsPointInStrokePathProc> fIsPointInStrokePath;
    GLPtrAlias<GrGLGetPathLengthProc> fGetPathLength;
    GLPtrAlias<GrGLPointAlongPathProc> fPointAlongPath;

    // Per-GL func callback
#if GR_GL_PER_GL_FUNC_CALLBACK
    GrGLInterfaceCallbackProc fCallback;
    GrGLInterfaceCallbackData fCallbackData;
#endif

};

#endif

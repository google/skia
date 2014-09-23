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

/** Function that returns a new interface identical to "interface" but with support for
    test version of GL_EXT_debug_marker. */
const GrGLInterface* GrGLInterfaceAddTestDebugMarker(const GrGLInterface*,
                                                     GrGLInsertEventMarkerProc insertEventMarkerFn,
                                                     GrGLPushGroupMarkerProc pushGroupMarkerFn,
                                                     GrGLPopGroupMarkerProc popGroupMarkerFn);

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

    bool hasExtension(const char ext[]) const { return fExtensions.has(ext); }

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
        GLPtr<GrGLColorMaskProc> fColorMask;
        GLPtr<GrGLCompileShaderProc> fCompileShader;
        GLPtr<GrGLCompressedTexImage2DProc> fCompressedTexImage2D;
        GLPtr<GrGLCompressedTexSubImage2DProc> fCompressedTexSubImage2D;
        GLPtr<GrGLCopyTexSubImage2DProc> fCopyTexSubImage2D;
        GLPtr<GrGLCopyTextureCHROMIUMProc> fCopyTextureCHROMIUM;
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
        GLPtr<GrGLDisableVertexAttribArrayProc> fDisableVertexAttribArray;
        GLPtr<GrGLDrawArraysProc> fDrawArrays;
        GLPtr<GrGLDrawBufferProc> fDrawBuffer;
        GLPtr<GrGLDrawBuffersProc> fDrawBuffers;
        GLPtr<GrGLDrawElementsProc> fDrawElements;
        GLPtr<GrGLEnableProc> fEnable;
        GLPtr<GrGLEnableVertexAttribArrayProc> fEnableVertexAttribArray;
        GLPtr<GrGLEndQueryProc> fEndQuery;
        GLPtr<GrGLFinishProc> fFinish;
        GLPtr<GrGLFlushProc> fFlush;
        GLPtr<GrGLFlushMappedBufferRangeProc> fFlushMappedBufferRange;
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
        GLPtr<GrGLInsertEventMarkerProc> fInsertEventMarker;
        GLPtr<GrGLInvalidateBufferDataProc> fInvalidateBufferData;
        GLPtr<GrGLInvalidateBufferSubDataProc> fInvalidateBufferSubData;
        GLPtr<GrGLInvalidateFramebufferProc> fInvalidateFramebuffer;
        GLPtr<GrGLInvalidateSubFramebufferProc> fInvalidateSubFramebuffer;
        GLPtr<GrGLInvalidateTexImageProc> fInvalidateTexImage;
        GLPtr<GrGLInvalidateTexSubImageProc> fInvalidateTexSubImage;
        GLPtr<GrGLLineWidthProc> fLineWidth;
        GLPtr<GrGLLinkProgramProc> fLinkProgram;
        GLPtr<GrGLMapBufferProc> fMapBuffer;
        GLPtr<GrGLMapBufferRangeProc> fMapBufferRange;
        GLPtr<GrGLMapBufferSubDataProc> fMapBufferSubData;
        GLPtr<GrGLMapTexSubImage2DProc> fMapTexSubImage2D;
        GLPtr<GrGLMatrixLoadfProc> fMatrixLoadf;
        GLPtr<GrGLMatrixLoadIdentityProc> fMatrixLoadIdentity;
        GLPtr<GrGLPixelStoreiProc> fPixelStorei;
        GLPtr<GrGLPopGroupMarkerProc> fPopGroupMarker;
        GLPtr<GrGLPushGroupMarkerProc> fPushGroupMarker;
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
        GLPtr<GrGLUnmapBufferSubDataProc> fUnmapBufferSubData;
        GLPtr<GrGLUnmapTexSubImage2DProc> fUnmapTexSubImage2D;
        GLPtr<GrGLUseProgramProc> fUseProgram;
        GLPtr<GrGLVertexAttrib4fvProc> fVertexAttrib4fv;
        GLPtr<GrGLVertexAttribPointerProc> fVertexAttribPointer;
        GLPtr<GrGLViewportProc> fViewport;

        // Experimental: Functions for GL_NV_path_rendering. These will be
        // alphabetized with the above functions once this is fully supported
        // (and functions we are unlikely to use will possibly be omitted).
        GLPtr<GrGLGetProgramResourceLocationProc> fGetProgramResourceLocation;
        GLPtr<GrGLPathCommandsProc> fPathCommands;
        GLPtr<GrGLPathCoordsProc> fPathCoords;
        GLPtr<GrGLPathParameteriProc> fPathParameteri;
        GLPtr<GrGLPathParameterfProc> fPathParameterf;
        GLPtr<GrGLGenPathsProc> fGenPaths;
        GLPtr<GrGLDeletePathsProc> fDeletePaths;
        GLPtr<GrGLIsPathProc> fIsPath;
        GLPtr<GrGLPathStencilFuncProc> fPathStencilFunc;
        GLPtr<GrGLStencilFillPathProc> fStencilFillPath;
        GLPtr<GrGLStencilStrokePathProc> fStencilStrokePath;
        GLPtr<GrGLStencilFillPathInstancedProc> fStencilFillPathInstanced;
        GLPtr<GrGLStencilStrokePathInstancedProc> fStencilStrokePathInstanced;
        GLPtr<GrGLPathTexGenProc> fPathTexGen;
        GLPtr<GrGLCoverFillPathProc> fCoverFillPath;
        GLPtr<GrGLCoverStrokePathProc> fCoverStrokePath;
        GLPtr<GrGLCoverFillPathInstancedProc> fCoverFillPathInstanced;
        GLPtr<GrGLCoverStrokePathInstancedProc> fCoverStrokePathInstanced;
        // NV_path_rendering v1.2
        GLPtr<GrGLStencilThenCoverFillPathProc> fStencilThenCoverFillPath;
        GLPtr<GrGLStencilThenCoverStrokePathProc> fStencilThenCoverStrokePath;
        GLPtr<GrGLStencilThenCoverFillPathInstancedProc> fStencilThenCoverFillPathInstanced;
        GLPtr<GrGLStencilThenCoverStrokePathInstancedProc> fStencilThenCoverStrokePathInstanced;
        // NV_path_rendering v1.3
        GLPtr<GrGLProgramPathFragmentInputGenProc> fProgramPathFragmentInputGen;
        GLPtr<GrGLPathMemoryGlyphIndexArrayProc> fPathMemoryGlyphIndexArray;
    } fFunctions;

    // Per-GL func callback
#if GR_GL_PER_GL_FUNC_CALLBACK
    GrGLInterfaceCallbackProc fCallback;
    GrGLInterfaceCallbackData fCallbackData;
#endif

    // This exists for internal testing.
    virtual void abandon() const {}
};

#endif

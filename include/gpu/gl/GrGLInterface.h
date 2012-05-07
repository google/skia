
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLInterface_DEFINED
#define GrGLInterface_DEFINED

#include "GrGLFunctions.h"
#include "GrRefCnt.h"

////////////////////////////////////////////////////////////////////////////////

/**
 * Classifies GL contexts (currently as Desktop vs. ES2). This is a bitfield.
 * A GrGLInterface (defined below) may support multiple bindings.
 */
enum GrGLBinding {
    kNone_GrGLBinding = 0x0,

    kDesktop_GrGLBinding = 0x01,
    kES2_GrGLBinding = 0x02,

    // for iteration of GrGLBindings
    kFirstGrGLBinding = kDesktop_GrGLBinding,
    kLastGrGLBinding = kES2_GrGLBinding
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Rather than depend on platform-specific GL headers and libraries, we require
 * the client to provide a struct of GL function pointers. This struct can be
 * specified per-GrContext as a parameter to GrContext::Create. If NULL is
 * passed to Create then the "default" GL interface is used. If the default is
 * also NULL GrContext creation will fail.
 *
 * The default interface is returned by GrGLDefaultInterface. This function's
 * implementation is platform-specific. Several have been provided, along with
 * an implementation that simply returns NULL. It is implementation-specific
 * whether the same GrGLInterface is returned or whether a new one is created
 * at each call. Some platforms may not be able to use a single GrGLInterface
 * because extension function ptrs vary across contexts. Note that GrGLInterface
 * is ref-counted. So if the same object is returned by multiple calls to 
 * GrGLDefaultInterface, each should bump the ref count.
 *
 * By defining GR_GL_PER_GL_CALL_IFACE_CALLBACK to 1 the client can specify a
 * callback function that will be called prior to each GL function call. See
 * comments in GrGLConfig.h
 */

struct GrGLInterface;

const GrGLInterface* GrGLDefaultInterface();

/**
 * Creates a GrGLInterface for a "native" GL context (e.g. WGL on windows,
 * GLX on linux, AGL on Mac). On platforms that have context-specific function
 * pointers for GL extensions (e.g. windows) the returned interface is only 
 * valid for the context that was current at creation.
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
const GrGLInterface* GrGLCreateNullInterface();

/**
 * Creates a debugging GrGLInterface that doesn't draw anything. Used for 
 * finding memory leaks and invalid memory accesses.
 */
const GrGLInterface* GrGLCreateDebugInterface();

#if GR_GL_PER_GL_FUNC_CALLBACK
typedef void (*GrGLInterfaceCallbackProc)(const GrGLInterface*);
typedef intptr_t GrGLInterfaceCallbackData;
#endif

/*
 * The following interface exports the OpenGL entry points used by the system.
 * Use of OpenGL calls is disallowed.  All calls should be invoked through
 * the global instance of this struct, defined above.
 *
 * IMPORTANT NOTE: The OpenGL entry points exposed here include both core GL
 * functions, and extensions.  The system assumes that the address of the
 * extension pointer will be valid across contexts.
 */
struct GR_API GrGLInterface : public GrRefCnt {
private:
    // simple wrapper class that exists only to initialize a pointer to NULL
    template <typename FNPTR_TYPE> class GLPtr {
    public:
        GLPtr() : fPtr(NULL) {}
        GLPtr operator =(FNPTR_TYPE ptr) { fPtr = ptr; return *this; }
        operator FNPTR_TYPE() const { return fPtr; }
    private:
        FNPTR_TYPE fPtr;
    };

public:
    GrGLInterface();

    // Validates that the GrGLInterface supports a binding. This means that
    // the GrGLinterface advertises the binding in fBindingsExported and all
    // the necessary function pointers have been initialized.
    bool validate(GrGLBinding binding) const;

    // Indicator variable specifying the type of GL implementation
    // exported:  GLES2 and/or Desktop.
    GrGLBinding fBindingsExported;

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
    GLPtr<GrGLBlendColorProc> fBlendColor;
    GLPtr<GrGLBlendFuncProc> fBlendFunc;
    GLPtr<GrGLBlendEquationProc> fBlendEquation;
    GLPtr<GrGLBlitFramebufferProc> fBlitFramebuffer;
    GLPtr<GrGLBufferDataProc> fBufferData;
    GLPtr<GrGLBufferSubDataProc> fBufferSubData;
    GLPtr<GrGLCheckFramebufferStatusProc> fCheckFramebufferStatus;
    GLPtr<GrGLClearProc> fClear;
    GLPtr<GrGLClearColorProc> fClearColor;
    GLPtr<GrGLClearStencilProc> fClearStencil;
    GLPtr<GrGLColorMaskProc> fColorMask;
    GLPtr<GrGLColorPointerProc> fColorPointer;
    GLPtr<GrGLCompileShaderProc> fCompileShader;
    GLPtr<GrGLCompressedTexImage2DProc> fCompressedTexImage2D;
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
    GLPtr<GrGLFramebufferRenderbufferProc> fFramebufferRenderbuffer;
    GLPtr<GrGLFramebufferTexture2DProc> fFramebufferTexture2D;
    GLPtr<GrGLFrontFaceProc> fFrontFace;
    GLPtr<GrGLGenBuffersProc> fGenBuffers;
    GLPtr<GrGLGenFramebuffersProc> fGenFramebuffers;
    GLPtr<GrGLGenQueriesProc> fGenQueries;
    GLPtr<GrGLGenRenderbuffersProc> fGenRenderbuffers;
    GLPtr<GrGLGenTexturesProc> fGenTextures;
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
    GLPtr<GrGLGetTexLevelParameterivProc> fGetTexLevelParameteriv;
    GLPtr<GrGLGetUniformLocationProc> fGetUniformLocation;
    GLPtr<GrGLLineWidthProc> fLineWidth;
    GLPtr<GrGLLinkProgramProc> fLinkProgram;
    GLPtr<GrGLMapBufferProc> fMapBuffer;
    GLPtr<GrGLPixelStoreiProc> fPixelStorei;
    GLPtr<GrGLQueryCounterProc> fQueryCounter;
    GLPtr<GrGLReadBufferProc> fReadBuffer;
    GLPtr<GrGLReadPixelsProc> fReadPixels;
    GLPtr<GrGLRenderbufferStorageProc> fRenderbufferStorage;
    GLPtr<GrGLRenderbufferStorageMultisampleProc> fRenderbufferStorageMultisample;
    GLPtr<GrGLRenderbufferStorageMultisampleCoverageProc> fRenderbufferStorageMultisampleCoverage;
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
    GLPtr<GrGLTexSubImage2DProc> fTexSubImage2D;
    GLPtr<GrGLTexStorage2DProc> fTexStorage2D;
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
    GLPtr<GrGLViewportProc> fViewport;

    // Per-GL func callback
#if GR_GL_PER_GL_FUNC_CALLBACK
    GrGLInterfaceCallbackProc fCallback;
    GrGLInterfaceCallbackData fCallbackData;
#endif

};

#endif

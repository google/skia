
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLInterface_DEFINED
#define GrGLInterface_DEFINED

#include "GrGLConfig.h"
#include "GrRefCnt.h"

#if !defined(GR_GL_FUNCTION_TYPE)
    #define GR_GL_FUNCTION_TYPE
#endif

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
 * Helpers for glGetString()
 */

typedef uint32_t GrGLVersion;
typedef uint32_t GrGLSLVersion;

#define GR_GL_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                 static_cast<int>(minor))
#define GR_GLSL_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                   static_cast<int>(minor))

// these variants assume caller already has a string from glGetString()
GrGLVersion GrGLGetVersionFromString(const char* versionString);
GrGLBinding GrGLGetBindingInUseFromString(const char* versionString);
GrGLSLVersion GrGLGetGLSLVersionFromString(const char* versionString);
bool GrGLHasExtensionFromString(const char* ext, const char* extensionString);

// these variants call glGetString()
bool GrGLHasExtension(const GrGLInterface*, const char* ext);
GrGLBinding GrGLGetBindingInUse(const GrGLInterface*);
GrGLVersion GrGLGetVersion(const GrGLInterface*);
GrGLSLVersion GrGLGetGLSLVersion(const GrGLInterface*);

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

typedef unsigned int GrGLenum;
typedef unsigned char GrGLboolean;
typedef unsigned int GrGLbitfield;
typedef signed char GrGLbyte;
typedef char GrGLchar;
typedef short GrGLshort;
typedef int GrGLint;
typedef int GrGLsizei;
typedef int64_t GrGLint64;
typedef unsigned char GrGLubyte;
typedef unsigned short GrGLushort;
typedef unsigned int GrGLuint;
typedef uint64_t GrGLuint64;
typedef float GrGLfloat;
typedef float GrGLclampf;
typedef double GrGLdouble;
typedef double GrGLclampd;
typedef void GrGLvoid;
typedef long GrGLintptr;
typedef long GrGLsizeiptr;

extern "C" {
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLActiveTextureProc)(GrGLenum texture);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLAttachShaderProc)(GrGLuint program, GrGLuint shader);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBeginQueryProc)(GrGLenum target, GrGLuint id);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindAttribLocationProc)(GrGLuint program, GrGLuint index, const char* name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindBufferProc)(GrGLenum target, GrGLuint buffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindFramebufferProc)(GrGLenum target, GrGLuint framebuffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindRenderbufferProc)(GrGLenum target, GrGLuint renderbuffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindTextureProc)(GrGLenum target, GrGLuint texture);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBlendColorProc)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindFragDataLocationProc)(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBindFragDataLocationIndexedProc)(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBlendFuncProc)(GrGLenum sfactor, GrGLenum dfactor);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBlitFramebufferProc)(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBufferDataProc)(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLBufferSubDataProc)(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data);
    typedef GrGLenum (GR_GL_FUNCTION_TYPE *GrGLCheckFramebufferStatusProc)(GrGLenum target);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLClearProc)(GrGLbitfield mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLClearColorProc)(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLClearStencilProc)(GrGLint s);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLColorMaskProc)(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLColorPointerProc)(GrGLint size, GrGLenum type, GrGLsizei stride, const GrGLvoid* pointer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLCompileShaderProc)(GrGLuint shader);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLCompressedTexImage2DProc)(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data);
    typedef GrGLuint (GR_GL_FUNCTION_TYPE *GrGLCreateProgramProc)(void);
    typedef GrGLuint (GR_GL_FUNCTION_TYPE *GrGLCreateShaderProc)(GrGLenum type);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLCullFaceProc)(GrGLenum mode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteBuffersProc)(GrGLsizei n, const GrGLuint* buffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteFramebuffersProc)(GrGLsizei n, const GrGLuint *framebuffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteProgramProc)(GrGLuint program);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteQueriesProc)(GrGLsizei n, const GrGLuint *ids);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteRenderbuffersProc)(GrGLsizei n, const GrGLuint *renderbuffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteShaderProc)(GrGLuint shader);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteTexturesProc)(GrGLsizei n, const GrGLuint* textures);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDepthMaskProc)(GrGLboolean flag);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDisableProc)(GrGLenum cap);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDisableVertexAttribArrayProc)(GrGLuint index);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDrawArraysProc)(GrGLenum mode, GrGLint first, GrGLsizei count);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDrawBufferProc)(GrGLenum mode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDrawBuffersProc)(GrGLsizei n, const GrGLenum* bufs);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLDrawElementsProc)(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLEnableProc)(GrGLenum cap);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLEnableVertexAttribArrayProc)(GrGLuint index);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLEndQueryProc)(GrGLenum target);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLFinishProc)();
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLFlushProc)();
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLFramebufferRenderbufferProc)(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLFramebufferTexture2DProc)(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLFrontFaceProc)(GrGLenum mode);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGenBuffersProc)(GrGLsizei n, GrGLuint* buffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGenFramebuffersProc)(GrGLsizei n, GrGLuint *framebuffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGenQueriesProc)(GrGLsizei n, GrGLuint *ids);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGenRenderbuffersProc)(GrGLsizei n, GrGLuint *renderbuffers);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGenTexturesProc)(GrGLsizei n, GrGLuint* textures);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetBufferParameterivProc)(GrGLenum target, GrGLenum pname, GrGLint* params);
    typedef GrGLenum (GR_GL_FUNCTION_TYPE *GrGLGetErrorProc)();
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetFramebufferAttachmentParameterivProc)(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetIntegervProc)(GrGLenum pname, GrGLint* params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetProgramInfoLogProc)(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, char* infolog);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetProgramivProc)(GrGLuint program, GrGLenum pname, GrGLint* params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetQueryivProc)(GrGLenum GLtarget, GrGLenum pname, GrGLint *params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetQueryObjecti64vProc)(GrGLuint id, GrGLenum pname, GrGLint64 *params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetQueryObjectivProc)(GrGLuint id, GrGLenum pname, GrGLint *params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetQueryObjectui64vProc)(GrGLuint id, GrGLenum pname, GrGLuint64 *params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetQueryObjectuivProc)(GrGLuint id, GrGLenum pname, GrGLuint *params);    
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetRenderbufferParameterivProc)(GrGLenum target, GrGLenum pname, GrGLint* params);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetShaderInfoLogProc)(GrGLuint shader, GrGLsizei bufsize, GrGLsizei* length, char* infolog);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetShaderivProc)(GrGLuint shader, GrGLenum pname, GrGLint* params);
    typedef const GrGLubyte* (GR_GL_FUNCTION_TYPE *GrGLGetStringProc)(GrGLenum name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLGetTexLevelParameterivProc)(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params);
    typedef GrGLint (GR_GL_FUNCTION_TYPE *GrGLGetUniformLocationProc)(GrGLuint program, const char* name);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLLineWidthProc)(GrGLfloat width);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLLinkProgramProc)(GrGLuint program);
    typedef GrGLvoid* (GR_GL_FUNCTION_TYPE *GrGLMapBufferProc)(GrGLenum target, GrGLenum access);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLPixelStoreiProc)(GrGLenum pname, GrGLint param);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLQueryCounterProc)(GrGLuint id, GrGLenum target);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLReadBufferProc)(GrGLenum src);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLReadPixelsProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLRenderbufferStorageProc)(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLRenderbufferStorageMultisampleProc)(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLRenderbufferStorageMultisampleCoverageProc)(GrGLenum target, GrGLsizei coverageSamples, GrGLsizei colorSamples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLResolveMultisampleFramebufferProc)();
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLScissorProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLShaderSourceProc)(GrGLuint shader, GrGLsizei count, const char** str, const GrGLint* length);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilFuncProc)(GrGLenum func, GrGLint ref, GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilFuncSeparateProc)(GrGLenum face, GrGLenum func, GrGLint ref, GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilMaskProc)(GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilMaskSeparateProc)(GrGLenum face, GrGLuint mask);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilOpProc)(GrGLenum fail, GrGLenum zfail, GrGLenum zpass);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilOpSeparateProc)(GrGLenum face, GrGLenum fail, GrGLenum zfail, GrGLenum zpass);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLTexImage2DProc)(GrGLenum target, GrGLint level, GrGLint internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLTexParameteriProc)(GrGLenum target, GrGLenum pname, GrGLint param);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLTexStorage2DProc)(GrGLenum target, GrGLsizei levels, GrGLenum internalformat, GrGLsizei width, GrGLsizei height);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLTexSubImage2DProc)(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform1fProc)(GrGLint location, GrGLfloat v0);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform1iProc)(GrGLint location, GrGLint v0);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform1fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform1ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform2fProc)(GrGLint location, GrGLfloat v0, GrGLfloat v1);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform2iProc)(GrGLint location, GrGLint v0, GrGLint v1);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform2fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform2ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform3fProc)(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform3iProc)(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform3fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform3ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform4fProc)(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2, GrGLfloat v3);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform4iProc)(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform4fvProc)(GrGLint location, GrGLsizei count, const GrGLfloat* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform4ivProc)(GrGLint location, GrGLsizei count, const GrGLint* v);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniformMatrix2fvProc)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniformMatrix3fvProc)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUniformMatrix4fvProc)(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value);
    typedef GrGLboolean (GR_GL_FUNCTION_TYPE *GrGLUnmapBufferProc)(GrGLenum target);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLUseProgramProc)(GrGLuint program);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLVertexAttrib4fvProc)(GrGLuint indx, const GrGLfloat* values);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLVertexAttribPointerProc)(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE *GrGLViewportProc)(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height);
}  // extern "C"

#if GR_GL_PER_GL_FUNC_CALLBACK
typedef void (*GrGLInterfaceCallbackProc)(const GrGLInterface*);
typedef intptr_t GrGLInterfaceCallbackData;
#endif


enum GrGLCapability {
    kProbe_GrGLCapability = -1
};

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
    // simple wrapper class that exists only to initialize a pointers to NULL
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

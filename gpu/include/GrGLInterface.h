/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrGLInterface_DEFINED
#define GrGLInterface_DEFINED

#include "GrGLPlatformIncludes.h"

#if !defined(GR_GL_FUNCTION_TYPE)
    #define GR_GL_FUNCTION_TYPE
#endif

////////////////////////////////////////////////////////////////////////////////

/**
 * Helpers for glGetString()
 */
bool has_gl_extension(const char* ext);
void gl_version(int* major, int* minor);

////////////////////////////////////////////////////////////////////////////////

/*
 * Routines managing the global interface used to invoke OpenGL calls.
 */
struct GrGLInterface;
extern GrGLInterface* GrGLGetGLInterface();
extern void GrGLSetGLInterface(GrGLInterface* gl_interface);

/*
 * Populates the global GrGLInterface pointer with an instance pointing to the
 * GL implementation linked with the executable.
 */
extern void GrGLSetDefaultGLInterface();

extern "C" {
/*
 * The following interface exports the OpenGL entry points used by the system.
 * Use of OpenGL calls is disallowed.  All calls should be invoked through
 * the global instance of this struct, defined above.
 *
 * IMPORTANT NOTE: The OpenGL entry points exposed here include both core GL
 * functions, and extensions.  The system assumes that the address of the
 * extension pointer will be valid across contexts.
 */
struct GrGLInterface {
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLActiveTextureProc)(GLenum texture);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLAttachShaderProc)(GLuint program, GLuint shader);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBindAttribLocationProc)(GLuint program, GLuint index, const char* name);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBindBufferProc)(GLenum target, GLuint buffer);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBindTextureProc)(GLenum target, GLuint texture);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBlendColorProc)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBlendFuncProc)(GLenum sfactor, GLenum dfactor);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBufferDataProc)(GLenum target, GLsizei size, const void* data, GLenum usage);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBufferSubDataProc)(GLenum target, GLint offset, GLsizei size, const void* data);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLClearProc)(GLbitfield mask);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLClearColorProc)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLClearStencilProc)(GLint s);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLClientActiveTextureProc)(GLenum texture);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLColor4ubProc)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLColorMaskProc)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLColorPointerProc)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLCompileShaderProc)(GLuint shader);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLCompressedTexImage2DProc)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
    typedef GLuint (GR_GL_FUNCTION_TYPE *GrGLCreateProgramProc)(void);
    typedef GLuint (GR_GL_FUNCTION_TYPE *GrGLCreateShaderProc)(GLenum type);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLCullFaceProc)(GLenum mode);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteBuffersProc)(GLsizei n, const GLuint* buffers);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteProgramProc)(GLuint program);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteShaderProc)(GLuint shader);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteTexturesProc)(GLsizei n, const GLuint* textures);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDepthMaskProc)(GLboolean flag);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDisableProc)(GLenum cap);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDisableClientStateProc)(GLenum array);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDisableVertexAttribArrayProc)(GLuint index);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDrawArraysProc)(GLenum mode, GLint first, GLsizei count);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDrawElementsProc)(GLenum mode, GLsizei count, GLenum type, const void* indices);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLEnableProc)(GLenum cap);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLEnableClientStateProc)(GLenum cap);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLEnableVertexAttribArrayProc)(GLuint index);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLFrontFaceProc)(GLenum mode);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGenBuffersProc)(GLsizei n, GLuint* buffers);
    typedef GLenum (GR_GL_FUNCTION_TYPE *GrGLGetErrorProc)(void);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGenTexturesProc)(GLsizei n, GLuint* textures);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGetBufferParameterivProc)(GLenum target, GLenum pname, GLint* params);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGetIntegervProc)(GLenum pname, GLint* params);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGetProgramInfoLogProc)(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGetProgramivProc)(GLuint program, GLenum pname, GLint* params);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGetShaderInfoLogProc)(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGetShaderivProc)(GLuint shader, GLenum pname, GLint* params);
    typedef const GLubyte* (GR_GL_FUNCTION_TYPE *GrGLGetStringProc)(GLenum name);
    typedef GLint (GR_GL_FUNCTION_TYPE *GrGLGetUniformLocationProc)(GLuint program, const char* name);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLLineWidthProc)(GLfloat width);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLLinkProgramProc)(GLuint program);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLLoadMatrixfProc)(const GLfloat* m);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLMatrixModeProc)(GLenum mode);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLPixelStoreiProc)(GLenum pname, GLint param);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLPointSizeProc)(GLfloat size);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLReadPixelsProc)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLScissorProc)(GLint x, GLint y, GLsizei width, GLsizei height);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLShadeModelProc)(GLenum mode);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLShaderSourceProc)(GLuint shader, GLsizei count, const char** str, const GLint* length);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilFuncProc)(GLenum func, GLint ref, GLuint mask);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilFuncSeparateProc)(GLenum face, GLenum func, GLint ref, GLuint mask);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilMaskProc)(GLuint mask);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilMaskSeparateProc)(GLenum face, GLuint mask);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilOpProc)(GLenum fail, GLenum zfail, GLenum zpass);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLStencilOpSeparateProc)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLTexCoordPointerProc)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLTexEnviProc)(GLenum target, GLenum pname, GLint param);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLTexImage2DProc)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLTexParameteriProc)(GLenum target, GLenum pname, GLint param);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLTexSubImage2DProc)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform1fvProc)(GLint location, GLsizei count, const GLfloat* v);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform1iProc)(GLint location, GLint x);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLUniform4fvProc)(GLint location, GLsizei count, const GLfloat* v);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLUniformMatrix3fvProc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLUseProgramProc)(GLuint program);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLVertexAttrib4fvProc)(GLuint indx, const GLfloat* values);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLVertexAttribPointerProc)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLVertexPointerProc)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLViewportProc)(GLint x, GLint y, GLsizei width, GLsizei height);

    // FBO Extension Functions
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBindFramebufferProc)(GLenum target, GLuint framebuffer);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBindRenderbufferProc)(GLenum target, GLuint renderbuffer);
    typedef GLenum (GR_GL_FUNCTION_TYPE *GrGLCheckFramebufferStatusProc)(GLenum target);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteFramebuffersProc)(GLsizei n, const GLuint *framebuffers);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLDeleteRenderbuffersProc)(GLsizei n, const GLuint *renderbuffers);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLFramebufferRenderbufferProc)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLFramebufferTexture2DProc)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGenFramebuffersProc)(GLsizei n, GLuint *framebuffers);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLGenRenderbuffersProc)(GLsizei n, GLuint *renderbuffers);
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLRenderbufferStorageProc)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

    // Multisampling Extension Functions
    // same prototype for ARB_FBO, EXT_FBO, GL 3.0, & Apple ES extension
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLRenderbufferStorageMultisampleProc)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    // desktop: ext_fbo_blit, arb_fbo, gl 3.0
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLBlitFramebufferProc)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    // apple's es extension
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLResolveMultisampleFramebufferProc)();

    // IMG'e es extension
    typedef GLvoid (GR_GL_FUNCTION_TYPE *GrGLFramebufferTexture2DMultisampleProc)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);

    // Buffer mapping (extension in ES).
    typedef GLvoid* (GR_GL_FUNCTION_TYPE *GrGLMapBufferProc)(GLenum target, GLenum access);
    typedef GLboolean (GR_GL_FUNCTION_TYPE *GrGLUnmapBufferProc)(GLenum target);

    GrGLActiveTextureProc fActiveTexture;
    GrGLAttachShaderProc fAttachShader;
    GrGLBindAttribLocationProc fBindAttribLocation;
    GrGLBindBufferProc fBindBuffer;
    GrGLBindTextureProc fBindTexture;
    GrGLBlendFuncProc fBlendFunc;
    GrGLBlendColorProc fBlendColor;
    GrGLBufferDataProc fBufferData;
    GrGLBufferSubDataProc fBufferSubData;
    GrGLClearProc fClear;
    GrGLClearColorProc fClearColor;
    GrGLClearStencilProc fClearStencil;
    GrGLClientActiveTextureProc fClientActiveTexture;
    GrGLColor4ubProc fColor4ub;
    GrGLColorMaskProc fColorMask;
    GrGLColorPointerProc fColorPointer;
    GrGLCompileShaderProc fCompileShader;
    GrGLCompressedTexImage2DProc fCompressedTexImage2D;
    GrGLCreateProgramProc fCreateProgram;
    GrGLCreateShaderProc fCreateShader;
    GrGLCullFaceProc fCullFace;
    GrGLDeleteBuffersProc fDeleteBuffers;
    GrGLDeleteProgramProc fDeleteProgram;
    GrGLDeleteShaderProc fDeleteShader;
    GrGLDeleteTexturesProc fDeleteTextures;
    GrGLDepthMaskProc fDepthMask;
    GrGLDisableProc fDisable;
    GrGLDisableClientStateProc fDisableClientState;
    GrGLDisableVertexAttribArrayProc fDisableVertexAttribArray;
    GrGLDrawArraysProc fDrawArrays;
    GrGLDrawElementsProc fDrawElements;
    GrGLEnableProc fEnable;
    GrGLEnableClientStateProc fEnableClientState;
    GrGLEnableVertexAttribArrayProc fEnableVertexAttribArray;
    GrGLFrontFaceProc fFrontFace;
    GrGLGenBuffersProc fGenBuffers;
    GrGLGenTexturesProc fGenTextures;
    GrGLGetBufferParameterivProc fGetBufferParameteriv;
    GrGLGetErrorProc fGetError;
    GrGLGetIntegervProc fGetIntegerv;
    GrGLGetProgramInfoLogProc fGetProgramInfoLog;
    GrGLGetProgramivProc fGetProgramiv;
    GrGLGetShaderInfoLogProc fGetShaderInfoLog;
    GrGLGetShaderivProc fGetShaderiv;
    GrGLGetStringProc fGetString;
    GrGLGetUniformLocationProc fGetUniformLocation;
    GrGLLineWidthProc fLineWidth;
    GrGLLinkProgramProc fLinkProgram;
    GrGLLoadMatrixfProc fLoadMatrixf;
    GrGLMatrixModeProc fMatrixMode;
    GrGLPixelStoreiProc fPixelStorei;
    GrGLPointSizeProc fPointSize;
    GrGLReadPixelsProc fReadPixels;
    GrGLScissorProc fScissor;
    GrGLShadeModelProc fShadeModel;
    GrGLShaderSourceProc fShaderSource;
    GrGLStencilFuncProc fStencilFunc;
    GrGLStencilFuncSeparateProc fStencilFuncSeparate;
    GrGLStencilMaskProc fStencilMask;
    GrGLStencilMaskSeparateProc fStencilMaskSeparate;
    GrGLStencilOpProc fStencilOp;
    GrGLStencilOpSeparateProc fStencilOpSeparate;
    GrGLTexCoordPointerProc fTexCoordPointer;
    GrGLTexEnviProc fTexEnvi;
    GrGLTexImage2DProc fTexImage2D;
    GrGLTexParameteriProc fTexParameteri;
    GrGLTexSubImage2DProc fTexSubImage2D;
    GrGLUniform1fvProc fUniform1fv;
    GrGLUniform1iProc fUniform1i;
    GrGLUniform4fvProc fUniform4fv;
    GrGLUniformMatrix3fvProc fUniformMatrix3fv;
    GrGLUseProgramProc fUseProgram;
    GrGLVertexAttrib4fvProc fVertexAttrib4fv;
    GrGLVertexAttribPointerProc fVertexAttribPointer;
    GrGLVertexPointerProc fVertexPointer;
    GrGLViewportProc fViewport;

    // FBO Extension Functions
    GrGLBindFramebufferProc fBindFramebuffer;
    GrGLBindRenderbufferProc fBindRenderbuffer;
    GrGLCheckFramebufferStatusProc fCheckFramebufferStatus;
    GrGLDeleteFramebuffersProc fDeleteFramebuffers;
    GrGLDeleteRenderbuffersProc fDeleteRenderbuffers;
    GrGLFramebufferRenderbufferProc fFramebufferRenderbuffer;
    GrGLFramebufferTexture2DProc fFramebufferTexture2D;
    GrGLGenFramebuffersProc fGenFramebuffers;
    GrGLGenRenderbuffersProc fGenRenderbuffers;
    GrGLRenderbufferStorageProc fRenderbufferStorage;

    // Multisampling Extension Functions
    // same prototype for ARB_FBO, EXT_FBO, GL 3.0, & Apple ES extension
    GrGLRenderbufferStorageMultisampleProc fRenderbufferStorageMultisample;
    // desktop: ext_fbo_blit, arb_fbo, gl 3.0
    GrGLBlitFramebufferProc fBlitFramebuffer;
    // apple's es extension
    GrGLResolveMultisampleFramebufferProc fResolveMultisampleFramebuffer;

    // IMG'e es extension
    GrGLFramebufferTexture2DMultisampleProc fFramebufferTexture2DMultisample;

    // Buffer mapping (extension in ES).
    GrGLMapBufferProc fMapBuffer;
    GrGLUnmapBufferProc fUnmapBuffer;
};

}  // extern "C"

#endif

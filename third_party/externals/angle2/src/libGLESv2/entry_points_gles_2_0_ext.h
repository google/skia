//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_2_0_ext.h : Defines the GLES 2.0 extension entry points.

#ifndef LIBGLESV2_ENTRYPOINTGLES20EXT_H_
#define LIBGLESV2_ENTRYPOINTGLES20EXT_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <export.h>

namespace gl
{

// GL_ANGLE_framebuffer_blit
ANGLE_EXPORT void GL_APIENTRY BlitFramebufferANGLE(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

// GL_ANGLE_framebuffer_multisample
ANGLE_EXPORT void GL_APIENTRY RenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

// GL_EXT_discard_framebuffer
ANGLE_EXPORT void GL_APIENTRY DiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments);

// GL_NV_fence
ANGLE_EXPORT void GL_APIENTRY DeleteFencesNV(GLsizei n, const GLuint* fences);
ANGLE_EXPORT void GL_APIENTRY GenFencesNV(GLsizei n, GLuint* fences);
ANGLE_EXPORT GLboolean GL_APIENTRY IsFenceNV(GLuint fence);
ANGLE_EXPORT GLboolean GL_APIENTRY TestFenceNV(GLuint fence);
ANGLE_EXPORT void GL_APIENTRY GetFenceivNV(GLuint fence, GLenum pname, GLint *params);
ANGLE_EXPORT void GL_APIENTRY FinishFenceNV(GLuint fence);
ANGLE_EXPORT void GL_APIENTRY SetFenceNV(GLuint fence, GLenum condition);

// GL_ANGLE_translated_shader_source
ANGLE_EXPORT void GL_APIENTRY GetTranslatedShaderSourceANGLE(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);

// GL_EXT_texture_storage
ANGLE_EXPORT void GL_APIENTRY TexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

// GL_EXT_robustness
ANGLE_EXPORT GLenum GL_APIENTRY GetGraphicsResetStatusEXT(void);
ANGLE_EXPORT void GL_APIENTRY ReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
ANGLE_EXPORT void GL_APIENTRY GetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, float *params);
ANGLE_EXPORT void GL_APIENTRY GetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint *params);

// GL_EXT_occlusion_query_boolean
ANGLE_EXPORT void GL_APIENTRY GenQueriesEXT(GLsizei n, GLuint *ids);
ANGLE_EXPORT void GL_APIENTRY DeleteQueriesEXT(GLsizei n, const GLuint *ids);
ANGLE_EXPORT GLboolean GL_APIENTRY IsQueryEXT(GLuint id);
ANGLE_EXPORT void GL_APIENTRY BeginQueryEXT(GLenum target, GLuint id);
ANGLE_EXPORT void GL_APIENTRY EndQueryEXT(GLenum target);
ANGLE_EXPORT void GL_APIENTRY GetQueryivEXT(GLenum target, GLenum pname, GLint *params);
ANGLE_EXPORT void GL_APIENTRY GetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params);

// GL_EXT_draw_buffers
ANGLE_EXPORT void GL_APIENTRY DrawBuffersEXT(GLsizei n, const GLenum *bufs);

// GL_ANGLE_instanced_arrays
ANGLE_EXPORT void GL_APIENTRY DrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
ANGLE_EXPORT void GL_APIENTRY DrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
ANGLE_EXPORT void GL_APIENTRY VertexAttribDivisorANGLE(GLuint index, GLuint divisor);

// GL_OES_get_program_binary
ANGLE_EXPORT void GL_APIENTRY GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
ANGLE_EXPORT void GL_APIENTRY ProgramBinaryOES(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLint length);

// GL_OES_mapbuffer
ANGLE_EXPORT void *GL_APIENTRY MapBufferOES(GLenum target, GLenum access);
ANGLE_EXPORT GLboolean GL_APIENTRY UnmapBufferOES(GLenum target);
ANGLE_EXPORT void GL_APIENTRY GetBufferPointervOES(GLenum target, GLenum pname, GLvoid **params);

// GL_EXT_map_buffer_range
ANGLE_EXPORT void *GL_APIENTRY MapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
ANGLE_EXPORT void GL_APIENTRY FlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length);

// GL_EXT_debug_marker
ANGLE_EXPORT void GL_APIENTRY InsertEventMarkerEXT(GLsizei length, const char *marker);
ANGLE_EXPORT void GL_APIENTRY PushGroupMarkerEXT(GLsizei length, const char *marker);
ANGLE_EXPORT void GL_APIENTRY PopGroupMarkerEXT();

// GL_OES_EGL_image
ANGLE_EXPORT void GL_APIENTRY EGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image);
ANGLE_EXPORT void GL_APIENTRY EGLImageTargetRenderbufferStorageOES(GLenum target,
                                                                   GLeglImageOES image);
}

#endif // LIBGLESV2_ENTRYPOINTGLES20EXT_H_

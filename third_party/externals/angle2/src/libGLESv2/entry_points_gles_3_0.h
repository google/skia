//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_3_0.h : Defines the GLES 3.0 entry points.

#ifndef LIBGLESV2_ENTRYPOINTGLES30_H_
#define LIBGLESV2_ENTRYPOINTGLES30_H_

#include <GLES3/gl3.h>
#include <export.h>

namespace gl
{

ANGLE_EXPORT void GL_APIENTRY ReadBuffer(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
ANGLE_EXPORT void GL_APIENTRY TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void GL_APIENTRY TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void GL_APIENTRY CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT void GL_APIENTRY CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void GL_APIENTRY CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void GL_APIENTRY GenQueries(GLsizei n, GLuint* ids);
ANGLE_EXPORT void GL_APIENTRY DeleteQueries(GLsizei n, const GLuint* ids);
ANGLE_EXPORT GLboolean GL_APIENTRY IsQuery(GLuint id);
ANGLE_EXPORT void GL_APIENTRY BeginQuery(GLenum target, GLuint id);
ANGLE_EXPORT void GL_APIENTRY EndQuery(GLenum target);
ANGLE_EXPORT void GL_APIENTRY GetQueryiv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params);
ANGLE_EXPORT GLboolean GL_APIENTRY UnmapBuffer(GLenum target);
ANGLE_EXPORT void GL_APIENTRY GetBufferPointerv(GLenum target, GLenum pname, GLvoid** params);
ANGLE_EXPORT void GL_APIENTRY DrawBuffers(GLsizei n, const GLenum* bufs);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
ANGLE_EXPORT void GL_APIENTRY RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
ANGLE_EXPORT void GL_APIENTRY FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
ANGLE_EXPORT GLvoid* GL_APIENTRY MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
ANGLE_EXPORT void GL_APIENTRY FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
ANGLE_EXPORT void GL_APIENTRY BindVertexArray(GLuint array);
ANGLE_EXPORT void GL_APIENTRY DeleteVertexArrays(GLsizei n, const GLuint* arrays);
ANGLE_EXPORT void GL_APIENTRY GenVertexArrays(GLsizei n, GLuint* arrays);
ANGLE_EXPORT GLboolean GL_APIENTRY IsVertexArray(GLuint array);
ANGLE_EXPORT void GL_APIENTRY GetIntegeri_v(GLenum target, GLuint index, GLint* data);
ANGLE_EXPORT void GL_APIENTRY BeginTransformFeedback(GLenum primitiveMode);
ANGLE_EXPORT void GL_APIENTRY EndTransformFeedback(void);
ANGLE_EXPORT void GL_APIENTRY BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
ANGLE_EXPORT void GL_APIENTRY BindBufferBase(GLenum target, GLuint index, GLuint buffer);
ANGLE_EXPORT void GL_APIENTRY TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
ANGLE_EXPORT void GL_APIENTRY GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
ANGLE_EXPORT void GL_APIENTRY VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribIiv(GLuint index, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params);
ANGLE_EXPORT void GL_APIENTRY VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
ANGLE_EXPORT void GL_APIENTRY VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
ANGLE_EXPORT void GL_APIENTRY VertexAttribI4iv(GLuint index, const GLint* v);
ANGLE_EXPORT void GL_APIENTRY VertexAttribI4uiv(GLuint index, const GLuint* v);
ANGLE_EXPORT void GL_APIENTRY GetUniformuiv(GLuint program, GLint location, GLuint* params);
ANGLE_EXPORT GLint GL_APIENTRY GetFragDataLocation(GLuint program, const GLchar *name);
ANGLE_EXPORT void GL_APIENTRY Uniform1ui(GLint location, GLuint v0);
ANGLE_EXPORT void GL_APIENTRY Uniform2ui(GLint location, GLuint v0, GLuint v1);
ANGLE_EXPORT void GL_APIENTRY Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
ANGLE_EXPORT void GL_APIENTRY Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
ANGLE_EXPORT void GL_APIENTRY Uniform1uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void GL_APIENTRY Uniform2uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void GL_APIENTRY Uniform3uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void GL_APIENTRY Uniform4uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void GL_APIENTRY ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value);
ANGLE_EXPORT void GL_APIENTRY ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value);
ANGLE_EXPORT void GL_APIENTRY ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
ANGLE_EXPORT const GLubyte *GL_APIENTRY GetStringi(GLenum name, GLuint index);
ANGLE_EXPORT void GL_APIENTRY CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
ANGLE_EXPORT void GL_APIENTRY GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
ANGLE_EXPORT void GL_APIENTRY GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
ANGLE_EXPORT GLuint GL_APIENTRY GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
ANGLE_EXPORT void GL_APIENTRY GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
ANGLE_EXPORT void GL_APIENTRY UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
ANGLE_EXPORT void GL_APIENTRY DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
ANGLE_EXPORT void GL_APIENTRY DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
ANGLE_EXPORT GLsync GL_APIENTRY FenceSync_(GLenum condition, GLbitfield flags);
ANGLE_EXPORT GLboolean GL_APIENTRY IsSync(GLsync sync);
ANGLE_EXPORT void GL_APIENTRY DeleteSync(GLsync sync);
ANGLE_EXPORT GLenum GL_APIENTRY ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
ANGLE_EXPORT void GL_APIENTRY WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
ANGLE_EXPORT void GL_APIENTRY GetInteger64v(GLenum pname, GLint64* params);
ANGLE_EXPORT void GL_APIENTRY GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
ANGLE_EXPORT void GL_APIENTRY GetInteger64i_v(GLenum target, GLuint index, GLint64* data);
ANGLE_EXPORT void GL_APIENTRY GetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params);
ANGLE_EXPORT void GL_APIENTRY GenSamplers(GLsizei count, GLuint* samplers);
ANGLE_EXPORT void GL_APIENTRY DeleteSamplers(GLsizei count, const GLuint* samplers);
ANGLE_EXPORT GLboolean GL_APIENTRY IsSampler(GLuint sampler);
ANGLE_EXPORT void GL_APIENTRY BindSampler(GLuint unit, GLuint sampler);
ANGLE_EXPORT void GL_APIENTRY SamplerParameteri(GLuint sampler, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param);
ANGLE_EXPORT void GL_APIENTRY SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param);
ANGLE_EXPORT void GL_APIENTRY GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY VertexAttribDivisor(GLuint index, GLuint divisor);
ANGLE_EXPORT void GL_APIENTRY BindTransformFeedback(GLenum target, GLuint id);
ANGLE_EXPORT void GL_APIENTRY DeleteTransformFeedbacks(GLsizei n, const GLuint* ids);
ANGLE_EXPORT void GL_APIENTRY GenTransformFeedbacks(GLsizei n, GLuint* ids);
ANGLE_EXPORT GLboolean GL_APIENTRY IsTransformFeedback(GLuint id);
ANGLE_EXPORT void GL_APIENTRY PauseTransformFeedback(void);
ANGLE_EXPORT void GL_APIENTRY ResumeTransformFeedback(void);
ANGLE_EXPORT void GL_APIENTRY GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
ANGLE_EXPORT void GL_APIENTRY ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
ANGLE_EXPORT void GL_APIENTRY ProgramParameteri(GLuint program, GLenum pname, GLint value);
ANGLE_EXPORT void GL_APIENTRY InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments);
ANGLE_EXPORT void GL_APIENTRY InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT void GL_APIENTRY TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
ANGLE_EXPORT void GL_APIENTRY TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
ANGLE_EXPORT void GL_APIENTRY GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

}

#endif // LIBGLESV2_ENTRYPOINTGLES30_H_

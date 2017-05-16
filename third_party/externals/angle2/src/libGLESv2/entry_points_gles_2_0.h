//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_2_0.h : Defines the GLES 2.0 entry points.

#ifndef LIBGLESV2_ENTRYPOINTGLES20_H_
#define LIBGLESV2_ENTRYPOINTGLES20_H_

#include <GLES2/gl2.h>
#include <export.h>

namespace gl
{

ANGLE_EXPORT void GL_APIENTRY ActiveTexture(GLenum texture);
ANGLE_EXPORT void GL_APIENTRY AttachShader(GLuint program, GLuint shader);
ANGLE_EXPORT void GL_APIENTRY BindAttribLocation(GLuint program, GLuint index, const GLchar* name);
ANGLE_EXPORT void GL_APIENTRY BindBuffer(GLenum target, GLuint buffer);
ANGLE_EXPORT void GL_APIENTRY BindFramebuffer(GLenum target, GLuint framebuffer);
ANGLE_EXPORT void GL_APIENTRY BindRenderbuffer(GLenum target, GLuint renderbuffer);
ANGLE_EXPORT void GL_APIENTRY BindTexture(GLenum target, GLuint texture);
ANGLE_EXPORT void GL_APIENTRY BlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
ANGLE_EXPORT void GL_APIENTRY BlendEquation(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
ANGLE_EXPORT void GL_APIENTRY BlendFunc(GLenum sfactor, GLenum dfactor);
ANGLE_EXPORT void GL_APIENTRY BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
ANGLE_EXPORT void GL_APIENTRY BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
ANGLE_EXPORT void GL_APIENTRY BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
ANGLE_EXPORT GLenum GL_APIENTRY CheckFramebufferStatus(GLenum target);
ANGLE_EXPORT void GL_APIENTRY Clear(GLbitfield mask);
ANGLE_EXPORT void GL_APIENTRY ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
ANGLE_EXPORT void GL_APIENTRY ClearDepthf(GLfloat depth);
ANGLE_EXPORT void GL_APIENTRY ClearStencil(GLint s);
ANGLE_EXPORT void GL_APIENTRY ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
ANGLE_EXPORT void GL_APIENTRY CompileShader(GLuint shader);
ANGLE_EXPORT void GL_APIENTRY CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void GL_APIENTRY CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void GL_APIENTRY CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
ANGLE_EXPORT void GL_APIENTRY CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT GLuint GL_APIENTRY CreateProgram(void);
ANGLE_EXPORT GLuint GL_APIENTRY CreateShader(GLenum type);
ANGLE_EXPORT void GL_APIENTRY CullFace(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY DeleteBuffers(GLsizei n, const GLuint* buffers);
ANGLE_EXPORT void GL_APIENTRY DeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
ANGLE_EXPORT void GL_APIENTRY DeleteProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
ANGLE_EXPORT void GL_APIENTRY DeleteShader(GLuint shader);
ANGLE_EXPORT void GL_APIENTRY DeleteTextures(GLsizei n, const GLuint* textures);
ANGLE_EXPORT void GL_APIENTRY DepthFunc(GLenum func);
ANGLE_EXPORT void GL_APIENTRY DepthMask(GLboolean flag);
ANGLE_EXPORT void GL_APIENTRY DepthRangef(GLfloat n, GLfloat f);
ANGLE_EXPORT void GL_APIENTRY DetachShader(GLuint program, GLuint shader);
ANGLE_EXPORT void GL_APIENTRY Disable(GLenum cap);
ANGLE_EXPORT void GL_APIENTRY DisableVertexAttribArray(GLuint index);
ANGLE_EXPORT void GL_APIENTRY DrawArrays(GLenum mode, GLint first, GLsizei count);
ANGLE_EXPORT void GL_APIENTRY DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
ANGLE_EXPORT void GL_APIENTRY Enable(GLenum cap);
ANGLE_EXPORT void GL_APIENTRY EnableVertexAttribArray(GLuint index);
ANGLE_EXPORT void GL_APIENTRY Finish(void);
ANGLE_EXPORT void GL_APIENTRY Flush(void);
ANGLE_EXPORT void GL_APIENTRY FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
ANGLE_EXPORT void GL_APIENTRY FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
ANGLE_EXPORT void GL_APIENTRY FrontFace(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY GenBuffers(GLsizei n, GLuint* buffers);
ANGLE_EXPORT void GL_APIENTRY GenerateMipmap(GLenum target);
ANGLE_EXPORT void GL_APIENTRY GenFramebuffers(GLsizei n, GLuint* framebuffers);
ANGLE_EXPORT void GL_APIENTRY GenRenderbuffers(GLsizei n, GLuint* renderbuffers);
ANGLE_EXPORT void GL_APIENTRY GenTextures(GLsizei n, GLuint* textures);
ANGLE_EXPORT void GL_APIENTRY GetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
ANGLE_EXPORT void GL_APIENTRY GetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
ANGLE_EXPORT void GL_APIENTRY GetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
ANGLE_EXPORT GLint GL_APIENTRY GetAttribLocation(GLuint program, const GLchar* name);
ANGLE_EXPORT void GL_APIENTRY GetBooleanv(GLenum pname, GLboolean* params);
ANGLE_EXPORT void GL_APIENTRY GetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT GLenum GL_APIENTRY GetError(void);
ANGLE_EXPORT void GL_APIENTRY GetFloatv(GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetIntegerv(GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetProgramiv(GLuint program, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
ANGLE_EXPORT void GL_APIENTRY GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetShaderiv(GLuint shader, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
ANGLE_EXPORT void GL_APIENTRY GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
ANGLE_EXPORT void GL_APIENTRY GetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
ANGLE_EXPORT const GLubyte *GL_APIENTRY GetString(GLenum name);
ANGLE_EXPORT void GL_APIENTRY GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetTexParameteriv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetUniformfv(GLuint program, GLint location, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetUniformiv(GLuint program, GLint location, GLint* params);
ANGLE_EXPORT GLint GL_APIENTRY GetUniformLocation(GLuint program, const GLchar* name);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribiv(GLuint index, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer);
ANGLE_EXPORT void GL_APIENTRY Hint(GLenum target, GLenum mode);
ANGLE_EXPORT GLboolean GL_APIENTRY IsBuffer(GLuint buffer);
ANGLE_EXPORT GLboolean GL_APIENTRY IsEnabled(GLenum cap);
ANGLE_EXPORT GLboolean GL_APIENTRY IsFramebuffer(GLuint framebuffer);
ANGLE_EXPORT GLboolean GL_APIENTRY IsProgram(GLuint program);
ANGLE_EXPORT GLboolean GL_APIENTRY IsRenderbuffer(GLuint renderbuffer);
ANGLE_EXPORT GLboolean GL_APIENTRY IsShader(GLuint shader);
ANGLE_EXPORT GLboolean GL_APIENTRY IsTexture(GLuint texture);
ANGLE_EXPORT void GL_APIENTRY LineWidth(GLfloat width);
ANGLE_EXPORT void GL_APIENTRY LinkProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY PixelStorei(GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY PolygonOffset(GLfloat factor, GLfloat units);
ANGLE_EXPORT void GL_APIENTRY ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
ANGLE_EXPORT void GL_APIENTRY ReleaseShaderCompiler(void);
ANGLE_EXPORT void GL_APIENTRY RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
ANGLE_EXPORT void GL_APIENTRY SampleCoverage(GLfloat value, GLboolean invert);
ANGLE_EXPORT void GL_APIENTRY Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT void GL_APIENTRY ShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
ANGLE_EXPORT void GL_APIENTRY ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
ANGLE_EXPORT void GL_APIENTRY StencilFunc(GLenum func, GLint ref, GLuint mask);
ANGLE_EXPORT void GL_APIENTRY StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
ANGLE_EXPORT void GL_APIENTRY StencilMask(GLuint mask);
ANGLE_EXPORT void GL_APIENTRY StencilMaskSeparate(GLenum face, GLuint mask);
ANGLE_EXPORT void GL_APIENTRY StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
ANGLE_EXPORT void GL_APIENTRY StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
ANGLE_EXPORT void GL_APIENTRY TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void GL_APIENTRY TexParameterf(GLenum target, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY TexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY TexParameteri(GLenum target, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY TexParameteriv(GLenum target, GLenum pname, const GLint* params);
ANGLE_EXPORT void GL_APIENTRY TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void GL_APIENTRY Uniform1f(GLint location, GLfloat x);
ANGLE_EXPORT void GL_APIENTRY Uniform1fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void GL_APIENTRY Uniform1i(GLint location, GLint x);
ANGLE_EXPORT void GL_APIENTRY Uniform1iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void GL_APIENTRY Uniform2f(GLint location, GLfloat x, GLfloat y);
ANGLE_EXPORT void GL_APIENTRY Uniform2fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void GL_APIENTRY Uniform2i(GLint location, GLint x, GLint y);
ANGLE_EXPORT void GL_APIENTRY Uniform2iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void GL_APIENTRY Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void GL_APIENTRY Uniform3fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void GL_APIENTRY Uniform3i(GLint location, GLint x, GLint y, GLint z);
ANGLE_EXPORT void GL_APIENTRY Uniform3iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void GL_APIENTRY Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void GL_APIENTRY Uniform4fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void GL_APIENTRY Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w);
ANGLE_EXPORT void GL_APIENTRY Uniform4iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void GL_APIENTRY UseProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY ValidateProgram(GLuint program);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1f(GLuint indx, GLfloat x);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib1fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2f(GLuint indx, GLfloat x, GLfloat y);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib2fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib3fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void GL_APIENTRY VertexAttrib4fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void GL_APIENTRY VertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
ANGLE_EXPORT void GL_APIENTRY Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

}

#endif // LIBGLESV2_ENTRYPOINTGLES20_H_

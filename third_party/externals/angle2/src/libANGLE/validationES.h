//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES.h: Validation functions for generic OpenGL ES entry point parameters

#ifndef LIBANGLE_VALIDATION_ES_H_
#define LIBANGLE_VALIDATION_ES_H_

#include "common/mathutil.h"

#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

namespace egl
{
class Display;
class Image;
}

namespace gl
{

class Context;
class Program;
class Shader;

bool ValidCap(const Context *context, GLenum cap);
bool ValidTextureTarget(const Context *context, GLenum target);
bool ValidTexture2DDestinationTarget(const Context *context, GLenum target);
bool ValidFramebufferTarget(GLenum target);
bool ValidBufferTarget(const Context *context, GLenum target);
bool ValidBufferParameter(const Context *context, GLenum pname);
bool ValidMipLevel(const Context *context, GLenum target, GLint level);
bool ValidImageSize(const Context *context, GLenum target, GLint level, GLsizei width, GLsizei height, GLsizei depth);
bool ValidCompressedImageSize(const Context *context, GLenum internalFormat, GLsizei width, GLsizei height);
bool ValidQueryType(const Context *context, GLenum queryType);

// Returns valid program if id is a valid program name
// Errors INVALID_OPERATION if valid shader is given and returns NULL
// Errors INVALID_VALUE otherwise and returns NULL
Program *GetValidProgram(Context *context, GLuint id);

// Returns valid shader if id is a valid shader name
// Errors INVALID_OPERATION if valid program is given and returns NULL
// Errors INVALID_VALUE otherwise and returns NULL
Shader *GetValidShader(Context *context, GLuint id);

bool ValidateAttachmentTarget(Context *context, GLenum attachment);
bool ValidateRenderbufferStorageParametersBase(Context *context, GLenum target, GLsizei samples,
                                               GLenum internalformat, GLsizei width, GLsizei height);
bool ValidateRenderbufferStorageParametersANGLE(Context *context, GLenum target, GLsizei samples,
                                                GLenum internalformat, GLsizei width, GLsizei height);

bool ValidateFramebufferRenderbufferParameters(Context *context, GLenum target, GLenum attachment,
                                               GLenum renderbuffertarget, GLuint renderbuffer);

bool ValidateBlitFramebufferParameters(Context *context, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                       GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask,
                                       GLenum filter, bool fromAngleExtension);

bool ValidateGetVertexAttribParameters(Context *context, GLenum pname);

bool ValidateTexParamParameters(Context *context, GLenum pname, GLint param);

bool ValidateSamplerObjectParameter(Context *context, GLenum pname);

bool ValidateReadPixelsParameters(Context *context, GLint x, GLint y, GLsizei width, GLsizei height,
                                               GLenum format, GLenum type, GLsizei *bufSize, GLvoid *pixels);

bool ValidateBeginQuery(Context *context, GLenum target, GLuint id);
bool ValidateEndQuery(Context *context, GLenum target);

bool ValidateUniform(Context *context, GLenum uniformType, GLint location, GLsizei count);
bool ValidateUniformMatrix(Context *context, GLenum matrixType, GLint location, GLsizei count,
                           GLboolean transpose);

bool ValidateStateQuery(Context *context, GLenum pname, GLenum *nativeType, unsigned int *numParams);

bool ValidateCopyTexImageParametersBase(Context *context, GLenum target, GLint level, GLenum internalformat, bool isSubImage,
                                        GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height,
                                        GLint border, GLenum *textureInternalFormatOut);

bool ValidateDrawArrays(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
bool ValidateDrawArraysInstanced(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
bool ValidateDrawArraysInstancedANGLE(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount);

bool ValidateDrawElements(Context *context,
                          GLenum mode,
                          GLsizei count,
                          GLenum type,
                          const GLvoid *indices,
                          GLsizei primcount,
                          IndexRange *indexRangeOut);

bool ValidateDrawElementsInstanced(Context *context,
                                   GLenum mode,
                                   GLsizei count,
                                   GLenum type,
                                   const GLvoid *indices,
                                   GLsizei primcount,
                                   IndexRange *indexRangeOut);
bool ValidateDrawElementsInstancedANGLE(Context *context,
                                        GLenum mode,
                                        GLsizei count,
                                        GLenum type,
                                        const GLvoid *indices,
                                        GLsizei primcount,
                                        IndexRange *indexRangeOut);

bool ValidateFramebufferTextureBase(Context *context, GLenum target, GLenum attachment,
                                    GLuint texture, GLint level);
bool ValidateFramebufferTexture2D(Context *context, GLenum target, GLenum attachment,
                                   GLenum textarget, GLuint texture, GLint level);

bool ValidateGetUniformBase(Context *context, GLuint program, GLint location);
bool ValidateGetUniformfv(Context *context, GLuint program, GLint location, GLfloat* params);
bool ValidateGetUniformiv(Context *context, GLuint program, GLint location, GLint* params);
bool ValidateGetnUniformfvEXT(Context *context, GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
bool ValidateGetnUniformivEXT(Context *context, GLuint program, GLint location, GLsizei bufSize, GLint* params);

bool ValidateDiscardFramebufferBase(Context *context, GLenum target, GLsizei numAttachments,
                                    const GLenum *attachments, bool defaultFramebuffer);

bool ValidateInsertEventMarkerEXT(Context *context, GLsizei length, const char *marker);
bool ValidatePushGroupMarkerEXT(Context *context, GLsizei length, const char *marker);

bool ValidateEGLImageTargetTexture2DOES(Context *context,
                                        egl::Display *display,
                                        GLenum target,
                                        egl::Image *image);
bool ValidateEGLImageTargetRenderbufferStorageOES(Context *context,
                                                  egl::Display *display,
                                                  GLenum target,
                                                  egl::Image *image);
}

#endif // LIBANGLE_VALIDATION_ES_H_

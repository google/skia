/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLTestInterface_DEFINED
#define GrGLTestInterface_DEFINED

#include "gl/GrGLInterface.h"
#include "GrGLDefines.h"

/**
 * Base class for interfaces used for Skia testing. We would like to move this to tools/gpu/gl
 * when Chromium is no longer using GrGLCreateNullInterface in its unit testing.
 */
class GrGLTestInterface : public GrGLInterface {
public:
    virtual GrGLvoid activeTexture(GrGLenum texture) {}
    virtual GrGLvoid attachShader(GrGLuint program, GrGLuint shader) {}
    virtual GrGLvoid beginQuery(GrGLenum target, GrGLuint id) {}
    virtual GrGLvoid bindAttribLocation(GrGLuint program, GrGLuint index, const char* name) {}
    virtual GrGLvoid bindBuffer(GrGLenum target, GrGLuint buffer) {}
    virtual GrGLvoid bindFramebuffer(GrGLenum target, GrGLuint framebuffer) {}
    virtual GrGLvoid bindRenderbuffer(GrGLenum target, GrGLuint renderbuffer) {}
    virtual GrGLvoid bindSampler(GrGLuint unit, GrGLuint sampler) {}
    virtual GrGLvoid bindTexture(GrGLenum target, GrGLuint texture) {}
    virtual GrGLvoid bindFragDataLocation(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name) {}
    virtual GrGLvoid bindFragDataLocationIndexed(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name) {}
    virtual GrGLvoid bindVertexArray(GrGLuint array) {}
    virtual GrGLvoid blendBarrier() {}
    virtual GrGLvoid blendColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
    virtual GrGLvoid blendEquation(GrGLenum mode) {}
    virtual GrGLvoid blendFunc(GrGLenum sfactor, GrGLenum dfactor) {}
    virtual GrGLvoid blitFramebuffer(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter) {}
    virtual GrGLvoid bufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage) {}
    virtual GrGLvoid bufferSubData(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data) {}
    virtual GrGLenum checkFramebufferStatus(GrGLenum target) { return GR_GL_FRAMEBUFFER_COMPLETE; }
    virtual GrGLvoid clear(GrGLbitfield mask) {}
    virtual GrGLvoid clearColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
    virtual GrGLvoid clearStencil(GrGLint s) {}
    virtual GrGLvoid colorMask(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha) {}
    virtual GrGLvoid compileShader(GrGLuint shader) {}
    virtual GrGLvoid compressedTexImage2D(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data) {}
    virtual GrGLvoid compressedTexSubImage2D(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLsizei imageSize, const GrGLvoid* data) {}
    virtual GrGLvoid copyTexSubImage2D(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLuint createProgram() { return 0; }
    virtual GrGLuint createShader(GrGLenum type) { return 0; }
    virtual GrGLvoid cullFace(GrGLenum mode) {}
    virtual GrGLvoid deleteBuffers(GrGLsizei n, const GrGLuint* buffers) {}
    virtual GrGLvoid deleteFramebuffers(GrGLsizei n, const GrGLuint *framebuffers) {}
    virtual GrGLvoid deleteProgram(GrGLuint program) {}
    virtual GrGLvoid deleteQueries(GrGLsizei n, const GrGLuint *ids) {}
    virtual GrGLvoid deleteRenderbuffers(GrGLsizei n, const GrGLuint *renderbuffers) {}
    virtual GrGLvoid deleteSamplers(GrGLsizei n, const GrGLuint* samplers) {}
    virtual GrGLvoid deleteShader(GrGLuint shader) {}
    virtual GrGLvoid deleteTextures(GrGLsizei n, const GrGLuint* textures) {}
    virtual GrGLvoid deleteVertexArrays(GrGLsizei n, const GrGLuint *arrays) {}
    virtual GrGLvoid depthMask(GrGLboolean flag) {}
    virtual GrGLvoid disable(GrGLenum cap) {}
    virtual GrGLvoid disableVertexAttribArray(GrGLuint index) {}
    virtual GrGLvoid drawArrays(GrGLenum mode, GrGLint first, GrGLsizei count) {}
    virtual GrGLvoid drawArraysInstanced(GrGLenum mode, GrGLint first, GrGLsizei count, GrGLsizei primcount) {}
    virtual GrGLvoid drawArraysIndirect(GrGLenum mode, const GrGLvoid* indirect) {}
    virtual GrGLvoid drawBuffer(GrGLenum mode) {}
    virtual GrGLvoid drawBuffers(GrGLsizei n, const GrGLenum* bufs) {}
    virtual GrGLvoid drawElements(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices) {}
    virtual GrGLvoid drawElementsInstanced(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid *indices, GrGLsizei primcount) {}
    virtual GrGLvoid drawElementsIndirect(GrGLenum mode, GrGLenum type, const GrGLvoid* indirect) {}
    virtual GrGLvoid drawRangeElements(GrGLenum mode, GrGLuint start, GrGLuint end, GrGLsizei count, GrGLenum type, const GrGLvoid* indices) {}
    virtual GrGLvoid enable(GrGLenum cap) {}
    virtual GrGLvoid enableVertexAttribArray(GrGLuint index) {}
    virtual GrGLvoid endQuery(GrGLenum target) {}
    virtual GrGLvoid finish() {}
    virtual GrGLvoid flush() {}
    virtual GrGLvoid flushMappedBufferRange(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length) {}
    virtual GrGLvoid framebufferRenderbuffer(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer) {}
    virtual GrGLvoid framebufferTexture2D(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level) {}
    virtual GrGLvoid framebufferTexture2DMultisample(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level, GrGLsizei samples) {}
    virtual GrGLvoid frontFace(GrGLenum mode) {}
    virtual GrGLvoid genBuffers(GrGLsizei n, GrGLuint* buffers) {}
    virtual GrGLvoid genFramebuffers(GrGLsizei n, GrGLuint *framebuffers) {}
    virtual GrGLvoid generateMipmap(GrGLenum target) {}
    virtual GrGLvoid genQueries(GrGLsizei n, GrGLuint *ids) {}
    virtual GrGLvoid genRenderbuffers(GrGLsizei n, GrGLuint *renderbuffers) {}
    virtual GrGLvoid genSamplers(GrGLsizei n, GrGLuint *samplers) {}
    virtual GrGLvoid genTextures(GrGLsizei n, GrGLuint* textures) {}
    virtual GrGLvoid genVertexArrays(GrGLsizei n, GrGLuint *arrays) {}
    virtual GrGLvoid getBufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {}
    virtual GrGLenum getError() { return GR_GL_NO_ERROR; }
    virtual GrGLvoid getFramebufferAttachmentParameteriv(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params) {}
    virtual GrGLvoid getIntegerv(GrGLenum pname, GrGLint* params) {}
    virtual GrGLvoid getMultisamplefv(GrGLenum pname, GrGLuint index, GrGLfloat* val) {}
    virtual GrGLvoid getProgramInfoLog(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, char* infolog) {}
    virtual GrGLvoid getProgramiv(GrGLuint program, GrGLenum pname, GrGLint* params) {}
    virtual GrGLvoid getQueryiv(GrGLenum GLtarget, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid getQueryObjecti64v(GrGLuint id, GrGLenum pname, GrGLint64 *params) {}
    virtual GrGLvoid getQueryObjectiv(GrGLuint id, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid getQueryObjectui64v(GrGLuint id, GrGLenum pname, GrGLuint64 *params) {}
    virtual GrGLvoid getQueryObjectuiv(GrGLuint id, GrGLenum pname, GrGLuint *params) {}
    virtual GrGLvoid getRenderbufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {}
    virtual GrGLvoid getShaderInfoLog(GrGLuint shader, GrGLsizei bufsize, GrGLsizei* length, char* infolog) {}
    virtual GrGLvoid getShaderiv(GrGLuint shader, GrGLenum pname, GrGLint* params) {}
    virtual GrGLvoid getShaderPrecisionFormat(GrGLenum shadertype, GrGLenum precisiontype, GrGLint *range, GrGLint *precision) {}
    virtual const GrGLubyte*  getString(GrGLenum name) { return nullptr; }
    virtual const GrGLubyte* getStringi(GrGLenum name, GrGLuint index) { return nullptr; }
    virtual GrGLvoid getTexLevelParameteriv(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params) {}
    virtual GrGLint getUniformLocation(GrGLuint program, const char* name) { return 0; }
    virtual GrGLvoid insertEventMarker(GrGLsizei length, const char* marker) {}
    virtual GrGLvoid invalidateBufferData(GrGLuint buffer) {}
    virtual GrGLvoid invalidateBufferSubData(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length) {}
    virtual GrGLvoid invalidateFramebuffer(GrGLenum target, GrGLsizei numAttachments,  const GrGLenum *attachments) {}
    virtual GrGLvoid invalidateSubFramebuffer(GrGLenum target, GrGLsizei numAttachments, const GrGLenum *attachments, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid invalidateTexImage(GrGLuint texture, GrGLint level) {}
    virtual GrGLvoid invalidateTexSubImage(GrGLuint texture, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth) {}
    virtual GrGLboolean isTexture(GrGLuint texture) { return GR_GL_FALSE; }
    virtual GrGLvoid lineWidth(GrGLfloat width) {}
    virtual GrGLvoid linkProgram(GrGLuint program) {}
    virtual GrGLvoid* mapBuffer(GrGLenum target, GrGLenum access) { return nullptr; }
    virtual GrGLvoid* mapBufferRange(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length, GrGLbitfield access) { return nullptr; }
    virtual GrGLvoid* mapBufferSubData(GrGLuint target, GrGLintptr offset, GrGLsizeiptr size, GrGLenum access) { return nullptr; }
    virtual GrGLvoid* mapTexSubImage2D(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLenum access) { return nullptr; }
    virtual GrGLvoid pixelStorei(GrGLenum pname, GrGLint param) {}
    virtual GrGLvoid polygonMode(GrGLenum face, GrGLenum mode) {}
    virtual GrGLvoid popGroupMarker() {}
    virtual GrGLvoid pushGroupMarker(GrGLsizei length, const char* marker) {}
    virtual GrGLvoid queryCounter(GrGLuint id, GrGLenum target) {}
    virtual GrGLvoid rasterSamples(GrGLuint samples, GrGLboolean fixedsamplelocations) {}
    virtual GrGLvoid readBuffer(GrGLenum src) {}
    virtual GrGLvoid readPixels(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels) {}
    virtual GrGLvoid renderbufferStorage(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid renderbufferStorageMultisample(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid resolveMultisampleFramebuffer() {}
    virtual GrGLvoid samplerParameteri(GrGLuint sampler, GrGLenum pname, GrGLint param) {}
    virtual GrGLvoid samplerParameteriv(GrGLuint sampler, GrGLenum pname, const GrGLint* param) {}
    virtual GrGLvoid scissor(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid bindUniformLocation(GrGLuint program, GrGLint location, const char* name) {}
    virtual GrGLvoid shaderSource(GrGLuint shader, GrGLsizei count, const char* const * str, const GrGLint* length) {}
    virtual GrGLvoid stencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask) {}
    virtual GrGLvoid stencilFuncSeparate(GrGLenum face, GrGLenum func, GrGLint ref, GrGLuint mask) {}
    virtual GrGLvoid stencilMask(GrGLuint mask) {}
    virtual GrGLvoid stencilMaskSeparate(GrGLenum face, GrGLuint mask) {}
    virtual GrGLvoid stencilOp(GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {}
    virtual GrGLvoid stencilOpSeparate(GrGLenum face, GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {}
    virtual GrGLvoid texBuffer(GrGLenum target, GrGLenum internalformat, GrGLuint buffer) {}
    virtual GrGLvoid texImage2D(GrGLenum target, GrGLint level, GrGLint internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid* pixels) {}
    virtual GrGLvoid texParameterf(GrGLenum target, GrGLenum pname, GrGLfloat param) {}
    virtual GrGLvoid texParameterfv(GrGLenum target, GrGLenum pname, const GrGLfloat* params) {}
    virtual GrGLvoid texParameteri(GrGLenum target, GrGLenum pname, GrGLint param) {}
    virtual GrGLvoid texParameteriv(GrGLenum target, GrGLenum pname, const GrGLint* params) {}
    virtual GrGLvoid texStorage2D(GrGLenum target, GrGLsizei levels, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid discardFramebuffer(GrGLenum target, GrGLsizei numAttachments, const GrGLenum* attachments) {}
    virtual GrGLvoid texSubImage2D(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels) {}
    virtual GrGLvoid textureBarrier() {}
    virtual GrGLvoid uniform1f(GrGLint location, GrGLfloat v0) {}
    virtual GrGLvoid uniform1i(GrGLint location, GrGLint v0) {}
    virtual GrGLvoid uniform1fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
    virtual GrGLvoid uniform1iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
    virtual GrGLvoid uniform2f(GrGLint location, GrGLfloat v0, GrGLfloat v1) {}
    virtual GrGLvoid uniform2i(GrGLint location, GrGLint v0, GrGLint v1) {}
    virtual GrGLvoid uniform2fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
    virtual GrGLvoid uniform2iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
    virtual GrGLvoid uniform3f(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2) {}
    virtual GrGLvoid uniform3i(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2) {}
    virtual GrGLvoid uniform3fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
    virtual GrGLvoid uniform3iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
    virtual GrGLvoid uniform4f(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2, GrGLfloat v3) {}
    virtual GrGLvoid uniform4i(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3) {}
    virtual GrGLvoid uniform4fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
    virtual GrGLvoid uniform4iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
    virtual GrGLvoid uniformMatrix2fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
    virtual GrGLvoid uniformMatrix3fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
    virtual GrGLvoid uniformMatrix4fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
    virtual GrGLboolean unmapBuffer(GrGLenum target) { return GR_GL_TRUE; }
    virtual GrGLvoid unmapBufferSubData(const GrGLvoid* mem) {}
    virtual GrGLvoid unmapTexSubImage2D(const GrGLvoid* mem) {}
    virtual GrGLvoid useProgram(GrGLuint program) {}
    virtual GrGLvoid vertexAttrib1f(GrGLuint indx, const GrGLfloat value) {}
    virtual GrGLvoid vertexAttrib2fv(GrGLuint indx, const GrGLfloat* values) {}
    virtual GrGLvoid vertexAttrib3fv(GrGLuint indx, const GrGLfloat* values) {}
    virtual GrGLvoid vertexAttrib4fv(GrGLuint indx, const GrGLfloat* values) {}
    virtual GrGLvoid vertexAttribDivisor(GrGLuint index, GrGLuint divisor) {}
    virtual GrGLvoid vertexAttribIPointer(GrGLuint indx, GrGLint size, GrGLenum type, GrGLsizei stride, const GrGLvoid* ptr) {}
    virtual GrGLvoid vertexAttribPointer(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr) {}
    virtual GrGLvoid viewport(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid matrixLoadf(GrGLenum matrixMode, const GrGLfloat* m) {}
    virtual GrGLvoid matrixLoadIdentity(GrGLenum) {}
    virtual GrGLvoid pathCommands(GrGLuint path, GrGLsizei numCommands, const GrGLubyte *commands, GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords) {}
    virtual GrGLvoid pathParameteri(GrGLuint path, GrGLenum pname, GrGLint value) {}
    virtual GrGLvoid pathParameterf(GrGLuint path, GrGLenum pname, GrGLfloat value) {}
    virtual GrGLuint genPaths(GrGLsizei range) { return 0; }
    virtual GrGLvoid deletePaths(GrGLuint path, GrGLsizei range) {}
    virtual GrGLboolean isPath(GrGLuint path) { return true; }
    virtual GrGLvoid pathStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask) {}
    virtual GrGLvoid stencilFillPath(GrGLuint path, GrGLenum fillMode, GrGLuint mask) {}
    virtual GrGLvoid stencilStrokePath(GrGLuint path, GrGLint reference, GrGLuint mask) {}
    virtual GrGLvoid stencilFillPathInstanced(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum transformType, const GrGLfloat *transformValues) {}
    virtual GrGLvoid stencilStrokePathInstanced(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum transformType, const GrGLfloat *transformValues) {}
    virtual GrGLvoid coverFillPath(GrGLuint path, GrGLenum coverMode) {}
    virtual GrGLvoid coverStrokePath(GrGLuint name, GrGLenum coverMode) {}
    virtual GrGLvoid coverFillPathInstanced(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues) {}
    virtual GrGLvoid coverStrokePathInstanced(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat* transformValues) {}
    virtual GrGLvoid stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode) {}
    virtual GrGLvoid stencilThenCoverStrokePath(GrGLuint path, GrGLint reference, GrGLuint mask, GrGLenum coverMode) {}
    virtual GrGLvoid stencilThenCoverFillPathInstanced(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues) {}
    virtual GrGLvoid stencilThenCoverStrokePathInstanced(GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths, GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode, GrGLenum transformType, const GrGLfloat *transformValues) {}
    virtual GrGLvoid programPathFragmentInputGen(GrGLuint program, GrGLint location, GrGLenum genMode, GrGLint components,const GrGLfloat *coeffs) {}
    virtual GrGLvoid bindFragmentInputLocation(GrGLuint program, GrGLint location, const GrGLchar* name) {}
    virtual GrGLint getProgramResourceLocation(GrGLuint program, GrGLenum programInterface, const GrGLchar *name) { return 0; }
    virtual GrGLvoid coverageModulation(GrGLenum components) {}
    virtual GrGLvoid multiDrawArraysIndirect(GrGLenum mode, const GrGLvoid *indirect, GrGLsizei drawcount, GrGLsizei stride) {}
    virtual GrGLvoid multiDrawElementsIndirect(GrGLenum mode, GrGLenum type, const GrGLvoid *indirect, GrGLsizei drawcount, GrGLsizei stride) {}
    virtual GrGLuint64 getTextureHandle(GrGLuint texture) { return 0; }
    virtual GrGLuint64 getTextureSamplerHandle(GrGLuint texture, GrGLuint sampler) { return 0; }
    virtual GrGLvoid makeTextureHandleResident(GrGLuint64 handle) {}
    virtual GrGLvoid makeTextureHandleNonResident(GrGLuint64 handle) {}
    virtual GrGLuint64 getImageHandle(GrGLuint texture, GrGLint level, GrGLboolean layered, GrGLint layer, GrGLint format) { return 0; }
    virtual GrGLvoid makeImageHandleResident(GrGLuint64 handle, GrGLenum access) {}
    virtual GrGLvoid makeImageHandleNonResident(GrGLuint64 handle) {}
    virtual GrGLboolean isTextureHandleResident(GrGLuint64 handle) { return GR_GL_FALSE; }
    virtual GrGLboolean isImageHandleResident(GrGLuint64 handle) { return GR_GL_FALSE; }
    virtual GrGLvoid uniformHandleui64(GrGLint location, GrGLuint64 v0) {}
    virtual GrGLvoid uniformHandleui64v(GrGLint location, GrGLsizei count, const GrGLuint64 *value) {}
    virtual GrGLvoid programUniformHandleui64(GrGLuint program, GrGLint location, GrGLuint64 v0) {}
    virtual GrGLvoid programUniformHandleui64v(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLuint64 *value) {}
    virtual GrGLvoid textureParameteri(GrGLuint texture, GrGLenum target, GrGLenum pname, GrGLint param) {}
    virtual GrGLvoid textureParameteriv(GrGLuint texture, GrGLenum target, GrGLenum pname, const GrGLint *param) {}
    virtual GrGLvoid textureParameterf(GrGLuint texture, GrGLenum target, GrGLenum pname, float param) {}
    virtual GrGLvoid textureParameterfv(GrGLuint texture, GrGLenum target, GrGLenum pname, const float *param) {}
    virtual GrGLvoid textureImage1D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint GrGLinternalformat, GrGLsizei width, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid *pixels) {}
    virtual GrGLvoid textureImage2D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid *pixels) {}
    virtual GrGLvoid textureSubImage1D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLsizei width, GrGLenum format, GrGLenum type, const GrGLvoid *pixels) {}
    virtual GrGLvoid textureSubImage2D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid *pixels) {}
    virtual GrGLvoid copyTextureImage1D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLint x, GrGLint y, GrGLsizei width, GrGLint border) {}
    virtual GrGLvoid copyTextureImage2D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLint border) {}
    virtual GrGLvoid copyTextureSubImage1D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint x, GrGLint y, GrGLsizei width) {}
    virtual GrGLvoid copyTextureSubImage2D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid getTextureImage(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum format, GrGLenum type, GrGLvoid *pixels) {}
    virtual GrGLvoid getTextureParameterfv(GrGLuint texture, GrGLenum target, GrGLenum pname, float *params) {}
    virtual GrGLvoid getTextureParameteriv(GrGLuint texture, GrGLenum target, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid getTextureLevelParameterfv(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum pname, float *params) {}
    virtual GrGLvoid getTextureLevelParameteriv(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid textureImage3D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid *pixels) {}
    virtual GrGLvoid textureSubImage3D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLenum format, GrGLenum type, const GrGLvoid *pixels) {}
    virtual GrGLvoid copyTextureSubImage3D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid compressedTextureImage3D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLint border, GrGLsizei imageSize, const GrGLvoid *data) {}
    virtual GrGLvoid compressedTextureImage2D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid *data) {}
    virtual GrGLvoid compressedTextureImage1D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLint border, GrGLsizei imageSize, const GrGLvoid *data) {}
    virtual GrGLvoid compressedTextureSubImage3D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLint zoffset, GrGLsizei width, GrGLsizei height, GrGLsizei depth, GrGLenum format, GrGLsizei imageSize, const GrGLvoid *data) {}
    virtual GrGLvoid compressedTextureSubImage2D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLsizei imageSize, const GrGLvoid *data) {}
    virtual GrGLvoid compressedTextureSubImage1D(GrGLuint texture, GrGLenum target, GrGLint level, GrGLint xoffset, GrGLsizei width, GrGLenum format, GrGLsizei imageSize, const GrGLvoid *data) {}
    virtual GrGLvoid getCompressedTextureImage(GrGLuint texture, GrGLenum target, GrGLint level, GrGLvoid *img) {}
    virtual GrGLvoid namedBufferData(GrGLuint buffer, GrGLsizeiptr size, const GrGLvoid *data, GrGLenum usage) {}
    virtual GrGLvoid namedBufferSubData(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid *data) {}
    virtual GrGLvoid* mapNamedBuffer(GrGLuint buffer, GrGLenum access) { return nullptr; }
    virtual GrGLboolean unmapNamedBuffer(GrGLuint buffer) { return GR_GL_FALSE; }
    virtual GrGLvoid getNamedBufferParameteriv(GrGLuint buffer, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid getNamedBufferPointerv(GrGLuint buffer, GrGLenum pname, GrGLvoid* *params) {}
    virtual GrGLvoid getNamedBufferSubData(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr size, GrGLvoid *data) {}
    virtual GrGLvoid programUniform1f(GrGLuint program, GrGLint location, float v0) {}
    virtual GrGLvoid programUniform2f(GrGLuint program, GrGLint location, float v0, float v1) {}
    virtual GrGLvoid programUniform3f(GrGLuint program, GrGLint location, float v0, float v1, float v2) {}
    virtual GrGLvoid programUniform4f(GrGLuint program, GrGLint location, float v0, float v1, float v2, float v3) {}
    virtual GrGLvoid programUniform1i(GrGLuint program, GrGLint location, GrGLint v0) {}
    virtual GrGLvoid programUniform2i(GrGLuint program, GrGLint location, GrGLint v0, GrGLint v1) {}
    virtual GrGLvoid programUniform3i(GrGLuint program, GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2) {}
    virtual GrGLvoid programUniform4i(GrGLuint program, GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3) {}
    virtual GrGLvoid programUniform1fv(GrGLuint program, GrGLint location, GrGLsizei count, const float *value) {}
    virtual GrGLvoid programUniform2fv(GrGLuint program, GrGLint location, GrGLsizei count, const float *value) {}
    virtual GrGLvoid programUniform3fv(GrGLuint program, GrGLint location, GrGLsizei count, const float *value) {}
    virtual GrGLvoid programUniform4fv(GrGLuint program, GrGLint location, GrGLsizei count, const float *value) {}
    virtual GrGLvoid programUniform1iv(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value) {}
    virtual GrGLvoid programUniform2iv(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value) {}
    virtual GrGLvoid programUniform3iv(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value) {}
    virtual GrGLvoid programUniform4iv(GrGLuint program, GrGLint location, GrGLsizei count, const GrGLint *value) {}
    virtual GrGLvoid programUniformMatrix2fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix3fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix4fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix2x3fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix3x2fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix2x4fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix4x2fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix3x4fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid programUniformMatrix4x3fv(GrGLuint program, GrGLint location, GrGLsizei count, GrGLboolean transpose, const float *value) {}
    virtual GrGLvoid namedRenderbufferStorage(GrGLuint renderbuffer, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLvoid getNamedRenderbufferParameteriv(GrGLuint renderbuffer, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid namedRenderbufferStorageMultisample(GrGLuint renderbuffer, GrGLsizei samples, GrGLenum GrGLinternalformat, GrGLsizei width, GrGLsizei height) {}
    virtual GrGLenum checkNamedFramebufferStatus(GrGLuint framebuffer, GrGLenum target) { return GR_GL_FRAMEBUFFER_COMPLETE; }
    virtual GrGLvoid namedFramebufferTexture1D(GrGLuint framebuffer, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level) {}
    virtual GrGLvoid namedFramebufferTexture2D(GrGLuint framebuffer, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level) {}
    virtual GrGLvoid namedFramebufferTexture3D(GrGLuint framebuffer, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level, GrGLint zoffset) {}
    virtual GrGLvoid namedFramebufferRenderbuffer(GrGLuint framebuffer, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer) {}
    virtual GrGLvoid getNamedFramebufferAttachmentParameteriv(GrGLuint framebuffer, GrGLenum attachment, GrGLenum pname, GrGLint *params) {}
    virtual GrGLvoid generateTextureMipmap(GrGLuint texture, GrGLenum target) {}
    virtual GrGLvoid framebufferDrawBuffer(GrGLuint framebuffer, GrGLenum mode) {}
    virtual GrGLvoid framebufferDrawBuffers(GrGLuint framebuffer, GrGLsizei n, const GrGLenum *bufs) {}
    virtual GrGLvoid framebufferReadBuffer(GrGLuint framebuffer, GrGLenum mode) {}
    virtual GrGLvoid getFramebufferParameteriv(GrGLuint framebuffer, GrGLenum pname, GrGLint *param) {}
    virtual GrGLvoid namedCopyBufferSubData(GrGLuint readBuffer, GrGLuint writeBuffer, GrGLintptr readOffset, GrGLintptr writeOffset, GrGLsizeiptr size) {}
    virtual GrGLvoid vertexArrayVertexOffset(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayColorOffset(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayEdgeFlagOffset(GrGLuint vaobj, GrGLuint buffer, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayIndexOffset(GrGLuint vaobj, GrGLuint buffer, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayNormalOffset(GrGLuint vaobj, GrGLuint buffer, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayTexCoordOffset(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayMultiTexCoordOffset(GrGLuint vaobj, GrGLuint buffer, GrGLenum texunit, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayFogCoordOffset(GrGLuint vaobj, GrGLuint buffer, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArraySecondaryColorOffset(GrGLuint vaobj, GrGLuint buffer, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayVertexAttribOffset(GrGLuint vaobj, GrGLuint buffer, GrGLuint index, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid vertexArrayVertexAttribIOffset(GrGLuint vaobj, GrGLuint buffer, GrGLuint index, GrGLint size, GrGLenum type, GrGLsizei stride, GrGLintptr offset) {}
    virtual GrGLvoid enableVertexArray(GrGLuint vaobj, GrGLenum array) {}
    virtual GrGLvoid disableVertexArray(GrGLuint vaobj, GrGLenum array) {}
    virtual GrGLvoid enableVertexArrayAttrib(GrGLuint vaobj, GrGLuint index) {}
    virtual GrGLvoid disableVertexArrayAttrib(GrGLuint vaobj, GrGLuint index) {}
    virtual GrGLvoid getVertexArrayIntegerv(GrGLuint vaobj, GrGLenum pname, GrGLint *param) {}
    virtual GrGLvoid getVertexArrayPointerv(GrGLuint vaobj, GrGLenum pname, GrGLvoid **param) {}
    virtual GrGLvoid getVertexArrayIntegeri_v(GrGLuint vaobj, GrGLuint index, GrGLenum pname, GrGLint *param) {}
    virtual GrGLvoid getVertexArrayPointeri_v(GrGLuint vaobj, GrGLuint index, GrGLenum pname, GrGLvoid **param) {}
    virtual GrGLvoid* mapNamedBufferRange(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length, GrGLbitfield access) { return nullptr; }
    virtual GrGLvoid flushMappedNamedBufferRange(GrGLuint buffer, GrGLintptr offset, GrGLsizeiptr length) {}
    virtual GrGLvoid textureBuffer(GrGLuint texture, GrGLenum target, GrGLenum internalformat, GrGLuint buffer) {}
    virtual GrGLsync fenceSync(GrGLenum condition, GrGLbitfield flags) { return nullptr;  }
    virtual GrGLboolean isSync(GrGLsync) { return false;  }
    virtual GrGLenum clientWaitSync(GrGLsync sync, GrGLbitfield flags, GrGLuint64 timeout) { return GR_GL_WAIT_FAILED;  }
    virtual GrGLvoid waitSync(GrGLsync sync, GrGLbitfield flags, GrGLuint64 timeout) {}
    virtual GrGLvoid deleteSync(GrGLsync sync) {}
    virtual GrGLvoid debugMessageControl(GrGLenum source, GrGLenum type, GrGLenum severity, GrGLsizei count, const GrGLuint* ids, GrGLboolean enabled) {}
    virtual GrGLvoid debugMessageInsert(GrGLenum source, GrGLenum type, GrGLuint id, GrGLenum severity, GrGLsizei length,  const GrGLchar* buf) {}
    virtual GrGLvoid debugMessageCallback(GRGLDEBUGPROC callback, const GrGLvoid* userParam) {}
    virtual GrGLuint getDebugMessageLog(GrGLuint count, GrGLsizei bufSize, GrGLenum* sources, GrGLenum* types, GrGLuint* ids, GrGLenum* severities, GrGLsizei* lengths,  GrGLchar* messageLog) { return 0; }
    virtual GrGLvoid pushDebugGroup(GrGLenum source, GrGLuint id, GrGLsizei length,  const GrGLchar * message) {}
    virtual GrGLvoid popDebugGroup() {}
    virtual GrGLvoid objectLabel(GrGLenum identifier, GrGLuint name, GrGLsizei length, const GrGLchar *label) {}
    virtual GrGLvoid getInternalformativ(GrGLenum target, GrGLenum internalformat, GrGLenum pname, GrGLsizei bufSize, GrGLint *params) {}
    virtual GrGLvoid programBinary(GrGLuint program, GrGLenum binaryFormat, void *binary, GrGLsizei length) {}
    virtual GrGLvoid getProgramBinary(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, GrGLenum *binaryFormat, void *binary) {}
    virtual GrGLvoid programParameteri(GrGLuint program, GrGLenum pname, GrGLint value) {}

protected:
    // This must be called by leaf class
    void init(GrGLStandard standard) {
        fStandard = standard;
        fExtensions.init(standard, fFunctions.fGetString, fFunctions.fGetStringi,
                         fFunctions.fGetIntegerv, nullptr, GR_EGL_NO_DISPLAY);
    }
    GrGLTestInterface();
};

#endif

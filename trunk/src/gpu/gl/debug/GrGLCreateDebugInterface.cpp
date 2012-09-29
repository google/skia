
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "GrDebugGL.h"
#include "GrShaderObj.h"
#include "GrProgramObj.h"
#include "GrBufferObj.h"
#include "GrTextureUnitObj.h"
#include "GrTextureObj.h"
#include "GrFrameBufferObj.h"
#include "GrRenderBufferObj.h"
#include "SkFloatingPoint.h"

// the OpenGLES 2.0 spec says this must be >= 128
static const GrGLint kDefaultMaxVertexUniformVectors = 128;

// the OpenGLES 2.0 spec says this must be >=16
static const GrGLint kDefaultMaxFragmentUniformVectors = 16;

// the OpenGLES 2.0 spec says this must be >= 8
static const GrGLint kDefaultMaxVertexAttribs = 8;

// the OpenGLES 2.0 spec says this must be >= 8
static const GrGLint kDefaultMaxVaryingVectors = 8;

namespace { // suppress no previsous prototype warning

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLActiveTexture(GrGLenum texture) {

    // Ganesh offsets the texture unit indices
    texture -= GR_GL_TEXTURE0;
    GrAlwaysAssert(texture < GrDebugGL::getInstance()->getMaxTextureUnits());

    GrDebugGL::getInstance()->setCurTextureUnit(texture);
}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLAttachShader(GrGLuint programID,
                                                 GrGLuint shaderID) {

    GrProgramObj *program = GR_FIND(programID, GrProgramObj,
                                    GrDebugGL::kProgram_ObjTypes);
    GrAlwaysAssert(program);

    GrShaderObj *shader = GR_FIND(shaderID,
                                  GrShaderObj,
                                  GrDebugGL::kShader_ObjTypes);
    GrAlwaysAssert(shader);

    program->AttachShader(shader);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBeginQuery(GrGLenum target, GrGLuint id) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindAttribLocation(GrGLuint program,
                                                       GrGLuint index,
                                                       const char* name) {
}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindTexture(GrGLenum target,
                                                GrGLuint textureID) {

    // we don't use cube maps
    GrAlwaysAssert(target == GR_GL_TEXTURE_2D);
                                    // || target == GR_GL_TEXTURE_CUBE_MAP);

    // a textureID of 0 is acceptable - it binds to the default texture target
    GrTextureObj *texture = GR_FIND(textureID, GrTextureObj,
                                    GrDebugGL::kTexture_ObjTypes);

    GrDebugGL::getInstance()->setTexture(texture);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlendColor(GrGLclampf red,
                                               GrGLclampf green,
                                               GrGLclampf blue,
                                               GrGLclampf alpha) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFragDataLocation(GrGLuint program,
                                                         GrGLuint colorNumber,
                                                         const GrGLchar* name) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlendFunc(GrGLenum sfactor,
                                              GrGLenum dfactor) {
}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBufferData(GrGLenum target,
                                               GrGLsizeiptr size,
                                               const GrGLvoid* data,
                                               GrGLenum usage) {
    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target ||
                   GR_GL_ELEMENT_ARRAY_BUFFER == target);
    GrAlwaysAssert(size >= 0);
    GrAlwaysAssert(GR_GL_STREAM_DRAW == usage ||
                   GR_GL_STATIC_DRAW == usage ||
                   GR_GL_DYNAMIC_DRAW == usage);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
        default:
            GrCrash("Unexpected target to glBufferData");
            break;
    }

    GrAlwaysAssert(buffer);
    GrAlwaysAssert(buffer->getBound());

    buffer->allocate(size, reinterpret_cast<const GrGLchar *>(data));
    buffer->setUsage(usage);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBufferSubData(GrGLenum target,
                                                  GrGLintptr offset,
                                                  GrGLsizeiptr size,
                                                  const GrGLvoid* data) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLClear(GrGLbitfield mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLClearColor(GrGLclampf red,
                                               GrGLclampf green,
                                               GrGLclampf blue,
                                               GrGLclampf alpha) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLClearStencil(GrGLint s) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLColorMask(GrGLboolean red,
                                              GrGLboolean green,
                                              GrGLboolean blue,
                                              GrGLboolean alpha) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLCompileShader(GrGLuint shader) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLCompressedTexImage2D(GrGLenum target,
                                                         GrGLint level,
                                                         GrGLenum internalformat,
                                                         GrGLsizei width,
                                                         GrGLsizei height,
                                                         GrGLint border,
                                                         GrGLsizei imageSize,
                                                         const GrGLvoid* data) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLCullFace(GrGLenum mode) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDepthMask(GrGLboolean flag) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDisable(GrGLenum cap) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDisableVertexAttribArray(GrGLuint index) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawArrays(GrGLenum mode,
                                               GrGLint first,
                                               GrGLsizei count) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawBuffer(GrGLenum mode) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawBuffers(GrGLsizei n,
                                                const GrGLenum* bufs) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawElements(GrGLenum mode,
                                                 GrGLsizei count,
                                                 GrGLenum type,
                                                 const GrGLvoid* indices) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLEnable(GrGLenum cap) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLEnableVertexAttribArray(GrGLuint index) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLEndQuery(GrGLenum target) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLFinish() {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLFlush() {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLFrontFace(GrGLenum mode) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLLineWidth(GrGLfloat width) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLLinkProgram(GrGLuint program) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLPixelStorei(GrGLenum pname,
                                                GrGLint param) {

    switch (pname) {
        case GR_GL_UNPACK_ROW_LENGTH:
            GrDebugGL::getInstance()->setUnPackRowLength(param);
            break;
        case GR_GL_PACK_ROW_LENGTH:
            GrDebugGL::getInstance()->setPackRowLength(param);
            break;
        case GR_GL_UNPACK_ALIGNMENT:
            break;
        case GR_GL_PACK_ALIGNMENT:
            GrAlwaysAssert(false);
            break;
        default:
            GrAlwaysAssert(false);
            break;
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLQueryCounter(GrGLuint id,
                                                 GrGLenum target) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLReadBuffer(GrGLenum src) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLReadPixels(GrGLint x,
                                               GrGLint y,
                                               GrGLsizei width,
                                               GrGLsizei height,
                                               GrGLenum format,
                                               GrGLenum type,
                                               GrGLvoid* pixels) {

    GrGLint pixelsInRow = width;
    if (0 < GrDebugGL::getInstance()->getPackRowLength()) {
        pixelsInRow = GrDebugGL::getInstance()->getPackRowLength();
    }

    GrGLint componentsPerPixel = 0;

    switch (format) {
        case GR_GL_RGBA:
            // fallthrough
        case GR_GL_BGRA:
            componentsPerPixel = 4;
            break;
        case GR_GL_RGB:
            componentsPerPixel = 3;
            break;
        case GR_GL_RED:
            componentsPerPixel = 1;
            break;
        default:
            GrAlwaysAssert(false);
            break;
    }

    GrGLint alignment = 4;  // the pack alignment (one of 1, 2, 4 or 8)
    // Ganesh currently doesn't support setting GR_GL_PACK_ALIGNMENT

    GrGLint componentSize = 0;  // size (in bytes) of a single component

    switch (type) {
        case GR_GL_UNSIGNED_BYTE:
            componentSize = 1;
            break;
        default:
            GrAlwaysAssert(false);
            break;
    }

    GrGLint rowStride = 0;  // number of components (not bytes) to skip
    if (componentSize >= alignment) {
        rowStride = componentsPerPixel * pixelsInRow;
    } else {
        float fTemp =
            sk_float_ceil(componentSize * componentsPerPixel * pixelsInRow /
                          static_cast<float>(alignment));
        rowStride = static_cast<GrGLint>(alignment * fTemp / componentSize);
    }

    GrGLchar *scanline = static_cast<GrGLchar *>(pixels);
    for (int y = 0; y < height; ++y) {
        memset(scanline, 0, componentsPerPixel * componentSize * width);
        scanline += rowStride;
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLScissor(GrGLint x,
                                            GrGLint y,
                                            GrGLsizei width,
                                            GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLShaderSource(GrGLuint shader,
                                                 GrGLsizei count,
                                                 const char** str,
                                                 const GrGLint* length) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilFunc(GrGLenum func,
                                                GrGLint ref,
                                                GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilFuncSeparate(GrGLenum face,
                                                        GrGLenum func,
                                                        GrGLint ref,
                                                        GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilMask(GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilMaskSeparate(GrGLenum face,
                                                        GrGLuint mask) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilOp(GrGLenum fail,
                                              GrGLenum zfail,
                                              GrGLenum zpass) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilOpSeparate(GrGLenum face,
                                                      GrGLenum fail,
                                                      GrGLenum zfail,
                                                      GrGLenum zpass) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexImage2D(GrGLenum target,
                                               GrGLint level,
                                               GrGLint internalformat,
                                               GrGLsizei width,
                                               GrGLsizei height,
                                               GrGLint border,
                                               GrGLenum format,
                                               GrGLenum type,
                                               const GrGLvoid* pixels) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexParameteri(GrGLenum target,
                                                  GrGLenum pname,
                                                  GrGLint param) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexParameteriv(GrGLenum target,
                                                   GrGLenum pname,
                                                   const GrGLint* params) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexStorage2D(GrGLenum target,
                                                 GrGLsizei levels,
                                                 GrGLenum internalformat,
                                                 GrGLsizei width,
                                                 GrGLsizei height) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexSubImage2D(GrGLenum target,
                                                  GrGLint level,
                                                  GrGLint xoffset,
                                                  GrGLint yoffset,
                                                  GrGLsizei width,
                                                  GrGLsizei height,
                                                  GrGLenum format,
                                                  GrGLenum type,
                                                  const GrGLvoid* pixels) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1f(GrGLint location,
                                              GrGLfloat v0) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1i(GrGLint location,
                                              GrGLint v0) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1fv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1iv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2f(GrGLint location,
                                              GrGLfloat v0,
                                              GrGLfloat v1) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2i(GrGLint location,
                                              GrGLint v0,
                                              GrGLint v1) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2fv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2iv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3f(GrGLint location,
                                              GrGLfloat v0,
                                              GrGLfloat v1,
                                              GrGLfloat v2) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3i(GrGLint location,
                                              GrGLint v0,
                                              GrGLint v1,
                                              GrGLint v2) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3fv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLfloat* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3iv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLint* v) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4f(GrGLint location,
                                              GrGLfloat v0,
                                              GrGLfloat v1,
                                              GrGLfloat v2,
                                              GrGLfloat v3) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4i(GrGLint location,
                                              GrGLint v0,
                                              GrGLint v1,
                                              GrGLint v2,
                                              GrGLint v3) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4fv(GrGLint location,
                                               GrGLsizei count,
                                               const GrGLfloat* v) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4iv(GrGLint location,
                                                GrGLsizei count,
                                                const GrGLint* v) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniformMatrix2fv(GrGLint location,
                                                      GrGLsizei count,
                                                      GrGLboolean transpose,
                                                      const GrGLfloat* value) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniformMatrix3fv(GrGLint location,
                                                      GrGLsizei count,
                                                      GrGLboolean transpose,
                                                      const GrGLfloat* value) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniformMatrix4fv(GrGLint location,
                                                      GrGLsizei count,
                                                      GrGLboolean transpose,
                                                      const GrGLfloat* value) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLUseProgram(GrGLuint programID) {

     // A programID of 0 is legal
     GrProgramObj *program = GR_FIND(programID,
                                     GrProgramObj,
                                     GrDebugGL::kProgram_ObjTypes);

     GrDebugGL::getInstance()->useProgram(program);
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLVertexAttrib4fv(GrGLuint indx,
                                                     const GrGLfloat* values) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLVertexAttribPointer(GrGLuint indx,
                                                         GrGLint size,
                                                         GrGLenum type,
                                                         GrGLboolean normalized,
                                                         GrGLsizei stride,
                                                         const GrGLvoid* ptr) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLViewport(GrGLint x,
                                              GrGLint y,
                                              GrGLsizei width,
                                              GrGLsizei height) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFramebuffer(GrGLenum target,
                                                     GrGLuint frameBufferID) {

     GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);

     // a frameBufferID of 0 is acceptable - it binds to the default
     // frame buffer
     GrFrameBufferObj *frameBuffer = GR_FIND(frameBufferID,
                                             GrFrameBufferObj,
                                             GrDebugGL::kFrameBuffer_ObjTypes);

     GrDebugGL::getInstance()->setFrameBuffer(frameBuffer);
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindRenderbuffer(GrGLenum target,
                                                      GrGLuint renderBufferID) {

     GrAlwaysAssert(GR_GL_RENDERBUFFER == target);

     // a renderBufferID of 0 is acceptable - it unbinds the bound render buffer
     GrRenderBufferObj *renderBuffer = GR_FIND(renderBufferID,
                                               GrRenderBufferObj,
                                               GrDebugGL::kRenderBuffer_ObjTypes);

     GrDebugGL::getInstance()->setRenderBuffer(renderBuffer);
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteTextures(GrGLsizei n,
                                                    const GrGLuint* textures) {

     // first potentially unbind the texture
     // TODO: move this into GrDebugGL as unBindTexture?
     for (unsigned int i = 0;
          i < GrDebugGL::getInstance()->getMaxTextureUnits();
          ++i) {
         GrTextureUnitObj *pTU = GrDebugGL::getInstance()->getTextureUnit(i);

         if (pTU->getTexture()) {
             for (int j = 0; j < n; ++j) {

                 if (textures[j] == pTU->getTexture()->getID()) {
                     // this ID is the current texture - revert the binding to 0
                     pTU->setTexture(NULL);
                 }
             }
         }
     }

     // TODO: fuse the following block with DeleteRenderBuffers?
     // Open GL will remove a deleted render buffer from the active
     // frame buffer but not from any other frame buffer
     if (GrDebugGL::getInstance()->getFrameBuffer()) {

         GrFrameBufferObj *frameBuffer = GrDebugGL::getInstance()->getFrameBuffer();

         for (int i = 0; i < n; ++i) {

             if (NULL != frameBuffer->getColor() &&
                 textures[i] == frameBuffer->getColor()->getID()) {
                 frameBuffer->setColor(NULL);
             }
             if (NULL != frameBuffer->getDepth() &&
                 textures[i] == frameBuffer->getDepth()->getID()) {
                 frameBuffer->setDepth(NULL);
             }
             if (NULL != frameBuffer->getStencil() &&
                 textures[i] == frameBuffer->getStencil()->getID()) {
                 frameBuffer->setStencil(NULL);
             }
         }
     }

     // then actually "delete" the buffers
     for (int i = 0; i < n; ++i) {
         GrTextureObj *buffer = GR_FIND(textures[i],
                                        GrTextureObj,
                                        GrDebugGL::kTexture_ObjTypes);
         GrAlwaysAssert(buffer);

         // OpenGL gives no guarantees if a texture is deleted while attached to
         // something other than the currently bound frame buffer
         GrAlwaysAssert(!buffer->getBound());

         GrAlwaysAssert(!buffer->getDeleted());
         buffer->deleteAction();
     }

 }


 GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteFramebuffers(GrGLsizei n,
                                                        const GrGLuint *frameBuffers) {

     // first potentially unbind the buffers
     if (GrDebugGL::getInstance()->getFrameBuffer()) {
         for (int i = 0; i < n; ++i) {

             if (frameBuffers[i] ==
                 GrDebugGL::getInstance()->getFrameBuffer()->getID()) {
                 // this ID is the current frame buffer - rebind to the default
                 GrDebugGL::getInstance()->setFrameBuffer(NULL);
             }
         }
     }

     // then actually "delete" the buffers
     for (int i = 0; i < n; ++i) {
         GrFrameBufferObj *buffer = GR_FIND(frameBuffers[i],
                                            GrFrameBufferObj,
                                            GrDebugGL::kFrameBuffer_ObjTypes);
         GrAlwaysAssert(buffer);

         GrAlwaysAssert(!buffer->getDeleted());
         buffer->deleteAction();
     }
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteRenderbuffers(GrGLsizei n,
                                                         const GrGLuint *renderBuffers) {

     // first potentially unbind the buffers
     if (GrDebugGL::getInstance()->getRenderBuffer()) {
         for (int i = 0; i < n; ++i) {

             if (renderBuffers[i] ==
                 GrDebugGL::getInstance()->getRenderBuffer()->getID()) {
                 // this ID is the current render buffer - make no
                 // render buffer be bound
                 GrDebugGL::getInstance()->setRenderBuffer(NULL);
             }
         }
     }

     // TODO: fuse the following block with DeleteTextures?
     // Open GL will remove a deleted render buffer from the active frame
     // buffer but not from any other frame buffer
     if (GrDebugGL::getInstance()->getFrameBuffer()) {

         GrFrameBufferObj *frameBuffer =
                               GrDebugGL::getInstance()->getFrameBuffer();

         for (int i = 0; i < n; ++i) {

             if (NULL != frameBuffer->getColor() &&
                 renderBuffers[i] == frameBuffer->getColor()->getID()) {
                 frameBuffer->setColor(NULL);
             }
             if (NULL != frameBuffer->getDepth() &&
                 renderBuffers[i] == frameBuffer->getDepth()->getID()) {
                 frameBuffer->setDepth(NULL);
             }
             if (NULL != frameBuffer->getStencil() &&
                 renderBuffers[i] == frameBuffer->getStencil()->getID()) {
                 frameBuffer->setStencil(NULL);
             }
         }
     }

     // then actually "delete" the buffers
     for (int i = 0; i < n; ++i) {
         GrRenderBufferObj *buffer = GR_FIND(renderBuffers[i],
                                             GrRenderBufferObj,
                                             GrDebugGL::kRenderBuffer_ObjTypes);
         GrAlwaysAssert(buffer);

         // OpenGL gives no guarantees if a render buffer is deleted
         // while attached to something other than the currently
         // bound frame buffer
         GrAlwaysAssert(!buffer->getColorBound());
         GrAlwaysAssert(!buffer->getDepthBound());
         GrAlwaysAssert(!buffer->getStencilBound());

         GrAlwaysAssert(!buffer->getDeleted());
         buffer->deleteAction();
     }
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLFramebufferRenderbuffer(GrGLenum target,
                                                             GrGLenum attachment,
                                                             GrGLenum renderbuffertarget,
                                                             GrGLuint renderBufferID) {

     GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);
     GrAlwaysAssert(GR_GL_COLOR_ATTACHMENT0 == attachment ||
                    GR_GL_DEPTH_ATTACHMENT == attachment ||
                    GR_GL_STENCIL_ATTACHMENT == attachment);
     GrAlwaysAssert(GR_GL_RENDERBUFFER == renderbuffertarget);

     GrFrameBufferObj *framebuffer = GrDebugGL::getInstance()->getFrameBuffer();
     // A render buffer cannot be attached to the default framebuffer
     GrAlwaysAssert(NULL != framebuffer);

     // a renderBufferID of 0 is acceptable - it unbinds the current
     // render buffer
     GrRenderBufferObj *renderbuffer = GR_FIND(renderBufferID,
                                               GrRenderBufferObj,
                                               GrDebugGL::kRenderBuffer_ObjTypes);

     switch (attachment) {
     case GR_GL_COLOR_ATTACHMENT0:
         framebuffer->setColor(renderbuffer);
         break;
     case GR_GL_DEPTH_ATTACHMENT:
         framebuffer->setDepth(renderbuffer);
         break;
     case GR_GL_STENCIL_ATTACHMENT:
         framebuffer->setStencil(renderbuffer);
         break;
     default:
         GrAlwaysAssert(false);
         break;
     };

 }

 ////////////////////////////////////////////////////////////////////////////////
 GrGLvoid GR_GL_FUNCTION_TYPE debugGLFramebufferTexture2D(GrGLenum target,
                                                          GrGLenum attachment,
                                                          GrGLenum textarget,
                                                          GrGLuint textureID,
                                                          GrGLint level) {

     GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);
     GrAlwaysAssert(GR_GL_COLOR_ATTACHMENT0 == attachment ||
                    GR_GL_DEPTH_ATTACHMENT == attachment ||
                    GR_GL_STENCIL_ATTACHMENT == attachment);
     GrAlwaysAssert(GR_GL_TEXTURE_2D == textarget);

     GrFrameBufferObj *framebuffer = GrDebugGL::getInstance()->getFrameBuffer();
     // A texture cannot be attached to the default framebuffer
     GrAlwaysAssert(NULL != framebuffer);

     // A textureID of 0 is allowed - it unbinds the currently bound texture
     GrTextureObj *texture = GR_FIND(textureID, GrTextureObj,
                                     GrDebugGL::kTexture_ObjTypes);
     if (texture) {
         // The texture shouldn't be bound to a texture unit - this
         // could lead to a feedback loop
         GrAlwaysAssert(!texture->getBound());
     }

     GrAlwaysAssert(0 == level);

     switch (attachment) {
     case GR_GL_COLOR_ATTACHMENT0:
         framebuffer->setColor(texture);
         break;
     case GR_GL_DEPTH_ATTACHMENT:
         framebuffer->setDepth(texture);
         break;
     case GR_GL_STENCIL_ATTACHMENT:
         framebuffer->setStencil(texture);
         break;
     default:
         GrAlwaysAssert(false);
         break;
     };
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetFramebufferAttachmentParameteriv(GrGLenum target,
                                                                         GrGLenum attachment,
                                                                         GrGLenum pname,
                                                                         GrGLint* params) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetRenderbufferParameteriv(GrGLenum target,
                                                                GrGLenum pname,
                                                                GrGLint* params) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLRenderbufferStorage(GrGLenum target,
                                                         GrGLenum internalformat,
                                                         GrGLsizei width,
                                                         GrGLsizei height) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLRenderbufferStorageMultisample(GrGLenum target,
                                                                    GrGLsizei samples,
                                                                    GrGLenum internalformat,
                                                                    GrGLsizei width,
                                                                    GrGLsizei height) {
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlitFramebuffer(GrGLint srcX0,
                                                     GrGLint srcY0,
                                                     GrGLint srcX1,
                                                     GrGLint srcY1,
                                                     GrGLint dstX0,
                                                    GrGLint dstY0,
                                                    GrGLint dstX1,
                                                    GrGLint dstY1,
                                                    GrGLbitfield mask,
                                                    GrGLenum filter) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLResolveMultisampleFramebuffer() {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFragDataLocationIndexed(GrGLuint program,
                                                                GrGLuint colorNumber,
                                                                GrGLuint index,
                                                                const GrGLchar * name) {
}

GrGLenum GR_GL_FUNCTION_TYPE debugGLCheckFramebufferStatus(GrGLenum target) {

    GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);

    return GR_GL_FRAMEBUFFER_COMPLETE;
}

GrGLuint GR_GL_FUNCTION_TYPE debugGLCreateProgram() {

    GrProgramObj *program = GR_CREATE(GrProgramObj,
                                      GrDebugGL::kProgram_ObjTypes);

    return program->getID();
}

GrGLuint GR_GL_FUNCTION_TYPE debugGLCreateShader(GrGLenum type) {

    GrAlwaysAssert(GR_GL_VERTEX_SHADER == type ||
                   GR_GL_FRAGMENT_SHADER == type);

    GrShaderObj *shader = GR_CREATE(GrShaderObj, GrDebugGL::kShader_ObjTypes);
    shader->setType(type);

    return shader->getID();
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteProgram(GrGLuint programID) {

    GrProgramObj *program = GR_FIND(programID,
                                    GrProgramObj,
                                    GrDebugGL::kProgram_ObjTypes);
    GrAlwaysAssert(program);

    if (program->getRefCount()) {
        // someone is still using this program so we can't delete it here
        program->setMarkedForDeletion();
    } else {
        program->deleteAction();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteShader(GrGLuint shaderID) {

    GrShaderObj *shader = GR_FIND(shaderID,
                                  GrShaderObj,
                                  GrDebugGL::kShader_ObjTypes);
    GrAlwaysAssert(shader);

    if (shader->getRefCount()) {
        // someone is still using this shader so we can't delete it here
        shader->setMarkedForDeletion();
    } else {
        shader->deleteAction();
    }
}

// same function used for all glGen*(GLsize i, GLuint*) functions
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenIds(GrGLsizei n, GrGLuint* ids) {
    static int gCurrID = 1;
    for (int i = 0; i < n; ++i) {
        ids[i] = ++gCurrID;
    }
}

GrGLvoid debugGenObjs(GrDebugGL::GrObjTypes type,
                      GrGLsizei n,
                      GrGLuint* ids) {

   for (int i = 0; i < n; ++i) {
        GrFakeRefObj *obj = GrDebugGL::getInstance()->createObj(type);
        GrAlwaysAssert(obj);
        ids[i] = obj->getID();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenBuffers(GrGLsizei n, GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kBuffer_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenFramebuffers(GrGLsizei n,
                                                    GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kFrameBuffer_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenRenderbuffers(GrGLsizei n,
                                                     GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kRenderBuffer_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenTextures(GrGLsizei n, GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kTexture_ObjTypes, n, ids);
}

// same delete function for all glDelete*(GLsize i, const GLuint*) except
// buffers
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteIds(GrGLsizei n,
                                              const GrGLuint* ids) {
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindBuffer(GrGLenum target,
                                               GrGLuint bufferID) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target ||
                   GR_GL_ELEMENT_ARRAY_BUFFER == target);

    GrBufferObj *buffer = GR_FIND(bufferID,
                                  GrBufferObj,
                                  GrDebugGL::kBuffer_ObjTypes);
    // 0 is a permissable bufferID - it unbinds the current buffer

    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            GrDebugGL::getInstance()->setArrayBuffer(buffer);
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            GrDebugGL::getInstance()->setElementArrayBuffer(buffer);
            break;
        default:
            GrCrash("Unexpected target to glBindBuffer");
            break;
    }
}

// deleting a bound buffer has the side effect of binding 0
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteBuffers(GrGLsizei n,
                                                  const GrGLuint* ids) {
    // first potentially unbind the buffers
    for (int i = 0; i < n; ++i) {

        if (GrDebugGL::getInstance()->getArrayBuffer() &&
            ids[i] == GrDebugGL::getInstance()->getArrayBuffer()->getID()) {
            // this ID is the current array buffer
            GrDebugGL::getInstance()->setArrayBuffer(NULL);
        }
        if (GrDebugGL::getInstance()->getElementArrayBuffer() &&
            ids[i] ==
                GrDebugGL::getInstance()->getElementArrayBuffer()->getID()) {
            // this ID is the current element array buffer
            GrDebugGL::getInstance()->setElementArrayBuffer(NULL);
        }
    }

    // then actually "delete" the buffers
    for (int i = 0; i < n; ++i) {
        GrBufferObj *buffer = GR_FIND(ids[i],
                                      GrBufferObj,
                                      GrDebugGL::kBuffer_ObjTypes);
        GrAlwaysAssert(buffer);

        GrAlwaysAssert(!buffer->getDeleted());
        buffer->deleteAction();
    }
}

// map a buffer to the caller's address space
GrGLvoid* GR_GL_FUNCTION_TYPE debugGLMapBuffer(GrGLenum target,
                                               GrGLenum access) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target ||
                   GR_GL_ELEMENT_ARRAY_BUFFER == target);
    // GR_GL_READ_ONLY == access ||  || GR_GL_READ_WRIT == access);
    GrAlwaysAssert(GR_GL_WRITE_ONLY == access);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
        default:
            GrCrash("Unexpected target to glMapBuffer");
            break;
    }

    if (buffer) {
        GrAlwaysAssert(!buffer->getMapped());
        buffer->setMapped();
        return buffer->getDataPtr();
    }

    GrAlwaysAssert(false);
    return NULL;        // no buffer bound to the target
}

// remove a buffer from the caller's address space
// TODO: check if the "access" method from "glMapBuffer" was honored
GrGLboolean GR_GL_FUNCTION_TYPE debugGLUnmapBuffer(GrGLenum target) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target ||
                   GR_GL_ELEMENT_ARRAY_BUFFER == target);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
        default:
            GrCrash("Unexpected target to glUnmapBuffer");
            break;
    }

    if (buffer) {
        GrAlwaysAssert(buffer->getMapped());
        buffer->resetMapped();
        return GR_GL_TRUE;
    }

    GrAlwaysAssert(false);
    return GR_GL_FALSE; // GR_GL_INVALID_OPERATION;
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetBufferParameteriv(GrGLenum target,
                                                         GrGLenum value,
                                                         GrGLint* params) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target ||
                   GR_GL_ELEMENT_ARRAY_BUFFER == target);
    GrAlwaysAssert(GR_GL_BUFFER_SIZE == value ||
                   GR_GL_BUFFER_USAGE == value);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
    }

    GrAlwaysAssert(buffer);

    switch (value) {
        case GR_GL_BUFFER_MAPPED:
            *params = GR_GL_FALSE;
            if (buffer)
                *params = buffer->getMapped() ? GR_GL_TRUE : GR_GL_FALSE;
            break;
        case GR_GL_BUFFER_SIZE:
            *params = 0;
            if (buffer)
                *params = buffer->getSize();
            break;
        case GR_GL_BUFFER_USAGE:
            *params = GR_GL_STATIC_DRAW;
            if (buffer)
                *params = buffer->getUsage();
            break;
        default:
            GrCrash("Unexpected value to glGetBufferParamateriv");
            break;
    }
};

GrGLenum GR_GL_FUNCTION_TYPE debugGLGetError() {
    return GR_GL_NO_ERROR;
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetIntegerv(GrGLenum pname,
                                                GrGLint* params) {
    // TODO: remove from Ganesh the #defines for gets we don't use.
    // We would like to minimize gets overall due to performance issues
    switch (pname) {
        case GR_GL_STENCIL_BITS:
            *params = 8;
            break;
        case GR_GL_SAMPLES:
            *params = 1;
            break;
        case GR_GL_FRAMEBUFFER_BINDING:
            *params = 0;
            break;
        case GR_GL_VIEWPORT:
            params[0] = 0;
            params[1] = 0;
            params[2] = 800;
            params[3] = 600;
            break;
        case GR_GL_MAX_TEXTURE_IMAGE_UNITS:
            *params = 8;
            break;
        case GR_GL_MAX_VERTEX_UNIFORM_VECTORS:
            *params = kDefaultMaxVertexUniformVectors;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            *params = kDefaultMaxFragmentUniformVectors;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
            *params = 16 * 4;
            break;
        case GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS:
            *params = 0;
            break;
        case GR_GL_COMPRESSED_TEXTURE_FORMATS:
            break;
        case GR_GL_MAX_TEXTURE_SIZE:
            *params = 8192;
            break;
        case GR_GL_MAX_RENDERBUFFER_SIZE:
            *params = 8192;
            break;
        case GR_GL_MAX_SAMPLES:
            *params = 32;
            break;
        case GR_GL_MAX_VERTEX_ATTRIBS:
            *params = kDefaultMaxVertexAttribs;
            break;
        case GR_GL_MAX_VARYING_VECTORS:
            *params = kDefaultMaxVaryingVectors;
            break;
        default:
            GrCrash("Unexpected pname to GetIntegerv");
    }
}
// used for both the program and shader info logs
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetInfoLog(GrGLuint program,
                                               GrGLsizei bufsize,
                                               GrGLsizei* length,
                                               char* infolog) {
    if (length) {
        *length = 0;
    }
    if (bufsize > 0) {
        *infolog = 0;
    }
}

// used for both the program and shader params
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetShaderOrProgramiv(GrGLuint program,
                                                         GrGLenum pname,
                                                         GrGLint* params) {
    switch (pname) {
        case GR_GL_LINK_STATUS:  // fallthru
        case GR_GL_COMPILE_STATUS:
            *params = GR_GL_TRUE;
            break;
        case GR_GL_INFO_LOG_LENGTH:
            *params = 0;
            break;
        // we don't expect any other pnames
        default:
            GrCrash("Unexpected pname to GetProgramiv");
            break;
    }
}

namespace {
template <typename T>
void query_result(GrGLenum GLtarget, GrGLenum pname, T *params) {
    switch (pname) {
        case GR_GL_QUERY_RESULT_AVAILABLE:
            *params = GR_GL_TRUE;
            break;
        case GR_GL_QUERY_RESULT:
            *params = 0;
            break;
        default:
            GrCrash("Unexpected pname passed to GetQueryObject.");
            break;
    }
}
}

// Queries on the null GL just don't do anything at all. We could potentially
// make the timers work.
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryiv(GrGLenum GLtarget,
                                               GrGLenum pname,
                                               GrGLint *params) {
    switch (pname) {
        case GR_GL_CURRENT_QUERY:
            *params = 0;
            break;
        case GR_GL_QUERY_COUNTER_BITS:
            *params = 32;
            break;
        default:
            GrCrash("Unexpected pname passed GetQueryiv.");
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjecti64v(GrGLuint id,
                                                       GrGLenum pname,
                                                       GrGLint64 *params) {
    query_result(id, pname, params);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjectiv(GrGLuint id,
                                                     GrGLenum pname,
                                                     GrGLint *params) {
    query_result(id, pname, params);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjectui64v(GrGLuint id,
                                                        GrGLenum pname,
                                                        GrGLuint64 *params) {
    query_result(id, pname, params);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjectuiv(GrGLuint id,
                                                      GrGLenum pname,
                                                      GrGLuint *params) {
    query_result(id, pname, params);
}

const GrGLubyte* GR_GL_FUNCTION_TYPE debugGLGetString(GrGLenum name) {
    switch (name) {
        case GR_GL_EXTENSIONS:
            return (const GrGLubyte*)"GL_ARB_framebuffer_object GL_ARB_blend_func_extended GL_ARB_timer_query GL_ARB_draw_buffers GL_ARB_occlusion_query GL_EXT_blend_color GL_EXT_stencil_wrap";
        case GR_GL_VERSION:
            return (const GrGLubyte*)"4.0 Debug GL";
        case GR_GL_SHADING_LANGUAGE_VERSION:
            return (const GrGLubyte*)"4.20.8 Debug GLSL";
        case GR_GL_VENDOR:
            return (const GrGLubyte*)"Debug Vendor";
        case GR_GL_RENDERER:
            return (const GrGLubyte*)"The Debug (Non-)Renderer";
        default:
            GrCrash("Unexpected name to GetString");
            return NULL;
    }
}

// we used to use this to query stuff about externally created textures,
// now we just require clients to tell us everything about the texture.
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetTexLevelParameteriv(GrGLenum target,
                                                           GrGLint level,
                                                           GrGLenum pname,
                                                           GrGLint* params) {
    GrCrash("Should never query texture parameters.");
}

GrGLint GR_GL_FUNCTION_TYPE debugGLGetUniformLocation(GrGLuint program,
                                                      const char* name) {
    static int gUniLocation = 0;
    return ++gUniLocation;
}

} // end of namespace

////////////////////////////////////////////////////////////////////////////////
struct GrDebugGLInterface : public GrGLInterface {

public:
    SK_DECLARE_INST_COUNT(GrDebugGLInterface)

    GrDebugGLInterface()
        : fWrapped(NULL) {
        GrDebugGL::staticRef();
    }

    virtual ~GrDebugGLInterface() {
        GrDebugGL::staticUnRef();
    }

    void setWrapped(GrGLInterface *interface) {
        fWrapped.reset(interface);
    }

    // TODO: there are some issues w/ wrapping another GL interface inside the
    // debug interface:
    //      Since none of the "gl" methods are member functions they don't get
    //      a "this" pointer through which to access "fWrapped"
    //      This could be worked around by having all of them access the
    //      "glInterface" pointer - i.e., treating the debug interface as a
    //      true singleton
    //
    //      The problem with this is that we also want to handle OpenGL
    //      contexts. The natural way to do this is to have multiple debug
    //      interfaces. Each of which represents a separate context. The
    //      static ID count would still uniquify IDs across all of them.
    //      The problem then is that we couldn't treat the debug GL
    //      interface as a singleton (since there would be one for each
    //      context).
    //
    //      The solution to this is probably to alter SkDebugGlContext's
    //      "makeCurrent" method to make a call like "makeCurrent(this)" to
    //      the debug GL interface (assuming that the application will create
    //      multiple SkGLContext's) to let it switch between the active
    //      context. Everything in the GrDebugGL object would then need to be
    //      moved to a GrContextObj and the GrDebugGL object would just switch
    //      between them. Note that this approach would also require that
    //      SkDebugGLContext wrap an arbitrary other context
    //      and then pass the wrapped interface to the debug GL interface.

protected:
private:

    SkAutoTUnref<GrGLInterface> fWrapped;

    typedef GrGLInterface INHERITED;
};

SK_DEFINE_INST_COUNT(GrDebugGLInterface)

////////////////////////////////////////////////////////////////////////////////
const GrGLInterface* GrGLCreateDebugInterface() {
    GrGLInterface* interface = SkNEW(GrDebugGLInterface);

    interface->fBindingsExported = kDesktop_GrGLBinding;
    interface->fActiveTexture = debugGLActiveTexture;
    interface->fAttachShader = debugGLAttachShader;
    interface->fBeginQuery = debugGLBeginQuery;
    interface->fBindAttribLocation = debugGLBindAttribLocation;
    interface->fBindBuffer = debugGLBindBuffer;
    interface->fBindFragDataLocation = debugGLBindFragDataLocation;
    interface->fBindTexture = debugGLBindTexture;
    interface->fBlendColor = debugGLBlendColor;
    interface->fBlendFunc = debugGLBlendFunc;
    interface->fBufferData = debugGLBufferData;
    interface->fBufferSubData = debugGLBufferSubData;
    interface->fClear = debugGLClear;
    interface->fClearColor = debugGLClearColor;
    interface->fClearStencil = debugGLClearStencil;
    interface->fColorMask = debugGLColorMask;
    interface->fCompileShader = debugGLCompileShader;
    interface->fCompressedTexImage2D = debugGLCompressedTexImage2D;
    interface->fCreateProgram = debugGLCreateProgram;
    interface->fCreateShader = debugGLCreateShader;
    interface->fCullFace = debugGLCullFace;
    interface->fDeleteBuffers = debugGLDeleteBuffers;
    interface->fDeleteProgram = debugGLDeleteProgram;
    interface->fDeleteQueries = debugGLDeleteIds;
    interface->fDeleteShader = debugGLDeleteShader;
    interface->fDeleteTextures = debugGLDeleteTextures;
    interface->fDepthMask = debugGLDepthMask;
    interface->fDisable = debugGLDisable;
    interface->fDisableVertexAttribArray = debugGLDisableVertexAttribArray;
    interface->fDrawArrays = debugGLDrawArrays;
    interface->fDrawBuffer = debugGLDrawBuffer;
    interface->fDrawBuffers = debugGLDrawBuffers;
    interface->fDrawElements = debugGLDrawElements;
    interface->fEnable = debugGLEnable;
    interface->fEnableVertexAttribArray = debugGLEnableVertexAttribArray;
    interface->fEndQuery = debugGLEndQuery;
    interface->fFinish = debugGLFinish;
    interface->fFlush = debugGLFlush;
    interface->fFrontFace = debugGLFrontFace;
    interface->fGenBuffers = debugGLGenBuffers;
    interface->fGenQueries = debugGLGenIds;
    interface->fGenTextures = debugGLGenTextures;
    interface->fGetBufferParameteriv = debugGLGetBufferParameteriv;
    interface->fGetError = debugGLGetError;
    interface->fGetIntegerv = debugGLGetIntegerv;
    interface->fGetQueryObjecti64v = debugGLGetQueryObjecti64v;
    interface->fGetQueryObjectiv = debugGLGetQueryObjectiv;
    interface->fGetQueryObjectui64v = debugGLGetQueryObjectui64v;
    interface->fGetQueryObjectuiv = debugGLGetQueryObjectuiv;
    interface->fGetQueryiv = debugGLGetQueryiv;
    interface->fGetProgramInfoLog = debugGLGetInfoLog;
    interface->fGetProgramiv = debugGLGetShaderOrProgramiv;
    interface->fGetShaderInfoLog = debugGLGetInfoLog;
    interface->fGetShaderiv = debugGLGetShaderOrProgramiv;
    interface->fGetString = debugGLGetString;
    interface->fGetTexLevelParameteriv = debugGLGetTexLevelParameteriv;
    interface->fGetUniformLocation = debugGLGetUniformLocation;
    interface->fLineWidth = debugGLLineWidth;
    interface->fLinkProgram = debugGLLinkProgram;
    interface->fPixelStorei = debugGLPixelStorei;
    interface->fQueryCounter = debugGLQueryCounter;
    interface->fReadBuffer = debugGLReadBuffer;
    interface->fReadPixels = debugGLReadPixels;
    interface->fScissor = debugGLScissor;
    interface->fShaderSource = debugGLShaderSource;
    interface->fStencilFunc = debugGLStencilFunc;
    interface->fStencilFuncSeparate = debugGLStencilFuncSeparate;
    interface->fStencilMask = debugGLStencilMask;
    interface->fStencilMaskSeparate = debugGLStencilMaskSeparate;
    interface->fStencilOp = debugGLStencilOp;
    interface->fStencilOpSeparate = debugGLStencilOpSeparate;
    interface->fTexImage2D = debugGLTexImage2D;
    interface->fTexParameteri = debugGLTexParameteri;
    interface->fTexParameteriv = debugGLTexParameteriv;
    interface->fTexSubImage2D = debugGLTexSubImage2D;
    interface->fTexStorage2D = debugGLTexStorage2D;
    interface->fUniform1f = debugGLUniform1f;
    interface->fUniform1i = debugGLUniform1i;
    interface->fUniform1fv = debugGLUniform1fv;
    interface->fUniform1iv = debugGLUniform1iv;
    interface->fUniform2f = debugGLUniform2f;
    interface->fUniform2i = debugGLUniform2i;
    interface->fUniform2fv = debugGLUniform2fv;
    interface->fUniform2iv = debugGLUniform2iv;
    interface->fUniform3f = debugGLUniform3f;
    interface->fUniform3i = debugGLUniform3i;
    interface->fUniform3fv = debugGLUniform3fv;
    interface->fUniform3iv = debugGLUniform3iv;
    interface->fUniform4f = debugGLUniform4f;
    interface->fUniform4i = debugGLUniform4i;
    interface->fUniform4fv = debugGLUniform4fv;
    interface->fUniform4iv = debugGLUniform4iv;
    interface->fUniformMatrix2fv = debugGLUniformMatrix2fv;
    interface->fUniformMatrix3fv = debugGLUniformMatrix3fv;
    interface->fUniformMatrix4fv = debugGLUniformMatrix4fv;
    interface->fUseProgram = debugGLUseProgram;
    interface->fVertexAttrib4fv = debugGLVertexAttrib4fv;
    interface->fVertexAttribPointer = debugGLVertexAttribPointer;
    interface->fViewport = debugGLViewport;
    interface->fBindFramebuffer = debugGLBindFramebuffer;
    interface->fBindRenderbuffer = debugGLBindRenderbuffer;
    interface->fCheckFramebufferStatus = debugGLCheckFramebufferStatus;
    interface->fDeleteFramebuffers = debugGLDeleteFramebuffers;
    interface->fDeleteRenderbuffers = debugGLDeleteRenderbuffers;
    interface->fFramebufferRenderbuffer = debugGLFramebufferRenderbuffer;
    interface->fFramebufferTexture2D = debugGLFramebufferTexture2D;
    interface->fGenFramebuffers = debugGLGenFramebuffers;
    interface->fGenRenderbuffers = debugGLGenRenderbuffers;
    interface->fGetFramebufferAttachmentParameteriv =
                                    debugGLGetFramebufferAttachmentParameteriv;
    interface->fGetRenderbufferParameteriv = debugGLGetRenderbufferParameteriv;
    interface->fRenderbufferStorage = debugGLRenderbufferStorage;
    interface->fRenderbufferStorageMultisample =
                                    debugGLRenderbufferStorageMultisample;
    interface->fBlitFramebuffer = debugGLBlitFramebuffer;
    interface->fResolveMultisampleFramebuffer =
                                    debugGLResolveMultisampleFramebuffer;
    interface->fMapBuffer = debugGLMapBuffer;
    interface->fUnmapBuffer = debugGLUnmapBuffer;
    interface->fBindFragDataLocationIndexed =
                                    debugGLBindFragDataLocationIndexed;

    return interface;
}

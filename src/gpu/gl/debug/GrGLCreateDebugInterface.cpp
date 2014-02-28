
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
#include "GrVertexArrayObj.h"
#include "SkFloatingPoint.h"
#include "../GrGLNoOpInterface.h"

namespace { // suppress no previous prototype warning

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

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLUseProgram(GrGLuint programID) {

     // A programID of 0 is legal
     GrProgramObj *program = GR_FIND(programID,
                                     GrProgramObj,
                                     GrDebugGL::kProgram_ObjTypes);

     GrDebugGL::getInstance()->useProgram(program);
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFramebuffer(GrGLenum target,
                                                     GrGLuint frameBufferID) {

     GrAlwaysAssert(GR_GL_FRAMEBUFFER == target ||
                    GR_GL_READ_FRAMEBUFFER == target ||
                    GR_GL_DRAW_FRAMEBUFFER);

     // a frameBufferID of 0 is acceptable - it binds to the default
     // frame buffer
     GrFrameBufferObj *frameBuffer = GR_FIND(frameBufferID,
                                             GrFrameBufferObj,
                                             GrDebugGL::kFrameBuffer_ObjTypes);

     GrDebugGL::getInstance()->setFrameBuffer(frameBuffer);
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindRenderbuffer(GrGLenum target, GrGLuint renderBufferID) {

     GrAlwaysAssert(GR_GL_RENDERBUFFER == target);

     // a renderBufferID of 0 is acceptable - it unbinds the bound render buffer
     GrRenderBufferObj *renderBuffer = GR_FIND(renderBufferID,
                                               GrRenderBufferObj,
                                               GrDebugGL::kRenderBuffer_ObjTypes);

     GrDebugGL::getInstance()->setRenderBuffer(renderBuffer);
 }

 GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteTextures(GrGLsizei n, const GrGLuint* textures) {

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
         // However, at GrContext destroy time we release all GrRsources and so stencil buffers
         // may get deleted before FBOs that refer to them.
         //GrAlwaysAssert(!buffer->getStencilBound());

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

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenerateMipmap(GrGLenum level) {
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

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenVertexArrays(GrGLsizei n, GrGLuint* ids) {
    debugGenObjs(GrDebugGL::kVertexArray_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteVertexArrays(GrGLsizei n, const GrGLuint* ids) {
    for (GrGLsizei i = 0; i < n; ++i) {
        GrVertexArrayObj* array =
            GR_FIND(ids[i], GrVertexArrayObj, GrDebugGL::kVertexArray_ObjTypes);
        GrAlwaysAssert(array);

        // Deleting the current vertex array binds object 0
        if (GrDebugGL::getInstance()->getVertexArray() == array) {
            GrDebugGL::getInstance()->setVertexArray(NULL);
        }

        if (array->getRefCount()) {
            // someone is still using this vertex array so we can't delete it here
            array->setMarkedForDeletion();
        } else {
            array->deleteAction();
        }
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindVertexArray(GrGLuint id) {
    GrVertexArrayObj* array = GR_FIND(id, GrVertexArrayObj, GrDebugGL::kVertexArray_ObjTypes);
    GrAlwaysAssert((0 == id) || NULL != array);
    GrDebugGL::getInstance()->setVertexArray(array);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindBuffer(GrGLenum target, GrGLuint bufferID) {
    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);

    GrBufferObj *buffer = GR_FIND(bufferID,
                                  GrBufferObj,
                                  GrDebugGL::kBuffer_ObjTypes);
    // 0 is a permissible bufferID - it unbinds the current buffer

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
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteBuffers(GrGLsizei n, const GrGLuint* ids) {
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
GrGLvoid* GR_GL_FUNCTION_TYPE debugGLMapBuffer(GrGLenum target, GrGLenum access) {

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
    //      multiple SkGLContextHelper's) to let it switch between the active
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

////////////////////////////////////////////////////////////////////////////////
const GrGLInterface* GrGLCreateDebugInterface() {
    GrGLInterface* interface = SkNEW(GrDebugGLInterface);

    interface->fStandard = kGL_GrGLStandard;

    GrGLInterface::Functions* functions = &interface->fFunctions;
    functions->fActiveTexture = debugGLActiveTexture;
    functions->fAttachShader = debugGLAttachShader;
    functions->fBeginQuery = debugGLBeginQuery;
    functions->fBindAttribLocation = debugGLBindAttribLocation;
    functions->fBindBuffer = debugGLBindBuffer;
    functions->fBindFragDataLocation = noOpGLBindFragDataLocation;
    functions->fBindTexture = debugGLBindTexture;
    functions->fBindVertexArray = debugGLBindVertexArray;
    functions->fBlendColor = noOpGLBlendColor;
    functions->fBlendFunc = noOpGLBlendFunc;
    functions->fBufferData = debugGLBufferData;
    functions->fBufferSubData = noOpGLBufferSubData;
    functions->fClear = noOpGLClear;
    functions->fClearColor = noOpGLClearColor;
    functions->fClearStencil = noOpGLClearStencil;
    functions->fColorMask = noOpGLColorMask;
    functions->fCompileShader = noOpGLCompileShader;
    functions->fCompressedTexImage2D = noOpGLCompressedTexImage2D;
    functions->fCopyTexSubImage2D = noOpGLCopyTexSubImage2D;
    functions->fCreateProgram = debugGLCreateProgram;
    functions->fCreateShader = debugGLCreateShader;
    functions->fCullFace = noOpGLCullFace;
    functions->fDeleteBuffers = debugGLDeleteBuffers;
    functions->fDeleteProgram = debugGLDeleteProgram;
    functions->fDeleteQueries = noOpGLDeleteIds;
    functions->fDeleteShader = debugGLDeleteShader;
    functions->fDeleteTextures = debugGLDeleteTextures;
    functions->fDeleteVertexArrays = debugGLDeleteVertexArrays;
    functions->fDepthMask = noOpGLDepthMask;
    functions->fDisable = noOpGLDisable;
    functions->fDisableVertexAttribArray = noOpGLDisableVertexAttribArray;
    functions->fDrawArrays = noOpGLDrawArrays;
    functions->fDrawBuffer = noOpGLDrawBuffer;
    functions->fDrawBuffers = noOpGLDrawBuffers;
    functions->fDrawElements = noOpGLDrawElements;
    functions->fEnable = noOpGLEnable;
    functions->fEnableVertexAttribArray = noOpGLEnableVertexAttribArray;
    functions->fEndQuery = noOpGLEndQuery;
    functions->fFinish = noOpGLFinish;
    functions->fFlush = noOpGLFlush;
    functions->fFrontFace = noOpGLFrontFace;
    functions->fGenerateMipmap = debugGLGenerateMipmap;
    functions->fGenBuffers = debugGLGenBuffers;
    functions->fGenQueries = noOpGLGenIds;
    functions->fGenTextures = debugGLGenTextures;
    functions->fGetBufferParameteriv = debugGLGetBufferParameteriv;
    functions->fGetError = noOpGLGetError;
    functions->fGetIntegerv = noOpGLGetIntegerv;
    functions->fGetQueryObjecti64v = noOpGLGetQueryObjecti64v;
    functions->fGetQueryObjectiv = noOpGLGetQueryObjectiv;
    functions->fGetQueryObjectui64v = noOpGLGetQueryObjectui64v;
    functions->fGetQueryObjectuiv = noOpGLGetQueryObjectuiv;
    functions->fGetQueryiv = noOpGLGetQueryiv;
    functions->fGetProgramInfoLog = noOpGLGetInfoLog;
    functions->fGetProgramiv = noOpGLGetShaderOrProgramiv;
    functions->fGetShaderInfoLog = noOpGLGetInfoLog;
    functions->fGetShaderiv = noOpGLGetShaderOrProgramiv;
    functions->fGetString = noOpGLGetString;
    functions->fGetStringi = noOpGLGetStringi;
    functions->fGetTexLevelParameteriv = noOpGLGetTexLevelParameteriv;
    functions->fGetUniformLocation = noOpGLGetUniformLocation;
    functions->fGenVertexArrays = debugGLGenVertexArrays;
    functions->fLoadIdentity = noOpGLLoadIdentity;
    functions->fLoadMatrixf = noOpGLLoadMatrixf;
    functions->fLineWidth = noOpGLLineWidth;
    functions->fLinkProgram = noOpGLLinkProgram;
    functions->fMatrixMode = noOpGLMatrixMode;
    functions->fPixelStorei = debugGLPixelStorei;
    functions->fQueryCounter = noOpGLQueryCounter;
    functions->fReadBuffer = noOpGLReadBuffer;
    functions->fReadPixels = debugGLReadPixels;
    functions->fScissor = noOpGLScissor;
    functions->fShaderSource = noOpGLShaderSource;
    functions->fStencilFunc = noOpGLStencilFunc;
    functions->fStencilFuncSeparate = noOpGLStencilFuncSeparate;
    functions->fStencilMask = noOpGLStencilMask;
    functions->fStencilMaskSeparate = noOpGLStencilMaskSeparate;
    functions->fStencilOp = noOpGLStencilOp;
    functions->fStencilOpSeparate = noOpGLStencilOpSeparate;
    functions->fTexGenfv = noOpGLTexGenfv;
    functions->fTexGeni = noOpGLTexGeni;
    functions->fTexImage2D = noOpGLTexImage2D;
    functions->fTexParameteri = noOpGLTexParameteri;
    functions->fTexParameteriv = noOpGLTexParameteriv;
    functions->fTexSubImage2D = noOpGLTexSubImage2D;
    functions->fTexStorage2D = noOpGLTexStorage2D;
    functions->fDiscardFramebuffer = noOpGLDiscardFramebuffer;
    functions->fUniform1f = noOpGLUniform1f;
    functions->fUniform1i = noOpGLUniform1i;
    functions->fUniform1fv = noOpGLUniform1fv;
    functions->fUniform1iv = noOpGLUniform1iv;
    functions->fUniform2f = noOpGLUniform2f;
    functions->fUniform2i = noOpGLUniform2i;
    functions->fUniform2fv = noOpGLUniform2fv;
    functions->fUniform2iv = noOpGLUniform2iv;
    functions->fUniform3f = noOpGLUniform3f;
    functions->fUniform3i = noOpGLUniform3i;
    functions->fUniform3fv = noOpGLUniform3fv;
    functions->fUniform3iv = noOpGLUniform3iv;
    functions->fUniform4f = noOpGLUniform4f;
    functions->fUniform4i = noOpGLUniform4i;
    functions->fUniform4fv = noOpGLUniform4fv;
    functions->fUniform4iv = noOpGLUniform4iv;
    functions->fUniformMatrix2fv = noOpGLUniformMatrix2fv;
    functions->fUniformMatrix3fv = noOpGLUniformMatrix3fv;
    functions->fUniformMatrix4fv = noOpGLUniformMatrix4fv;
    functions->fUseProgram = debugGLUseProgram;
    functions->fVertexAttrib4fv = noOpGLVertexAttrib4fv;
    functions->fVertexAttribPointer = noOpGLVertexAttribPointer;
    functions->fViewport = noOpGLViewport;
    functions->fBindFramebuffer = debugGLBindFramebuffer;
    functions->fBindRenderbuffer = debugGLBindRenderbuffer;
    functions->fCheckFramebufferStatus = noOpGLCheckFramebufferStatus;
    functions->fDeleteFramebuffers = debugGLDeleteFramebuffers;
    functions->fDeleteRenderbuffers = debugGLDeleteRenderbuffers;
    functions->fFramebufferRenderbuffer = debugGLFramebufferRenderbuffer;
    functions->fFramebufferTexture2D = debugGLFramebufferTexture2D;
    functions->fGenFramebuffers = debugGLGenFramebuffers;
    functions->fGenRenderbuffers = debugGLGenRenderbuffers;
    functions->fGetFramebufferAttachmentParameteriv =
                                    noOpGLGetFramebufferAttachmentParameteriv;
    functions->fGetRenderbufferParameteriv = noOpGLGetRenderbufferParameteriv;
    functions->fRenderbufferStorage = noOpGLRenderbufferStorage;
    functions->fRenderbufferStorageMultisample =
                                    noOpGLRenderbufferStorageMultisample;
    functions->fBlitFramebuffer = noOpGLBlitFramebuffer;
    functions->fResolveMultisampleFramebuffer =
                                    noOpGLResolveMultisampleFramebuffer;
    functions->fMapBuffer = debugGLMapBuffer;
    functions->fUnmapBuffer = debugGLUnmapBuffer;
    functions->fBindFragDataLocationIndexed =
                                    noOpGLBindFragDataLocationIndexed;

    interface->fExtensions.init(kGL_GrGLStandard, functions->fGetString, functions->fGetStringi,
                                functions->fGetIntegerv);

    return interface;
}

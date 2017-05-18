
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DebugGLTestContext.h"

#include "GrBufferObj.h"
#include "GrFrameBufferObj.h"
#include "GrProgramObj.h"
#include "GrRenderBufferObj.h"
#include "GrShaderObj.h"
#include "GrTextureObj.h"
#include "GrTextureUnitObj.h"
#include "GrVertexArrayObj.h"
#include "gl/GrGLTestInterface.h"

#include "SkMutex.h"

namespace {

// Helper macro to make creating an object (where you need to get back a derived type) easier
#define CREATE(className, classEnum)                     \
    reinterpret_cast<className *>(this->createObj(classEnum))

// Helper macro to make creating an object (where you need to get back a derived type) easier
#define FIND(id, className, classEnum)                   \
    reinterpret_cast<className *>(this->findObject(id, classEnum))

class DebugInterface : public GrGLTestInterface {
public:
    DebugInterface()
        : fCurrGenericID(0)
        , fCurrTextureUnit(0)
        , fVertexArray(nullptr)
        , fPackRowLength(0)
        , fUnpackRowLength(0)
        , fPackAlignment(4)
        , fFrameBuffer(nullptr)
        , fRenderBuffer(nullptr)
        , fProgram(nullptr)
        , fAbandoned(false) {
        for (int i = 0; i < kDefaultMaxTextureUnits; ++i) {
            fTextureUnits[i] =
                reinterpret_cast<GrTextureUnitObj*>(this->createObj(kTextureUnit_ObjTypes));
            fTextureUnits[i]->ref();
            fTextureUnits[i]->setNumber(i);
        }
        memset(fBoundBuffers, 0, sizeof(fBoundBuffers));
        this->init(kGL_GrGLStandard);
    }

    ~DebugInterface() override {
        // unref & delete the texture units first so they don't show up on the leak report
        for (int i = 0; i < kDefaultMaxTextureUnits; ++i) {
            fTextureUnits[i]->unref();
            fTextureUnits[i]->deleteAction();
        }
        for (int i = 0; i < fObjects.count(); ++i) {
            delete fObjects[i];
        }
        fObjects.reset();

        memset(fBoundBuffers, 0, sizeof(fBoundBuffers));
        fVertexArray = nullptr;

        this->report();
    }

    void abandon() const override { fAbandoned = true; }

    GrGLvoid activeTexture(GrGLenum texture) override {
        // Ganesh offsets the texture unit indices
        texture -= GR_GL_TEXTURE0;
        GrAlwaysAssert(texture < kDefaultMaxTextureUnits);
        fCurrTextureUnit = texture;
    }

    GrGLvoid attachShader(GrGLuint programID, GrGLuint shaderID) override {

        GrProgramObj *program = FIND(programID, GrProgramObj, kProgram_ObjTypes);
        GrAlwaysAssert(program);

        GrShaderObj *shader = FIND(shaderID, GrShaderObj, kShader_ObjTypes);
        GrAlwaysAssert(shader);

        program->AttachShader(shader);
    }

    ////////////////////////////////////////////////////////////////////////////////
    GrGLvoid bindTexture(GrGLenum target, GrGLuint textureID) override {
        GrAlwaysAssert(target == GR_GL_TEXTURE_2D ||
                       target == GR_GL_TEXTURE_RECTANGLE ||
                       target == GR_GL_TEXTURE_EXTERNAL);

        // a textureID of 0 is acceptable - it binds to the default texture target
        GrTextureObj *texture = FIND(textureID, GrTextureObj, kTexture_ObjTypes);

        this->setTexture(texture);
    }

    ////////////////////////////////////////////////////////////////////////////////
    GrGLvoid bufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data,
                        GrGLenum usage) override {
        GrAlwaysAssert(size >= 0);
        GrAlwaysAssert(GR_GL_STREAM_DRAW == usage ||
                       GR_GL_STATIC_DRAW == usage ||
                       GR_GL_DYNAMIC_DRAW == usage);

        GrBufferObj *buffer = fBoundBuffers[GetBufferIndex(target)];
        GrAlwaysAssert(buffer);
        GrAlwaysAssert(buffer->getBound());

        buffer->allocate(size, reinterpret_cast<const GrGLchar *>(data));
        buffer->setUsage(usage);
    }


    GrGLvoid pixelStorei(GrGLenum pname, GrGLint param) override {

        switch (pname) {
            case GR_GL_UNPACK_ROW_LENGTH:
                fUnpackRowLength = param;
                break;
            case GR_GL_PACK_ROW_LENGTH:
                fPackRowLength = param;
                break;
            case GR_GL_UNPACK_ALIGNMENT:
                break;
            case GR_GL_PACK_ALIGNMENT:
                fPackAlignment = param;
                break;
            default:
                GrAlwaysAssert(false);
                break;
        }
    }

    GrGLvoid readPixels(GrGLint x,
                        GrGLint y,
                        GrGLsizei width,
                        GrGLsizei height,
                        GrGLenum format,
                        GrGLenum type,
                        GrGLvoid* pixels) override {

        GrGLint pixelsInRow = width;
        if (fPackRowLength > 0) {
            pixelsInRow = fPackRowLength;
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

        GrGLint alignment = fPackAlignment;

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

    GrGLvoid useProgram(GrGLuint programID) override {

        // A programID of 0 is legal
        GrProgramObj *program = FIND(programID, GrProgramObj, kProgram_ObjTypes);

        this->useProgram(program);
    }

    GrGLvoid bindFramebuffer(GrGLenum target, GrGLuint frameBufferID) override {

        GrAlwaysAssert(GR_GL_FRAMEBUFFER == target ||
                       GR_GL_READ_FRAMEBUFFER == target ||
                       GR_GL_DRAW_FRAMEBUFFER);

        // a frameBufferID of 0 is acceptable - it binds to the default
        // frame buffer
        GrFrameBufferObj *frameBuffer = FIND(frameBufferID, GrFrameBufferObj,
                                             kFrameBuffer_ObjTypes);

        this->setFrameBuffer(frameBuffer);
    }

    GrGLvoid bindRenderbuffer(GrGLenum target, GrGLuint renderBufferID) override {

        GrAlwaysAssert(GR_GL_RENDERBUFFER == target);

        // a renderBufferID of 0 is acceptable - it unbinds the bound render buffer
        GrRenderBufferObj *renderBuffer = FIND(renderBufferID, GrRenderBufferObj,
                                               kRenderBuffer_ObjTypes);

        this->setRenderBuffer(renderBuffer);
    }

    GrGLvoid deleteTextures(GrGLsizei n, const GrGLuint* textures) override {
        // first potentially unbind the texture
        for (unsigned int i = 0; i < kDefaultMaxTextureUnits; ++i) {
            GrTextureUnitObj *pTU = this->getTextureUnit(i);

            if (pTU->getTexture()) {
                for (int j = 0; j < n; ++j) {

                    if (textures[j] == pTU->getTexture()->getID()) {
                        // this ID is the current texture - revert the binding to 0
                        pTU->setTexture(nullptr);
                    }
                }
            }
        }

        // TODO: fuse the following block with DeleteRenderBuffers?
        // Open GL will remove a deleted render buffer from the active
        // frame buffer but not from any other frame buffer
        if (this->getFrameBuffer()) {

            GrFrameBufferObj *frameBuffer = this->getFrameBuffer();

            for (int i = 0; i < n; ++i) {

                if (frameBuffer->getColor() &&
                    textures[i] == frameBuffer->getColor()->getID()) {
                    frameBuffer->setColor(nullptr);
                }
                if (frameBuffer->getDepth() &&
                    textures[i] == frameBuffer->getDepth()->getID()) {
                    frameBuffer->setDepth(nullptr);
                }
                if (frameBuffer->getStencil() &&
                    textures[i] == frameBuffer->getStencil()->getID()) {
                    frameBuffer->setStencil(nullptr);
                }
            }
        }

        // then actually "delete" the buffers
        for (int i = 0; i < n; ++i) {
            GrTextureObj *buffer = FIND(textures[i], GrTextureObj, kTexture_ObjTypes);
            GrAlwaysAssert(buffer);

            // OpenGL gives no guarantees if a texture is deleted while attached to
            // something other than the currently bound frame buffer
            GrAlwaysAssert(!buffer->getBound());

            GrAlwaysAssert(!buffer->getDeleted());
            buffer->deleteAction();
        }

    }

    GrGLvoid deleteFramebuffers(GrGLsizei n, const GrGLuint *frameBuffers) override {

        // first potentially unbind the buffers
        if (this->getFrameBuffer()) {
            for (int i = 0; i < n; ++i) {

                if (frameBuffers[i] ==
                    this->getFrameBuffer()->getID()) {
                    // this ID is the current frame buffer - rebind to the default
                    this->setFrameBuffer(nullptr);
                }
            }
        }

        // then actually "delete" the buffers
        for (int i = 0; i < n; ++i) {
            GrFrameBufferObj *buffer = FIND(frameBuffers[i], GrFrameBufferObj,
                                            kFrameBuffer_ObjTypes);
            GrAlwaysAssert(buffer);

            GrAlwaysAssert(!buffer->getDeleted());
            buffer->deleteAction();
        }
    }

    GrGLvoid deleteRenderbuffers(GrGLsizei n,const GrGLuint *renderBuffers) override {

        // first potentially unbind the buffers
        if (this->getRenderBuffer()) {
            for (int i = 0; i < n; ++i) {

                if (renderBuffers[i] ==
                    this->getRenderBuffer()->getID()) {
                    // this ID is the current render buffer - make no
                    // render buffer be bound
                    this->setRenderBuffer(nullptr);
                }
            }
        }

        // TODO: fuse the following block with DeleteTextures?
        // Open GL will remove a deleted render buffer from the active frame
        // buffer but not from any other frame buffer
        if (this->getFrameBuffer()) {

            GrFrameBufferObj *frameBuffer = this->getFrameBuffer();

            for (int i = 0; i < n; ++i) {

                if (frameBuffer->getColor() &&
                    renderBuffers[i] == frameBuffer->getColor()->getID()) {
                    frameBuffer->setColor(nullptr);
                }
                if (frameBuffer->getDepth() &&
                    renderBuffers[i] == frameBuffer->getDepth()->getID()) {
                    frameBuffer->setDepth(nullptr);
                }
                if (frameBuffer->getStencil() &&
                    renderBuffers[i] == frameBuffer->getStencil()->getID()) {
                    frameBuffer->setStencil(nullptr);
                }
            }
        }

        // then actually "delete" the buffers
        for (int i = 0; i < n; ++i) {
            GrRenderBufferObj *buffer = FIND(renderBuffers[i], GrRenderBufferObj,
                                             kRenderBuffer_ObjTypes);
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

    GrGLvoid renderbufferStorage(GrGLenum target, GrGLenum internalformat, GrGLsizei width,
                                 GrGLsizei height) override {
        GrAlwaysAssert(GR_GL_RENDERBUFFER == target);
        GrRenderBufferObj* renderBuffer = this->getRenderBuffer();
        GrAlwaysAssert(renderBuffer);
        renderBuffer->setNumSamples(1);
    }

    GrGLvoid renderbufferStorageMultisample(GrGLenum target, GrGLsizei samples,
                                            GrGLenum internalformat, GrGLsizei width,
                                            GrGLsizei height) override {
        GrAlwaysAssert(GR_GL_RENDERBUFFER == target);
        GrRenderBufferObj* renderBuffer = this->getRenderBuffer();
        GrAlwaysAssert(renderBuffer);
        renderBuffer->setNumSamples(samples);
    }

    GrGLvoid namedRenderbufferStorage(GrGLuint renderbuffer, GrGLenum GrGLinternalformat,
                                      GrGLsizei width, GrGLsizei height) override {
        SK_ABORT("Not implemented");
    }

    GrGLvoid namedRenderbufferStorageMultisample(GrGLuint renderbuffer, GrGLsizei samples,
                                                 GrGLenum GrGLinternalformat, GrGLsizei width,
                                                 GrGLsizei height) override {
        SK_ABORT("Not implemented");
    }

    GrGLvoid framebufferRenderbuffer(GrGLenum target,
                                     GrGLenum attachment,
                                     GrGLenum renderbuffertarget,
                                     GrGLuint renderBufferID) override {

        GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);
        GrAlwaysAssert(GR_GL_COLOR_ATTACHMENT0 == attachment ||
                       GR_GL_DEPTH_ATTACHMENT == attachment ||
                       GR_GL_STENCIL_ATTACHMENT == attachment);
        GrAlwaysAssert(GR_GL_RENDERBUFFER == renderbuffertarget);

        GrFrameBufferObj *framebuffer = this->getFrameBuffer();
        // A render buffer cannot be attached to the default framebuffer
        GrAlwaysAssert(framebuffer);

        // a renderBufferID of 0 is acceptable - it unbinds the current
        // render buffer
        GrRenderBufferObj *renderbuffer = FIND(renderBufferID, GrRenderBufferObj,
                                               kRenderBuffer_ObjTypes);

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

    GrGLvoid namedFramebufferRenderbuffer(GrGLuint framebuffer, GrGLenum attachment,
                                          GrGLenum renderbuffertarget,
                                          GrGLuint renderbuffer) override {
        SK_ABORT("Not implemented");
    }

    ////////////////////////////////////////////////////////////////////////////////
    GrGLvoid framebufferTexture2D(GrGLenum target, GrGLenum attachment, GrGLenum textarget,
                                  GrGLuint textureID, GrGLint level) override {

        GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);
        GrAlwaysAssert(GR_GL_COLOR_ATTACHMENT0 == attachment ||
                       GR_GL_DEPTH_ATTACHMENT == attachment ||
                       GR_GL_STENCIL_ATTACHMENT == attachment);
        GrAlwaysAssert(GR_GL_TEXTURE_2D == textarget);

        GrFrameBufferObj *framebuffer = this->getFrameBuffer();
        // A texture cannot be attached to the default framebuffer
        GrAlwaysAssert(framebuffer);

        // A textureID of 0 is allowed - it unbinds the currently bound texture
        GrTextureObj *texture = FIND(textureID, GrTextureObj, kTexture_ObjTypes);
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

    GrGLvoid framebufferTexture2DMultisample(GrGLenum target, GrGLenum attachment,
                                             GrGLenum textarget, GrGLuint texture, GrGLint level,
                                             GrGLsizei samples) override {
        SK_ABORT("Not implemented");
    }

    GrGLvoid namedFramebufferTexture1D(GrGLuint framebuffer, GrGLenum attachment,
                                       GrGLenum textarget, GrGLuint texture,
                                       GrGLint level) override {
        SK_ABORT("Not implemented");
    }

    GrGLvoid namedFramebufferTexture2D(GrGLuint framebuffer, GrGLenum attachment,
                                       GrGLenum textarget, GrGLuint texture,
                                       GrGLint level) override {
        SK_ABORT("Not implemented");
    }

    GrGLvoid namedFramebufferTexture3D(GrGLuint framebuffer, GrGLenum attachment,
                                       GrGLenum textarget, GrGLuint texture, GrGLint level,
                                       GrGLint zoffset) override {
        SK_ABORT("Not implemented");
    }

    GrGLuint createProgram() override {

        GrProgramObj *program = CREATE(GrProgramObj, kProgram_ObjTypes);

        return program->getID();
    }

    GrGLuint createShader(GrGLenum type) override {

        GrAlwaysAssert(GR_GL_VERTEX_SHADER == type ||
                       GR_GL_FRAGMENT_SHADER == type);

        GrShaderObj *shader = CREATE(GrShaderObj, kShader_ObjTypes);
        shader->setType(type);

        return shader->getID();
    }

    GrGLenum checkFramebufferStatus(GrGLenum target) override { return GR_GL_FRAMEBUFFER_COMPLETE; }

    GrGLvoid deleteProgram(GrGLuint programID) override {

        GrProgramObj *program = FIND(programID, GrProgramObj, kProgram_ObjTypes);
        GrAlwaysAssert(program);

        if (program->getRefCount()) {
            // someone is still using this program so we can't delete it here
            program->setMarkedForDeletion();
        } else {
            program->deleteAction();
        }
    }

    GrGLvoid deleteShader(GrGLuint shaderID) override {

        GrShaderObj *shader = FIND(shaderID, GrShaderObj, kShader_ObjTypes);
        GrAlwaysAssert(shader);

        if (shader->getRefCount()) {
            // someone is still using this shader so we can't delete it here
            shader->setMarkedForDeletion();
        } else {
            shader->deleteAction();
        }
    }

    GrGLvoid genBuffers(GrGLsizei n, GrGLuint* ids) override {
        this->genObjs(kBuffer_ObjTypes, n, ids);
    }

    GrGLvoid genFramebuffers(GrGLsizei n, GrGLuint* ids) override {
        this->genObjs(kFrameBuffer_ObjTypes, n, ids);
    }

    GrGLvoid genRenderbuffers(GrGLsizei n, GrGLuint* ids) override {
        this->genObjs(kRenderBuffer_ObjTypes, n, ids);
    }

    GrGLvoid genTextures(GrGLsizei n, GrGLuint* ids) override {
        this->genObjs(kTexture_ObjTypes, n, ids);
    }

    GrGLvoid genVertexArrays(GrGLsizei n, GrGLuint* ids) override {
        this->genObjs(kVertexArray_ObjTypes, n, ids);
    }

    GrGLvoid genQueries(GrGLsizei n, GrGLuint *ids) override { this->genGenericIds(n, ids); }

    GrGLenum getError() override { return GR_GL_NO_ERROR; }

    GrGLvoid getIntegerv(GrGLenum pname, GrGLint* params) override {
        // TODO: remove from Ganesh the #defines for gets we don't use.
        // We would like to minimize gets overall due to performance issues
        switch (pname) {
            case GR_GL_CONTEXT_PROFILE_MASK:
                *params = GR_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT;
                break;
            case GR_GL_STENCIL_BITS:
                *params = 8;
                break;
            case GR_GL_SAMPLES: {
                GrFrameBufferObj* framebuffer = this->getFrameBuffer();
                GrAlwaysAssert(framebuffer);
                int numSamples = 0;

                if (GrFBBindableObj* stencil = framebuffer->getStencil()) {
                    numSamples = stencil->numSamples();
                }
                if (GrFBBindableObj* depth = framebuffer->getDepth()) {
                    GrAlwaysAssert(!numSamples || numSamples == depth->numSamples());
                    numSamples = depth->numSamples();
                }
                if (GrFBBindableObj* color = framebuffer->getColor()) {
                    GrAlwaysAssert(!numSamples || numSamples == color->numSamples());
                    numSamples = color->numSamples();
                }
                GrAlwaysAssert(numSamples);
                *params = numSamples;
                break;
            }
            case GR_GL_FRAMEBUFFER_BINDING:
                *params = 0;
                break;
            case GR_GL_VIEWPORT:
                params[0] = 0;
                params[1] = 0;
                params[2] = 800;
                params[3] = 600;
                break;
            case GR_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
            case GR_GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS:
            case GR_GL_MAX_TEXTURE_IMAGE_UNITS:
            case GR_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
                *params = 8;
                break;
            case GR_GL_MAX_TEXTURE_COORDS:
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
            case GR_GL_NUM_EXTENSIONS: {
                GrGLint i = 0;
                while (kExtensions[i++]);
                *params = i;
                break;
            }
            default:
                SkFAIL("Unexpected pname to GetIntegerv");
        }
    }

    GrGLvoid getMultisamplefv(GrGLenum pname, GrGLuint index, GrGLfloat* val) override {
        val[0] = val[1] = 0.5f;
    }

    GrGLvoid getProgramiv(GrGLuint program, GrGLenum pname, GrGLint* params) override {
        this->getShaderOrProgramiv(program, pname, params);
    }

    GrGLvoid getProgramInfoLog(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length,
                               char* infolog) override {
        this->getInfoLog(program, bufsize, length, infolog);
    }

    GrGLvoid getQueryiv(GrGLenum GLtarget, GrGLenum pname, GrGLint *params) override {
        switch (pname) {
            case GR_GL_CURRENT_QUERY:
                *params = 0;
                break;
            case GR_GL_QUERY_COUNTER_BITS:
                *params = 32;
                break;
            default:
                SkFAIL("Unexpected pname passed GetQueryiv.");
        }
    }

    GrGLvoid getQueryObjecti64v(GrGLuint id, GrGLenum pname, GrGLint64 *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getQueryObjectiv(GrGLuint id, GrGLenum pname, GrGLint *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getQueryObjectui64v(GrGLuint id, GrGLenum pname, GrGLuint64 *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getQueryObjectuiv(GrGLuint id, GrGLenum pname, GrGLuint *params) override {
        this->queryResult(id, pname, params);
    }

    GrGLvoid getShaderiv(GrGLuint shader, GrGLenum pname, GrGLint* params) override {
        this->getShaderOrProgramiv(shader, pname, params);
    }

    GrGLvoid getShaderInfoLog(GrGLuint shader, GrGLsizei bufsize, GrGLsizei* length,
                              char* infolog) override {
        this->getInfoLog(shader, bufsize, length, infolog);
    }

    const GrGLubyte* getString(GrGLenum name) override {
        switch (name) {
            case GR_GL_EXTENSIONS:
                return CombinedExtensionString();
            case GR_GL_VERSION:
                return (const GrGLubyte*)"4.0 Debug GL";
            case GR_GL_SHADING_LANGUAGE_VERSION:
                return (const GrGLubyte*)"4.20.8 Debug GLSL";
            case GR_GL_VENDOR:
                return (const GrGLubyte*)"Debug Vendor";
            case GR_GL_RENDERER:
                return (const GrGLubyte*)"The Debug (Non-)Renderer";
            default:
                SkFAIL("Unexpected name passed to GetString");
                return nullptr;
        }
    }

    const GrGLubyte* getStringi(GrGLenum name, GrGLuint i) override {
        switch (name) {
            case GR_GL_EXTENSIONS: {
                GrGLint count;
                this->getIntegerv(GR_GL_NUM_EXTENSIONS, &count);
                if ((GrGLint)i <= count) {
                    return (const GrGLubyte*) kExtensions[i];
                } else {
                    return nullptr;
                }
            }
            default:
                SkFAIL("Unexpected name passed to GetStringi");
                return nullptr;
        }
    }

    GrGLvoid getTexLevelParameteriv(GrGLenum target, GrGLint level, GrGLenum pname,
                                    GrGLint* params) override {
        // we used to use this to query stuff about externally created textures,
        // now we just require clients to tell us everything about the texture.
        SkFAIL("Should never query texture parameters.");
    }

    GrGLvoid deleteVertexArrays(GrGLsizei n, const GrGLuint* ids) override {
        for (GrGLsizei i = 0; i < n; ++i) {
            GrVertexArrayObj* array = FIND(ids[i], GrVertexArrayObj, kVertexArray_ObjTypes);
            GrAlwaysAssert(array);

            // Deleting the current vertex array binds object 0
            if (this->getVertexArray() == array) {
                this->setVertexArray(nullptr);
            }

            if (array->getRefCount()) {
                // someone is still using this vertex array so we can't delete it here
                array->setMarkedForDeletion();
            } else {
                array->deleteAction();
            }
        }
    }

    GrGLvoid bindVertexArray(GrGLuint id) override {
        GrVertexArrayObj* array = FIND(id, GrVertexArrayObj, kVertexArray_ObjTypes);
        GrAlwaysAssert((0 == id) || array);
        this->setVertexArray(array);
    }

    GrGLvoid bindBuffer(GrGLenum target, GrGLuint bufferID) override {
        GrBufferObj *buffer = FIND(bufferID, GrBufferObj, kBuffer_ObjTypes);
        // 0 is a permissible bufferID - it unbinds the current buffer

        this->setBuffer(GetBufferIndex(target), buffer);
    }

    // deleting a bound buffer has the side effect of binding 0
    GrGLvoid deleteBuffers(GrGLsizei n, const GrGLuint* ids) override {
        // first potentially unbind the buffers
        for (int buffIdx = 0; buffIdx < kNumBufferTargets; ++buffIdx) {
            GrBufferObj* buffer = fBoundBuffers[buffIdx];
            if (!buffer) {
                continue;
            }
            for (int i = 0; i < n; ++i) {
                if (ids[i] == buffer->getID()) {
                    this->setBuffer(buffIdx, nullptr);
                    break;
                }
            }
        }

        // then actually "delete" the buffers
        for (int i = 0; i < n; ++i) {
            GrBufferObj *buffer = FIND(ids[i], GrBufferObj, kBuffer_ObjTypes);
            GrAlwaysAssert(buffer);

            GrAlwaysAssert(!buffer->getDeleted());
            buffer->deleteAction();
        }
    }

    // map a buffer to the caller's address space
    GrGLvoid* mapBufferRange(GrGLenum target, GrGLintptr offset, GrGLsizeiptr length,
                             GrGLbitfield access) override {
        // We only expect read access and we expect that the buffer or range is always invalidated.
        GrAlwaysAssert(!SkToBool(GR_GL_MAP_READ_BIT & access));
        GrAlwaysAssert((GR_GL_MAP_INVALIDATE_BUFFER_BIT | GR_GL_MAP_INVALIDATE_RANGE_BIT) & access);

        GrBufferObj *buffer = fBoundBuffers[GetBufferIndex(target)];
        if (buffer) {
            GrAlwaysAssert(offset >= 0 && offset + length <= buffer->getSize());
            GrAlwaysAssert(!buffer->getMapped());
            buffer->setMapped(offset, length);
            return buffer->getDataPtr() + offset;
        }

        GrAlwaysAssert(false);
        return nullptr;        // no buffer bound to the target
    }

    GrGLvoid* mapBuffer(GrGLenum target, GrGLenum access) override {
        GrAlwaysAssert(GR_GL_WRITE_ONLY == access);
        GrBufferObj *buffer = fBoundBuffers[GetBufferIndex(target)];
        return this->mapBufferRange(target, 0, buffer->getSize(),
                                    GR_GL_MAP_WRITE_BIT | GR_GL_MAP_INVALIDATE_BUFFER_BIT);
    }

    // remove a buffer from the caller's address space
    // TODO: check if the "access" method from "glMapBuffer" was honored
    GrGLboolean unmapBuffer(GrGLenum target) override {
        GrBufferObj *buffer = fBoundBuffers[GetBufferIndex(target)];
        if (buffer) {
            GrAlwaysAssert(buffer->getMapped());
            buffer->resetMapped();
            return GR_GL_TRUE;
        }

        GrAlwaysAssert(false);
        return GR_GL_FALSE; // GR_GL_INVALID_OPERATION;
    }

    GrGLvoid flushMappedBufferRange(GrGLenum target, GrGLintptr offset,
                                    GrGLsizeiptr length) override {
        GrBufferObj *buffer = fBoundBuffers[GetBufferIndex(target)];
        if (buffer) {
            GrAlwaysAssert(buffer->getMapped());
            GrAlwaysAssert(offset >= 0 && (offset + length) <= buffer->getMappedLength());
        } else {
            GrAlwaysAssert(false);
        }
    }

    GrGLvoid getBufferParameteriv(GrGLenum target, GrGLenum value, GrGLint* params) override {

        GrAlwaysAssert(GR_GL_BUFFER_SIZE == value ||
                       GR_GL_BUFFER_USAGE == value);

        GrBufferObj *buffer = fBoundBuffers[GetBufferIndex(target)];
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
                    *params = SkToInt(buffer->getSize());
                break;
            case GR_GL_BUFFER_USAGE:
                *params = GR_GL_STATIC_DRAW;
                if (buffer)
                    *params = buffer->getUsage();
                break;
            default:
                SkFAIL("Unexpected value to glGetBufferParamateriv");
                break;
        }
    }

private:
    inline int static GetBufferIndex(GrGLenum glTarget) {
        switch (glTarget) {
            default:                           SkFAIL("Unexpected GL target to GetBufferIndex");
            case GR_GL_ARRAY_BUFFER:           return 0;
            case GR_GL_ELEMENT_ARRAY_BUFFER:   return 1;
            case GR_GL_TEXTURE_BUFFER:         return 2;
            case GR_GL_DRAW_INDIRECT_BUFFER:   return 3;
        }
    }
    constexpr int static kNumBufferTargets = 4;

    // the OpenGLES 2.0 spec says this must be >= 128
    static const GrGLint kDefaultMaxVertexUniformVectors = 128;

    // the OpenGLES 2.0 spec says this must be >=16
    static const GrGLint kDefaultMaxFragmentUniformVectors = 16;

    // the OpenGLES 2.0 spec says this must be >= 8
    static const GrGLint kDefaultMaxVertexAttribs = 8;

    // the OpenGLES 2.0 spec says this must be >= 8
    static const GrGLint kDefaultMaxVaryingVectors = 8;

    // the OpenGLES 2.0 spec says this must be >= 2
    static const GrGLint kDefaultMaxTextureUnits = 8;

    static const char* kExtensions[];

    GrGLuint                    fCurrGenericID;
    GrGLuint                    fCurrTextureUnit;
    GrTextureUnitObj*           fTextureUnits[kDefaultMaxTextureUnits];
    GrBufferObj*                fBoundBuffers[kNumBufferTargets];
    GrVertexArrayObj*           fVertexArray;
    GrGLint                     fPackRowLength;
    GrGLint                     fUnpackRowLength;
    GrGLint                     fPackAlignment;
    GrFrameBufferObj*           fFrameBuffer;
    GrRenderBufferObj*          fRenderBuffer;
    GrProgramObj*               fProgram;
    mutable bool                fAbandoned;
    // global store of all objects
    SkTArray<GrFakeRefObj *>    fObjects;

    static const GrGLubyte* CombinedExtensionString() {
        static SkString gExtString;
        static SkMutex gMutex;
        gMutex.acquire();
        if (0 == gExtString.size()) {
            int i = 0;
            while (kExtensions[i]) {
                if (i > 0) {
                    gExtString.append(" ");
                }
                gExtString.append(kExtensions[i]);
                ++i;
            }
        }
        gMutex.release();
        return (const GrGLubyte*) gExtString.c_str();
    }

    GrGLvoid genGenericIds(GrGLsizei n, GrGLuint* ids) {
        for (int i = 0; i < n; ++i) {
            ids[i] = ++fCurrGenericID;
        }
    }

    GrGLvoid getInfoLog(GrGLuint object, GrGLsizei bufsize, GrGLsizei* length,
                        char* infolog) {
        if (length) {
            *length = 0;
        }
        if (bufsize > 0) {
            *infolog = 0;
        }
    }

    GrGLvoid getShaderOrProgramiv(GrGLuint object,  GrGLenum pname, GrGLint* params) {
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
                SkFAIL("Unexpected pname to GetProgramiv");
                break;
        }
    }

    template <typename T>
    void queryResult(GrGLenum GLtarget, GrGLenum pname, T *params) {
        switch (pname) {
            case GR_GL_QUERY_RESULT_AVAILABLE:
                *params = GR_GL_TRUE;
                break;
            case GR_GL_QUERY_RESULT:
                *params = 0;
                break;
            default:
                SkFAIL("Unexpected pname passed to GetQueryObject.");
                break;
        }
    }

    enum ObjTypes {
        kTexture_ObjTypes = 0,
        kBuffer_ObjTypes,
        kRenderBuffer_ObjTypes,
        kFrameBuffer_ObjTypes,
        kShader_ObjTypes,
        kProgram_ObjTypes,
        kTextureUnit_ObjTypes,
        kVertexArray_ObjTypes,
        kObjTypeCount
    };

    typedef GrFakeRefObj *(*Create)();

    static Create gFactoryFunc[kObjTypeCount];

    GrGLvoid genObjs(ObjTypes type, GrGLsizei n, GrGLuint* ids) {
        for (int i = 0; i < n; ++i) {
            GrAlwaysAssert(ids[i] == 0);
            GrFakeRefObj *obj = this->createObj(type);
            GrAlwaysAssert(obj);
            ids[i] = obj->getID();
        }
    }

    GrFakeRefObj* createObj(ObjTypes type) {
        GrFakeRefObj *temp = (*gFactoryFunc[type])();

        fObjects.push_back(temp);

        return temp;
    }

    GrFakeRefObj* findObject(GrGLuint ID, ObjTypes type) {
        for (int i = 0; i < fObjects.count(); ++i) {
            if (fObjects[i]->getID() == ID) { // && fObjects[i]->getType() == type) {
                // The application shouldn't be accessing objects
                // that (as far as OpenGL knows) were already deleted
                GrAlwaysAssert(!fObjects[i]->getDeleted());
                GrAlwaysAssert(!fObjects[i]->getMarkedForDeletion());
                return fObjects[i];
            }
        }
        return nullptr;
    }

    GrTextureUnitObj* getTextureUnit(int unit) {
        GrAlwaysAssert(0 <= unit && kDefaultMaxTextureUnits > unit);

        return fTextureUnits[unit];
    }

    GrGLvoid setBuffer(int buffIdx, GrBufferObj* buffer) {
        if (fBoundBuffers[buffIdx]) {
            // automatically break the binding of the old buffer
            GrAlwaysAssert(fBoundBuffers[buffIdx]->getBound());
            fBoundBuffers[buffIdx]->resetBound();

            GrAlwaysAssert(!fBoundBuffers[buffIdx]->getDeleted());
            fBoundBuffers[buffIdx]->unref();
        }

        if (buffer) {
            GrAlwaysAssert(!buffer->getDeleted());
            buffer->ref();

            GrAlwaysAssert(!buffer->getBound());
            buffer->setBound();
        }

        fBoundBuffers[buffIdx] = buffer;
    }

    void setVertexArray(GrVertexArrayObj* vertexArray) {
        if (vertexArray) {
            SkASSERT(!vertexArray->getDeleted());
        }
        SkRefCnt_SafeAssign(fVertexArray, vertexArray);
    }

    GrVertexArrayObj* getVertexArray() { return fVertexArray; }

    void setTexture(GrTextureObj *texture) {
        fTextureUnits[fCurrTextureUnit]->setTexture(texture);
    }

    void setFrameBuffer(GrFrameBufferObj *frameBuffer) {
        if (fFrameBuffer) {
            GrAlwaysAssert(fFrameBuffer->getBound());
            fFrameBuffer->resetBound();

            GrAlwaysAssert(!fFrameBuffer->getDeleted());
            fFrameBuffer->unref();
        }

        fFrameBuffer = frameBuffer;

        if (fFrameBuffer) {
            GrAlwaysAssert(!fFrameBuffer->getDeleted());
            fFrameBuffer->ref();

            GrAlwaysAssert(!fFrameBuffer->getBound());
            fFrameBuffer->setBound();
        }
    }

    GrFrameBufferObj *getFrameBuffer() { return fFrameBuffer; }

    void setRenderBuffer(GrRenderBufferObj *renderBuffer) {
        if (fRenderBuffer) {
            GrAlwaysAssert(fRenderBuffer->getBound());
            fRenderBuffer->resetBound();

            GrAlwaysAssert(!fRenderBuffer->getDeleted());
            fRenderBuffer->unref();
        }

        fRenderBuffer = renderBuffer;

        if (fRenderBuffer) {
            GrAlwaysAssert(!fRenderBuffer->getDeleted());
            fRenderBuffer->ref();

            GrAlwaysAssert(!fRenderBuffer->getBound());
            fRenderBuffer->setBound();
        }
    }
    GrRenderBufferObj *getRenderBuffer() { return fRenderBuffer; }

    void useProgram(GrProgramObj *program) {
        if (fProgram) {
            GrAlwaysAssert(fProgram->getInUse());
            fProgram->resetInUse();

            GrAlwaysAssert(!fProgram->getDeleted());
            fProgram->unref();
        }

        fProgram = program;

        if (fProgram) {
            GrAlwaysAssert(!fProgram->getDeleted());
            fProgram->ref();

            GrAlwaysAssert(!fProgram->getInUse());
            fProgram->setInUse();
        }
    }

    void report() const {
        for (int i = 0; i < fObjects.count(); ++i) {
            if (!fAbandoned) {
                GrAlwaysAssert(0 == fObjects[i]->getRefCount());
                GrAlwaysAssert(fObjects[i]->getDeleted());
            }
        }
    }

    typedef GrGLTestInterface INHERITED;
};

#undef CREATE
#undef FIND

DebugInterface::Create DebugInterface::gFactoryFunc[kObjTypeCount] = {
    GrTextureObj::createGrTextureObj,
    GrBufferObj::createGrBufferObj,
    GrRenderBufferObj::createGrRenderBufferObj,
    GrFrameBufferObj::createGrFrameBufferObj,
    GrShaderObj::createGrShaderObj,
    GrProgramObj::createGrProgramObj,
    GrTextureUnitObj::createGrTextureUnitObj,
    GrVertexArrayObj::createGrVertexArrayObj,
};

const char* DebugInterface::kExtensions[] = {
    "GL_ARB_framebuffer_object",
    "GL_ARB_blend_func_extended",
    "GL_ARB_timer_query",
    "GL_ARB_draw_buffers",
    "GL_ARB_occlusion_query",
    "GL_EXT_stencil_wrap",
    nullptr, // signifies the end of the array.
};

class DebugGLContext : public sk_gpu_test::GLTestContext {
public:
   DebugGLContext() {
       this->init(new DebugInterface());
   }

   ~DebugGLContext() override { this->teardown(); }

private:
    void onPlatformMakeCurrent() const override {}
    void onPlatformSwapBuffers() const override {}
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override { return nullptr; }
};
}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreateDebugGLTestContext(GLTestContext* shareContext) {
    if (shareContext) {
        return nullptr;
    }
    GLTestContext* ctx = new DebugGLContext();
    if (ctx->isValid()) {
        return ctx;
    }
    delete ctx;
    return nullptr;
}
}

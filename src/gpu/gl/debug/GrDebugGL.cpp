
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDebugGL.h"
#include "GrTextureObj.h"
#include "GrBufferObj.h"
#include "GrRenderBufferObj.h"
#include "GrFrameBufferObj.h"
#include "GrShaderObj.h"
#include "GrProgramObj.h"
#include "GrTextureUnitObj.h"
#include "GrVertexArrayObj.h"

GrDebugGL* GrDebugGL::gObj = NULL;
int GrDebugGL::gStaticRefCount = 0;
GrDebugGL::Create GrDebugGL::gFactoryFunc[kObjTypeCount] = {
    GrTextureObj::createGrTextureObj,
    GrBufferObj::createGrBufferObj,
    GrRenderBufferObj::createGrRenderBufferObj,
    GrFrameBufferObj::createGrFrameBufferObj,
    GrShaderObj::createGrShaderObj,
    GrProgramObj::createGrProgramObj,
    GrTextureUnitObj::createGrTextureUnitObj,
    GrVertexArrayObj::createGrVertexArrayObj,
};


GrDebugGL::GrDebugGL()
    : fPackRowLength(0)
    , fUnPackRowLength(0)
    , fCurTextureUnit(0)
    , fArrayBuffer(NULL)
    , fElementArrayBuffer(NULL)
    , fFrameBuffer(NULL)
    , fRenderBuffer(NULL)
    , fProgram(NULL)
    , fTexture(NULL)
    , fVertexArray(NULL)
    , fAbandoned(false) {

    for (int i = 0; i < kDefaultMaxTextureUnits; ++i) {

        fTextureUnits[i] = reinterpret_cast<GrTextureUnitObj *>(
                            createObj(GrDebugGL::kTextureUnit_ObjTypes));
        fTextureUnits[i]->ref();

        fTextureUnits[i]->setNumber(i);
    }
}

GrDebugGL::~GrDebugGL() {
    // unref & delete the texture units first so they don't show up on the leak report
    for (int i = 0; i < kDefaultMaxTextureUnits; ++i) {
        fTextureUnits[i]->unref();
        fTextureUnits[i]->deleteAction();
    }

    this->report();

    for (int i = 0; i < fObjects.count(); ++i) {
        delete fObjects[i];
    }
    fObjects.reset();

    fArrayBuffer = NULL;
    fElementArrayBuffer = NULL;
    fFrameBuffer = NULL;
    fRenderBuffer = NULL;
    fProgram = NULL;
    fTexture = NULL;
    fVertexArray = NULL;
}

GrFakeRefObj *GrDebugGL::findObject(GrGLuint ID, GrObjTypes type) {
    for (int i = 0; i < fObjects.count(); ++i) {
        if (fObjects[i]->getID() == ID) { // && fObjects[i]->getType() == type) {
            // The application shouldn't be accessing objects
            // that (as far as OpenGL knows) were already deleted
            GrAlwaysAssert(!fObjects[i]->getDeleted());
            GrAlwaysAssert(!fObjects[i]->getMarkedForDeletion());
            return fObjects[i];
        }
    }

    return NULL;
}

void GrDebugGL::setArrayBuffer(GrBufferObj *arrayBuffer) {
    if (fArrayBuffer) {
        // automatically break the binding of the old buffer
        GrAlwaysAssert(fArrayBuffer->getBound());
        fArrayBuffer->resetBound();

        GrAlwaysAssert(!fArrayBuffer->getDeleted());
        fArrayBuffer->unref();
    }

    fArrayBuffer = arrayBuffer;

    if (fArrayBuffer) {
        GrAlwaysAssert(!fArrayBuffer->getDeleted());
        fArrayBuffer->ref();

        GrAlwaysAssert(!fArrayBuffer->getBound());
        fArrayBuffer->setBound();
    }
}

void GrDebugGL::setVertexArray(GrVertexArrayObj* vertexArray) {
    if (vertexArray) {
        SkASSERT(!vertexArray->getDeleted());
    }
    SkRefCnt_SafeAssign(fVertexArray, vertexArray);
}

void GrDebugGL::setElementArrayBuffer(GrBufferObj *elementArrayBuffer) {
    if (fElementArrayBuffer) {
        // automatically break the binding of the old buffer
        GrAlwaysAssert(fElementArrayBuffer->getBound());
        fElementArrayBuffer->resetBound();

        GrAlwaysAssert(!fElementArrayBuffer->getDeleted());
        fElementArrayBuffer->unref();
    }

    fElementArrayBuffer = elementArrayBuffer;

    if (fElementArrayBuffer) {
        GrAlwaysAssert(!fElementArrayBuffer->getDeleted());
        fElementArrayBuffer->ref();

        GrAlwaysAssert(!fElementArrayBuffer->getBound());
        fElementArrayBuffer->setBound();
    }
}

void GrDebugGL::setTexture(GrTextureObj *texture)  {
    fTextureUnits[fCurTextureUnit]->setTexture(texture);
}

void GrDebugGL::setFrameBuffer(GrFrameBufferObj *frameBuffer)  {
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

void GrDebugGL::setRenderBuffer(GrRenderBufferObj *renderBuffer)  {
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

void GrDebugGL::useProgram(GrProgramObj *program) {
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

void GrDebugGL::report() const {
    for (int i = 0; i < fObjects.count(); ++i) {
        GrAlwaysAssert(0 < fObjects[i]->getHighRefCount());
        if (!fAbandoned) {
            GrAlwaysAssert(0 == fObjects[i]->getRefCount());
            GrAlwaysAssert(fObjects[i]->getDeleted());
        }
    }
}

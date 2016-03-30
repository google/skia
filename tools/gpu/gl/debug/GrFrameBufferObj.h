
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFrameBufferObj_DEFINED
#define GrFrameBufferObj_DEFINED

#include "GrFakeRefObj.h"
class GrFBBindableObj;

////////////////////////////////////////////////////////////////////////////////
// TODO: when a framebuffer obj is bound the GL_SAMPLES query must return 0
// TODO: GL_STENCIL_BITS must also be redirected to the framebuffer
class GrFrameBufferObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrFrameBufferObj);

public:
    GrFrameBufferObj()
        : GrFakeRefObj()
        , fBound(false)
        , fColorBuffer(nullptr)
        , fDepthBuffer(nullptr)
        , fStencilBuffer(nullptr) {
    }

    virtual ~GrFrameBufferObj() {
        fColorBuffer = nullptr;
        fDepthBuffer = nullptr;
        fStencilBuffer = nullptr;
    }

    void setBound()         { fBound = true; }
    void resetBound()       { fBound = false; }
    bool getBound() const   { return fBound; }

    void setColor(GrFBBindableObj *buffer);
    GrFBBindableObj *getColor()       { return fColorBuffer; }

    void setDepth(GrFBBindableObj *buffer);
    GrFBBindableObj *getDepth()       { return fDepthBuffer; }

    void setStencil(GrFBBindableObj *buffer);
    GrFBBindableObj *getStencil()     { return fStencilBuffer; }

    void deleteAction() override {

        setColor(nullptr);
        setDepth(nullptr);
        setStencil(nullptr);

        this->INHERITED::deleteAction();
    }

protected:
private:
    bool fBound;        // is this frame buffer currently bound via "glBindFramebuffer"?
    GrFBBindableObj * fColorBuffer;
    GrFBBindableObj * fDepthBuffer;
    GrFBBindableObj * fStencilBuffer;

    typedef GrFakeRefObj INHERITED;
};

#endif // GrFrameBufferObj_DEFINED

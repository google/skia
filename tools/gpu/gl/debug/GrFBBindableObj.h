
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFBBindableObj_DEFINED
#define GrFBBindableObj_DEFINED

#include "SkTDArray.h"
#include "GrFakeRefObj.h"

////////////////////////////////////////////////////////////////////////////////
// A common base class for render buffers and textures
class GrFBBindableObj : public GrFakeRefObj {

public:
    GrFBBindableObj()
        : GrFakeRefObj()
        , fNumSamples(1) {
    }

    virtual ~GrFBBindableObj() {
        GrAlwaysAssert(0 == fColorReferees.count());
        GrAlwaysAssert(0 == fDepthReferees.count());
        GrAlwaysAssert(0 == fStencilReferees.count());
    }

    void setColorBound(GrFakeRefObj *referee) {
        fColorReferees.append(1, &referee);
    }
    void resetColorBound(GrFakeRefObj *referee) {
        int index = fColorReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fColorReferees.removeShuffle(index);
    }
    bool getColorBound(GrFakeRefObj *referee) const {
        int index = fColorReferees.find(referee);
        return 0 <= index;
    }
    bool getColorBound() const {
        return 0 != fColorReferees.count();
    }

    void setDepthBound(GrFakeRefObj *referee) {
        fDepthReferees.append(1, &referee);
    }
    void resetDepthBound(GrFakeRefObj *referee) {
        int index = fDepthReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fDepthReferees.removeShuffle(index);
    }
    bool getDepthBound(GrFakeRefObj *referee) const {
        int index = fDepthReferees.find(referee);
        return 0 <= index;
    }
    bool getDepthBound() const {
        return 0 != fDepthReferees.count();
    }

    void setStencilBound(GrFakeRefObj *referee) {
        fStencilReferees.append(1, &referee);
    }
    void resetStencilBound(GrFakeRefObj *referee) {
        int index = fStencilReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fStencilReferees.removeShuffle(index);
    }
    bool getStencilBound(GrFakeRefObj *referee) const {
        int index = fStencilReferees.find(referee);
        return 0 <= index;
    }
    bool getStencilBound() const {
        return 0 != fStencilReferees.count();
    }

    int numSamples() { return fNumSamples; }

protected:
    int fNumSamples;

private:
    SkTDArray<GrFakeRefObj *> fColorReferees;   // frame buffers that use this as a color buffer (via "glFramebufferRenderbuffer" or "glFramebufferTexture2D")
    SkTDArray<GrFakeRefObj *> fDepthReferees;   // frame buffers that use this as a depth buffer (via "glFramebufferRenderbuffer" or "glFramebufferTexture2D")
    SkTDArray<GrFakeRefObj *> fStencilReferees; // frame buffers that use this as a stencil buffer (via "glFramebufferRenderbuffer" or "glFramebufferTexture2D")

    typedef GrFakeRefObj INHERITED;
};

#endif // GrFBBindableObj_DEFINED

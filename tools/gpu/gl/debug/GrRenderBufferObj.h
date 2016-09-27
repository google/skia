
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderBufferObj_DEFINED
#define GrRenderBufferObj_DEFINED

#include "GrFBBindableObj.h"

////////////////////////////////////////////////////////////////////////////////
class GrRenderBufferObj : public GrFBBindableObj {
    GR_DEFINE_CREATOR(GrRenderBufferObj)

public:
    GrRenderBufferObj()
        : GrFBBindableObj()
        , fBound(false) {
    }

    void setBound()         { fBound = true; }
    void resetBound()       { fBound = false; }
    bool getBound() const   { return fBound; }

    void deleteAction() override {

        this->INHERITED::deleteAction();
    }

    void setNumSamples(int numSamples) {
        GrAlwaysAssert(numSamples > 0);
        fNumSamples = numSamples;
    }

protected:
private:
    bool fBound;           // is this render buffer currently bound via "glBindRenderbuffer"?

    typedef GrFBBindableObj INHERITED;
};

#endif // GrRenderBufferObj_DEFINED

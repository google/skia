
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLStencilBuffer_DEFINED
#define GrGLStencilBuffer_DEFINED

#include "GrGLInterface.h"
#include "GrStencilBuffer.h"

class GrGLStencilBuffer : public GrStencilBuffer {
public:
    static const GrGLenum kUnknownInternalFormat = ~0;
    struct Format {
        GrGLenum  fInternalFormat;
        GrGLuint  fStencilBits;
        GrGLuint  fTotalBits;
        bool      fPacked;
    };

    GrGLStencilBuffer(GrGpu* gpu, GrGLint rbid, 
                      int width, int height,
                      int sampleCnt,
                      const Format& format) 
        : GrStencilBuffer(gpu, width, height, format.fStencilBits, sampleCnt)
        , fFormat(format)
        , fRenderbufferID(rbid) {
    }

    virtual ~GrGLStencilBuffer() {
        this->release();
    }

    virtual size_t sizeInBytes() const {
        uint64_t size = this->width();
        size *= this->height();
        size *= fFormat.fTotalBits;
        size *= GrMax(1,this->numSamples());
        return (size_t)(size / 8);
    }

    GrGLuint renderbufferID() const {
        return fRenderbufferID;
    }

    const Format& format() const {
        return fFormat;
    }

protected:
    virtual void onRelease() {
        if (0 != fRenderbufferID) {
            GR_GL(DeleteRenderbuffers(1, &fRenderbufferID));
            fRenderbufferID = 0;
        }
    }

    virtual void onAbandon() {
        fRenderbufferID = 0;
    }

private:
    Format fFormat;
    // may be zero for external SBs associated with external RTs
    // (we don't require the client to give us the id, just tell
    // us how many bits of stencil there are).
    GrGLuint fRenderbufferID;

    typedef GrStencilBuffer INHERITED;
};

#endif

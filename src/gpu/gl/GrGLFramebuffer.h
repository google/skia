/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFramebuffer_DEFINED
#define GrGLFramebuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/gl/GrGLTypes.h"

class GrGLAttachment;

class GrGLFramebuffer : public SkRefCnt {
public:
    GrGLFramebuffer(GrGLuint fboID,
                    sk_sp<GrGLAttachment> colorAttachment,
                    sk_sp<GrGLAttachment> stencilAttachment);

    SkISize dimensions() const;
    int height() const { return this->dimensions().height(); }

    GrGLuint fboID() const { return fFBOID; }
    bool glFBOIDis0() const { return fFBOID == 0;}

    const GrGLAttachment* colorAttachment() const { return fColorAttachment.get(); }
    int numColorSamples() const;

    bool hasStencilAttachment() const { return fStencilAttachment.get(); }
    int numStencilBits() const;

    uint32_t uniqueID() const { return fUniqueID; }

private:
    static uint32_t GenID() {
        static std::atomic<uint32_t> nextID{1};
        uint32_t id;
        do {
            id = nextID++;
        } while (id == SK_InvalidUniqueID);
        return id;
    }

    GrGLuint fFBOID;

    sk_sp<GrGLAttachment> fColorAttachment;
    sk_sp<GrGLAttachment> fStencilAttachment;

    uint32_t fUniqueID;
};
#endif

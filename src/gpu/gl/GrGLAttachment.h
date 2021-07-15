/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLMtlAttachment_DEFINED
#define GrGLMtlAttachment_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/GrAttachment.h"

class GrGLGpu;

class GrGLAttachment : public GrAttachment {
public:
    static sk_sp<GrGLAttachment> MakeStencil(GrGLGpu* gpu,
                                             SkISize dimensions,
                                             int sampleCnt,
                                             GrGLFormat format);

    static sk_sp<GrGLAttachment> MakeMSAA(GrGLGpu* gpu,
                                          SkISize dimensions,
                                          int sampleCnt,
                                          GrGLFormat format);

    static sk_sp<GrGLAttachment> MakeWrappedRenderBuffer(GrGpu* gpu,
                                                         GrGLuint renderbufferID,
                                                         SkISize dimensions,
                                                         UsageFlags supportedUsages,
                                                         int sampleCnt,
                                                         GrGLFormat format) {
        return sk_sp<GrGLAttachment>(
                new GrGLAttachment(gpu, renderbufferID, dimensions, supportedUsages, sampleCnt,
                                   format));
    }

    GrBackendFormat backendFormat() const override;

    GrGLuint renderbufferID() const { return fRenderbufferID; }

    GrGLFormat format() const { return fFormat; }

protected:
    // overrides of GrResource
    void onRelease() override;
    void onAbandon() override;
    void setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                          const SkString& dumpName) const override;

private:
    GrGLAttachment(GrGpu* gpu,
                   GrGLuint renderbufferID,
                   SkISize dimensions,
                   UsageFlags supportedUsages,
                   int sampleCnt,
                   GrGLFormat format)
            : GrAttachment(gpu, dimensions, supportedUsages, sampleCnt, GrMipmapped::kNo,
                           GrProtected::kNo)
            , fFormat(format)
            , fRenderbufferID(renderbufferID) {
        SkASSERT(supportedUsages == UsageFlags::kStencilAttachment ||
                 supportedUsages == UsageFlags::kColorAttachment);
        this->registerWithCache(SkBudgeted::kYes);
    }

    GrGLFormat fFormat;

    // may be zero for external SBs associated with external RTs
    // (we don't require the client to give us the id, just tell
    // us how many bits of stencil there are).
    GrGLuint fRenderbufferID;

    using INHERITED = GrAttachment;
};

#endif

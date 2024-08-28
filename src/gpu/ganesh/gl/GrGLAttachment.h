/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLMtlAttachment_DEFINED
#define GrGLMtlAttachment_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrAttachment.h"

#include <string_view>

class GrGLGpu;
class GrGpu;
class SkString;
class SkTraceMemoryDump;

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
        return sk_sp<GrGLAttachment>(new GrGLAttachment(
                gpu, renderbufferID, dimensions, supportedUsages, sampleCnt, format,
                /*label=*/"MakeWrappedRenderBuffer"));
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
                   GrGLFormat format,
                   std::string_view label)
            : GrAttachment(gpu,
                           dimensions,
                           supportedUsages,
                           sampleCnt,
                           skgpu::Mipmapped::kNo,
                           GrProtected::kNo,
                           label)
            , fFormat(format)
            , fRenderbufferID(renderbufferID) {
        SkASSERT(supportedUsages == UsageFlags::kStencilAttachment ||
                 supportedUsages == UsageFlags::kColorAttachment);
        this->registerWithCache(skgpu::Budgeted::kYes);
    }

    void onSetLabel() override;

    GrGLFormat fFormat;

    // may be zero for external SBs associated with external RTs
    // (we don't require the client to give us the id, just tell
    // us how many bits of stencil there are).
    GrGLuint fRenderbufferID;

    using INHERITED = GrAttachment;
};

#endif

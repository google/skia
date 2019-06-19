/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTraceMemoryDump.h"

#include "tests/Test.h"

#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLGpu.h"

/*
 * Build test for SkTraceMemoryDump.
 */
class TestSkTraceMemoryDump : public SkTraceMemoryDump {
public:
    TestSkTraceMemoryDump(bool shouldDumpWrappedObjects)
            : fShouldDumpWrappedObjects(shouldDumpWrappedObjects) {}
    ~TestSkTraceMemoryDump() override { }

    void dumpNumericValue(const char* dumpName, const char* valueName, const char* units,
                          uint64_t value) override {
        // Only count "size" dumps, others are just providing metadata.
        if (SkString("size") == SkString(valueName)) {
            ++fNumDumpedObjects;
            fDumpedObjectsSize += value;
        }
    }
    void setMemoryBacking(const char* dumpName, const char* backingType,
                          const char* backingObjectId) override { }
    void setDiscardableMemoryBacking(
        const char* dumpName,
        const SkDiscardableMemory& discardableMemoryObject) override { }
    LevelOfDetail getRequestedDetails() const override {
        return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
    }
    bool shouldDumpWrappedObjects() const override { return fShouldDumpWrappedObjects; }

    size_t numDumpedObjects() const { return fNumDumpedObjects; }
    size_t dumpedObjectsSize() const { return fDumpedObjectsSize; }

private:
    bool fShouldDumpWrappedObjects;
    size_t fNumDumpedObjects = 0;
    size_t fDumpedObjectsSize = 0;
};

void ValidateMemoryDumps(skiatest::Reporter* reporter, GrContext* context, size_t size,
                         bool isOwned) {
    // Note than one entry in the dumped objects is expected for the text blob cache.
    TestSkTraceMemoryDump dump_with_wrapped(true /* shouldDumpWrappedObjects */);
    context->dumpMemoryStatistics(&dump_with_wrapped);
    REPORTER_ASSERT(reporter, 2 == dump_with_wrapped.numDumpedObjects());
    REPORTER_ASSERT(reporter, size == dump_with_wrapped.dumpedObjectsSize());

    TestSkTraceMemoryDump dump_no_wrapped(false /* shouldDumpWrappedObjects */);
    context->dumpMemoryStatistics(&dump_no_wrapped);
    if (isOwned) {
        REPORTER_ASSERT(reporter, 2 == dump_no_wrapped.numDumpedObjects());
        REPORTER_ASSERT(reporter, size == dump_no_wrapped.dumpedObjectsSize());
    } else {
        REPORTER_ASSERT(reporter, 1 == dump_no_wrapped.numDumpedObjects());
        REPORTER_ASSERT(reporter, 0 == dump_no_wrapped.dumpedObjectsSize());
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_ownedGLBuffer, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->priv().getGpu());
    const size_t kMemorySize = 1024;
    sk_sp<GrGLBuffer> buffer =
            GrGLBuffer::Make(gpu, kMemorySize, GrGpuBufferType::kVertex, kDynamic_GrAccessPattern);

    ValidateMemoryDumps(reporter, context, kMemorySize, true /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_ownedGLTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->priv().getGpu());

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 1;

    GrGLTextureInfo glInfo;
    glInfo.fTarget = GR_GL_TEXTURE_2D;
    glInfo.fID = 7;  // Arbitrary, we don't actually use the texture.
    glInfo.fFormat = GR_GL_RGBA8;

    GrGLTexture::IDDesc idDesc;
    idDesc.fInfo = glInfo;
    idDesc.fOwnership = GrBackendObjectOwnership::kOwned;

    auto texture = sk_make_sp<GrGLTexture>(gpu, SkBudgeted::kNo, desc, idDesc,
                                           GrMipMapsStatus::kNotAllocated);

    ValidateMemoryDumps(reporter, context, texture->gpuMemorySize(), true /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_unownedGLTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->priv().getGpu());

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 1;

    GrGLTextureInfo glInfo;
    glInfo.fTarget = GR_GL_TEXTURE_2D;
    glInfo.fID = 7;  // Arbitrary, we don't actually use the texture.
    glInfo.fFormat = GR_GL_RGBA8;

    GrGLTexture::IDDesc idDesc;
    idDesc.fInfo = glInfo;
    idDesc.fOwnership = GrBackendObjectOwnership::kBorrowed;

    auto params = sk_make_sp<GrGLTextureParameters>();

    auto texture =
            GrGLTexture::MakeWrapped(gpu, desc, GrMipMapsStatus::kNotAllocated, idDesc,
                                     std::move(params), GrWrapCacheable::kNo, kRead_GrIOType);

    ValidateMemoryDumps(reporter, context, texture->gpuMemorySize(), false /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_ownedGLRenderTarget, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->priv().getGpu());

    GrSurfaceDesc sd;
    sd.fFlags = kRenderTarget_GrSurfaceFlag;
    sd.fWidth = 64;
    sd.fHeight = 64;
    sd.fConfig = kRGBA_8888_GrPixelConfig;

    GrGLRenderTarget::IDDesc iddesc;
    iddesc.fRTFBOID = 20;
    iddesc.fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    iddesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    iddesc.fMSColorRenderbufferID = 22;

    sk_sp<GrGLRenderTarget> rt = GrGLRenderTarget::MakeWrapped(gpu, sd, GR_GL_RGBA8, iddesc, 0);

    ValidateMemoryDumps(reporter, context, rt->gpuMemorySize(), true /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_unownedGLRenderTarget, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->priv().getGpu());

    GrSurfaceDesc sd;
    sd.fFlags = kRenderTarget_GrSurfaceFlag;
    sd.fWidth = 64;
    sd.fHeight = 64;
    sd.fConfig = kRGBA_8888_GrPixelConfig;

    GrGLRenderTarget::IDDesc iddesc;
    iddesc.fRTFBOID = 20;
    iddesc.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;
    iddesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    iddesc.fMSColorRenderbufferID = 22;

    sk_sp<GrGLRenderTarget> rt = GrGLRenderTarget::MakeWrapped(gpu, sd, GR_GL_RGBA8, iddesc, 0);

    ValidateMemoryDumps(reporter, context, rt->gpuMemorySize(), false /* isOwned */);
}

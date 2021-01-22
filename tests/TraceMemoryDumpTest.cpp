/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrDirectContext.h"

#include "tests/Test.h"

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#ifdef SK_GL
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLGpu.h"
#endif

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

void ValidateMemoryDumps(skiatest::Reporter* reporter, GrDirectContext* dContext, size_t size,
                         bool isOwned) {
    // Note than one entry in the dumped objects is expected for the text blob cache.
    TestSkTraceMemoryDump dump_with_wrapped(true /* shouldDumpWrappedObjects */);
    dContext->dumpMemoryStatistics(&dump_with_wrapped);
    REPORTER_ASSERT(reporter, 2 == dump_with_wrapped.numDumpedObjects());
    REPORTER_ASSERT(reporter, size == dump_with_wrapped.dumpedObjectsSize());

    TestSkTraceMemoryDump dump_no_wrapped(false /* shouldDumpWrappedObjects */);
    dContext->dumpMemoryStatistics(&dump_no_wrapped);
    if (isOwned) {
        REPORTER_ASSERT(reporter, 2 == dump_no_wrapped.numDumpedObjects());
        REPORTER_ASSERT(reporter, size == dump_no_wrapped.dumpedObjectsSize());
    } else {
        REPORTER_ASSERT(reporter, 1 == dump_no_wrapped.numDumpedObjects());
        REPORTER_ASSERT(reporter, 0 == dump_no_wrapped.dumpedObjectsSize());
    }
}

#ifdef SK_GL
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_ownedGLBuffer, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());
    const size_t kMemorySize = 1024;
    sk_sp<GrGLBuffer> buffer =
            GrGLBuffer::Make(gpu, kMemorySize, GrGpuBufferType::kVertex, kDynamic_GrAccessPattern);

    ValidateMemoryDumps(reporter, dContext, kMemorySize, true /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_ownedGLTexture, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    GrGLTexture::Desc desc;
    desc.fTarget = GR_GL_TEXTURE_2D;
    desc.fID = 7;  // Arbitrary, we don't actually use the texture.
    desc.fFormat = GrGLFormat::kRGBA8;
    desc.fOwnership = GrBackendObjectOwnership::kOwned;
    desc.fSize = SkISize::Make(64, 64);

    auto texture =
            sk_make_sp<GrGLTexture>(gpu, SkBudgeted::kNo, desc, GrMipmapStatus::kNotAllocated);

    ValidateMemoryDumps(reporter, dContext, texture->gpuMemorySize(), true /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_unownedGLTexture, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    GrGLTexture::Desc desc;
    desc.fTarget = GR_GL_TEXTURE_2D;
    desc.fID = 7;  // Arbitrary, we don't actually use the texture.
    desc.fFormat = GrGLFormat::kRGBA8;
    desc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    desc.fSize = SkISize::Make(64, 64);

    auto params = sk_make_sp<GrGLTextureParameters>();

    auto texture =
            GrGLTexture::MakeWrapped(gpu, GrMipmapStatus::kNotAllocated, desc, std::move(params),
                                     GrWrapCacheable::kNo, kRead_GrIOType);

    ValidateMemoryDumps(reporter, dContext, texture->gpuMemorySize(), false /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_ownedGLRenderTarget, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    static constexpr auto kSize = SkISize::Make(64, 64);

    GrGLRenderTarget::IDs rtIDs;
    rtIDs.fRTFBOID = 20;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    rtIDs.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    rtIDs.fMSColorRenderbufferID = 22;

    sk_sp<GrGLRenderTarget> rt =
            GrGLRenderTarget::MakeWrapped(gpu, kSize, GrGLFormat::kRGBA8, 1, rtIDs, 0);

    ValidateMemoryDumps(reporter, dContext, rt->gpuMemorySize(), true /* isOwned */);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(SkTraceMemoryDump_unownedGLRenderTarget, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    static constexpr auto kSize = SkISize::Make(64, 64);

    GrGLRenderTarget::IDs rtIDs;
    rtIDs.fRTFBOID = 20;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;
    rtIDs.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    rtIDs.fMSColorRenderbufferID = 22;

    sk_sp<GrGLRenderTarget> rt =
            GrGLRenderTarget::MakeWrapped(gpu, kSize, GrGLFormat::kRGBA8, 1, rtIDs, 0);

    ValidateMemoryDumps(reporter, dContext, rt->gpuMemorySize(), false /* isOwned */);
}
#endif  // SK_GL

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_GL)
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/gpu/ganesh/GrGLTypesPriv.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLBuffer.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLRenderTarget.h"
#include "src/gpu/ganesh/gl/GrGLTexture.h"
#include "src/gpu/ganesh/gl/GrGLTextureRenderTarget.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class SkDiscardableMemory;
struct GrContextOptions;

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

void ValidateMemoryDumps(skiatest::Reporter* reporter, GrDirectContext* dContext,
                         size_t numDumpedObjects, size_t size, bool isOwned) {
    // Note than one entry in the dumped objects is expected for the text blob cache.
    TestSkTraceMemoryDump dump_with_wrapped(true /* shouldDumpWrappedObjects */);
    dContext->dumpMemoryStatistics(&dump_with_wrapped);
    REPORTER_ASSERT(reporter, numDumpedObjects == dump_with_wrapped.numDumpedObjects());
    REPORTER_ASSERT(reporter, size == dump_with_wrapped.dumpedObjectsSize());

    TestSkTraceMemoryDump dump_no_wrapped(false /* shouldDumpWrappedObjects */);
    dContext->dumpMemoryStatistics(&dump_no_wrapped);
    if (isOwned) {
        REPORTER_ASSERT(reporter, numDumpedObjects == dump_no_wrapped.numDumpedObjects());
        REPORTER_ASSERT(reporter, size == dump_no_wrapped.dumpedObjectsSize());
    } else {
        REPORTER_ASSERT(reporter, 1 == dump_no_wrapped.numDumpedObjects());
        REPORTER_ASSERT(reporter, 0 == dump_no_wrapped.dumpedObjectsSize());
    }
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkTraceMemoryDump_ownedGLBuffer,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());
    const size_t kMemorySize = 1024;
    sk_sp<GrGLBuffer> buffer =
            GrGLBuffer::Make(gpu, kMemorySize, GrGpuBufferType::kVertex, kDynamic_GrAccessPattern);

    ValidateMemoryDumps(reporter, dContext, 2, kMemorySize, true /* isOwned */);
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkTraceMemoryDump_ownedGLTexture,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    GrGLTexture::Desc desc;
    desc.fTarget = GR_GL_TEXTURE_2D;
    desc.fID = 7;  // Arbitrary, we don't actually use the texture.
    desc.fFormat = GrGLFormat::kRGBA8;
    desc.fOwnership = GrBackendObjectOwnership::kOwned;
    desc.fIsProtected = skgpu::Protected::kNo;
    desc.fSize = SkISize::Make(64, 64);

    auto texture = sk_make_sp<GrGLTexture>(gpu,
                                           skgpu::Budgeted::kNo,
                                           desc,
                                           GrMipmapStatus::kNotAllocated,
                                           /*label=*/"SkTraceMemoryDump_ownedGLTexture");

    ValidateMemoryDumps(reporter, dContext, 2, texture->gpuMemorySize(), true /* isOwned */);
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkTraceMemoryDump_unownedGLTexture,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    GrGLTexture::Desc desc;
    desc.fTarget = GR_GL_TEXTURE_2D;
    desc.fID = 7;  // Arbitrary, we don't actually use the texture.
    desc.fFormat = GrGLFormat::kRGBA8;
    desc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    desc.fSize = SkISize::Make(64, 64);
    desc.fIsProtected = skgpu::Protected::kNo;

    auto params = sk_make_sp<GrGLTextureParameters>();

    auto texture = GrGLTexture::MakeWrapped(gpu,
                                            GrMipmapStatus::kNotAllocated,
                                            desc,
                                            std::move(params),
                                            GrWrapCacheable::kNo,
                                            kRead_GrIOType,
                                            /*label=*/{});

    ValidateMemoryDumps(reporter, dContext, 2, texture->gpuMemorySize(), false /* isOwned */);
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkTraceMemoryDump_ownedGLRenderTarget,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    static constexpr auto kSize = SkISize::Make(64, 64);

    GrGLRenderTarget::IDs rtIDs;
    rtIDs.fMultisampleFBOID = 0;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    rtIDs.fSingleSampleFBOID = 20;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fTotalMemorySamplesPerPixel = 1;

    sk_sp<GrGLRenderTarget> rt = GrGLRenderTarget::MakeWrapped(gpu,
                                                               kSize,
                                                               GrGLFormat::kRGBA8,
                                                               1,
                                                               rtIDs,
                                                               0,
                                                               skgpu::Protected::kNo,
                                                               /*label=*/{});

    ValidateMemoryDumps(reporter, dContext, 2, rt->gpuMemorySize(), true /* isOwned */);
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkTraceMemoryDump_unownedGLRenderTarget,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    static constexpr auto kSize = SkISize::Make(64, 64);

    GrGLRenderTarget::IDs rtIDs;
    rtIDs.fMultisampleFBOID = 12;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;
    rtIDs.fSingleSampleFBOID = 0;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fTotalMemorySamplesPerPixel = 4;

    sk_sp<GrGLRenderTarget> rt = GrGLRenderTarget::MakeWrapped(gpu,
                                                               kSize,
                                                               GrGLFormat::kRGBA8,
                                                               4,
                                                               rtIDs,
                                                               0,
                                                               skgpu::Protected::kNo,
                                                               /*label=*/{});

    ValidateMemoryDumps(reporter, dContext, 2, rt->gpuMemorySize(), false /* isOwned */);
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(SkTraceMemoryDump_ownedGLTextureRenderTarget,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    static constexpr auto kSize = SkISize::Make(64, 64);

    GrGLTexture::Desc texDesc;
    texDesc.fSize = kSize;
    texDesc.fTarget = GR_GL_TEXTURE_2D;
    texDesc.fID = 17;
    texDesc.fFormat = GrGLFormat::kRGBA8;
    texDesc.fOwnership = GrBackendObjectOwnership::kOwned;
    texDesc.fIsProtected = skgpu::Protected::kNo;

    GrGLRenderTarget::IDs rtIDs;
    rtIDs.fMultisampleFBOID = 12;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    rtIDs.fSingleSampleFBOID = 20;
    rtIDs.fMSColorRenderbufferID = 22;
    rtIDs.fTotalMemorySamplesPerPixel = 9;

    auto texRT = sk_make_sp<GrGLTextureRenderTarget>(
            gpu,
            skgpu::Budgeted::kYes,
            8,
            texDesc,
            rtIDs,
            GrMipmapStatus::kNotAllocated,
            /*label=*/"SkTraceMemoryDump_ownedGLTextureRenderTarget");

    ValidateMemoryDumps(reporter, dContext, 3, texRT->gpuMemorySize(), true /* isOwned */);
}

#endif  // defined(SK_GL)

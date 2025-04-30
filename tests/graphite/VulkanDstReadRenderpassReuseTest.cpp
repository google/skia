/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

/**
 * Implementation of SkTraceMemoryDump which tracks the number of resources whose type values
 * contain a provided string within a ResourceCache. Could easily be elevated to be shared if other
 * tests could benefit from this utility, but for now is only used within this test.
 */
class ResourceTypeQuantifier : public SkTraceMemoryDump {
public:
    ResourceTypeQuantifier() = delete;
    ~ResourceTypeQuantifier() override {}
    ResourceTypeQuantifier(SkString resourceTypeSubstr) : fResourceTypeSubstr(resourceTypeSubstr) {}

    // Override relevant methods
    void dumpStringValue(const char* dumpName, const char* valueName, const char* value) override {
        if (SkString("type").equals(valueName)) {
            if (SkString(value).contains(fResourceTypeSubstr.c_str())) {
                fNumResourcesWithTypeSubstr++;
            }
        }
    }

    LevelOfDetail getRequestedDetails() const override {
        return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
    }

    bool shouldDumpSizelessObjects() const override { return true; }

    // Define methods unique to this implementation
    uint8_t numResourcesWithTypeSubstr() const { return fNumResourcesWithTypeSubstr; }
    void resetCount() { fNumResourcesWithTypeSubstr = 0; }

    // Override unneeded methods to be no-ops
    void setMemoryBacking(const char* dumpName, const char* backingType,
                          const char* backingObjectId) override {}
    void setDiscardableMemoryBacking(
            const char* dumpName, const SkDiscardableMemory& discardableMemoryObject) override {}
    void dumpNumericValue(const char* dumpName, const char* valueName, const char* units,
                          uint64_t value) override {}

private:
    SkString fResourceTypeSubstr;
    int fNumResourcesWithTypeSubstr = 0;
};

/**
 * Unit test to ensure that the Vulkan backend shares renderpasses between draws that do and
 * do not read from the dst texture as intended for optimization purposes. Could be expanded to test
 * every blend mode that reads from the dst against all that do not, but testing just one should be
 * sufficient to prove out that renderpasses can be shared between the different blend/draw types.
 */
DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(VulkanDstReadsShareRenderpass, reporter, context,
                                     CtsEnforcement::kApiLevel_202504) {
    // Set up a render target surface which we should be able to read from as an input attachment.
    // Our Vulkan backend makes render targets usable as input attachments by default.
    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(10, 10),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    // Select which blend modes to use for testing: one that we do expect to require a dst read
    // and one we do not. Assert that the dst read requirements of the modes align to
    // expectations. For the sake of this test, assume Coverage::kNone.
    static const SkBlendMode simpleBlendMode = SkBlendMode::kSrcATop;
    static const SkBlendMode dstReadBlendMode = SkBlendMode::kLighten;
    REPORTER_ASSERT(reporter, CanUseHardwareBlending(caps, simpleBlendMode, Coverage::kNone));
    REPORTER_ASSERT(reporter, !CanUseHardwareBlending(caps, dstReadBlendMode, Coverage::kNone));

    // Perform draw operations which do not read from the dst. Clear the canvas to initialize it to
    // any color, drawing a color over it afterwards using the simple blend mode.
    canvas->clear(SK_ColorMAGENTA);
    canvas->drawColor(SK_ColorCYAN, simpleBlendMode);
    std::unique_ptr<Recording> simpleBlendRecording = recorder->snap();

    // Determine the number of renderpasses used for later comparison, asserting we have at least 1.
    ResourceTypeQuantifier renderpassQuantifier { SkString("RenderPass") };
    recorder->dumpMemoryStatistics(&renderpassQuantifier);
    const int numRenderPassesNoDstRead = renderpassQuantifier.numResourcesWithTypeSubstr();
    REPORTER_ASSERT(reporter, numRenderPassesNoDstRead > 0);

    // Perform a draw using a blend mode which does require reading from the dst texture. Determine
    // how many renderpasses are in the cache after this operation.
    canvas->drawColor(SK_ColorYELLOW, dstReadBlendMode);
    std::unique_ptr<Recording> dstReadRecording = recorder->snap();
    renderpassQuantifier.resetCount();
    recorder->dumpMemoryStatistics(&renderpassQuantifier);
    const int numRenderPassesAfterDstRead = renderpassQuantifier.numResourcesWithTypeSubstr();

    // Assert that no new renderpasses have been added to the cache, meaning that one was reused
    // from the draw that did not require a dst read.
    REPORTER_ASSERT(reporter,
                    numRenderPassesNoDstRead == numRenderPassesAfterDstRead,
                    "# renderpasses with no dst read: %d ; after dst read: %d",
                    numRenderPassesNoDstRead,
                    numRenderPassesAfterDstRead);
}

}  // namespace skgpu::graphite

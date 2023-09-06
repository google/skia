/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/Slug.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontMetricsPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTypeface_remote.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/GlyphRun.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlob.h"

#include <atomic>
#include <cstring>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

class SkPaint;

using namespace skia_private;
using namespace sktext;
using namespace sktext::gpu;
using namespace skglyph;

namespace {

// -- StrikeSpec -----------------------------------------------------------------------------------
struct StrikeSpec {
    StrikeSpec() = default;
    StrikeSpec(SkTypefaceID typefaceID, SkDiscardableHandleId discardableHandleId)
            : fTypefaceID{typefaceID}, fDiscardableHandleId(discardableHandleId) {}
    SkTypefaceID fTypefaceID = 0u;
    SkDiscardableHandleId fDiscardableHandleId = 0u;
};

// -- RemoteStrike ----------------------------------------------------------------------------
class RemoteStrike final : public sktext::StrikeForGPU {
public:
    // N.B. RemoteStrike is not valid until ensureScalerContext is called.
    RemoteStrike(const SkStrikeSpec& strikeSpec,
                 std::unique_ptr<SkScalerContext> context,
                 SkDiscardableHandleId discardableHandleId);
    ~RemoteStrike() override = default;

    void lock() override {}
    void unlock() override {}
    SkGlyphDigest digestFor(skglyph::ActionType, SkPackedGlyphID) override;
    bool prepareForImage(SkGlyph* glyph) override {
        this->ensureScalerContext();
        glyph->setImage(&fAlloc, fContext.get());
        return glyph->image() != nullptr;
    }
    bool prepareForPath(SkGlyph* glyph) override {
        this->ensureScalerContext();
        glyph->setPath(&fAlloc, fContext.get());
        return glyph->path() != nullptr;
    }
    bool prepareForDrawable(SkGlyph* glyph) override {
        this->ensureScalerContext();
        glyph->setDrawable(&fAlloc, fContext.get());
        return glyph->drawable() != nullptr;
    }

    void writePendingGlyphs(SkWriteBuffer& buffer);

    SkDiscardableHandleId discardableHandleId() const { return fDiscardableHandleId; }

    const SkDescriptor& getDescriptor() const override {
        return *fDescriptor.getDesc();
    }

    void setStrikeSpec(const SkStrikeSpec& strikeSpec);

    const SkGlyphPositionRoundingSpec& roundingSpec() const override {
        return fRoundingSpec;
    }

    sktext::SkStrikePromise strikePromise() override;

    bool hasPendingGlyphs() const {
        return !fMasksToSend.empty() || !fPathsToSend.empty() || !fDrawablesToSend.empty();
    }

    void resetScalerContext();

private:
    void ensureScalerContext();

    const SkAutoDescriptor fDescriptor;
    const SkDiscardableHandleId fDiscardableHandleId;

    const SkGlyphPositionRoundingSpec fRoundingSpec;

    // The context built using fDescriptor
    std::unique_ptr<SkScalerContext> fContext;

    // fStrikeSpec is set every time getOrCreateCache is called. This allows the code to maintain
    // the fContext as lazy as possible.
    const SkStrikeSpec* fStrikeSpec;

    // Have the metrics been sent for this strike. Only send them once.
    bool fHaveSentFontMetrics{false};

    // The masks and paths that currently reside in the GPU process.
    THashTable<SkGlyphDigest, SkPackedGlyphID, SkGlyphDigest> fSentGlyphs;

    // The Masks, SDFT Mask, and Paths that need to be sent to the GPU task for the processed
    // TextBlobs. Cleared after diffs are serialized.
    std::vector<SkGlyph> fMasksToSend;
    std::vector<SkGlyph> fPathsToSend;
    std::vector<SkGlyph> fDrawablesToSend;

    // Alloc for storing bits and pieces of paths and drawables, Cleared after diffs are serialized.
    SkArenaAllocWithReset fAlloc{256};
};

RemoteStrike::RemoteStrike(
        const SkStrikeSpec& strikeSpec,
        std::unique_ptr<SkScalerContext> context,
        uint32_t discardableHandleId)
        : fDescriptor{strikeSpec.descriptor()}
        , fDiscardableHandleId(discardableHandleId)
        , fRoundingSpec{context->isSubpixel(), context->computeAxisAlignmentForHText()}
        // N.B. context must come last because it is used above.
        , fContext{std::move(context)} {
    SkASSERT(fDescriptor.getDesc() != nullptr);
    SkASSERT(fContext != nullptr);
}

void RemoteStrike::writePendingGlyphs(SkWriteBuffer& buffer) {
    SkASSERT(this->hasPendingGlyphs());

    buffer.writeUInt(fContext->getTypeface()->uniqueID());
    buffer.writeUInt(fDiscardableHandleId);
    fDescriptor.getDesc()->flatten(buffer);

    buffer.writeBool(fHaveSentFontMetrics);
    if (!fHaveSentFontMetrics) {
        // Write FontMetrics if not sent before.
        SkFontMetrics fontMetrics;
        fContext->getFontMetrics(&fontMetrics);
        SkFontMetricsPriv::Flatten(buffer, fontMetrics);
        fHaveSentFontMetrics = true;
    }

    // Make sure to install all the mask data into the glyphs before sending.
    for (SkGlyph& glyph: fMasksToSend) {
        this->prepareForImage(&glyph);
    }

    // Make sure to install all the path data into the glyphs before sending.
    for (SkGlyph& glyph: fPathsToSend) {
        this->prepareForPath(&glyph);
    }

    // Make sure to install all the drawable data into the glyphs before sending.
    for (SkGlyph& glyph: fDrawablesToSend) {
        this->prepareForDrawable(&glyph);
    }

    // Send all the pending glyph information.
    SkStrike::FlattenGlyphsByType(buffer, fMasksToSend, fPathsToSend, fDrawablesToSend);

    // Reset all the sending data.
    fMasksToSend.clear();
    fPathsToSend.clear();
    fDrawablesToSend.clear();
    fAlloc.reset();
}

void RemoteStrike::ensureScalerContext() {
    if (fContext == nullptr) {
        fContext = fStrikeSpec->createScalerContext();
    }
}

void RemoteStrike::resetScalerContext() {
    fContext = nullptr;
    fStrikeSpec = nullptr;
}

void RemoteStrike::setStrikeSpec(const SkStrikeSpec& strikeSpec) {
    fStrikeSpec = &strikeSpec;
}

SkGlyphDigest RemoteStrike::digestFor(ActionType actionType, SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* digestPtr = fSentGlyphs.find(packedGlyphID);
    if (digestPtr != nullptr && digestPtr->actionFor(actionType) != GlyphAction::kUnset) {
        return *digestPtr;
    }

    SkGlyph* glyph;
    this->ensureScalerContext();
    switch (actionType) {
        case skglyph::kPath: {
            fPathsToSend.emplace_back(fContext->makeGlyph(packedGlyphID, &fAlloc));
            glyph = &fPathsToSend.back();
            break;
        }
        case skglyph::kDrawable: {
            fDrawablesToSend.emplace_back(fContext->makeGlyph(packedGlyphID, &fAlloc));
            glyph = &fDrawablesToSend.back();
            break;
        }
        default: {
            fMasksToSend.emplace_back(fContext->makeGlyph(packedGlyphID, &fAlloc));
            glyph = &fMasksToSend.back();
            break;
        }
    }

    if (digestPtr == nullptr) {
        digestPtr = fSentGlyphs.set(SkGlyphDigest{0, *glyph});
    }

    digestPtr->setActionFor(actionType, glyph, this);

    return *digestPtr;
}

sktext::SkStrikePromise RemoteStrike::strikePromise() {
    return sktext::SkStrikePromise{*this->fStrikeSpec};
}
}  // namespace

// -- SkStrikeServerImpl ---------------------------------------------------------------------------
class SkStrikeServerImpl final : public sktext::StrikeForGPUCacheInterface {
public:
    explicit SkStrikeServerImpl(
            SkStrikeServer::DiscardableHandleManager* discardableHandleManager);

    // SkStrikeServer API methods
    void writeStrikeData(std::vector<uint8_t>* memory);

    sk_sp<sktext::StrikeForGPU> findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) override;

    // Methods for testing
    void setMaxEntriesInDescriptorMapForTesting(size_t count);
    size_t remoteStrikeMapSizeForTesting() const;

private:
    inline static constexpr size_t kMaxEntriesInDescriptorMap = 2000u;

    void checkForDeletedEntries();

    sk_sp<RemoteStrike> getOrCreateCache(const SkStrikeSpec& strikeSpec);

    struct MapOps {
        size_t operator()(const SkDescriptor* key) const {
            return key->getChecksum();
        }
        bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
            return *lhs == *rhs;
        }
    };

    using DescToRemoteStrike =
        std::unordered_map<const SkDescriptor*, sk_sp<RemoteStrike>, MapOps, MapOps>;
    DescToRemoteStrike fDescToRemoteStrike;

    SkStrikeServer::DiscardableHandleManager* const fDiscardableHandleManager;
    THashSet<SkTypefaceID> fCachedTypefaces;
    size_t fMaxEntriesInDescriptorMap = kMaxEntriesInDescriptorMap;

    // State cached until the next serialization.
    THashSet<RemoteStrike*> fRemoteStrikesToSend;
    std::vector<SkTypefaceProxyPrototype> fTypefacesToSend;
};

SkStrikeServerImpl::SkStrikeServerImpl(SkStrikeServer::DiscardableHandleManager* dhm)
        : fDiscardableHandleManager(dhm) {
    SkASSERT(fDiscardableHandleManager);
}

void SkStrikeServerImpl::setMaxEntriesInDescriptorMapForTesting(size_t count) {
    fMaxEntriesInDescriptorMap = count;
}
size_t SkStrikeServerImpl::remoteStrikeMapSizeForTesting() const {
    return fDescToRemoteStrike.size();
}

void SkStrikeServerImpl::writeStrikeData(std::vector<uint8_t>* memory) {
    SkBinaryWriteBuffer buffer{nullptr, 0};

    // Gather statistics about what needs to be sent.
    size_t strikesToSend = 0;
    fRemoteStrikesToSend.foreach([&](RemoteStrike* strike) {
        if (strike->hasPendingGlyphs()) {
            strikesToSend++;
        } else {
            // This strike has nothing to send, so drop its scaler context to reduce memory.
            strike->resetScalerContext();
        }
    });

    // If there are no strikes or typefaces to send, then cleanup and return.
    if (strikesToSend == 0 && fTypefacesToSend.empty()) {
        fRemoteStrikesToSend.reset();
        return;
    }

    // Send newly seen typefaces.
    SkASSERT_RELEASE(SkTFitsIn<int>(fTypefacesToSend.size()));
    buffer.writeInt(fTypefacesToSend.size());
    for (const auto& typeface: fTypefacesToSend) {
        SkTypefaceProxyPrototype proto{typeface};
        proto.flatten(buffer);
    }
    fTypefacesToSend.clear();

    buffer.writeInt(strikesToSend);
    fRemoteStrikesToSend.foreach(
            [&](RemoteStrike* strike) {
                if (strike->hasPendingGlyphs()) {
                    strike->writePendingGlyphs(buffer);
                    strike->resetScalerContext();
                }
            }
    );
    fRemoteStrikesToSend.reset();

    // Copy data into the vector.
    auto data = buffer.snapshotAsData();
    memory->assign(data->bytes(), data->bytes() + data->size());
}

sk_sp<StrikeForGPU> SkStrikeServerImpl::findOrCreateScopedStrike(
        const SkStrikeSpec& strikeSpec) {
    return this->getOrCreateCache(strikeSpec);
}

void SkStrikeServerImpl::checkForDeletedEntries() {
    auto it = fDescToRemoteStrike.begin();
    while (fDescToRemoteStrike.size() > fMaxEntriesInDescriptorMap &&
           it != fDescToRemoteStrike.end()) {
        RemoteStrike* strike = it->second.get();
        if (fDiscardableHandleManager->isHandleDeleted(strike->discardableHandleId())) {
            // If we are trying to send the strike, then do not erase it.
            if (!fRemoteStrikesToSend.contains(strike)) {
                // Erase returns the iterator following the removed element.
                it = fDescToRemoteStrike.erase(it);
                continue;
            }
        }
        ++it;
    }
}

sk_sp<RemoteStrike> SkStrikeServerImpl::getOrCreateCache(const SkStrikeSpec& strikeSpec) {
    // In cases where tracing is turned off, make sure not to get an unused function warning.
    // Lambdaize the function.
    TRACE_EVENT1("skia", "RecForDesc", "rec",
                 TRACE_STR_COPY(
                         [&strikeSpec](){
                             auto ptr =
                                 strikeSpec.descriptor().findEntry(kRec_SkDescriptorTag, nullptr);
                             SkScalerContextRec rec;
                             std::memcpy((void*)&rec, ptr, sizeof(rec));
                             return rec.dump();
                         }().c_str()
                 )
    );

    if (auto it = fDescToRemoteStrike.find(&strikeSpec.descriptor());
        it != fDescToRemoteStrike.end())
    {
        // We have processed the RemoteStrike before. Reuse it.
        sk_sp<RemoteStrike> strike = it->second;
        strike->setStrikeSpec(strikeSpec);
        if (fRemoteStrikesToSend.contains(strike.get())) {
            // Already tracking
            return strike;
        }

        // Strike is in unknown state on GPU. Start tracking strike on GPU by locking it.
        bool locked = fDiscardableHandleManager->lockHandle(it->second->discardableHandleId());
        if (locked) {
            fRemoteStrikesToSend.add(strike.get());
            return strike;
        }

        // If it wasn't locked, then forget this strike, and build it anew below.
        fDescToRemoteStrike.erase(it);
    }

    const SkTypeface& typeface = strikeSpec.typeface();
    // Create a new RemoteStrike. Start by processing the typeface.
    const SkTypefaceID typefaceId = typeface.uniqueID();
    if (!fCachedTypefaces.contains(typefaceId)) {
        fCachedTypefaces.add(typefaceId);
        fTypefacesToSend.emplace_back(typeface);
    }

    auto context = strikeSpec.createScalerContext();
    auto newHandle = fDiscardableHandleManager->createHandle();  // Locked on creation
    auto remoteStrike = sk_make_sp<RemoteStrike>(strikeSpec, std::move(context), newHandle);
    remoteStrike->setStrikeSpec(strikeSpec);
    fRemoteStrikesToSend.add(remoteStrike.get());
    auto d = &remoteStrike->getDescriptor();
    fDescToRemoteStrike[d] = remoteStrike;

    checkForDeletedEntries();

    return remoteStrike;
}

// -- GlyphTrackingDevice --------------------------------------------------------------------------
class GlyphTrackingDevice final : public SkNoPixelsDevice {
public:
    GlyphTrackingDevice(
            const SkISize& dimensions, const SkSurfaceProps& props, SkStrikeServerImpl* server,
            sk_sp<SkColorSpace> colorSpace, sktext::gpu::SDFTControl SDFTControl)
            : SkNoPixelsDevice(SkIRect::MakeSize(dimensions), props, std::move(colorSpace))
            , fStrikeServerImpl(server)
            , fSDFTControl(SDFTControl) {
        SkASSERT(fStrikeServerImpl != nullptr);
    }

    sk_sp<SkDevice> createDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return sk_make_sp<GlyphTrackingDevice>(cinfo.fInfo.dimensions(),
                                               surfaceProps,
                                               fStrikeServerImpl,
                                               cinfo.fInfo.refColorSpace(),
                                               fSDFTControl);
    }

    SkStrikeDeviceInfo strikeDeviceInfo() const override {
        return {this->surfaceProps(), this->scalerContextFlags(), &fSDFTControl};
    }

protected:
    void onDrawGlyphRunList(SkCanvas*,
                            const sktext::GlyphRunList& glyphRunList,
                            const SkPaint& initialPaint,
                            const SkPaint& drawingPaint) override {
        SkMatrix drawMatrix = this->localToDevice();
        drawMatrix.preTranslate(glyphRunList.origin().x(), glyphRunList.origin().y());

        // Just ignore the resulting SubRunContainer. Since we're passing in a null SubRunAllocator
        // no SubRuns will be produced.
        STSubRunAllocator<sizeof(SubRunContainer), alignof(SubRunContainer)> tempAlloc;
        auto container = SubRunContainer::MakeInAlloc(glyphRunList,
                                                      drawMatrix,
                                                      drawingPaint,
                                                      this->strikeDeviceInfo(),
                                                      fStrikeServerImpl,
                                                      &tempAlloc,
                                                      SubRunContainer::kStrikeCalculationsOnly,
                                                      "Cache Diff");
        // Calculations only. No SubRuns.
        SkASSERT(container->isEmpty());
    }

    sk_sp<sktext::gpu::Slug> convertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                                       const SkPaint& initialPaint,
                                                       const SkPaint& drawingPaint) override {
        // Full matrix for placing glyphs.
        SkMatrix positionMatrix = this->localToDevice();
        positionMatrix.preTranslate(glyphRunList.origin().x(), glyphRunList.origin().y());

        // Use the SkStrikeServer's strike cache to generate the Slug.
        return sktext::gpu::MakeSlug(this->localToDevice(),
                                     glyphRunList,
                                     initialPaint,
                                     drawingPaint,
                                     this->strikeDeviceInfo(),
                                     fStrikeServerImpl);
    }

private:
    SkStrikeServerImpl* const fStrikeServerImpl;
    const sktext::gpu::SDFTControl fSDFTControl;
};

// -- SkStrikeServer -------------------------------------------------------------------------------
SkStrikeServer::SkStrikeServer(DiscardableHandleManager* dhm)
        : fImpl(new SkStrikeServerImpl{dhm}) { }

SkStrikeServer::~SkStrikeServer() = default;

std::unique_ptr<SkCanvas> SkStrikeServer::makeAnalysisCanvas(int width, int height,
                                                             const SkSurfaceProps& props,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             bool DFTSupport,
                                                             bool DFTPerspSupport) {
#if !defined(SK_DISABLE_SDF_TEXT)
    // These are copied from the defaults in GrContextOptions for historical reasons.
    // TODO(herb, jvanverth) pipe in parameters that can be used for both Ganesh and Graphite
    // backends instead of just using the defaults.
    constexpr float kMinDistanceFieldFontSize = 18.f;

#if defined(SK_BUILD_FOR_ANDROID)
    constexpr float kGlyphsAsPathsFontSize = 384.f;
#elif defined(SK_BUILD_FOR_MAC)
    constexpr float kGlyphsAsPathsFontSize = 256.f;
#else
    constexpr float kGlyphsAsPathsFontSize = 324.f;
#endif
    auto control = sktext::gpu::SDFTControl{DFTSupport,
                                            props.isUseDeviceIndependentFonts(),
                                            DFTPerspSupport,
                                            kMinDistanceFieldFontSize,
                                            kGlyphsAsPathsFontSize};
#else
    auto control = sktext::gpu::SDFTControl{};
#endif

    sk_sp<SkDevice> trackingDevice = sk_make_sp<GlyphTrackingDevice>(
            SkISize::Make(width, height),
            props, this->impl(),
            std::move(colorSpace),
            control);
    return std::make_unique<SkCanvas>(std::move(trackingDevice));
}

void SkStrikeServer::writeStrikeData(std::vector<uint8_t>* memory) {
    fImpl->writeStrikeData(memory);
}

SkStrikeServerImpl* SkStrikeServer::impl() { return fImpl.get(); }

void SkStrikeServer::setMaxEntriesInDescriptorMapForTesting(size_t count) {
    fImpl->setMaxEntriesInDescriptorMapForTesting(count);
}
size_t SkStrikeServer::remoteStrikeMapSizeForTesting() const {
    return fImpl->remoteStrikeMapSizeForTesting();
}

// -- DiscardableStrikePinner ----------------------------------------------------------------------
class DiscardableStrikePinner : public SkStrikePinner {
public:
    DiscardableStrikePinner(SkDiscardableHandleId discardableHandleId,
                            sk_sp<SkStrikeClient::DiscardableHandleManager> manager)
            : fDiscardableHandleId(discardableHandleId), fManager(std::move(manager)) {}

    ~DiscardableStrikePinner() override = default;
    bool canDelete() override { return fManager->deleteHandle(fDiscardableHandleId); }
    void assertValid() override { fManager->assertHandleValid(fDiscardableHandleId); }

private:
    const SkDiscardableHandleId fDiscardableHandleId;
    sk_sp<SkStrikeClient::DiscardableHandleManager> fManager;
};

// -- SkStrikeClientImpl ---------------------------------------------------------------------------
class SkStrikeClientImpl {
public:
    explicit SkStrikeClientImpl(sk_sp<SkStrikeClient::DiscardableHandleManager>,
                                bool isLogging = true,
                                SkStrikeCache* strikeCache = nullptr);

    bool readStrikeData(const volatile void* memory, size_t memorySize);
    bool translateTypefaceID(SkAutoDescriptor* descriptor) const;
    sk_sp<SkTypeface> retrieveTypefaceUsingServerID(SkTypefaceID) const;

private:
    class PictureBackedGlyphDrawable final : public SkDrawable {
    public:
        PictureBackedGlyphDrawable(sk_sp<SkPicture> self) : fSelf(std::move(self)) {}
    private:
        sk_sp<SkPicture> fSelf;
        SkRect onGetBounds() override { return fSelf->cullRect();  }
        size_t onApproximateBytesUsed() override {
            return sizeof(PictureBackedGlyphDrawable) + fSelf->approximateBytesUsed();
        }
        void onDraw(SkCanvas* canvas) override { canvas->drawPicture(fSelf); }
    };

    sk_sp<SkTypeface> addTypeface(const SkTypefaceProxyPrototype& typefaceProto);

    THashMap<SkTypefaceID, sk_sp<SkTypeface>> fServerTypefaceIdToTypeface;
    sk_sp<SkStrikeClient::DiscardableHandleManager> fDiscardableHandleManager;
    SkStrikeCache* const fStrikeCache;
    const bool fIsLogging;
};

SkStrikeClientImpl::SkStrikeClientImpl(
        sk_sp<SkStrikeClient::DiscardableHandleManager>
        discardableManager,
        bool isLogging,
        SkStrikeCache* strikeCache)
    : fDiscardableHandleManager(std::move(discardableManager)),
      fStrikeCache{strikeCache ? strikeCache : SkStrikeCache::GlobalStrikeCache()},
      fIsLogging{isLogging} {}

// Change the path count to track the line number of the failing read.
// TODO: change __LINE__ back to glyphPathsCount when bug chromium:1287356 is closed.
#define READ_FAILURE                                                        \
    {                                                                       \
        SkDebugf("Bad font data serialization line: %d", __LINE__);         \
        SkStrikeClient::DiscardableHandleManager::ReadFailureData data = {  \
                memorySize,  deserializer.bytesRead(), typefaceSize,        \
                strikeCount, glyphImagesCount, __LINE__};                   \
        fDiscardableHandleManager->notifyReadFailure(data);                 \
        return false;                                                       \
    }

bool SkStrikeClientImpl::readStrikeData(const volatile void* memory, size_t memorySize) {
    SkASSERT(memorySize != 0);
    SkASSERT(memory != nullptr);

    SkReadBuffer buffer{const_cast<const void*>(memory), memorySize};
    // Limit the kinds of effects that appear in a glyph's drawable (crbug.com/1442140):
    buffer.setAllowSkSL(false);

    int curTypeface = 0,
        curStrike = 0;

    auto postError = [&](int line) {
        SkDebugf("Read Error Posted %s : %d", __FILE__, line);
        SkStrikeClient::DiscardableHandleManager::ReadFailureData data{
                memorySize,
                buffer.offset(),
                SkTo<uint64_t>(curTypeface),
                SkTo<uint64_t>(curStrike),
                SkTo<uint64_t>(0),
                SkTo<uint64_t>(0)};
        fDiscardableHandleManager->notifyReadFailure(data);
    };

    // Read the number of typefaces sent.
    const int typefaceCount = buffer.readInt();
    for (curTypeface = 0; curTypeface < typefaceCount; ++curTypeface) {
        auto proto = SkTypefaceProxyPrototype::MakeFromBuffer(buffer);
        if (proto) {
            this->addTypeface(proto.value());
        } else {
            postError(__LINE__);
            return false;
        }
    }

    // Read the number of strikes sent.
    const int stirkeCount = buffer.readInt();
    for (curStrike = 0; curStrike < stirkeCount; ++curStrike) {

        const SkTypefaceID serverTypefaceID = buffer.readUInt();
        if (serverTypefaceID == 0 && !buffer.isValid()) {
            postError(__LINE__);
            return false;
        }

        const SkDiscardableHandleId discardableHandleID = buffer.readUInt();
        if (discardableHandleID == 0 && !buffer.isValid()) {
            postError(__LINE__);
            return false;
        }

        std::optional<SkAutoDescriptor> serverDescriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
        if (!buffer.validate(serverDescriptor.has_value())) {
            postError(__LINE__);
            return false;
        }

        const bool fontMetricsInitialized = buffer.readBool();
        if (!fontMetricsInitialized && !buffer.isValid()) {
            postError(__LINE__);
            return false;
        }

        std::optional<SkFontMetrics> fontMetrics;
        if (!fontMetricsInitialized) {
            fontMetrics = SkFontMetricsPriv::MakeFromBuffer(buffer);
            if (!fontMetrics || !buffer.isValid()) {
                postError(__LINE__);
                return false;
            }
        }

        auto* clientTypeface = fServerTypefaceIdToTypeface.find(serverTypefaceID);
        if (clientTypeface == nullptr) {
            postError(__LINE__);
            return false;
        }

        if (!this->translateTypefaceID(&serverDescriptor.value())) {
            postError(__LINE__);
            return false;
        }

        SkDescriptor* clientDescriptor = serverDescriptor->getDesc();
        auto strike = fStrikeCache->findStrike(*clientDescriptor);

        if (strike == nullptr) {
            // Metrics are only sent the first time. If creating a new strike, then the metrics
            // are not initialized.
            if (fontMetricsInitialized) {
                postError(__LINE__);
                return false;
            }
            SkStrikeSpec strikeSpec{*clientDescriptor, *clientTypeface};
            strike = fStrikeCache->createStrike(
                    strikeSpec, &fontMetrics.value(),
                    std::make_unique<DiscardableStrikePinner>(
                            discardableHandleID, fDiscardableHandleManager));
        }

        // Make sure this strike is pinned on the GPU side.
        strike->verifyPinnedStrike();

        if (!strike->mergeFromBuffer(buffer)) {
            postError(__LINE__);
            return false;
        }
    }

    return true;
}

bool SkStrikeClientImpl::translateTypefaceID(SkAutoDescriptor* toChange) const {
    SkDescriptor& descriptor = *toChange->getDesc();

    // Rewrite the typefaceID in the rec.
    {
        uint32_t size;
        // findEntry returns a const void*, remove the const in order to update in place.
        void* ptr = const_cast<void *>(descriptor.findEntry(kRec_SkDescriptorTag, &size));
        SkScalerContextRec rec;
        if (!ptr || size != sizeof(rec)) { return false; }
        std::memcpy((void*)&rec, ptr, size);
        // Get the local typeface from remote typefaceID.
        auto* tfPtr = fServerTypefaceIdToTypeface.find(rec.fTypefaceID);
        // Received a strike for a typeface which doesn't exist.
        if (!tfPtr) { return false; }
        // Update the typeface id to work with the client side.
        rec.fTypefaceID = tfPtr->get()->uniqueID();
        std::memcpy(ptr, &rec, size);
    }

    descriptor.computeChecksum();

    return true;
}

sk_sp<SkTypeface> SkStrikeClientImpl::retrieveTypefaceUsingServerID(SkTypefaceID typefaceID) const {
    auto* tfPtr = fServerTypefaceIdToTypeface.find(typefaceID);
    return tfPtr != nullptr ? *tfPtr : nullptr;
}

sk_sp<SkTypeface> SkStrikeClientImpl::addTypeface(const SkTypefaceProxyPrototype& typefaceProto) {
    sk_sp<SkTypeface>* typeface =
            fServerTypefaceIdToTypeface.find(typefaceProto.serverTypefaceID());

    // We already have the typeface.
    if (typeface != nullptr)  {
        return *typeface;
    }

    auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            typefaceProto, fDiscardableHandleManager, fIsLogging);
    fServerTypefaceIdToTypeface.set(typefaceProto.serverTypefaceID(), newTypeface);
    return newTypeface;
}

// SkStrikeClient ----------------------------------------------------------------------------------
SkStrikeClient::SkStrikeClient(sk_sp<DiscardableHandleManager> discardableManager,
                               bool isLogging,
                               SkStrikeCache* strikeCache)
       : fImpl{new SkStrikeClientImpl{std::move(discardableManager), isLogging, strikeCache}} {}

SkStrikeClient::~SkStrikeClient() = default;

bool SkStrikeClient::readStrikeData(const volatile void* memory, size_t memorySize) {
    return fImpl->readStrikeData(memory, memorySize);
}

sk_sp<SkTypeface> SkStrikeClient::retrieveTypefaceUsingServerIDForTest(
        SkTypefaceID typefaceID) const {
    return fImpl->retrieveTypefaceUsingServerID(typefaceID);
}

bool SkStrikeClient::translateTypefaceID(SkAutoDescriptor* descriptor) const {
    return fImpl->translateTypefaceID(descriptor);
}

sk_sp<sktext::gpu::Slug> SkStrikeClient::deserializeSlugForTest(const void* data, size_t size) const {
    return sktext::gpu::Slug::Deserialize(data, size, this);
}

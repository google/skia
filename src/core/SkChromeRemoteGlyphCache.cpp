/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkChecksum.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkDraw.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTypeface_remote.h"
#include "src/text/GlyphRun.h"
#include "src/text/StrikeForGPU.h"

#include <algorithm>
#include <bitset>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <tuple>
#include <unordered_map>

#if SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlob.h"
#endif

using namespace sktext;
using namespace sktext::gpu;
using namespace skglyph;

namespace {
// -- Serializer -----------------------------------------------------------------------------------
size_t pad(size_t size, size_t alignment) { return (size + (alignment - 1)) & ~(alignment - 1); }

// Alignment between x86 and x64 differs for some types, in particular
// int64_t and doubles have 4 and 8-byte alignment, respectively.
// Be consistent even when writing and reading across different architectures.
template<typename T>
size_t serialization_alignment() {
  return sizeof(T) == 8 ? 8 : alignof(T);
}

class Serializer {
public:
    explicit Serializer(std::vector<uint8_t>* buffer) : fBuffer{buffer} {}

    template <typename T, typename... Args>
    T* emplace(Args&&... args) {
        auto result = this->allocate(sizeof(T), serialization_alignment<T>());
        return new (result) T{std::forward<Args>(args)...};
    }

    template <typename T>
    void write(const T& data) {
        T* result = (T*)this->allocate(sizeof(T), serialization_alignment<T>());
        memcpy(result, &data, sizeof(T));
    }

    void writeDescriptor(const SkDescriptor& desc) {
        write(desc.getLength());
        auto result = this->allocate(desc.getLength(), alignof(SkDescriptor));
        memcpy(result, &desc, desc.getLength());
    }

    void* allocate(size_t size, size_t alignment) {
        size_t aligned = pad(fBuffer->size(), alignment);
        fBuffer->resize(aligned + size);
        return &(*fBuffer)[aligned];
    }

private:
    std::vector<uint8_t>* fBuffer;
};

// -- Deserializer -------------------------------------------------------------------------------
// Note that the Deserializer is reading untrusted data, we need to guard against invalid data.
class Deserializer {
public:
    Deserializer(const volatile char* memory, size_t memorySize)
            : fMemory(memory), fMemorySize(memorySize) {}

    template <typename T>
    bool read(T* val) {
        auto* result = this->ensureAtLeast(sizeof(T), serialization_alignment<T>());
        if (!result) return false;

        memcpy(val, const_cast<const char*>(result), sizeof(T));
        return true;
    }

    bool readDescriptor(SkAutoDescriptor* ad) {
        uint32_t descLength = 0u;
        if (!this->read<uint32_t>(&descLength)) return false;

        auto* underlyingBuffer = this->ensureAtLeast(descLength, alignof(SkDescriptor));
        if (!underlyingBuffer) return false;
        SkReadBuffer buffer((void*)underlyingBuffer, descLength);
        auto autoDescriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
        if (!autoDescriptor.has_value()) { return false; }

        *ad = std::move(*autoDescriptor);
        return true;
    }

    const volatile void* read(size_t size, size_t alignment) {
      return this->ensureAtLeast(size, alignment);
    }

    size_t bytesRead() const { return fBytesRead; }

private:
    const volatile char* ensureAtLeast(size_t size, size_t alignment) {
        size_t padded = pad(fBytesRead, alignment);

        // Not enough data.
        if (padded > fMemorySize) return nullptr;
        if (size > fMemorySize - padded) return nullptr;

        auto* result = fMemory + padded;
        fBytesRead = padded + size;
        return result;
    }

    // Note that we read each piece of memory only once to guard against TOCTOU violations.
    const volatile char* fMemory;
    size_t fMemorySize;
    size_t fBytesRead = 0u;
};

// Paths use a SkWriter32 which requires 4 byte alignment.
static const size_t kPathAlignment = 4u;
static const size_t kDrawableAlignment = 8u;

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

    void writePendingGlyphs(Serializer* serializer);
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
    void writeGlyphPath(const SkGlyph& glyph, Serializer* serializer) const;
    void writeGlyphDrawable(const SkGlyph& glyph, Serializer* serializer) const;
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
    SkTHashMap<SkPackedGlyphID, SkGlyphDigest, SkPackedGlyphID::Hash> fSentGlyphs;

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

// No need to write fScalerContextBits because any needed image is already generated.
void write_glyph(const SkGlyph& glyph, Serializer* serializer) {
    serializer->write<SkPackedGlyphID>(glyph.getPackedID());
    serializer->write<float>(glyph.advanceX());
    serializer->write<float>(glyph.advanceY());
    serializer->write<uint16_t>(glyph.width());
    serializer->write<uint16_t>(glyph.height());
    serializer->write<int16_t>(glyph.top());
    serializer->write<int16_t>(glyph.left());
    serializer->write<uint8_t>(glyph.maskFormat());
}

void RemoteStrike::writePendingGlyphs(Serializer* serializer) {
    SkASSERT(this->hasPendingGlyphs());

    // Write the desc.
    serializer->emplace<StrikeSpec>(fContext->getTypeface()->uniqueID(), fDiscardableHandleId);
    serializer->writeDescriptor(*fDescriptor.getDesc());

    serializer->emplace<bool>(fHaveSentFontMetrics);
    if (!fHaveSentFontMetrics) {
        // Write FontMetrics if not sent before.
        SkFontMetrics fontMetrics;
        fContext->getFontMetrics(&fontMetrics);
        serializer->write<SkFontMetrics>(fontMetrics);
        fHaveSentFontMetrics = true;
    }

    // Write mask glyphs
    serializer->emplace<uint64_t>(fMasksToSend.size());
    for (SkGlyph& glyph : fMasksToSend) {
        SkASSERT(SkMask::IsValidFormat(glyph.maskFormat()));

        write_glyph(glyph, serializer);
        auto imageSize = glyph.imageSize();
        if (imageSize > 0 && SkGlyphDigest::FitsInAtlas(glyph)) {
            glyph.setImage(serializer->allocate(imageSize, glyph.formatAlignment()));
            fContext->getImage(glyph);
        }
    }
    fMasksToSend.clear();

    // Write glyphs paths.
    serializer->emplace<uint64_t>(fPathsToSend.size());
    for (SkGlyph& glyph : fPathsToSend) {
        SkASSERT(SkMask::IsValidFormat(glyph.maskFormat()));

        write_glyph(glyph, serializer);
        this->writeGlyphPath(glyph, serializer);
    }
    fPathsToSend.clear();

    // Write glyphs drawables.
    serializer->emplace<uint64_t>(fDrawablesToSend.size());
    for (SkGlyph& glyph : fDrawablesToSend) {
        SkASSERT(SkMask::IsValidFormat(glyph.maskFormat()));

        write_glyph(glyph, serializer);
        writeGlyphDrawable(glyph, serializer);
    }
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

void RemoteStrike::writeGlyphPath(const SkGlyph& glyph, Serializer* serializer) const {
    if (glyph.isEmpty()) {
        serializer->write<uint64_t>(0u);
        return;
    }

    const SkPath* path = glyph.path();

    if (path == nullptr) {
        serializer->write<uint64_t>(0u);
        return;
    }

    size_t pathSize = path->writeToMemory(nullptr);
    serializer->write<uint64_t>(pathSize);
    path->writeToMemory(serializer->allocate(pathSize, kPathAlignment));

    serializer->write<bool>(glyph.pathIsHairline());
}

void RemoteStrike::writeGlyphDrawable(const SkGlyph& glyph, Serializer* serializer) const {
    if (glyph.isEmpty()) {
        serializer->write<uint64_t>(0u);
        return;
    }

    SkDrawable* drawable = glyph.drawable();

    if (drawable == nullptr) {
        serializer->write<uint64_t>(0u);
        return;
    }

    sk_sp<SkPicture> picture(drawable->newPictureSnapshot());
    sk_sp<SkData> data = picture->serialize();
    serializer->write<uint64_t>(data->size());
    memcpy(serializer->allocate(data->size(), kDrawableAlignment), data->data(), data->size());
}

SkGlyphDigest RemoteStrike::digestFor(ActionType actionType, SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* digestPtr = fSentGlyphs.find(packedGlyphID);
    if (digestPtr != nullptr && digestPtr->actionFor(actionType) != GlyphAction::kUnset) {
        return *digestPtr;
    }

    SkGlyph* glyph;
    this->ensureScalerContext();
    switch (actionType) {
        case kPath: {
            fPathsToSend.emplace_back(fContext->makeGlyph(packedGlyphID, &fAlloc));
            glyph = &fPathsToSend.back();
            break;
        }
        case kDrawable: {
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
        digestPtr = fSentGlyphs.set(packedGlyphID, SkGlyphDigest{0, *glyph});
    }

    digestPtr->setActionFor(actionType, glyph, this);

    return *digestPtr;
}

sktext::SkStrikePromise RemoteStrike::strikePromise() {
    return sktext::SkStrikePromise{*this->fStrikeSpec};
}

// -- WireTypeface ---------------------------------------------------------------------------------
struct WireTypeface {
    WireTypeface() = default;
    WireTypeface(SkTypefaceID typefaceId, int glyphCount, SkFontStyle style,
                 bool isFixed, bool needsCurrentColor)
      : fTypefaceID(typefaceId), fGlyphCount(glyphCount), fStyle(style),
        fIsFixed(isFixed), fGlyphMaskNeedsCurrentColor(needsCurrentColor) {}

    SkTypefaceID    fTypefaceID{0};
    int             fGlyphCount{0};
    SkFontStyle     fStyle;
    bool            fIsFixed{false};
    // Used for COLRv0 or COLRv1 fonts that may need the 0xFFFF special palette
    // index to represent foreground color. This information needs to be on here
    // to determine how this typeface can be cached.
    bool            fGlyphMaskNeedsCurrentColor{false};
};
}  // namespace

// -- SkStrikeServerImpl ---------------------------------------------------------------------------
class SkStrikeServerImpl final : public sktext::StrikeForGPUCacheInterface {
public:
    explicit SkStrikeServerImpl(
            SkStrikeServer::DiscardableHandleManager* discardableHandleManager);

    // SkStrikeServer API methods
    sk_sp<SkData> serializeTypeface(SkTypeface*);
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
    SkTHashSet<SkTypefaceID> fCachedTypefaces;
    size_t fMaxEntriesInDescriptorMap = kMaxEntriesInDescriptorMap;

    // Cached serialized typefaces.
    SkTHashMap<SkTypefaceID, sk_sp<SkData>> fSerializedTypefaces;

    // State cached until the next serialization.
    SkTHashSet<RemoteStrike*> fRemoteStrikesToSend;
    std::vector<WireTypeface> fTypefacesToSend;
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

sk_sp<SkData> SkStrikeServerImpl::serializeTypeface(SkTypeface* tf) {
    auto* data = fSerializedTypefaces.find(SkTypeface::UniqueID(tf));
    if (data) {
        return *data;
    }

    WireTypeface wire(SkTypeface::UniqueID(tf), tf->countGlyphs(), tf->fontStyle(),
                      tf->isFixedPitch(), tf->glyphMaskNeedsCurrentColor());
    data = fSerializedTypefaces.set(SkTypeface::UniqueID(tf),
                                    SkData::MakeWithCopy(&wire, sizeof(wire)));
    return *data;
}

void SkStrikeServerImpl::writeStrikeData(std::vector<uint8_t>* memory) {
    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        SkString msg;
        msg.appendf("\nBegin send strike differences\n");
    #endif
    size_t strikesToSend = 0;
    fRemoteStrikesToSend.foreach ([&](RemoteStrike* strike) {
        if (strike->hasPendingGlyphs()) {
            strikesToSend++;
        } else {
            strike->resetScalerContext();
        }
    });

    if (strikesToSend == 0 && fTypefacesToSend.empty()) {
        fRemoteStrikesToSend.reset();
        return;
    }

    Serializer serializer(memory);
    serializer.emplace<uint64_t>(fTypefacesToSend.size());
    for (const auto& tf : fTypefacesToSend) {
        serializer.write<WireTypeface>(tf);
    }
    fTypefacesToSend.clear();

    serializer.emplace<uint64_t>(SkTo<uint64_t>(strikesToSend));
    fRemoteStrikesToSend.foreach (
        [&](RemoteStrike* strike) {
            if (strike->hasPendingGlyphs()) {
                strike->writePendingGlyphs(&serializer);
                strike->resetScalerContext();
            }
            #ifdef SK_DEBUG
                auto it = fDescToRemoteStrike.find(&strike->getDescriptor());
                SkASSERT(it != fDescToRemoteStrike.end());
                SkASSERT(it->second.get() == strike);
            #endif
            #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
                msg.append(strike->getDescriptor().dumpRec());
            #endif
        }
    );
    fRemoteStrikesToSend.reset();
    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        msg.appendf("End send strike differences");
        SkDebugf("%s\n", msg.c_str());
    #endif
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
        fTypefacesToSend.emplace_back(typefaceId, typeface.countGlyphs(),
                                      typeface.fontStyle(),
                                      typeface.isFixedPitch(),
                                      typeface.glyphMaskNeedsCurrentColor());
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
#if SK_SUPPORT_GPU
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

    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new GlyphTrackingDevice(cinfo.fInfo.dimensions(), surfaceProps, fStrikeServerImpl,
                                       cinfo.fInfo.refColorSpace(), fSDFTControl);
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

#ifdef SK_SUPPORT_LEGACY_SLUG_CONVERT
        // TODO these two passes can be converted into one when the SkRemoteGlyphCache's strike
        //  cache is fortified with enough information for supporting slug creation.

        // Use the lightweight strike cache provided by SkRemoteGlyphCache through fPainter to do
        // the analysis. Just ignore the resulting SubRunContainer. Since we're passing in a null
        // SubRunAllocator no SubRuns will be produced.
        STSubRunAllocator<sizeof(SubRunContainer), alignof(SubRunContainer)> tempAlloc;
        auto container = SubRunContainer::MakeInAlloc(glyphRunList,
                                                      positionMatrix,
                                                      drawingPaint,
                                                      this->strikeDeviceInfo(),
                                                      fStrikeServerImpl,
                                                      &tempAlloc,
                                                      SubRunContainer::kStrikeCalculationsOnly,
                                                      "Convert Slug Analysis");
        // Calculations only. No SubRuns.
        SkASSERT(container->isEmpty());

        // Use the glyph strike cache to get actual glyph information.
        return skgpu::v1::MakeSlug(this->localToDevice(),
                                   glyphRunList,
                                   initialPaint,
                                   drawingPaint,
                                   this->strikeDeviceInfo(),
                                   SkStrikeCache::GlobalStrikeCache());
#else
        // Use the SkStrikeServer's strike cache to generate the Slug.
        return skgpu::v1::MakeSlug(this->localToDevice(),
                                   glyphRunList,
                                   initialPaint,
                                   drawingPaint,
                                   this->strikeDeviceInfo(),
                                   fStrikeServerImpl);
#endif
    }

private:
    SkStrikeServerImpl* const fStrikeServerImpl;
    const sktext::gpu::SDFTControl fSDFTControl;
};
#endif  // SK_SUPPORT_GPU

// -- SkStrikeServer -------------------------------------------------------------------------------
SkStrikeServer::SkStrikeServer(DiscardableHandleManager* dhm)
        : fImpl(new SkStrikeServerImpl{dhm}) { }

SkStrikeServer::~SkStrikeServer() = default;

std::unique_ptr<SkCanvas> SkStrikeServer::makeAnalysisCanvas(int width, int height,
                                                             const SkSurfaceProps& props,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             bool DFTSupport,
                                                             bool DFTPerspSupport) {
#if SK_SUPPORT_GPU
    GrContextOptions ctxOptions;
#if !defined(SK_DISABLE_SDF_TEXT)
    auto control = sktext::gpu::SDFTControl{DFTSupport,
                                            props.isUseDeviceIndependentFonts(),
                                            DFTPerspSupport,
                                            ctxOptions.fMinDistanceFieldFontSize,
                                            ctxOptions.fGlyphsAsPathsFontSize};
#else
    auto control = sktext::gpu::SDFTControl{};
#endif

    sk_sp<SkBaseDevice> trackingDevice(new GlyphTrackingDevice(
            SkISize::Make(width, height),
            props, this->impl(),
            std::move(colorSpace),
            control));
#else
    sk_sp<SkBaseDevice> trackingDevice(new SkNoPixelsDevice(
            SkIRect::MakeWH(width, height), props, std::move(colorSpace)));
#endif
    return std::make_unique<SkCanvas>(std::move(trackingDevice));
}

sk_sp<SkData> SkStrikeServer::serializeTypeface(SkTypeface* tf) {
    return fImpl->serializeTypeface(tf);
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

    sk_sp<SkTypeface> deserializeTypeface(const void* data, size_t length);

    bool readStrikeData(const volatile void* memory, size_t memorySize);
    bool translateTypefaceID(SkAutoDescriptor* descriptor) const;

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

    static bool ReadGlyph(SkTLazy<SkGlyph>& glyph, Deserializer* deserializer);
    sk_sp<SkTypeface> addTypeface(const WireTypeface& wire);

    SkTHashMap<SkTypefaceID, sk_sp<SkTypeface>> fRemoteTypefaceIdToTypeface;
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

// No need to write fScalerContextBits because any needed image is already generated.
bool SkStrikeClientImpl::ReadGlyph(SkTLazy<SkGlyph>& glyph, Deserializer* deserializer) {
    SkPackedGlyphID glyphID;
    if (!deserializer->read<SkPackedGlyphID>(&glyphID)) return false;
    glyph.init(glyphID);
    if (!deserializer->read<float>(&glyph->fAdvanceX)) return false;
    if (!deserializer->read<float>(&glyph->fAdvanceY)) return false;
    if (!deserializer->read<uint16_t>(&glyph->fWidth)) return false;
    if (!deserializer->read<uint16_t>(&glyph->fHeight)) return false;
    if (!deserializer->read<int16_t>(&glyph->fTop)) return false;
    if (!deserializer->read<int16_t>(&glyph->fLeft)) return false;
    uint8_t maskFormat;
    if (!deserializer->read<uint8_t>(&maskFormat)) return false;
    if (!SkMask::IsValidFormat(maskFormat)) return false;
    glyph->fMaskFormat = static_cast<SkMask::Format>(maskFormat);
    SkDEBUGCODE(glyph->fAdvancesBoundsFormatAndInitialPathDone = true;)

    return true;
}
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
    SkASSERT(memorySize != 0u);
    Deserializer deserializer(static_cast<const volatile char*>(memory), memorySize);

    uint64_t typefaceSize = 0;
    uint64_t strikeCount = 0;
    uint64_t glyphImagesCount = 0;
    uint64_t glyphPathsCount = 0;
    uint64_t glyphDrawablesCount = 0;

    if (!deserializer.read<uint64_t>(&typefaceSize)) READ_FAILURE
    for (size_t i = 0; i < typefaceSize; ++i) {
        WireTypeface wire;
        if (!deserializer.read<WireTypeface>(&wire)) READ_FAILURE

        this->addTypeface(wire);
    }

    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        SkString msg;
        msg.appendf("\nBegin receive strike differences\n");
    #endif

    if (!deserializer.read<uint64_t>(&strikeCount)) READ_FAILURE

    for (size_t i = 0; i < strikeCount; ++i) {
        StrikeSpec spec;
        if (!deserializer.read<StrikeSpec>(&spec)) READ_FAILURE

        SkAutoDescriptor ad;
        if (!deserializer.readDescriptor(&ad)) READ_FAILURE
        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  Received descriptor:\n%s", ad.getDesc()->dumpRec().c_str());
        #endif

        bool fontMetricsInitialized;
        if (!deserializer.read(&fontMetricsInitialized)) READ_FAILURE

        SkFontMetrics fontMetrics{};
        if (!fontMetricsInitialized) {
            if (!deserializer.read<SkFontMetrics>(&fontMetrics)) READ_FAILURE
        }

        // Preflight the TypefaceID before doing the Descriptor translation.
        auto* tfPtr = fRemoteTypefaceIdToTypeface.find(spec.fTypefaceID);
        // Received a TypefaceID for a typeface we don't know about.
        if (!tfPtr) READ_FAILURE

        // Replace the ContextRec in the desc from the server to create the client
        // side descriptor.
        if (!this->translateTypefaceID(&ad)) READ_FAILURE
        SkDescriptor* clientDesc = ad.getDesc();

        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  Mapped descriptor:\n%s", clientDesc->dumpRec().c_str());
        #endif
        auto strike = fStrikeCache->findStrike(*clientDesc);

        // Make sure strike is pinned
        if (strike) {
            strike->verifyPinnedStrike();
        }

        // Metrics are only sent the first time. If the metrics are not initialized, there must
        // be an existing strike.
        if (fontMetricsInitialized && strike == nullptr) READ_FAILURE
        if (strike == nullptr) {
            // Note that we don't need to deserialize the effects since we won't be generating any
            // glyphs here anyway, and the desc is still correct since it includes the serialized
            // effects.
            SkStrikeSpec strikeSpec{*clientDesc, *tfPtr};
            strike = fStrikeCache->createStrike(
                    strikeSpec, &fontMetrics,
                    std::make_unique<DiscardableStrikePinner>(
                            spec.fDiscardableHandleId, fDiscardableHandleManager));
        }

        if (!deserializer.read<uint64_t>(&glyphImagesCount)) READ_FAILURE
        for (size_t j = 0; j < glyphImagesCount; j++) {
            SkTLazy<SkGlyph> glyph;
            if (!ReadGlyph(glyph, &deserializer)) READ_FAILURE

            if (!glyph->isEmpty() && SkGlyphDigest::FitsInAtlas(*glyph)) {
                const volatile void* image =
                        deserializer.read(glyph->imageSize(), glyph->formatAlignment());
                if (!image) READ_FAILURE
                glyph->fImage = (void*)image;
            }

            strike->mergeGlyphAndImage(glyph->getPackedID(), *glyph);
        }

        if (!deserializer.read<uint64_t>(&glyphPathsCount)) READ_FAILURE
        for (size_t j = 0; j < glyphPathsCount; j++) {
            SkTLazy<SkGlyph> glyph;
            if (!ReadGlyph(glyph, &deserializer)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->mergeGlyphAndImage(glyph->getPackedID(), *glyph);

            SkPath* pathPtr = nullptr;
            SkPath path;
            uint64_t pathSize = 0u;
            bool hairline = false;
            if (!deserializer.read<uint64_t>(&pathSize)) READ_FAILURE

            if (pathSize > 0) {
                auto* pathData = deserializer.read(pathSize, kPathAlignment);
                if (!pathData) READ_FAILURE
                if (!path.readFromMemory(const_cast<const void*>(pathData), pathSize)) READ_FAILURE
                pathPtr = &path;
                if (!deserializer.read<bool>(&hairline)) READ_FAILURE
            }

            strike->mergePath(allocatedGlyph, pathPtr, hairline);
        }

        if (!deserializer.read<uint64_t>(&glyphDrawablesCount)) READ_FAILURE
        for (size_t j = 0; j < glyphDrawablesCount; j++) {
            SkTLazy<SkGlyph> glyph;
            if (!ReadGlyph(glyph, &deserializer)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->mergeGlyphAndImage(glyph->getPackedID(), *glyph);

            sk_sp<SkDrawable> drawable;
            uint64_t drawableSize = 0u;
            if (!deserializer.read<uint64_t>(&drawableSize)) READ_FAILURE

            if (drawableSize > 0) {
                auto* drawableData = deserializer.read(drawableSize, kDrawableAlignment);
                if (!drawableData) READ_FAILURE
                sk_sp<SkPicture> picture(SkPicture::MakeFromData(
                        const_cast<const void*>(drawableData), drawableSize));
                if (!picture) READ_FAILURE

                drawable = sk_make_sp<PictureBackedGlyphDrawable>(std::move(picture));
            }

            strike->mergeDrawable(allocatedGlyph, std::move(drawable));
        }
    }

#if defined(SK_TRACE_GLYPH_RUN_PROCESS)
    msg.appendf("End receive strike differences");
    SkDebugf("%s\n", msg.c_str());
#endif

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
        std::memcpy((void*)&rec, ptr, size);
        // Get the local typeface from remote typefaceID.
        auto* tfPtr = fRemoteTypefaceIdToTypeface.find(rec.fTypefaceID);
        // Received a strike for a typeface which doesn't exist.
        if (!tfPtr) { return false; }
        // Update the typeface id to work with the client side.
        rec.fTypefaceID = tfPtr->get()->uniqueID();
        std::memcpy(ptr, &rec, size);
    }

    descriptor.computeChecksum();

    return true;
}

sk_sp<SkTypeface> SkStrikeClientImpl::deserializeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len != sizeof(wire)) return nullptr;
    memcpy(&wire, buf, sizeof(wire));
    return this->addTypeface(wire);
}

sk_sp<SkTypeface> SkStrikeClientImpl::addTypeface(const WireTypeface& wire) {
    auto* typeface = fRemoteTypefaceIdToTypeface.find(wire.fTypefaceID);
    if (typeface) return *typeface;

    auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.fTypefaceID, wire.fGlyphCount, wire.fStyle, wire.fIsFixed,
            wire.fGlyphMaskNeedsCurrentColor, fDiscardableHandleManager, fIsLogging);
    fRemoteTypefaceIdToTypeface.set(wire.fTypefaceID, newTypeface);
    return std::move(newTypeface);
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

sk_sp<SkTypeface> SkStrikeClient::deserializeTypeface(const void* buf, size_t len) {
    return fImpl->deserializeTypeface(buf, len);
}

bool SkStrikeClient::translateTypefaceID(SkAutoDescriptor* descriptor) const {
    return fImpl->translateTypefaceID(descriptor);
}

#if SK_SUPPORT_GPU
sk_sp<sktext::gpu::Slug> SkStrikeClient::deserializeSlug(const void* data, size_t size) const {
    return sktext::gpu::Slug::Deserialize(data, size, this);
}
#endif  // SK_SUPPORT_GPU

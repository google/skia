/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"

#include <bitset>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <tuple>

#include "include/core/SkSerialProcs.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkTHash.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDraw.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeForGPU.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTypeface_remote.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/text/GrSDFTControl.h"
#endif

// This essentially replaces the font_id used on the RendererSide with the font_id on the GPU side.
static SkDescriptor* auto_descriptor_from_desc(const SkDescriptor* source_desc,
                                               SkFontID font_id,
                                               SkAutoDescriptor* ad) {
    ad->reset(source_desc->getLength());
    auto* desc = ad->getDesc();

    // Rec.
    {
        uint32_t size;
        auto ptr = source_desc->findEntry(kRec_SkDescriptorTag, &size);
        SkScalerContextRec rec;
        std::memcpy((void*)&rec, ptr, size);
        rec.fFontID = font_id;
        desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    }

    // Effects.
    {
        uint32_t size;
        auto ptr = source_desc->findEntry(kEffects_SkDescriptorTag, &size);
        if (ptr) { desc->addEntry(kEffects_SkDescriptorTag, size, ptr); }
    }

    desc->computeChecksum();
    return desc;
}

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
        auto result = allocate(sizeof(T), serialization_alignment<T>());
        return new (result) T{std::forward<Args>(args)...};
    }

    template <typename T>
    void write(const T& data) {
        T* result = (T*)allocate(sizeof(T), serialization_alignment<T>());
        memcpy(result, &data, sizeof(T));
    }

    template <typename T>
    T* allocate() {
        T* result = (T*)allocate(sizeof(T), serialization_alignment<T>());
        return result;
    }

    void writeDescriptor(const SkDescriptor& desc) {
        write(desc.getLength());
        auto result = allocate(desc.getLength(), alignof(SkDescriptor));
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
        if (!read<uint32_t>(&descLength)) return false;
        if (descLength < sizeof(SkDescriptor)) return false;
        if (descLength != SkAlign4(descLength)) return false;

        auto* result = this->ensureAtLeast(descLength, alignof(SkDescriptor));
        if (!result) return false;

        ad->reset(descLength);
        memcpy(ad->getDesc(), const_cast<const char*>(result), descLength);

        if (ad->getDesc()->getLength() > descLength) return false;
        return ad->getDesc()->isValid();
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

bool SkFuzzDeserializeSkDescriptor(sk_sp<SkData> bytes, SkAutoDescriptor* ad) {
    auto d = Deserializer(reinterpret_cast<const volatile char*>(bytes->data()), bytes->size());
    return d.readDescriptor(ad);
}

// Paths use a SkWriter32 which requires 4 byte alignment.
static const size_t kPathAlignment  = 4u;

// -- StrikeSpec -----------------------------------------------------------------------------------
struct StrikeSpec {
    StrikeSpec() = default;
    StrikeSpec(SkFontID typefaceID_, SkDiscardableHandleId discardableHandleId_)
            : typefaceID{typefaceID_}, discardableHandleId(discardableHandleId_) {}
    SkFontID typefaceID = 0u;
    SkDiscardableHandleId discardableHandleId = 0u;
    /* desc */
    /* n X (glyphs ids) */
};

// Represent a set of x sub-pixel-position glyphs with a glyph id < kMaxGlyphID and
// y sub-pxiel-position must be 0. Most sub-pixel-positioned glyphs have been x-axis aligned
// forcing the y sub-pixel position to be zero. We can organize the SkPackedGlyphID to check that
// the glyph id and the y position == 0 with a single compare in the following way:
//    <y-sub-pixel-position>:2 | <glyphid:16> | <x-sub-pixel-position>:2
// This organization allows a single check of a packed-id to be:
//    packed-id < kMaxGlyphID * possible-x-sub-pixel-positions
// where possible-x-sub-pixel-positions == 4.
class LowerRangeBitVector {
public:
    bool test(SkPackedGlyphID packedID) const {
        uint32_t bit = packedID.value();
        return bit < kMaxIndex && fBits.test(bit);
    }
    void setIfLower(SkPackedGlyphID packedID) {
        uint32_t bit = packedID.value();
        if (bit < kMaxIndex) {
            fBits.set(bit);
        }
    }

private:
    using GID = SkPackedGlyphID;
    static_assert(GID::kSubPixelX < GID::kGlyphID && GID::kGlyphID < GID::kSubPixelY,
            "SkPackedGlyphID must be organized: sub-y | glyph id | sub-x");
    inline static constexpr int kMaxGlyphID = 128;
    inline static constexpr int kMaxIndex = kMaxGlyphID * (1u << GID::kSubPixelPosLen);
    std::bitset<kMaxIndex> fBits;
};

// -- MapOps ---------------------------------------------------------------------------------------
struct MapOps {
    size_t operator()(const SkDescriptor* key) const;
    bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const;
};
size_t MapOps::operator()(const SkDescriptor* key) const {
    return key->getChecksum();
}

bool MapOps::operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
    return *lhs == *rhs;
}

// -- RemoteStrike ----------------------------------------------------------------------------
class RemoteStrike final : public SkStrikeForGPU {
public:
    // N.B. RemoteStrike is not valid until ensureScalerContext is called.
    RemoteStrike(const SkStrikeSpec& strikeSpec,
                 std::unique_ptr<SkScalerContext> context,
                 SkDiscardableHandleId discardableHandleId);
    ~RemoteStrike() override = default;

    void writePendingGlyphs(Serializer* serializer);
    SkDiscardableHandleId discardableHandleId() const { return fDiscardableHandleId; }

    const SkDescriptor& getDescriptor() const override {
        return *fDescriptor.getDesc();
    }

    void setStrikeSpec(const SkStrikeSpec& strikeSpec);

    const SkGlyphPositionRoundingSpec& roundingSpec() const override {
        return fRoundingSpec;
    }

    void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override;

    void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override;

    void prepareForPathDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override;

    void onAboutToExitScope() override {}

    sk_sp<SkStrike> getUnderlyingStrike() const override { return nullptr; }

    bool hasPendingGlyphs() const {
        return !fMasksToSend.empty() || !fPathsToSend.empty();
    }

    void resetScalerContext();

private:
    template <typename Rejector>
    void commonMaskLoop(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects, Rejector&& reject);

    // Keep track of if the glyph draw has been totally satisfied. It could be that this
    // strike can not draw the glyph, and it must be rejected to be handled by fallback.
    // For example, if a glyph has canDrawAsMask sent, then that data is on the GPU, and this
    // strike totally satisfies this result. If canDrawAsMask is false, then this glyph must be
    // rejected, and handled by a later stage using a latter strike.
    struct MaskSummary {
        static_assert(SkPackedGlyphID::kMaskAll < (1u << 30), "SkPackedGlyphID is too big.");
        uint32_t packedID:30;
        uint32_t canDrawAsMask:1;
        uint32_t canDrawAsSDFT:1;
    };

    struct MaskSummaryTraits {
        static SkPackedGlyphID GetKey(MaskSummary summary) {
            return SkPackedGlyphID{summary.packedID};
        }

        static uint32_t Hash(SkPackedGlyphID packedID) {
            return packedID.hash();
        }
    };

    // Same thing as MaskSummary, but for paths.
    struct PathSummary {
        constexpr static uint16_t kIsPath = 0;
        SkPackedGlyphID packedID;
        // If drawing glyphID can be done with a path, this is 0, otherwise it is the max
        // dimension of the glyph.
        uint16_t maxDimensionOrPath;
    };

    struct PathSummaryTraits {
        static SkPackedGlyphID GetKey(PathSummary summary) {
            return summary.packedID;
        }

        static uint32_t Hash(SkPackedGlyphID packedID) {
            return SkChecksum::CheapMix(packedID.value());
        }
    };

    void writeGlyphPath(const SkGlyph& glyph, Serializer* serializer) const;
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

    LowerRangeBitVector fSentLowGlyphIDs;

    // The masks and paths that currently reside in the GPU process.
    SkTHashTable<MaskSummary, SkPackedGlyphID, MaskSummaryTraits> fSentGlyphs;
    SkTHashTable<PathSummary, SkPackedGlyphID, PathSummaryTraits> fSentPaths;

    // The Masks, SDFT Mask, and Paths that need to be sent to the GPU task for the processed
    // TextBlobs. Cleared after diffs are serialized.
    std::vector<SkGlyph> fMasksToSend;
    std::vector<SkGlyph> fPathsToSend;

    // Alloc for storing bits and pieces of paths, Cleared after diffs are serialized.
    SkArenaAllocWithReset fPathAlloc{256};
};

RemoteStrike::RemoteStrike(
        const SkStrikeSpec& strikeSpec,
        std::unique_ptr<SkScalerContext> context,
        uint32_t discardableHandleId)
        : fDescriptor{strikeSpec.descriptor()}
        , fDiscardableHandleId(discardableHandleId)
        , fRoundingSpec{context->isSubpixel(), context->computeAxisAlignmentForHText()}
        // N.B. context must come last because it is used above.
        , fContext{std::move(context)}
        , fSentLowGlyphIDs{} {
    SkASSERT(fDescriptor.getDesc() != nullptr);
    SkASSERT(fContext != nullptr);
}

// No need to write fForceBW because it is a flag private to SkScalerContext_DW, which will never
// be called on the GPU side.
static void writeGlyph(const SkGlyph& glyph, Serializer* serializer) {
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
        SkASSERT(SkMask::IsValidFormat(glyph.fMaskFormat));

        writeGlyph(glyph, serializer);
        auto imageSize = glyph.imageSize();
        if (imageSize > 0 && FitsInAtlas(glyph)) {
            glyph.fImage = serializer->allocate(imageSize, glyph.formatAlignment());
            fContext->getImage(glyph);
        }
    }
    fMasksToSend.clear();

    // Write glyphs paths.
    serializer->emplace<uint64_t>(fPathsToSend.size());
    for (SkGlyph& glyph : fPathsToSend) {
        SkASSERT(SkMask::IsValidFormat(glyph.fMaskFormat));

        writeGlyph(glyph, serializer);
        writeGlyphPath(glyph, serializer);
    }
    fPathsToSend.clear();
    fPathAlloc.reset();
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

void RemoteStrike::writeGlyphPath(
        const SkGlyph& glyph, Serializer* serializer) const {
    if (glyph.isColor() || glyph.isEmpty()) {
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

template <typename Rejector>
void RemoteStrike::commonMaskLoop(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects, Rejector&& reject) {
    drawables->forEachGlyphID(
            [&](size_t i, SkPackedGlyphID packedID, SkPoint position) {
                MaskSummary* summary = fSentGlyphs.find(packedID);
                if (summary == nullptr) {
                    // Put the new SkGlyph in the glyphs to send.
                    this->ensureScalerContext();
                    fMasksToSend.emplace_back(fContext->makeGlyph(packedID, &fPathAlloc));
                    SkGlyph* glyph = &fMasksToSend.back();

                    MaskSummary newSummary =
                            {packedID.value(), CanDrawAsMask(*glyph), CanDrawAsSDFT(*glyph)};
                    summary = fSentGlyphs.set(newSummary);
                }

                // Reject things that are too big.
                if (reject(*summary)) {
                    rejects->reject(i);
                }
            });
}

void RemoteStrike::prepareForMaskDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    for (auto [i, variant, _] : SkMakeEnumerate(drawables->input())) {
        SkPackedGlyphID packedID = variant.packedID();
        if (fSentLowGlyphIDs.test(packedID)) {
            #ifdef SK_DEBUG
            MaskSummary* summary = fSentGlyphs.find(packedID);
            SkASSERT(summary != nullptr);
            SkASSERT(summary->canDrawAsMask && summary->canDrawAsSDFT);
            #endif
            continue;
        }

        MaskSummary* summary = fSentGlyphs.find(packedID);
        if (summary == nullptr) {

            // Put the new SkGlyph in the glyphs to send.
            this->ensureScalerContext();
            fMasksToSend.emplace_back(fContext->makeGlyph(packedID, &fPathAlloc));
            SkGlyph* glyph = &fMasksToSend.back();

            MaskSummary newSummary =
                    {packedID.value(), CanDrawAsMask(*glyph), CanDrawAsSDFT(*glyph)};

            summary = fSentGlyphs.set(newSummary);

            if (summary->canDrawAsMask && summary->canDrawAsSDFT) {
                fSentLowGlyphIDs.setIfLower(packedID);
            }
        }

        // Reject things that are too big.
        if (!summary->canDrawAsMask) {
            rejects->reject(i);
        }
    }
}

void RemoteStrike::prepareForSDFTDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    this->commonMaskLoop(drawables, rejects,
                         [](MaskSummary summary){return !summary.canDrawAsSDFT;});
}

void RemoteStrike::prepareForPathDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    drawables->forEachGlyphID(
            [&](size_t i, SkPackedGlyphID packedID, SkPoint position) {
                PathSummary* summary = fSentPaths.find(packedID);
                if (summary == nullptr) {

                    // Put the new SkGlyph in the glyphs to send.
                    this->ensureScalerContext();
                    fPathsToSend.emplace_back(fContext->makeGlyph(packedID, &fPathAlloc));
                    SkGlyph* glyph = &fPathsToSend.back();

                    uint16_t maxDimensionOrPath = glyph->maxDimension();
                    glyph->setPath(&fPathAlloc, fContext.get());
                    if (glyph->path() != nullptr) {
                        maxDimensionOrPath = PathSummary::kIsPath;
                    }

                    PathSummary newSummary = {packedID, maxDimensionOrPath};
                    summary = fSentPaths.set(newSummary);
                }

                if (summary->maxDimensionOrPath != PathSummary::kIsPath) {
                    rejects->reject(i, (int)summary->maxDimensionOrPath);
                }
            });
}

// -- WireTypeface ---------------------------------------------------------------------------------
struct WireTypeface {
    WireTypeface() = default;
    WireTypeface(SkFontID typeface_id, int glyph_count, SkFontStyle style,
                 bool is_fixed, bool needsCurrentColor)
      : typefaceID(typeface_id), glyphCount(glyph_count), style(style),
        isFixed(is_fixed), glyphMaskNeedsCurrentColor(needsCurrentColor) {}

    SkFontID        typefaceID{0};
    int             glyphCount{0};
    SkFontStyle     style;
    bool            isFixed{false};
    // Used for COLRv0 or COLRv1 fonts that may need the 0xFFFF special palette
    // index to represent foreground color. This information needs to be on here
    // to determine how this typeface can be cached.
    bool            glyphMaskNeedsCurrentColor{false};
};

// -- SkStrikeServerImpl ---------------------------------------------------------------------------
class SkStrikeServerImpl final : public SkStrikeForGPUCacheInterface {
public:
    explicit SkStrikeServerImpl(
            SkStrikeServer::DiscardableHandleManager* discardableHandleManager);

    // SkStrikeServer API methods
    sk_sp<SkData> serializeTypeface(SkTypeface*);
    void writeStrikeData(std::vector<uint8_t>* memory);

    SkScopedStrikeForGPU findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) override;

    // Methods for testing
    void setMaxEntriesInDescriptorMapForTesting(size_t count);
    size_t remoteStrikeMapSizeForTesting() const;

private:
    inline static constexpr size_t kMaxEntriesInDescriptorMap = 2000u;

    void checkForDeletedEntries();

    RemoteStrike* getOrCreateCache(const SkStrikeSpec& strikeSpec);


    using DescToRemoteStrike =
    std::unordered_map<const SkDescriptor*, std::unique_ptr<RemoteStrike>, MapOps, MapOps>;
    DescToRemoteStrike fDescToRemoteStrike;

    SkStrikeServer::DiscardableHandleManager* const fDiscardableHandleManager;
    SkTHashSet<SkFontID> fCachedTypefaces;
    size_t fMaxEntriesInDescriptorMap = kMaxEntriesInDescriptorMap;

    // Cached serialized typefaces.
    SkTHashMap<SkFontID, sk_sp<SkData>> fSerializedTypefaces;

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
#ifdef SK_DEBUG
            [&](RemoteStrike* strike) {
                if (strike->hasPendingGlyphs()) {
                    strike->writePendingGlyphs(&serializer);
                    strike->resetScalerContext();
                }
                auto it = fDescToRemoteStrike.find(&strike->getDescriptor());
                SkASSERT(it != fDescToRemoteStrike.end());
                SkASSERT(it->second.get() == strike);
                #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
                    msg.append(strike->getDescriptor().dumpRec());
                #endif
            }

#else
            [&serializer](RemoteStrike* strike) {
                if (strike->hasPendingGlyphs()) {
                    strike->writePendingGlyphs(&serializer);
                    strike->resetScalerContext();
                }
                #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
                    msg.append(strike->getDescriptor().dumpRec());
                #endif
            }
#endif
    );
    fRemoteStrikesToSend.reset();
    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        msg.appendf("End send strike differences");
        SkDebugf("%s\n", msg.c_str());
    #endif
}

SkScopedStrikeForGPU SkStrikeServerImpl::findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) {
    return SkScopedStrikeForGPU{this->getOrCreateCache(strikeSpec)};
}

void SkStrikeServerImpl::checkForDeletedEntries() {
    auto it = fDescToRemoteStrike.begin();
    while (fDescToRemoteStrike.size() > fMaxEntriesInDescriptorMap &&
           it != fDescToRemoteStrike.end()) {
        RemoteStrike* strike = it->second.get();
        if (fDiscardableHandleManager->isHandleDeleted(strike->discardableHandleId())) {
            // If we are removing the strike, we better not be trying to send it at the same time.
            SkASSERT(!fRemoteStrikesToSend.contains(strike));
            it = fDescToRemoteStrike.erase(it);
        } else {
            ++it;
        }
    }
}

RemoteStrike* SkStrikeServerImpl::getOrCreateCache(const SkStrikeSpec& strikeSpec) {

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

    auto it = fDescToRemoteStrike.find(&strikeSpec.descriptor());
    if (it != fDescToRemoteStrike.end()) {
        // We have processed the RemoteStrike before. Reuse it.
        RemoteStrike* strike = it->second.get();
        strike->setStrikeSpec(strikeSpec);
        if (fRemoteStrikesToSend.contains(strike)) {
            // Already tracking
            return strike;
        }

        // Strike is in unknown state on GPU. Start tracking strike on GPU by locking it.
        bool locked = fDiscardableHandleManager->lockHandle(it->second->discardableHandleId());
        if (locked) {
            fRemoteStrikesToSend.add(strike);
            return strike;
        }

        fDescToRemoteStrike.erase(it);
    }

    const SkTypeface& typeface = strikeSpec.typeface();
    // Create a new RemoteStrike. Start by processing the typeface.
    const SkFontID typefaceId = typeface.uniqueID();
    if (!fCachedTypefaces.contains(typefaceId)) {
        fCachedTypefaces.add(typefaceId);
        fTypefacesToSend.emplace_back(typefaceId, typeface.countGlyphs(),
                                      typeface.fontStyle(),
                                      typeface.isFixedPitch(),
                                      typeface.glyphMaskNeedsCurrentColor());
    }

    auto context = strikeSpec.createScalerContext();
    auto newHandle = fDiscardableHandleManager->createHandle();  // Locked on creation
    auto remoteStrike = std::make_unique<RemoteStrike>(strikeSpec, std::move(context), newHandle);
    remoteStrike->setStrikeSpec(strikeSpec);
    auto remoteStrikePtr = remoteStrike.get();
    fRemoteStrikesToSend.add(remoteStrikePtr);
    auto d = &remoteStrike->getDescriptor();
    fDescToRemoteStrike[d] = std::move(remoteStrike);

    checkForDeletedEntries();

    return remoteStrikePtr;
}

// -- GlyphTrackingDevice --------------------------------------------------------------------------
class GlyphTrackingDevice final : public SkNoPixelsDevice {
public:
    GlyphTrackingDevice(
            const SkISize& dimensions, const SkSurfaceProps& props, SkStrikeServerImpl* server,
            sk_sp<SkColorSpace> colorSpace, bool DFTSupport)
            : SkNoPixelsDevice(SkIRect::MakeSize(dimensions), props, std::move(colorSpace))
            , fStrikeServerImpl(server)
            , fDFTSupport(DFTSupport)
            , fPainter{props, kUnknown_SkColorType, imageInfo().colorSpace(), fStrikeServerImpl} {
        SkASSERT(fStrikeServerImpl != nullptr);
    }

    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new GlyphTrackingDevice(cinfo.fInfo.dimensions(), surfaceProps, fStrikeServerImpl,
                                       cinfo.fInfo.refColorSpace(), fDFTSupport);
    }

protected:
    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override {
        #if SK_SUPPORT_GPU
        GrContextOptions ctxOptions;
        GrSDFTControl control =
                GrSDFTControl{fDFTSupport,
                              this->surfaceProps().isUseDeviceIndependentFonts(),
                              ctxOptions.fMinDistanceFieldFontSize,
                              ctxOptions.fGlyphsAsPathsFontSize};

        SkMatrix drawMatrix = this->localToDevice();
        drawMatrix.preTranslate(glyphRunList.origin().x(), glyphRunList.origin().y());
        const uint64_t uniqueID = glyphRunList.uniqueID();
        for (auto& glyphRun : glyphRunList) {
            fPainter.processGlyphRun(glyphRun,
                                     drawMatrix,
                                     paint,
                                     control,
                                     nullptr,
                                     "Cache Diff",
                                     uniqueID);
        }
        #endif  // SK_SUPPORT_GPU
    }

private:
    SkStrikeServerImpl* const fStrikeServerImpl;
    const bool fDFTSupport{false};
    SkGlyphRunListPainter fPainter;
};

// -- SkStrikeServer -------------------------------------------------------------------------------
SkStrikeServer::SkStrikeServer(DiscardableHandleManager* dhm)
        : fImpl(new SkStrikeServerImpl{dhm}) { }

SkStrikeServer::~SkStrikeServer() = default;

std::unique_ptr<SkCanvas> SkStrikeServer::makeAnalysisCanvas(int width, int height,
                                                             const SkSurfaceProps& props,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             bool DFTSupport) {
    sk_sp<SkBaseDevice> trackingDevice(new GlyphTrackingDevice(SkISize::Make(width, height),
                                                               props, this->impl(),
                                                               std::move(colorSpace),
                                                               DFTSupport));
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

private:
    static bool ReadGlyph(SkTLazy<SkGlyph>& glyph, Deserializer* deserializer);
    sk_sp<SkTypeface> addTypeface(const WireTypeface& wire);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fRemoteFontIdToTypeface;
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

// No need to read fForceBW because it is a flag private to SkScalerContext_DW, which will never
// be called on the GPU side.
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

#define READ_FAILURE                                                        \
    {                                                                       \
        SkDebugf("Bad font data serialization line: %d", __LINE__);         \
        SkStrikeClient::DiscardableHandleManager::ReadFailureData data = {  \
                memorySize,  deserializer.bytesRead(), typefaceSize,        \
                strikeCount, glyphImagesCount,         glyphPathsCount};    \
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

    if (!deserializer.read<uint64_t>(&typefaceSize)) READ_FAILURE
    for (size_t i = 0; i < typefaceSize; ++i) {
        WireTypeface wire;
        if (!deserializer.read<WireTypeface>(&wire)) READ_FAILURE

        // TODO(khushalsagar): The typeface no longer needs a reference to the
        // SkStrikeClient, since all needed glyphs must have been pushed before
        // raster.
        addTypeface(wire);
    }

    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        SkString msg;
        msg.appendf("\nBegin receive strike differences\n");
    #endif

    if (!deserializer.read<uint64_t>(&strikeCount)) READ_FAILURE

    for (size_t i = 0; i < strikeCount; ++i) {
        StrikeSpec spec;
        if (!deserializer.read<StrikeSpec>(&spec)) READ_FAILURE

        SkAutoDescriptor sourceAd;
        if (!deserializer.readDescriptor(&sourceAd)) READ_FAILURE
        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  Received descriptor:\n%s", sourceAd.getDesc()->dumpRec().c_str());
        #endif

        bool fontMetricsInitialized;
        if (!deserializer.read(&fontMetricsInitialized)) READ_FAILURE

        SkFontMetrics fontMetrics{};
        if (!fontMetricsInitialized) {
            if (!deserializer.read<SkFontMetrics>(&fontMetrics)) READ_FAILURE
        }

        // Get the local typeface from remote fontID.
        auto* tfPtr = fRemoteFontIdToTypeface.find(spec.typefaceID);
        // Received strikes for a typeface which doesn't exist.
        if (!tfPtr) READ_FAILURE
        auto* tf = tfPtr->get();

        // Replace the ContextRec in the desc from the server to create the client
        // side descriptor.
        // TODO: Can we do this in-place and re-compute checksum? Instead of a complete copy.
        SkAutoDescriptor ad;
        auto* client_desc = auto_descriptor_from_desc(sourceAd.getDesc(), tf->uniqueID(), &ad);

        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  Mapped descriptor:\n%s", client_desc->dumpRec().c_str());
        #endif
        auto strike = fStrikeCache->findStrike(*client_desc);
        // Metrics are only sent the first time. If the metrics are not initialized, there must
        // be an existing strike.
        if (fontMetricsInitialized && strike == nullptr) READ_FAILURE
        if (strike == nullptr) {
            // Note that we don't need to deserialize the effects since we won't be generating any
            // glyphs here anyway, and the desc is still correct since it includes the serialized
            // effects.
            SkStrikeSpec strikeSpec{*client_desc, *tfPtr};
            strike = fStrikeCache->createStrike(
                    strikeSpec, &fontMetrics,
                    std::make_unique<DiscardableStrikePinner>(
                            spec.discardableHandleId, fDiscardableHandleManager));
        }

        if (!deserializer.read<uint64_t>(&glyphImagesCount)) READ_FAILURE
        for (size_t j = 0; j < glyphImagesCount; j++) {
            SkTLazy<SkGlyph> glyph;
            if (!ReadGlyph(glyph, &deserializer)) READ_FAILURE

            if (!glyph->isEmpty() && SkStrikeForGPU::FitsInAtlas(*glyph)) {
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
    }

#if defined(SK_TRACE_GLYPH_RUN_PROCESS)
    msg.appendf("End receive strike differences");
    SkDebugf("%s\n", msg.c_str());
#endif

    return true;
}

sk_sp<SkTypeface> SkStrikeClientImpl::deserializeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len != sizeof(wire)) return nullptr;
    memcpy(&wire, buf, sizeof(wire));
    return this->addTypeface(wire);
}

sk_sp<SkTypeface> SkStrikeClientImpl::addTypeface(const WireTypeface& wire) {
    auto* typeface = fRemoteFontIdToTypeface.find(wire.typefaceID);
    if (typeface) return *typeface;

    auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.typefaceID, wire.glyphCount, wire.style, wire.isFixed,
            wire.glyphMaskNeedsCurrentColor, fDiscardableHandleManager, fIsLogging);
    fRemoteFontIdToTypeface.set(wire.typefaceID, newTypeface);
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

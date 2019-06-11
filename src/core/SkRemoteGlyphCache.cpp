/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRemoteGlyphCache.h"

#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <tuple>

#include "src/core/SkDevice.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkRemoteGlyphCacheImpl.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTypeface_remote.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/text/GrTextContext.h"
#endif

static SkDescriptor* auto_descriptor_from_desc(const SkDescriptor* source_desc,
                                               SkFontID font_id,
                                               SkAutoDescriptor* ad) {
    ad->reset(source_desc->getLength());
    auto* desc = ad->getDesc();
    desc->init();

    // Rec.
    {
        uint32_t size;
        auto ptr = source_desc->findEntry(kRec_SkDescriptorTag, &size);
        SkScalerContextRec rec;
        std::memcpy(&rec, ptr, size);
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

static const SkDescriptor* create_descriptor(
        const SkPaint& paint, const SkFont& font, const SkMatrix& m,
        const SkSurfaceProps& props, SkScalerContextFlags flags,
        SkAutoDescriptor* ad, SkScalerContextEffects* effects) {
    SkScalerContextRec rec;
    SkScalerContext::MakeRecAndEffects(font, paint, props, flags, m, &rec, effects);
    return SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, *effects, ad);
}

// -- Serializer ----------------------------------------------------------------------------------

size_t pad(size_t size, size_t alignment) { return (size + (alignment - 1)) & ~(alignment - 1); }

class Serializer {
public:
    Serializer(std::vector<uint8_t>* buffer) : fBuffer{buffer} { }

    template <typename T, typename... Args>
    T* emplace(Args&&... args) {
        auto result = allocate(sizeof(T), alignof(T));
        return new (result) T{std::forward<Args>(args)...};
    }

    template <typename T>
    void write(const T& data) {
        T* result = (T*)allocate(sizeof(T), alignof(T));
        memcpy(result, &data, sizeof(T));
    }

    template <typename T>
    T* allocate() {
        T* result = (T*)allocate(sizeof(T), alignof(T));
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
        auto* result = this->ensureAtLeast(sizeof(T), alignof(T));
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
static const size_t kPathAlignment  = 4u;

bool read_path(Deserializer* deserializer, SkGlyph* glyph, SkStrike* cache) {
    uint64_t pathSize = 0u;
    if (!deserializer->read<uint64_t>(&pathSize)) return false;

    if (pathSize == 0u) {
        cache->initializePath(glyph, nullptr, 0u);
        return true;
    }

    auto* path = deserializer->read(pathSize, kPathAlignment);
    if (!path) return false;

    // Don't overwrite the path if we already have one. We could have used a fallback if the
    // glyph was missing earlier.
    if (glyph->fPathData != nullptr) return true;

    return cache->initializePath(glyph, path, pathSize);
}

size_t SkDescriptorMapOperators::operator()(const SkDescriptor* key) const {
    return key->getChecksum();
}

bool SkDescriptorMapOperators::operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
    return *lhs == *rhs;
}

struct StrikeSpec {
    StrikeSpec() {}
    StrikeSpec(SkFontID typefaceID_, SkDiscardableHandleId discardableHandleId_)
            : typefaceID{typefaceID_}, discardableHandleId(discardableHandleId_) {}
    SkFontID typefaceID = 0u;
    SkDiscardableHandleId discardableHandleId = 0u;
    /* desc */
    /* n X (glyphs ids) */
};

// -- TrackLayerDevice -----------------------------------------------------------------------------
SkTextBlobCacheDiffCanvas::TrackLayerDevice::TrackLayerDevice(
        const SkIRect& bounds, const SkSurfaceProps& props, SkStrikeServer* server,
        sk_sp<SkColorSpace> colorSpace, const SkTextBlobCacheDiffCanvas::Settings& settings)
        : SkNoPixelsDevice(bounds, props, std::move(colorSpace))
        , fStrikeServer(server)
        , fSettings(settings)
        , fPainter{props, kUnknown_SkColorType, imageInfo().colorSpace(), fStrikeServer} {
    SkASSERT(fStrikeServer);
}

SkBaseDevice* SkTextBlobCacheDiffCanvas::TrackLayerDevice::onCreateDevice(
        const CreateInfo& cinfo, const SkPaint*) {
    const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
    return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps, fStrikeServer,
                                cinfo.fInfo.refColorSpace(), fSettings);
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::drawGlyphRunList(
        const SkGlyphRunList& glyphRunList) {

    #if SK_SUPPORT_GPU
    GrTextContext::Options options;
    options.fMinDistanceFieldFontSize = fSettings.fMinDistanceFieldFontSize;
    options.fMaxDistanceFieldFontSize = fSettings.fMaxDistanceFieldFontSize;
    GrTextContext::SanitizeOptions(&options);

    fPainter.processGlyphRunList(glyphRunList,
                                 this->ctm(),
                                 this->surfaceProps(),
                                 fSettings.fContextSupportsDistanceFieldText,
                                 options,
                                 nullptr);
    #endif  // SK_SUPPORT_GPU

}

// -- SkTextBlobCacheDiffCanvas -------------------------------------------------------------------
SkTextBlobCacheDiffCanvas::Settings::Settings() = default;

SkTextBlobCacheDiffCanvas::SkTextBlobCacheDiffCanvas(
        int width, int height, const SkSurfaceProps& props,
        SkStrikeServer* strikeServer, Settings settings)
    : SkNoDrawCanvas{sk_make_sp<TrackLayerDevice>(
            SkIRect::MakeWH(width, height), props, strikeServer, nullptr, settings)} {}

SkTextBlobCacheDiffCanvas::SkTextBlobCacheDiffCanvas(int width, int height,
                                                     const SkSurfaceProps& props,
                                                     SkStrikeServer* strikeServer,
                                                     sk_sp<SkColorSpace> colorSpace,
                                                     Settings settings)
    : SkNoDrawCanvas{sk_make_sp<TrackLayerDevice>(SkIRect::MakeWH(width, height), props,
                                                  strikeServer, std::move(colorSpace), settings)} {}

SkTextBlobCacheDiffCanvas::~SkTextBlobCacheDiffCanvas() = default;

SkCanvas::SaveLayerStrategy SkTextBlobCacheDiffCanvas::getSaveLayerStrategy(
        const SaveLayerRec& rec) {
    return kFullLayer_SaveLayerStrategy;
}

bool SkTextBlobCacheDiffCanvas::onDoSaveBehind(const SkRect*) {
    return false;
}

void SkTextBlobCacheDiffCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                               const SkPaint& paint) {
    ///
    SkCanvas::onDrawTextBlob(blob, x, y, paint);
}

struct WireTypeface {
    WireTypeface() = default;
    WireTypeface(SkFontID typeface_id, int glyph_count, SkFontStyle style, bool is_fixed)
            : typefaceID(typeface_id), glyphCount(glyph_count), style(style), isFixed(is_fixed) {}

    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
};

// SkStrikeServer -----------------------------------------

SkStrikeServer::SkStrikeServer(DiscardableHandleManager* discardableHandleManager)
        : fDiscardableHandleManager(discardableHandleManager) {
    SkASSERT(fDiscardableHandleManager);
}

SkStrikeServer::~SkStrikeServer() = default;

sk_sp<SkData> SkStrikeServer::serializeTypeface(SkTypeface* tf) {
    auto* data = fSerializedTypefaces.find(SkTypeface::UniqueID(tf));
    if (data) {
        return *data;
    }

    WireTypeface wire(SkTypeface::UniqueID(tf), tf->countGlyphs(), tf->fontStyle(),
                      tf->isFixedPitch());
    data = fSerializedTypefaces.set(SkTypeface::UniqueID(tf),
                                    SkData::MakeWithCopy(&wire, sizeof(wire)));
    return *data;
}

void SkStrikeServer::writeStrikeData(std::vector<uint8_t>* memory) {
    if (fLockedDescs.empty() && fTypefacesToSend.empty()) {
        return;
    }

    Serializer serializer(memory);
    serializer.emplace<uint64_t>(fTypefacesToSend.size());
    for (const auto& tf : fTypefacesToSend) serializer.write<WireTypeface>(tf);
    fTypefacesToSend.clear();

    serializer.emplace<uint64_t>(fLockedDescs.size());
    for (const auto* desc : fLockedDescs) {
        auto it = fRemoteGlyphStateMap.find(desc);
        SkASSERT(it != fRemoteGlyphStateMap.end());
        it->second->writePendingGlyphs(&serializer);
    }
    fLockedDescs.clear();
}

SkStrikeServer::SkGlyphCacheState* SkStrikeServer::getOrCreateCache(
        const SkPaint& paint,
        const SkFont& font,
        const SkSurfaceProps& props,
        const SkMatrix& matrix,
        SkScalerContextFlags flags,
        SkScalerContextEffects* effects) {
    SkAutoDescriptor descStorage;
    auto desc = create_descriptor(paint, font, matrix, props, flags, &descStorage, effects);

    return this->getOrCreateCache(*desc, *font.getTypefaceOrDefault(), *effects);

}

SkScopedStrike SkStrikeServer::findOrCreateScopedStrike(const SkDescriptor& desc,
                                                        const SkScalerContextEffects& effects,
                                                        const SkTypeface& typeface) {
    return SkScopedStrike{this->getOrCreateCache(desc, typeface, effects)};
}

void SkStrikeServer::checkForDeletedEntries() {
    auto it = fRemoteGlyphStateMap.begin();
    while (fRemoteGlyphStateMap.size() > fMaxEntriesInDescriptorMap &&
           it != fRemoteGlyphStateMap.end()) {
        if (fDiscardableHandleManager->isHandleDeleted(it->second->discardableHandleId())) {
            it = fRemoteGlyphStateMap.erase(it);
        } else {
            ++it;
        }
    }
}

SkStrikeServer::SkGlyphCacheState* SkStrikeServer::getOrCreateCache(
        const SkDescriptor& desc, const SkTypeface& typeface, SkScalerContextEffects effects) {

    // In cases where tracing is turned off, make sure not to get an unused function warning.
    // Lambdaize the function.
    TRACE_EVENT1("skia", "RecForDesc", "rec",
            TRACE_STR_COPY(
                    [&desc](){
                        auto ptr = desc.findEntry(kRec_SkDescriptorTag, nullptr);
                        SkScalerContextRec rec;
                        std::memcpy(&rec, ptr, sizeof(rec));
                        return rec.dump();
                    }().c_str()
            )
    );

    // Already locked.
    if (fLockedDescs.find(&desc) != fLockedDescs.end()) {
        auto it = fRemoteGlyphStateMap.find(&desc);
        SkASSERT(it != fRemoteGlyphStateMap.end());
        SkGlyphCacheState* cache = it->second.get();
        cache->setTypefaceAndEffects(&typeface, effects);
        return cache;
    }

    // Try to lock.
    auto it = fRemoteGlyphStateMap.find(&desc);
    if (it != fRemoteGlyphStateMap.end()) {
        SkGlyphCacheState* cache = it->second.get();
        bool locked = fDiscardableHandleManager->lockHandle(it->second->discardableHandleId());
        if (locked) {
            fLockedDescs.insert(it->first);
            cache->setTypefaceAndEffects(&typeface, effects);
            return cache;
        }

        // If the lock failed, the entry was deleted on the client. Remove our
        // tracking.
        fRemoteGlyphStateMap.erase(it);
    }

    const SkFontID typefaceId = typeface.uniqueID();
    if (!fCachedTypefaces.contains(typefaceId)) {
        fCachedTypefaces.add(typefaceId);
        fTypefacesToSend.emplace_back(typefaceId, typeface.countGlyphs(),
                                      typeface.fontStyle(),
                                      typeface.isFixedPitch());
    }

    auto context = typeface.createScalerContext(effects, &desc);

    // Create a new cache state and insert it into the map.
    auto newHandle = fDiscardableHandleManager->createHandle();
    auto cacheState = skstd::make_unique<SkGlyphCacheState>(desc, std::move(context), newHandle);

    auto* cacheStatePtr = cacheState.get();

    fLockedDescs.insert(&cacheStatePtr->getDescriptor());
    fRemoteGlyphStateMap[&cacheStatePtr->getDescriptor()] = std::move(cacheState);

    checkForDeletedEntries();

    cacheStatePtr->setTypefaceAndEffects(&typeface, effects);
    return cacheStatePtr;
}

// -- SkGlyphCacheState ----------------------------------------------------------------------------
SkStrikeServer::SkGlyphCacheState::SkGlyphCacheState(
        const SkDescriptor& descriptor,
        std::unique_ptr<SkScalerContext> context,
        uint32_t discardableHandleId)
        : fDescriptor{descriptor}
        , fDiscardableHandleId(discardableHandleId)
        , fIsSubpixel{context->isSubpixel()}
        , fAxisAlignmentForHText{context->computeAxisAlignmentForHText()}
        // N.B. context must come last because it is used above.
        , fContext{std::move(context)} {
    SkASSERT(fDescriptor.getDesc() != nullptr);
    SkASSERT(fContext != nullptr);
}

SkStrikeServer::SkGlyphCacheState::~SkGlyphCacheState() = default;

void SkStrikeServer::SkGlyphCacheState::addGlyph(SkPackedGlyphID glyph, bool asPath) {
    auto* cache = asPath ? &fCachedGlyphPaths : &fCachedGlyphImages;
    auto* pending = asPath ? &fPendingGlyphPaths : &fPendingGlyphImages;

    // Already cached.
    if (cache->contains(glyph)) {
        return;
    }

    // A glyph is going to be sent. Make sure we have a scaler context to send it.
    this->ensureScalerContext();

    // Serialize and cache. Also create the scalar context to use when serializing
    // this glyph.
    cache->add(glyph);
    pending->push_back(glyph);
}

// No need to write fForceBW because it is a flag private to SkScalerContext_DW, which will never
// be called on the GPU side.
static void writeGlyph(SkGlyph* glyph, Serializer* serializer) {
    serializer->write<SkPackedGlyphID>(glyph->getPackedID());
    serializer->write<float>(glyph->fAdvanceX);
    serializer->write<float>(glyph->fAdvanceY);
    serializer->write<uint16_t>(glyph->fWidth);
    serializer->write<uint16_t>(glyph->fHeight);
    serializer->write<int16_t>(glyph->fTop);
    serializer->write<int16_t>(glyph->fLeft);
    serializer->write<uint8_t>(glyph->fMaskFormat);
}

void SkStrikeServer::SkGlyphCacheState::writePendingGlyphs(Serializer* serializer) {
    // TODO(khushalsagar): Write a strike only if it has any pending glyphs.
    serializer->emplace<bool>(this->hasPendingGlyphs());
    if (!this->hasPendingGlyphs()) {
        this->resetScalerContext();
        return;
    }

    // Write the desc.
    serializer->emplace<StrikeSpec>(fContext->getTypeface()->uniqueID(), fDiscardableHandleId);
    serializer->writeDescriptor(*fDescriptor.getDesc());

    // Write FontMetrics.
    // TODO(khushalsagar): Do we need to re-send each time?
    SkFontMetrics fontMetrics;
    fContext->getFontMetrics(&fontMetrics);
    serializer->write<SkFontMetrics>(fontMetrics);

    // Write glyphs images.
    serializer->emplace<uint64_t>(fPendingGlyphImages.size());
    for (const auto& glyphID : fPendingGlyphImages) {
        SkGlyph glyph{glyphID};
        fContext->getMetrics(&glyph);
        SkASSERT(SkMask::IsValidFormat(glyph.fMaskFormat));

        writeGlyph(&glyph, serializer);
        auto imageSize = glyph.computeImageSize();
        if (imageSize == 0u) continue;

        glyph.fImage = serializer->allocate(imageSize, glyph.formatAlignment());
        fContext->getImage(glyph);
        // TODO: Generating the image can change the mask format, do we need to update it in the
        // serialized glyph?
    }
    fPendingGlyphImages.clear();

    // Write glyphs paths.
    serializer->emplace<uint64_t>(fPendingGlyphPaths.size());
    for (const auto& glyphID : fPendingGlyphPaths) {
        SkGlyph glyph{glyphID};
        fContext->getMetrics(&glyph);
        SkASSERT(SkMask::IsValidFormat(glyph.fMaskFormat));

        writeGlyph(&glyph, serializer);
        writeGlyphPath(glyphID, serializer);
    }
    fPendingGlyphPaths.clear();
    this->resetScalerContext();
}

void SkStrikeServer::SkGlyphCacheState::ensureScalerContext() {
    if (fContext == nullptr) {
        fContext = fTypeface->createScalerContext(fEffects, fDescriptor.getDesc());
    }
}

void SkStrikeServer::SkGlyphCacheState::resetScalerContext() {
    fContext.reset();
    fTypeface = nullptr;
}

void SkStrikeServer::SkGlyphCacheState::setTypefaceAndEffects(
        const SkTypeface* typeface, SkScalerContextEffects effects) {
    fTypeface = typeface;
    fEffects = effects;
}

SkVector SkStrikeServer::SkGlyphCacheState::rounding() const {
    return SkStrikeCommon::PixelRounding(fIsSubpixel, fAxisAlignmentForHText);
}

// Note: In the split Renderer/GPU architecture, if getGlyphMetrics is called in the Renderer
// process, then it will be called on the GPU process because they share the rendering code. Any
// data that is created in the Renderer process needs to be found in the GPU process. By
// implication, any cache-miss/glyph-creation data needs to be sent to the GPU.
const SkGlyph& SkStrikeServer::SkGlyphCacheState::getGlyphMetrics(
        SkGlyphID glyphID, SkPoint position) {
    SkIPoint lookupPoint = SkStrikeCommon::SubpixelLookup(fAxisAlignmentForHText, position);
    SkPackedGlyphID packedGlyphID = fIsSubpixel ? SkPackedGlyphID{glyphID, lookupPoint}
                                                : SkPackedGlyphID{glyphID};

    // Check the cache for the glyph.
    SkGlyph* glyphPtr = fGlyphMap.findOrNull(packedGlyphID);

    // Has this glyph ever been seen before?
    if (glyphPtr == nullptr) {

        // Never seen before. Make a new glyph.
        glyphPtr = fAlloc.make<SkGlyph>(packedGlyphID);
        fGlyphMap.set(glyphPtr);
        this->ensureScalerContext();
        fContext->getMetrics(glyphPtr);

        // Make sure to send the glyph to the GPU because we always send the image for a glyph.
        fCachedGlyphImages.add(packedGlyphID);
        fPendingGlyphImages.push_back(packedGlyphID);
    }

    return *glyphPtr;
}

// Because the strike calls between the Renderer and the GPU are mirror images of each other, the
// information needed to make the call in the Renderer needs to be sent to the GPU so it can also
// make the call. If there is a path then it should be sent, and the path is queued to be sent and
// true returned. Otherwise, false is returned signaling an empty glyph.
//
// A key reason for no path is the fact that the glyph is a color image or is a bitmap only
// font.
void SkStrikeServer::SkGlyphCacheState::generatePath(const SkGlyph& glyph) {

    // Check to see if we have processed this glyph for a path before.
    if (glyph.fPathData == nullptr) {

        // Never checked for a path before. Add the path now.
        auto path = const_cast<SkGlyph&>(glyph).addPath(fContext.get(), &fAlloc);
        if (path != nullptr) {

            // A path was added make sure to send it to the GPU.
            fCachedGlyphPaths.add(glyph.getPackedID());
            fPendingGlyphPaths.push_back(glyph.getPackedID());
        }
    }
}

void SkStrikeServer::SkGlyphCacheState::writeGlyphPath(const SkPackedGlyphID& glyphID,
                                                       Serializer* serializer) const {
    SkPath path;
    if (!fContext->getPath(glyphID, &path)) {
        serializer->write<uint64_t>(0u);
        return;
    }

    size_t pathSize = path.writeToMemory(nullptr);
    serializer->write<uint64_t>(pathSize);
    path.writeToMemory(serializer->allocate(pathSize, kPathAlignment));
}


// Be sure to read and understand the comment for prepareForDrawing in SkStrikeInterface.h before
// working on this code.
SkSpan<const SkGlyphPos> SkStrikeServer::SkGlyphCacheState::prepareForDrawing(
        const SkGlyphID glyphIDs[],
        const SkPoint positions[],
        size_t n,
        int maxDimension,
        PreparationDetail detail,
        SkGlyphPos results[]) {

    for (size_t i = 0; i < n; i++) {
        SkPoint glyphPos = positions[i];
        SkGlyphID glyphID = glyphIDs[i];
        SkIPoint lookupPoint = SkStrikeCommon::SubpixelLookup(fAxisAlignmentForHText, glyphPos);
        SkPackedGlyphID packedGlyphID = fIsSubpixel ? SkPackedGlyphID{glyphID, lookupPoint}
                                                    : SkPackedGlyphID{glyphID};

        // Check the cache for the glyph.
        SkGlyph* glyphPtr = fGlyphMap.findOrNull(packedGlyphID);

        // Has this glyph ever been seen before?
        if (glyphPtr == nullptr) {

            // Never seen before. Make a new glyph.
            glyphPtr = fAlloc.make<SkGlyph>(packedGlyphID);
            fGlyphMap.set(glyphPtr);
            this->ensureScalerContext();
            fContext->getMetrics(glyphPtr);

            if (glyphPtr->maxDimension() <= maxDimension) {
                // do nothing
            } else if (glyphPtr->fMaskFormat != SkMask::kARGB32_Format) {

                // The glyph is too big for the atlas, but it is not color, so it is handled with a
                // path.
                if (glyphPtr->fPathData == nullptr) {

                    // Never checked for a path before. Add the path now.
                    const_cast<SkGlyph&>(*glyphPtr).addPath(fContext.get(), &fAlloc);

                    // Always send the path data, even if its not available, to make sure empty
                    // paths are not incorrectly assumed to be cache misses.
                    fCachedGlyphPaths.add(glyphPtr->getPackedID());
                    fPendingGlyphPaths.push_back(glyphPtr->getPackedID());
                }
            } else {

                // This will be handled by the fallback strike.
                SkASSERT(glyphPtr->maxDimension() > maxDimension
                         && glyphPtr->fMaskFormat == SkMask::kARGB32_Format);
            }

            // Make sure to send the glyph to the GPU because we always send the image for a glyph.
            fCachedGlyphImages.add(packedGlyphID);
            fPendingGlyphImages.push_back(packedGlyphID);
        }

        // Each glyph needs to be added as per the contract for prepareForDrawing.
        // TODO(herb): check if the empty glyphs need to be added here. They certainly need to be
        //             sent, but do the need to be processed by the painter?
        results[i] = {i, glyphPtr, glyphPos};
    }
    return SkSpan<const SkGlyphPos>{results, n};
}

// SkStrikeClient -----------------------------------------
class SkStrikeClient::DiscardableStrikePinner : public SkStrikePinner {
public:
    DiscardableStrikePinner(SkDiscardableHandleId discardableHandleId,
                            sk_sp<DiscardableHandleManager> manager)
            : fDiscardableHandleId(discardableHandleId), fManager(std::move(manager)) {}

    ~DiscardableStrikePinner() override = default;
    bool canDelete() override { return fManager->deleteHandle(fDiscardableHandleId); }

private:
    const SkDiscardableHandleId fDiscardableHandleId;
    sk_sp<DiscardableHandleManager> fManager;
};

SkStrikeClient::SkStrikeClient(sk_sp<DiscardableHandleManager> discardableManager,
                               bool isLogging,
                               SkStrikeCache* strikeCache)
        : fDiscardableHandleManager(std::move(discardableManager))
        , fStrikeCache{strikeCache ? strikeCache : SkStrikeCache::GlobalStrikeCache()}
        , fIsLogging{isLogging} {}

SkStrikeClient::~SkStrikeClient() = default;

#define READ_FAILURE                                                \
    {                                                               \
        SkDebugf("Bad font data serialization line: %d", __LINE__); \
        return false;                                               \
    }

// No need to read fForceBW because it is a flag private to SkScalerContext_DW, which will never
// be called on the GPU side.
static bool readGlyph(SkTLazy<SkGlyph>& glyph, Deserializer* deserializer) {
    SkPackedGlyphID glyphID;
    if (!deserializer->read<SkPackedGlyphID>(&glyphID)) return false;
    glyph.init(glyphID);
    if (!deserializer->read<float>(&glyph->fAdvanceX)) return false;
    if (!deserializer->read<float>(&glyph->fAdvanceY)) return false;
    if (!deserializer->read<uint16_t>(&glyph->fWidth)) return false;
    if (!deserializer->read<uint16_t>(&glyph->fHeight)) return false;
    if (!deserializer->read<int16_t>(&glyph->fTop)) return false;
    if (!deserializer->read<int16_t>(&glyph->fLeft)) return false;
    if (!deserializer->read<uint8_t>(&glyph->fMaskFormat)) return false;
    if (!SkMask::IsValidFormat(glyph->fMaskFormat)) return false;

    return true;
}

bool SkStrikeClient::readStrikeData(const volatile void* memory, size_t memorySize) {
    SkASSERT(memorySize != 0u);
    Deserializer deserializer(static_cast<const volatile char*>(memory), memorySize);

    uint64_t typefaceSize = 0u;
    if (!deserializer.read<uint64_t>(&typefaceSize)) READ_FAILURE

    for (size_t i = 0; i < typefaceSize; ++i) {
        WireTypeface wire;
        if (!deserializer.read<WireTypeface>(&wire)) READ_FAILURE

        // TODO(khushalsagar): The typeface no longer needs a reference to the
        // SkStrikeClient, since all needed glyphs must have been pushed before
        // raster.
        addTypeface(wire);
    }

    uint64_t strikeCount = 0u;
    if (!deserializer.read<uint64_t>(&strikeCount)) READ_FAILURE

    for (size_t i = 0; i < strikeCount; ++i) {
        bool has_glyphs = false;
        if (!deserializer.read<bool>(&has_glyphs)) READ_FAILURE

        if (!has_glyphs) continue;

        StrikeSpec spec;
        if (!deserializer.read<StrikeSpec>(&spec)) READ_FAILURE

        SkAutoDescriptor sourceAd;
        if (!deserializer.readDescriptor(&sourceAd)) READ_FAILURE

        SkFontMetrics fontMetrics;
        if (!deserializer.read<SkFontMetrics>(&fontMetrics)) READ_FAILURE

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

        auto strike = fStrikeCache->findStrikeExclusive(*client_desc);
        if (strike == nullptr) {
            // Note that we don't need to deserialize the effects since we won't be generating any
            // glyphs here anyway, and the desc is still correct since it includes the serialized
            // effects.
            SkScalerContextEffects effects;
            auto scaler = SkStrikeCache::CreateScalerContext(*client_desc, effects, *tf);
            strike = fStrikeCache->createStrikeExclusive(
                    *client_desc, std::move(scaler), &fontMetrics,
                    skstd::make_unique<DiscardableStrikePinner>(spec.discardableHandleId,
                                                                fDiscardableHandleManager));
            auto proxyContext = static_cast<SkScalerContextProxy*>(strike->getScalerContext());
            proxyContext->initCache(strike.get(), fStrikeCache);
        }

        uint64_t glyphImagesCount = 0u;
        if (!deserializer.read<uint64_t>(&glyphImagesCount)) READ_FAILURE
        for (size_t j = 0; j < glyphImagesCount; j++) {
            SkTLazy<SkGlyph> glyph;
            if (!readGlyph(glyph, &deserializer)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph->getPackedID());

            // Update the glyph unless it's already got an image (from fallback),
            // preserving any path that might be present.
            if (allocatedGlyph->fImage == nullptr) {
                auto* glyphPath = allocatedGlyph->fPathData;
                *allocatedGlyph = *glyph;
                allocatedGlyph->fPathData = glyphPath;
            }

            auto imageSize = glyph->computeImageSize();
            if (imageSize == 0u) continue;

            auto* image = deserializer.read(imageSize, glyph->formatAlignment());
            if (!image) READ_FAILURE

            // Don't overwrite the image if we already have one. We could have used a fallback if
            // the glyph was missing earlier.
            if (allocatedGlyph->fImage == nullptr) {
                strike->initializeImage(image, imageSize, allocatedGlyph);
            }
        }

        uint64_t glyphPathsCount = 0u;
        if (!deserializer.read<uint64_t>(&glyphPathsCount)) READ_FAILURE
        for (size_t j = 0; j < glyphPathsCount; j++) {
            SkTLazy<SkGlyph> glyph;
            if (!readGlyph(glyph, &deserializer)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph->getPackedID());

            // Update the glyph unless it's already got a path (from fallback),
            // preserving any image that might be present.
            if (allocatedGlyph->fPathData == nullptr) {
                auto* glyphImage = allocatedGlyph->fImage;
                *allocatedGlyph = *glyph;
                allocatedGlyph->fImage = glyphImage;
            }

            if (!read_path(&deserializer, allocatedGlyph, strike.get())) READ_FAILURE
        }
    }

    return true;
}

sk_sp<SkTypeface> SkStrikeClient::deserializeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len != sizeof(wire)) return nullptr;
    memcpy(&wire, buf, sizeof(wire));
    return this->addTypeface(wire);
}

sk_sp<SkTypeface> SkStrikeClient::addTypeface(const WireTypeface& wire) {
    auto* typeface = fRemoteFontIdToTypeface.find(wire.typefaceID);
    if (typeface) return *typeface;

    auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.typefaceID, wire.glyphCount, wire.style, wire.isFixed,
            fDiscardableHandleManager, fIsLogging);
    fRemoteFontIdToTypeface.set(wire.typefaceID, newTypeface);
    return std::move(newTypeface);
}

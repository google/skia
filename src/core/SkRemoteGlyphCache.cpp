/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <tuple>

#include "SkDevice.h"
#include "SkDraw.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphRun.h"
#include "SkPathEffect.h"
#include "SkRemoteGlyphCacheImpl.h"
#include "SkStrikeCache.h"
#include "SkTraceEvent.h"
#include "SkTypeface_remote.h"

#if SK_SUPPORT_GPU
#include "GrDrawOpAtlas.h"
#include "text/GrTextContext.h"
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

    // Path effect.
    {
        uint32_t size;
        auto ptr = source_desc->findEntry(kPathEffect_SkDescriptorTag, &size);
        if (ptr) desc->addEntry(kPathEffect_SkDescriptorTag, size, ptr);
    }

    // Mask filter.
    {
        uint32_t size;
        auto ptr = source_desc->findEntry(kMaskFilter_SkDescriptorTag, &size);
        if (ptr) desc->addEntry(kMaskFilter_SkDescriptorTag, size, ptr);
    }

    desc->computeChecksum();
    return desc;
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
        uint32_t desc_length = 0u;
        if (!read<uint32_t>(&desc_length)) return false;

        auto* result = this->ensureAtLeast(desc_length, alignof(SkDescriptor));
        if (!result) return false;

        ad->reset(desc_length);
        memcpy(ad->getDesc(), const_cast<const char*>(result), desc_length);
        return true;
    }

    const volatile void* read(size_t size, size_t alignment) {
      return this->ensureAtLeast(size, alignment);
    }

private:
    const volatile char* ensureAtLeast(size_t size, size_t alignment) {
        size_t padded = pad(fBytesRead, alignment);

        // Not enough data
        if (padded + size > fMemorySize) return nullptr;

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

bool read_path(Deserializer* deserializer, SkGlyph* glyph, SkGlyphCache* cache) {
    size_t pathSize = 0u;
    if (!deserializer->read<size_t>(&pathSize)) return false;

    if (pathSize == 0u) return true;

    auto* path = deserializer->read(pathSize, kPathAlignment);
    if (!path) return false;

    return cache->initializePath(glyph, path, pathSize);
}

#if SK_SUPPORT_GPU
SkScalar glyph_size_limit(const SkTextBlobCacheDiffCanvas::Settings& settings) {
    return GrGlyphCache::ComputeGlyphSizeLimit(settings.fMaxTextureSize, settings.fMaxTextureBytes);
}

void add_glyph_to_cache(SkStrikeServer::SkGlyphCacheState* cache, SkTypeface* tf,
                        const SkScalerContextEffects& effects, SkGlyphID glyphID) {
    SkASSERT(cache != nullptr);

    cache->addGlyph(SkPackedGlyphID(glyphID, 0, 0), false);
}

void add_fallback_text_to_cache(const GrTextContext::FallbackGlyphRunHelper& helper,
                                const SkSurfaceProps& props,
                                const SkMatrix& matrix,
                                const SkPaint& origPaint,
                                SkStrikeServer* server) {
    if (helper.fallbackText().empty()) {
        return;
    }

    TRACE_EVENT0("skia", "add_fallback_text_to_cache");
    SkPaint fallbackPaint{origPaint};
    SkScalar textRatio;
    SkMatrix fallbackMatrix = matrix;
    helper.initializeForDraw(&fallbackPaint, &textRatio, &fallbackMatrix);

    SkScalerContextEffects effects;
    auto* glyphCacheState =
            server->getOrCreateCache(fallbackPaint, &props, &fallbackMatrix,
                                     SkScalerContextFlags::kFakeGammaAndBoostContrast, &effects);

    for (auto glyphID : helper.fallbackText()) {
        add_glyph_to_cache(glyphCacheState, fallbackPaint.getTypeface(), effects, glyphID);
    }
}
#endif

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
        const SkTextBlobCacheDiffCanvas::Settings& settings)
    : SkNoPixelsDevice(bounds, props)
    , fStrikeServer(server)
    , fSettings(settings)
    , fPainter{props, kUnknown_SkColorType, SkScalerContextFlags::kFakeGammaAndBoostContrast} {
    SkASSERT(fStrikeServer);
}

SkBaseDevice* SkTextBlobCacheDiffCanvas::TrackLayerDevice::onCreateDevice(
        const CreateInfo& cinfo, const SkPaint*) {
    const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
    return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps, fStrikeServer, fSettings);
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::drawGlyphRunList(
        const SkGlyphRunList& glyphRunList) {
    for (auto& glyphRun : glyphRunList) {
        this->processGlyphRun(glyphRunList.origin(), glyphRun);
    }
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRun(
        const SkPoint& origin, const SkGlyphRun& glyphRun) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRun");

    const SkPaint& runPaint = glyphRun.paint();
    const SkMatrix& runMatrix = this->ctm();

    // If the matrix has perspective, we fall back to using distance field text or paths.
    #if SK_SUPPORT_GPU
    if (this->maybeProcessGlyphRunForDFT(glyphRun, runMatrix)) {
        return;
    } else
    #endif
    if (SkDraw::ShouldDrawTextAsPaths(runPaint, runMatrix)) {
        this->processGlyphRunForPaths(glyphRun, runMatrix);
    } else {
        this->processGlyphRunForMask(glyphRun, runMatrix, origin);
    }
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRunForMask(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix, SkPoint origin) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForMask");
    const SkPaint& runPaint = glyphRun.paint();

    SkScalerContextEffects effects;
    auto* glyphCacheState = fStrikeServer->getOrCreateCache(
            runPaint, &this->surfaceProps(), &runMatrix,
            SkScalerContextFlags::kFakeGammaAndBoostContrast, &effects);
    SkASSERT(glyphCacheState);

    auto perGlyph = [glyphCacheState] (const SkGlyph& glyph, SkPoint mappedPt) {
        glyphCacheState->addGlyph(glyph.getPackedID(), false);
    };

    fPainter.drawGlyphRunAsBMPWithPathFallback(
            glyphCacheState, glyphRun, origin, runMatrix, perGlyph, perGlyph);
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRunForPaths(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForPaths");
    const SkPaint& runPaint = glyphRun.paint();

    // The code below borrowed from GrTextContext::DrawBmpPosTextAsPaths.
    SkPaint pathPaint(runPaint);
#if SK_SUPPORT_GPU
    SkScalar matrixScale = pathPaint.setupForAsPaths();
    GrTextContext::FallbackGlyphRunHelper fallbackTextHelper(
            runMatrix, runPaint, glyph_size_limit(fSettings), matrixScale);
    const SkPoint emptyPosition{0, 0};
#else
    pathPaint.setupForAsPaths();
#endif

    pathPaint.setStyle(SkPaint::kFill_Style);
    pathPaint.setPathEffect(nullptr);

    SkScalerContextEffects effects;
    auto* glyphCacheState = fStrikeServer->getOrCreateCache(
            pathPaint, &this->surfaceProps(), nullptr,
            SkScalerContextFlags::kFakeGammaAndBoostContrast, &effects);

    const bool asPath = true;
    auto glyphs = glyphRun.shuntGlyphsIDs();
    for (uint32_t index = 0; index < glyphRun.runSize(); index++) {
        auto glyphID = glyphs[index];
#if SK_SUPPORT_GPU
        const auto& glyph =
                glyphCacheState->findGlyph(glyphID);
        if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
            // Note that we send data for the original glyph even in the case of fallback
            // since its glyph metrics will still be used on the client.
            fallbackTextHelper.appendGlyph(glyph, glyphID, emptyPosition);
        }
#endif
        glyphCacheState->addGlyph(glyphID, asPath);
    }

#if SK_SUPPORT_GPU
    add_fallback_text_to_cache(fallbackTextHelper, this->surfaceProps(), runMatrix, runPaint,
                               fStrikeServer);
#endif
}

#if SK_SUPPORT_GPU
bool SkTextBlobCacheDiffCanvas::TrackLayerDevice::maybeProcessGlyphRunForDFT(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::maybeProcessGlyphRunForDFT");

    const SkPaint& runPaint = glyphRun.paint();

    GrTextContext::Options options;
    options.fMinDistanceFieldFontSize = fSettings.fMinDistanceFieldFontSize;
    options.fMaxDistanceFieldFontSize = fSettings.fMaxDistanceFieldFontSize;
    GrTextContext::SanitizeOptions(&options);
    if (!GrTextContext::CanDrawAsDistanceFields(runPaint, runMatrix, this->surfaceProps(),
                                                fSettings.fContextSupportsDistanceFieldText,
                                                options)) {
        return false;
    }

    SkScalar textRatio;
    SkPaint dfPaint(runPaint);
    SkScalerContextFlags flags;
    GrTextContext::InitDistanceFieldPaint(nullptr, &dfPaint, runMatrix, options, &textRatio,
                                          &flags);
    SkScalerContextEffects effects;
    auto* glyphCacheState = fStrikeServer->getOrCreateCache(dfPaint, &this->surfaceProps(),
                                                            nullptr, flags, &effects);

    GrTextContext::FallbackGlyphRunHelper fallbackTextHelper(
            runMatrix, runPaint, glyph_size_limit(fSettings), textRatio);
    const bool asPath = false;
    const SkPoint emptyPosition{0, 0};
    auto glyphs = glyphRun.shuntGlyphsIDs();
    for (uint32_t index = 0; index < glyphRun.runSize(); index++) {
        auto glyphID = glyphs[index];
        const auto& glyph =
                glyphCacheState->findGlyph(glyphID);
        if (glyph.fMaskFormat != SkMask::kSDF_Format) {
            // Note that we send data for the original glyph even in the case of fallback
            // since its glyph metrics will still be used on the client.
            fallbackTextHelper.appendGlyph(glyph, glyphID, emptyPosition);
        }

        glyphCacheState->addGlyph(SkPackedGlyphID(glyphs[index]), asPath);
    }

    add_fallback_text_to_cache(fallbackTextHelper, this->surfaceProps(), runMatrix, runPaint,
                               fStrikeServer);
    return true;
}
#endif


// -- SkTextBlobCacheDiffCanvas -------------------------------------------------------------------
SkTextBlobCacheDiffCanvas::Settings::Settings() = default;
SkTextBlobCacheDiffCanvas::Settings::~Settings() = default;

SkTextBlobCacheDiffCanvas::SkTextBlobCacheDiffCanvas(int width, int height,
                                                     const SkMatrix& deviceMatrix,
                                                     const SkSurfaceProps& props,
                                                     SkStrikeServer* strikeServer,
                                                     Settings settings)
        : SkNoDrawCanvas{sk_make_sp<TrackLayerDevice>(SkIRect::MakeWH(width, height), props,
                                                      strikeServer, settings)} {}

SkTextBlobCacheDiffCanvas::~SkTextBlobCacheDiffCanvas() = default;

SkCanvas::SaveLayerStrategy SkTextBlobCacheDiffCanvas::getSaveLayerStrategy(
        const SaveLayerRec& rec) {
    return kFullLayer_SaveLayerStrategy;
}

void SkTextBlobCacheDiffCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                               const SkPaint& paint) {
    SkCanvas::onDrawTextBlob(blob, x, y, paint);
}

struct WireTypeface {
    WireTypeface() = default;
    WireTypeface(SkFontID typeface_id, int glyph_count, SkFontStyle style, bool is_fixed)
            : typefaceID(typeface_id), glyphCount(glyph_count), style(style), isFixed(is_fixed) {}

    // std::thread::id thread_id;  // TODO:need to figure a good solution
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
    WireTypeface wire(SkTypeface::UniqueID(tf), tf->countGlyphs(), tf->fontStyle(),
                      tf->isFixedPitch());
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

void SkStrikeServer::writeStrikeData(std::vector<uint8_t>* memory) {
    if (fLockedDescs.empty() && fTypefacesToSend.empty()) {
        return;
    }

    Serializer serializer(memory);
    serializer.emplace<size_t>(fTypefacesToSend.size());
    for (const auto& tf : fTypefacesToSend) serializer.write<WireTypeface>(tf);
    fTypefacesToSend.clear();

    serializer.emplace<size_t>(fLockedDescs.size());
    for (const auto* desc : fLockedDescs) {
        auto it = fRemoteGlyphStateMap.find(desc);
        SkASSERT(it != fRemoteGlyphStateMap.end());
        it->second->writePendingGlyphs(&serializer);
    }
    fLockedDescs.clear();
}

SkStrikeServer::SkGlyphCacheState* SkStrikeServer::getOrCreateCache(
        const SkPaint& paint,
        const SkSurfaceProps* props,
        const SkMatrix* matrix,
        SkScalerContextFlags flags,
        SkScalerContextEffects* effects) {
    SkScalerContextRec keyRec;
    SkScalerContext::MakeRecAndEffects(paint, props, matrix, flags, &keyRec, effects, false);
    auto keyDesc = SkScalerContext::DescriptorGivenRecAndEffects(keyRec, *effects);
    TRACE_EVENT1("skia", "RecForDesc", "rec", TRACE_STR_COPY(keyRec.dump().c_str()));

    // Already locked.
    if (fLockedDescs.find(keyDesc.get()) != fLockedDescs.end()) {
        auto it = fRemoteGlyphStateMap.find(keyDesc.get());
        SkASSERT(it != fRemoteGlyphStateMap.end());
        SkGlyphCacheState* cache = it->second.get();
        cache->ensureScalerContext(paint, props, matrix, flags, effects);
        return cache;
    }

    // Try to lock.
    auto it = fRemoteGlyphStateMap.find(keyDesc.get());
    if (it != fRemoteGlyphStateMap.end()) {
        SkGlyphCacheState* cache = it->second.get();
#ifdef SK_DEBUG
        SkScalerContextRec deviceRec;
        SkScalerContext::MakeRecAndEffects(paint, props, matrix, flags, &deviceRec, effects, true);
        auto deviceDesc = SkScalerContext::DescriptorGivenRecAndEffects(deviceRec, *effects);
        SkASSERT(cache->getDeviceDescriptor() == *deviceDesc);
#endif
        bool locked = fDiscardableHandleManager->lockHandle(it->second->discardableHandleId());
        if (locked) {
            fLockedDescs.insert(it->first);
            cache->ensureScalerContext(paint, props, matrix, flags, effects);
            return cache;
        }

        // If the lock failed, the entry was deleted on the client. Remove our
        // tracking.
        fRemoteGlyphStateMap.erase(it);
    }

    auto* tf = paint.getTypeface();
    const SkFontID typefaceId = tf->uniqueID();
    if (!fCachedTypefaces.contains(typefaceId)) {
        fCachedTypefaces.add(typefaceId);
        fTypefacesToSend.emplace_back(typefaceId, tf->countGlyphs(), tf->fontStyle(),
                                      tf->isFixedPitch());
    }

    // Create a new cache state and insert it into the map.
    auto* keyDescPtr = keyDesc.get();
    auto newHandle = fDiscardableHandleManager->createHandle();
    auto cacheState = skstd::make_unique<SkGlyphCacheState>(std::move(keyDesc), newHandle);
    auto* cacheStatePtr = cacheState.get();

    fLockedDescs.insert(keyDescPtr);
    fRemoteGlyphStateMap[keyDescPtr] = std::move(cacheState);
    cacheStatePtr->ensureScalerContext(paint, props, matrix, flags, effects);
    return cacheStatePtr;
}

// -- SkGlyphCacheState ----------------------------------------------------------------------------
SkStrikeServer::SkGlyphCacheState::SkGlyphCacheState(
        std::unique_ptr<SkDescriptor> keyDescriptor, uint32_t discardableHandleId)
        : fKeyDescriptor(std::move(keyDescriptor))
        , fDiscardableHandleId(discardableHandleId) {
    SkASSERT(fKeyDescriptor);
}

SkStrikeServer::SkGlyphCacheState::~SkGlyphCacheState() = default;

void SkStrikeServer::SkGlyphCacheState::addGlyph(SkPackedGlyphID glyph, bool asPath) {
    auto* cache = asPath ? &fCachedGlyphPaths : &fCachedGlyphImages;
    auto* pending = asPath ? &fPendingGlyphPaths : &fPendingGlyphImages;

    // Already cached.
    if (cache->contains(glyph)) {
        return;
    }

    // Serialize and cache. Also create the scalar context to use when serializing
    // this glyph.
    cache->add(glyph);
    pending->push_back(glyph);
}

void SkStrikeServer::SkGlyphCacheState::writePendingGlyphs(Serializer* serializer) {
    // TODO(khushalsagar): Write a strike only if it has any pending glyphs.
    serializer->emplace<bool>(this->hasPendingGlyphs());
    if (!this->hasPendingGlyphs()) {
        fContext.reset();
        return;
    }

    // Write the desc.
    serializer->emplace<StrikeSpec>(fContext->getTypeface()->uniqueID(), fDiscardableHandleId);
    serializer->writeDescriptor(*fKeyDescriptor.get());

    // Write FontMetrics.
    // TODO(khushalsagar): Do we need to re-send each time?
    SkPaint::FontMetrics fontMetrics;
    fContext->getFontMetrics(&fontMetrics);
    serializer->write<SkPaint::FontMetrics>(fontMetrics);

    // Write glyphs images.
    serializer->emplace<size_t>(fPendingGlyphImages.size());
    for (const auto& glyphID : fPendingGlyphImages) {
        SkGlyph stationaryGlyph;
        {
            auto glyph = serializer->emplace<SkGlyph>();
            glyph->initWithGlyphID(glyphID);
            fContext->getMetrics(glyph);
            glyph->fPathData = nullptr;
            glyph->fImage = nullptr;

            // Since the allocate can move glyph, make one that stays in one place.
            stationaryGlyph = *glyph;
        }

        // Glyphs which are too large for the atlas still request images when computing the bounds
        // for the glyph, which is why its necessary to send both. See related code in
        // get_packed_glyph_bounds in GrGlyphCache.cpp and crbug.com/510931.
        bool tooLargeForAtlas = false;
#if SK_SUPPORT_GPU
        tooLargeForAtlas = GrDrawOpAtlas::GlyphTooLargeForAtlas(stationaryGlyph.fWidth,
                                                                stationaryGlyph.fHeight);
#endif
        if (tooLargeForAtlas) {
            // Add this to the path cache, since we will always fall back to using paths
            // for this glyph.
            fCachedGlyphPaths.add(glyphID);
            writeGlyphPath(glyphID, serializer);
        }

        auto imageSize = stationaryGlyph.computeImageSize();
        if (imageSize == 0u) continue;

        stationaryGlyph.fImage = serializer->allocate(imageSize, stationaryGlyph.formatAlignment());
        fContext->getImage(stationaryGlyph);
        // TODO: Generating the image can change the mask format, do we need to update it in the
        // serialized glyph?
    }
    fPendingGlyphImages.clear();

    // Write glyphs paths.
    serializer->emplace<size_t>(fPendingGlyphPaths.size());
    for (const auto& glyphID : fPendingGlyphPaths) {
        {
            auto glyph = serializer->emplace<SkGlyph>();
            glyph->initWithGlyphID(glyphID);
            fContext->getMetrics(glyph);
            glyph->fPathData = nullptr;
            glyph->fImage = nullptr;
        }
        writeGlyphPath(glyphID, serializer);
    }
    fPendingGlyphPaths.clear();
    fContext.reset();
}

const SkGlyph& SkStrikeServer::SkGlyphCacheState::findGlyph(SkPackedGlyphID glyphID) {
    auto* glyph = fGlyphMap.find(glyphID);
    if (glyph == nullptr) {
        glyph = fGlyphMap.set(glyphID, SkGlyph());
        glyph->initWithGlyphID(glyphID);
        fContext->getMetrics(glyph);
    }

    return *glyph;
}

void SkStrikeServer::SkGlyphCacheState::ensureScalerContext(
        const SkPaint& paint,
        const SkSurfaceProps* props,
        const SkMatrix* matrix,
        SkScalerContextFlags flags,
        SkScalerContextEffects* effects) {
    if (fContext == nullptr || fDeviceDescriptor == nullptr) {
        SkScalerContextRec deviceRec;
        SkScalerContext::MakeRecAndEffects(paint, props, matrix, flags, &deviceRec, effects, true);
        auto deviceDesc = SkScalerContext::DescriptorGivenRecAndEffects(deviceRec, *effects);
        auto* tf = paint.getTypeface();
        fContext = tf->createScalerContext(*effects, deviceDesc.get(), false);
        fIsSubpixel = fContext->isSubpixel();
        fAxisAlignmentForHText = fContext->computeAxisAlignmentForHText();
        fDeviceDescriptor = std::move(deviceDesc);
    }
}

SkVector SkStrikeServer::SkGlyphCacheState::rounding() const {
    return SkGlyphCacheCommon::PixelRounding(fIsSubpixel, fAxisAlignmentForHText);
}

const SkGlyph& SkStrikeServer::SkGlyphCacheState::getGlyphMetrics(
        SkGlyphID glyphID, SkPoint position) {
    SkIPoint lookupPoint = SkGlyphCacheCommon::SubpixelLookup(fAxisAlignmentForHText, position);
    SkPackedGlyphID packedGlyphID = fIsSubpixel ? SkPackedGlyphID{glyphID, lookupPoint}
                                                : SkPackedGlyphID{glyphID};

    return this->findGlyph(packedGlyphID);
}

void SkStrikeServer::SkGlyphCacheState::writeGlyphPath(const SkPackedGlyphID& glyphID,
                                                       Serializer* serializer) const {
    SkPath path;
    if (!fContext->getPath(glyphID, &path)) {
        serializer->write<size_t>(0u);
        return;
    }

    size_t pathSize = path.writeToMemory(nullptr);
    serializer->write<size_t>(pathSize);
    path.writeToMemory(serializer->allocate(pathSize, kPathAlignment));
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

#define READ_FAILURE                      \
    {                                     \
        SkDEBUGFAIL("Bad serialization"); \
        return false;                     \
    }

bool SkStrikeClient::readStrikeData(const volatile void* memory, size_t memorySize) {
    SkASSERT(memorySize != 0u);
    Deserializer deserializer(static_cast<const volatile char*>(memory), memorySize);

    size_t typefaceSize = 0u;
    if (!deserializer.read<size_t>(&typefaceSize)) READ_FAILURE

    for (size_t i = 0; i < typefaceSize; ++i) {
        WireTypeface wire;
        if (!deserializer.read<WireTypeface>(&wire)) READ_FAILURE

        // TODO(khushalsagar): The typeface no longer needs a reference to the
        // SkStrikeClient, since all needed glyphs must have been pushed before
        // raster.
        addTypeface(wire);
    }

    size_t strikeCount = 0u;
    if (!deserializer.read<size_t>(&strikeCount)) READ_FAILURE

    for (size_t i = 0; i < strikeCount; ++i) {
        bool has_glyphs = false;
        if (!deserializer.read<bool>(&has_glyphs)) READ_FAILURE

        if (!has_glyphs) continue;

        StrikeSpec spec;
        if (!deserializer.read<StrikeSpec>(&spec)) READ_FAILURE

        SkAutoDescriptor sourceAd;
        if (!deserializer.readDescriptor(&sourceAd)) READ_FAILURE

        SkPaint::FontMetrics fontMetrics;
        if (!deserializer.read<SkPaint::FontMetrics>(&fontMetrics)) READ_FAILURE

        // Get the local typeface from remote fontID.
        auto* tf = fRemoteFontIdToTypeface.find(spec.typefaceID)->get();
        // Received strikes for a typeface which doesn't exist.
        if (!tf) READ_FAILURE

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

        size_t glyphImagesCount = 0u;
        if (!deserializer.read<size_t>(&glyphImagesCount)) READ_FAILURE
        for (size_t j = 0; j < glyphImagesCount; j++) {
            SkGlyph glyph;
            if (!deserializer.read<SkGlyph>(&glyph)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph.getPackedID());

            // Update the glyph unless it's already got an image (from fallback),
            // preserving any path that might be present.
            if (allocatedGlyph->fImage == nullptr) {
                auto* glyphPath = allocatedGlyph->fPathData;
                *allocatedGlyph = glyph;
                allocatedGlyph->fPathData = glyphPath;
            }

            bool tooLargeForAtlas = false;
#if SK_SUPPORT_GPU
            tooLargeForAtlas = GrDrawOpAtlas::GlyphTooLargeForAtlas(glyph.fWidth, glyph.fHeight);
#endif
            if (tooLargeForAtlas) {
                if (!read_path(&deserializer, allocatedGlyph, strike.get())) READ_FAILURE
            }

            auto imageSize = glyph.computeImageSize();
            if (imageSize == 0u) continue;

            auto* image = deserializer.read(imageSize, allocatedGlyph->formatAlignment());
            if (!image) READ_FAILURE
            strike->initializeImage(image, imageSize, allocatedGlyph);
        }

        size_t glyphPathsCount = 0u;
        if (!deserializer.read<size_t>(&glyphPathsCount)) READ_FAILURE
        for (size_t j = 0; j < glyphPathsCount; j++) {
            SkGlyph glyph;
            if (!deserializer.read<SkGlyph>(&glyph)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph.getPackedID());

            // Update the glyph unless it's already got a path (from fallback),
            // preserving any image that might be present.
            if (allocatedGlyph->fPathData == nullptr) {
                auto* glyphImage = allocatedGlyph->fImage;
                *allocatedGlyph = glyph;
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

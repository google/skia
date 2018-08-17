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
#include "SkPathEffect.h"
#include "SkStrikeCache.h"
#include "SkTextBlobRunIterator.h"
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
                        const SkScalerContextEffects& effects, const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    cache->addGlyph(tf, effects, SkPackedGlyphID(glyphID, 0, 0), false);
}

void add_fallback_text_to_cache(const GrTextContext::FallbackTextHelper& helper,
                                const SkSurfaceProps& props,
                                const SkMatrix& matrix,
                                const SkPaint& origPaint,
                                SkStrikeServer* server) {
    if (!helper.fallbackText().count()) return;

    TRACE_EVENT0("skia", "add_fallback_text_to_cache");
    SkPaint fallbackPaint{origPaint};
    SkScalar textRatio;
    SkMatrix fallbackMatrix = matrix;
    helper.initializeForDraw(&fallbackPaint, &textRatio, &fallbackMatrix);

    SkScalerContextRec deviceSpecificRec;
    SkScalerContextEffects effects;
    auto* glyphCacheState = server->getOrCreateCache(
            fallbackPaint, &props, &fallbackMatrix,
            SkScalerContextFlags::kFakeGammaAndBoostContrast, &deviceSpecificRec, &effects);

    const char* text = helper.fallbackText().begin();
    const char* stop = text + helper.fallbackText().count();
    while (text < stop) {
        add_glyph_to_cache(glyphCacheState, fallbackPaint.getTypeface(), effects, &text);
    }
}
#endif

size_t SkDescriptorMapOperators::operator()(const SkDescriptor* key) const {
    return key->getChecksum();
}

bool SkDescriptorMapOperators::operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
    return *lhs == *rhs;
}

// -- TrackLayerDevice -----------------------------------------------------------------------------
#define FAIL_AND_RETURN                         \
    SkDEBUGFAIL("Failed to process glyph run"); \
    return;

class SkTextBlobCacheDiffCanvas::TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props, SkStrikeServer* server,
                     const SkTextBlobCacheDiffCanvas::Settings& settings)
            : SkNoPixelsDevice(bounds, props), fStrikeServer(server), fSettings(settings) {
        SkASSERT(fStrikeServer);
    }
    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps, fStrikeServer,
                                    fSettings);
    }

protected:
    void drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                      const SkPaint& paint) override {
        // The looper should be applied by the SkCanvas.
        SkASSERT(paint.getDrawLooper() == nullptr);

        SkPoint position{x, y};
        SkPaint runPaint{paint};
        SkTextBlobRunIterator it(blob);
        for (; !it.done(); it.next()) {
            // applyFontToPaint() always overwrites the exact same attributes,
            // so it is safe to not re-seed the paint for this reason.
            it.applyFontToPaint(&runPaint);
            this->processGlyphRun(position, it, runPaint);
        }
    }

    void drawGlyphRunList(SkGlyphRunList* glyphRunList) override {
        auto blob = glyphRunList->blob();

        SkASSERT(blob != nullptr);

        if (blob != nullptr) {
            auto origin = glyphRunList->origin();
            auto paint = glyphRunList->paint();
            this->drawTextBlob(blob, origin.x(), origin.y(), paint);
        }
    }


private:
    void processGlyphRun(const SkPoint& position,
                         const SkTextBlobRunIterator& it,
                         const SkPaint& runPaint) {
        TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRun");

        if (runPaint.getTextEncoding() != SkPaint::TextEncoding::kGlyphID_TextEncoding) {
            TRACE_EVENT0("skia", "kGlyphID_TextEncoding");
            FAIL_AND_RETURN
        }

        // All other alignment modes need the glyph advances. Use the slow drawing mode.
        if (runPaint.getTextAlign() != SkPaint::kLeft_Align) {
            TRACE_EVENT0("skia", "kLeft_Align");
            FAIL_AND_RETURN
        }

        if (it.positioning() == SkTextBlob::kDefault_Positioning) {
            // Default positioning needs advances. Can't do that.
            TRACE_EVENT0("skia", "kDefault_Positioning");
            FAIL_AND_RETURN
        }

        const SkMatrix& runMatrix = this->ctm();
#if SK_SUPPORT_GPU
        if (this->processGlyphRunForDFT(it, runPaint, runMatrix)) {
            return;
        }
#endif

        // If the matrix has perspective, we fall back to using distance field text or paths.
        if (SkDraw::ShouldDrawTextAsPaths(runPaint, runMatrix)) {
            this->processGlyphRunForPaths(it, runPaint, runMatrix);
            return;
        }

        using PosFn = SkPoint (*)(int index, const SkScalar* pos);
        PosFn posFn;
        SkSTArenaAlloc<120> arena;
        SkFindAndPlaceGlyph::MapperInterface* mapper = nullptr;
        switch (it.positioning()) {
            case SkTextBlob::kHorizontal_Positioning:
                posFn = [](int index, const SkScalar* pos) { return SkPoint{pos[index], 0}; };
                mapper = SkFindAndPlaceGlyph::CreateMapper(
                        runMatrix, SkPoint::Make(position.x(), position.y() + it.offset().y()), 1,
                        &arena);
                break;
            case SkTextBlob::kFull_Positioning:
                posFn = [](int index, const SkScalar* pos) {
                    return SkPoint{pos[2 * index], pos[2 * index + 1]};
                };
                mapper = SkFindAndPlaceGlyph::CreateMapper(runMatrix, position, 2, &arena);
                break;
            default:
                posFn = nullptr;
                SK_ABORT("unhandled positioning mode");
        }

        SkScalerContextRec deviceSpecificRec;
        SkScalerContextEffects effects;
        auto* glyphCacheState = fStrikeServer->getOrCreateCache(
                runPaint, &this->surfaceProps(), &runMatrix,
                SkScalerContextFlags::kFakeGammaAndBoostContrast, &deviceSpecificRec, &effects);
        SkASSERT(glyphCacheState);

        const bool asPath = false;
        bool isSubpixel =
                SkToBool(deviceSpecificRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag);
        SkAxisAlignment axisAlignment = deviceSpecificRec.computeAxisAlignmentForHText();
        auto pos = it.pos();
        const uint16_t* glyphs = it.glyphs();
        for (uint32_t index = 0; index < it.glyphCount(); index++) {
            SkIPoint subPixelPos{0, 0};
            if (isSubpixel) {
                SkPoint glyphPos = mapper->map(posFn(index, pos));
                subPixelPos = SkFindAndPlaceGlyph::SubpixelAlignment(axisAlignment, glyphPos);
            }

            glyphCacheState->addGlyph(
                    runPaint.getTypeface(),
                    effects,
                    SkPackedGlyphID(glyphs[index], subPixelPos.x(), subPixelPos.y()),
                    asPath);
        }
    }

    void processGlyphRunForPaths(const SkTextBlobRunIterator& it,
                                 const SkPaint& runPaint,
                                 const SkMatrix& runMatrix) {
        TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForPaths");

        // The code below borrowed from GrTextContext::DrawBmpPosTextAsPaths.
        SkPaint pathPaint(runPaint);
#if SK_SUPPORT_GPU
        SkScalar matrixScale = pathPaint.setupForAsPaths();
        GrTextContext::FallbackTextHelper fallbackTextHelper(
                runMatrix, runPaint, glyph_size_limit(fSettings), matrixScale);
        const SkPoint emptyPosition{0, 0};
#else
        pathPaint.setupForAsPaths();
#endif

        pathPaint.setStyle(SkPaint::kFill_Style);
        pathPaint.setPathEffect(nullptr);

        SkScalerContextRec deviceSpecificRec;
        SkScalerContextEffects effects;
        auto* glyphCacheState = fStrikeServer->getOrCreateCache(
                pathPaint, &this->surfaceProps(), nullptr,
                SkScalerContextFlags::kFakeGammaAndBoostContrast, &deviceSpecificRec, &effects);

        const bool asPath = true;
        const SkIPoint subPixelPos{0, 0};
        const uint16_t* glyphs = it.glyphs();
        for (uint32_t index = 0; index < it.glyphCount(); index++) {
            auto glyphID = SkPackedGlyphID(glyphs[index], subPixelPos.x(), subPixelPos.y());
#if SK_SUPPORT_GPU
            const auto& glyph =
                    glyphCacheState->findGlyph(runPaint.getTypeface(), effects, glyphID);
            if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
                // Note that we send data for the original glyph even in the case of fallback
                // since its glyph metrics will still be used on the client.
                fallbackTextHelper.appendText(glyph, sizeof(uint16_t), (const char*)&glyphs[index],
                                              emptyPosition);
            }
#endif
            glyphCacheState->addGlyph(runPaint.getTypeface(), effects, glyphID, asPath);
        }

#if SK_SUPPORT_GPU
        add_fallback_text_to_cache(fallbackTextHelper, this->surfaceProps(), runMatrix, runPaint,
                                   fStrikeServer);
#endif
    }

#if SK_SUPPORT_GPU
    bool processGlyphRunForDFT(const SkTextBlobRunIterator& it,
                               const SkPaint& runPaint,
                               const SkMatrix& runMatrix) {
        TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForDFT");

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
        SkScalerContextRec deviceSpecificRec;
        SkScalerContextEffects effects;
        auto* glyphCacheState = fStrikeServer->getOrCreateCache(
                dfPaint, &this->surfaceProps(), nullptr, flags, &deviceSpecificRec, &effects);

        GrTextContext::FallbackTextHelper fallbackTextHelper(
                runMatrix, runPaint, glyph_size_limit(fSettings), textRatio);
        const bool asPath = false;
        const SkIPoint subPixelPos{0, 0};
        const SkPoint emptyPosition{0, 0};
        const uint16_t* glyphs = it.glyphs();
        for (uint32_t index = 0; index < it.glyphCount(); index++) {
            auto glyphID = SkPackedGlyphID(glyphs[index], subPixelPos.x(), subPixelPos.y());
            const auto& glyph =
                    glyphCacheState->findGlyph(runPaint.getTypeface(), effects, glyphID);
            if (glyph.fMaskFormat != SkMask::kSDF_Format) {
                // Note that we send data for the original glyph even in the case of fallback
                // since its glyph metrics will still be used on the client.
                fallbackTextHelper.appendText(glyph, sizeof(uint16_t), (const char*)&glyphs[index],
                                              emptyPosition);
            }

            glyphCacheState->addGlyph(
                    runPaint.getTypeface(),
                    effects,
                    SkPackedGlyphID(glyphs[index], subPixelPos.x(), subPixelPos.y()),
                    asPath);
        }

        add_fallback_text_to_cache(fallbackTextHelper, this->surfaceProps(), runMatrix, runPaint,
                                   fStrikeServer);
        return true;
    }
#endif

    SkStrikeServer* const fStrikeServer;
    const SkTextBlobCacheDiffCanvas::Settings fSettings;
};

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

struct StrikeSpec {
    StrikeSpec() {}
    StrikeSpec(SkFontID typefaceID_, SkDiscardableHandleId discardableHandleId_)
            : typefaceID{typefaceID_}, discardableHandleId(discardableHandleId_) {}
    SkFontID typefaceID = 0u;
    SkDiscardableHandleId discardableHandleId = 0u;
    /* desc */
    /* n X (glyphs ids) */
};

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
    if (fLockedDescs.empty() && fTypefacesToSend.empty()) return;

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
        SkScalerContextRec* deviceRec,
        SkScalerContextEffects* effects) {
    SkScalerContextRec keyRec;
    SkScalerContext::MakeRecAndEffects(paint, props, matrix, flags, deviceRec, effects, true);
    SkScalerContext::MakeRecAndEffects(paint, props, matrix, flags, &keyRec, effects, false);
    TRACE_EVENT1("skia", "RecForDesc", "rec", TRACE_STR_COPY(keyRec.dump().c_str()));

    // TODO: possible perf improvement - don't recompute the device desc on cache hit.
    auto deviceDesc = SkScalerContext::DescriptorGivenRecAndEffects(*deviceRec, *effects);
    auto keyDesc = SkScalerContext::DescriptorGivenRecAndEffects(keyRec, *effects);

    // Already locked.
    if (fLockedDescs.find(keyDesc.get()) != fLockedDescs.end()) {
        auto it = fRemoteGlyphStateMap.find(keyDesc.get());
        SkASSERT(it != fRemoteGlyphStateMap.end());
        return it->second.get();
    }

    // Try to lock.
    auto it = fRemoteGlyphStateMap.find(keyDesc.get());
    if (it != fRemoteGlyphStateMap.end()) {
        SkASSERT(it->second->getDeviceDescriptor() == *deviceDesc);
        bool locked = fDiscardableHandleManager->lockHandle(it->second->discardableHandleId());
        if (locked) {
            fLockedDescs.insert(it->first);
            return it->second.get();
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

    auto* keyDescPtr = keyDesc.get();
    auto newHandle = fDiscardableHandleManager->createHandle();
    auto cacheState = skstd::make_unique<SkGlyphCacheState>(std::move(deviceDesc),
                                                            std::move(keyDesc), newHandle);
    auto* cacheStatePtr = cacheState.get();

    fLockedDescs.insert(keyDescPtr);
    fRemoteGlyphStateMap[keyDescPtr] = std::move(cacheState);
    return cacheStatePtr;
}

SkStrikeServer::SkGlyphCacheState::SkGlyphCacheState(std::unique_ptr<SkDescriptor> deviceDescriptor,
                                                     std::unique_ptr<SkDescriptor>
                                                             keyDescriptor,
                                                     uint32_t discardableHandleId)
        : fDeviceDescriptor(std::move(deviceDescriptor))
        , fKeyDescriptor(std::move(keyDescriptor))
        , fDiscardableHandleId(discardableHandleId) {
    SkASSERT(fDeviceDescriptor);
    SkASSERT(fKeyDescriptor);
}

SkStrikeServer::SkGlyphCacheState::~SkGlyphCacheState() = default;

void SkStrikeServer::SkGlyphCacheState::addGlyph(SkTypeface* typeface,
                                                 const SkScalerContextEffects& effects,
                                                 SkPackedGlyphID glyph,
                                                 bool asPath) {
    auto* cache = asPath ? &fCachedGlyphPaths : &fCachedGlyphImages;
    auto* pending = asPath ? &fPendingGlyphPaths : &fPendingGlyphImages;

    // Already cached.
    if (cache->contains(glyph)) return;

    // Serialize and cache. Also create the scalar context to use when serializing
    // this glyph.
    cache->add(glyph);
    pending->push_back(glyph);
    if (!fContext) {
        fContext = typeface->createScalerContext(effects, fDeviceDescriptor.get(), false);
    }
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

    // Note that we reset the context after serializing pending glyphs since we
    // don't want to extend the lifetime of the typeface.
    fContext.reset();
}

const SkGlyph& SkStrikeServer::SkGlyphCacheState::findGlyph(SkTypeface* tf,
                                                            const SkScalerContextEffects& effects,
                                                            SkPackedGlyphID glyphID) {
    auto* glyph = fGlyphMap.find(glyphID);
    if (glyph) return *glyph;

    glyph = fGlyphMap.set(glyphID, SkGlyph());
    glyph->initWithGlyphID(glyphID);
    if (!fContext) {
        fContext = tf->createScalerContext(effects, fDeviceDescriptor.get(), false);
    }
    fContext->getMetrics(glyph);
    return *glyph;
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

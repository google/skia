/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

#include <iterator>
#include <memory>
#include <string>
#include <tuple>

#include "SkDevice.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkStrikeCache.h"
#include "SkTextBlobRunIterator.h"
#include "SkTraceEvent.h"
#include "SkTypeface_remote.h"

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

template <typename T>
class ArraySlice final : public std::tuple<const T*, size_t> {
public:
    // Additional constructors as needed.
    ArraySlice(const T* data, size_t size) : fData{data}, fSize{size} { }
    ArraySlice() : ArraySlice<T>(nullptr, 0) { }

    const T* begin() {
        return this->data();
    }
    const T* end() {
        return &this->data()[this->size()];
    }

    const T* data() const {
        return fData;
    }

    size_t size() const {
        return fSize;
    }

private:
    const T* fData;
    size_t   fSize;
};

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

    template <typename T>
    T* allocateArray(int count) {
        auto result = allocate(sizeof(T) * count, alignof(T));
        return new (result) T[count];
    }

private:
    void* allocate(size_t size, size_t alignment) {
        size_t aligned = pad(fBuffer->size(), alignment);
        fBuffer->resize(aligned + size);
        return &(*fBuffer)[aligned];
    }

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

    template <typename T>
    ArraySlice<T> readArray(int count) {
        size_t size = count * sizeof(T);
        const T* base = (const T*)this->ensureAtLeast(size, alignof(T));
        if (!base) return ArraySlice<T>();

        ArraySlice<T> result = ArraySlice<T>{base, (uint32_t)count};
        return result;
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

size_t SkDescriptorMapOperators::operator()(const SkDescriptor* key) const {
    return key->getChecksum();
    }

    bool SkDescriptorMapOperators::operator()(const SkDescriptor* lhs,
                                              const SkDescriptor* rhs) const {
        return *lhs == *rhs;
    }

// -- TrackLayerDevice -----------------------------------------------------------------------------
class TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props)
            : SkNoPixelsDevice(bounds, props) { }
    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps);
    }
};

// -- SkTextBlobCacheDiffCanvas -------------------------------------------------------------------
SkTextBlobCacheDiffCanvas::SkTextBlobCacheDiffCanvas(int width, int height,
                                                     const SkMatrix& deviceMatrix,
                                                     const SkSurfaceProps& props,
                                                     SkStrikeServer* strikeSever)
        : SkNoDrawCanvas{sk_make_sp<TrackLayerDevice>(SkIRect::MakeWH(width, height), props)}
        , fDeviceMatrix{deviceMatrix}
        , fSurfaceProps{props}
        , fStrikeServer{strikeSever} {
    SkASSERT(fStrikeServer);
}

SkTextBlobCacheDiffCanvas::~SkTextBlobCacheDiffCanvas() = default;

SkCanvas::SaveLayerStrategy SkTextBlobCacheDiffCanvas::getSaveLayerStrategy(
    const SaveLayerRec&rec)
{
    return kFullLayer_SaveLayerStrategy;
}

void SkTextBlobCacheDiffCanvas::onDrawTextBlob(
        const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) {
    SkPoint position{x, y};

    SkPaint runPaint{paint};
    SkTextBlobRunIterator it(blob);
        for (;!it.done(); it.next()) {
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);
        if (auto looper = runPaint.getLooper()) {
            this->processLooper(position, it, runPaint, looper);
        } else {
            this->processGlyphRun(position, it, runPaint);
        }
    }
}

void SkTextBlobCacheDiffCanvas::processLooper(
        const SkPoint& position,
        const SkTextBlobRunIterator& it,
        const SkPaint& origPaint,
        SkDrawLooper* looper)
{
    SkSTArenaAlloc<48> alloc;
    auto context = looper->makeContext(this, &alloc);
    SkPaint runPaint = origPaint;
    while (context->next(this, &runPaint)) {
        this->save();
        this->processGlyphRun(position, it, runPaint);
        this->restore();
        runPaint = origPaint;
    }
}

#define FAIL_AND_RETURN                         \
    SkDEBUGFAIL("Failed to process glyph run"); \
    return;

void SkTextBlobCacheDiffCanvas::processGlyphRun(
        const SkPoint& position,
        const SkTextBlobRunIterator& it,
        const SkPaint& runPaint)
{

    if (runPaint.getTextEncoding() != SkPaint::TextEncoding::kGlyphID_TextEncoding) {
        TRACE_EVENT0("skia", "kGlyphID_TextEncoding");
        FAIL_AND_RETURN
    }

    // All other alignment modes need the glyph advances. Use the slow drawing mode.
    if (runPaint.getTextAlign() != SkPaint::kLeft_Align) {
        TRACE_EVENT0("skia", "kLeft_Align");
        FAIL_AND_RETURN
    }

    using PosFn = SkPoint(*)(int index, const SkScalar* pos);
    PosFn posFn;
    switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning: {
            // Default positioning needs advances. Can't do that.
            TRACE_EVENT0("skia", "kDefault_Positioning");
            FAIL_AND_RETURN
        }

        case SkTextBlob::kHorizontal_Positioning:
            posFn = [](int index, const SkScalar* pos) {
                return SkPoint{pos[index], 0};
            };

            break;

        case SkTextBlob::kFull_Positioning:
            posFn = [](int index, const SkScalar* pos) {
                return SkPoint{pos[2 * index], pos[2 * index + 1]};
            };
            break;

        default:
            posFn = nullptr;
            SK_ABORT("unhandled positioning mode");
    }

    SkMatrix blobMatrix{fDeviceMatrix};
    blobMatrix.preConcat(this->getTotalMatrix());
    if (blobMatrix.hasPerspective()) {
        TRACE_EVENT0("skia", "hasPerspective");
        FAIL_AND_RETURN
    }
    blobMatrix.preTranslate(position.x(), position.y());

    SkMatrix runMatrix{blobMatrix};
    runMatrix.preTranslate(it.offset().x(), it.offset().y());

    using MapFn = SkPoint(*)(const SkMatrix& m, SkPoint pt);
    MapFn mapFn;
    switch ((int)runMatrix.getType()) {
        case SkMatrix::kIdentity_Mask:
        case SkMatrix::kTranslate_Mask:
            mapFn = [](const SkMatrix& m, SkPoint pt) {
                pt.offset(m.getTranslateX(), m.getTranslateY());
                return pt;
            };
            break;
        case SkMatrix::kScale_Mask:
        case SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask:
            mapFn = [](const SkMatrix& m, SkPoint pt) {
                return SkPoint{pt.x() * m.getScaleX() + m.getTranslateX(),
                               pt.y() * m.getScaleY() + m.getTranslateY()};
            };
            break;
        case SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask:
        case SkMatrix::kAffine_Mask | SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask:
            mapFn = [](const SkMatrix& m, SkPoint pt) {
                return SkPoint{
                        pt.x() * m.getScaleX() + pt.y() * m.getSkewX() + m.getTranslateX(),
                        pt.x() * m.getSkewY() + pt.y() * m.getScaleY() + m.getTranslateY()};
            };
            break;
        default:
            mapFn = nullptr;
            SK_ABORT("Bad matrix.");
    }

    SkScalerContextRec rec;
    SkScalerContextEffects effects;

    // TODO(crbug.com/831354): The typeface proxy on the client does not replicate the
    // filtering done by the typeface on the server.
    const bool enableTypefaceFiltering = false;
    SkScalerContext::MakeRecAndEffects(runPaint, &fSurfaceProps, &runMatrix,
                                       SkScalerContextFlags::kFakeGammaAndBoostContrast, &rec,
                                       &effects, enableTypefaceFiltering);

    TRACE_EVENT1("skia", "RecForDesc", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    auto desc = SkScalerContext::DescriptorGivenRecAndEffects(rec, effects);
    auto* glyphCacheState = static_cast<SkStrikeServer*>(fStrikeServer)
                                    ->getOrCreateCache(runPaint.getTypeface(), std::move(desc));
    SkASSERT(glyphCacheState);

    bool isSubpixel = SkToBool(rec.fFlags & SkScalerContext::kSubpixelPositioning_Flag);
    SkAxisAlignment axisAlignment = SkAxisAlignment::kNone_SkAxisAlignment;
    if (it.positioning() == SkTextBlob::kHorizontal_Positioning) {
        axisAlignment = rec.computeAxisAlignmentForHText();
    }
    auto pos = it.pos();
    const uint16_t* glyphs = it.glyphs();
    for (uint32_t index = 0; index < it.glyphCount(); index++) {
        SkIPoint subPixelPos{0, 0};
        if (runPaint.isAntiAlias() && isSubpixel) {
            SkPoint glyphPos = mapFn(runMatrix, posFn(index, pos));
            subPixelPos = SkFindAndPlaceGlyph::SubpixelAlignment(axisAlignment, glyphPos);
        }

        glyphCacheState->addGlyph(runPaint.getTypeface(),
                                  effects,
                                  SkPackedGlyphID(glyphs[index], subPixelPos.x(), subPixelPos.y()));
    }
}

struct StrikeSpec {
    StrikeSpec() {}
    StrikeSpec(SkFontID typefaceID_, size_t glyphCount_, SkDiscardableHandleId discardableHandleId_)
            : typefaceID{typefaceID_}
            , glyphCount{glyphCount_}
            , discardableHandleId(discardableHandleId_) {}
    SkFontID typefaceID = 0u;
    size_t glyphCount = 0u;
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

        // TODO: This is unnecessary, write only the descs which has any glyphs
        // to send. It was getting awkward to write the size after writing the
        // descs because the vector reallocs.
        serializer.emplace<bool>(it->second->has_pending_glyphs());
        if (!it->second->has_pending_glyphs()) continue;

        it->second->writePendingGlyphs(&serializer);
    }
    fLockedDescs.clear();
}

SkStrikeServer::SkGlyphCacheState* SkStrikeServer::getOrCreateCache(
        SkTypeface* tf, std::unique_ptr<SkDescriptor> desc) {
    SkASSERT(desc);

    // Already locked.
    if (fLockedDescs.find(desc.get()) != fLockedDescs.end()) {
        auto it = fRemoteGlyphStateMap.find(desc.get());
        SkASSERT(it != fRemoteGlyphStateMap.end());
        return it->second.get();
    }

    // Try to lock.
    auto it = fRemoteGlyphStateMap.find(desc.get());
    if (it != fRemoteGlyphStateMap.end()) {
        bool locked = fDiscardableHandleManager->lockHandle(it->second->discardable_handle_id());
        if (locked) {
            fLockedDescs.insert(it->first);
            return it->second.get();
        }

        // If the lock failed, the entry was deleted on the client. Remove our
        // tracking.
        fRemoteGlyphStateMap.erase(it);
    }

    const SkFontID typeface_id = tf->uniqueID();
    if (!fCachedTypefaces.contains(typeface_id)) {
        fCachedTypefaces.add(typeface_id);
        fTypefacesToSend.emplace_back(typeface_id, tf->countGlyphs(), tf->fontStyle(),
                                      tf->isFixedPitch());
    }

    auto* desc_ptr = desc.get();
    auto new_handle = fDiscardableHandleManager->createHandle();
    auto cache_state = skstd::make_unique<SkGlyphCacheState>(std::move(desc), new_handle);
    auto* cache_state_ptr = cache_state.get();

    fLockedDescs.insert(desc_ptr);
    fRemoteGlyphStateMap[desc_ptr] = std::move(cache_state);
    return cache_state_ptr;
}

SkStrikeServer::SkGlyphCacheState::SkGlyphCacheState(std::unique_ptr<SkDescriptor> desc,
                                                     uint32_t discardable_handle_id)
        : fDesc(std::move(desc)), fDiscardableHandleId(discardable_handle_id) {
    SkASSERT(fDesc);
}

SkStrikeServer::SkGlyphCacheState::~SkGlyphCacheState() = default;

void SkStrikeServer::SkGlyphCacheState::addGlyph(SkTypeface* typeface,
                                                 const SkScalerContextEffects& effects,
                                                 SkPackedGlyphID glyph) {
    // Already cached.
    if (fCachedGlyphs.contains(glyph)) return;

    // Serialize and cache. Also create the scalar context to use when serializing
    // this glyph.
    fCachedGlyphs.add(glyph);
    fPendingGlyphs.push_back(glyph);
    if (!fContext) fContext = typeface->createScalerContext(effects, fDesc.get(), false);
}

void SkStrikeServer::SkGlyphCacheState::writePendingGlyphs(Serializer* serializer) {
    // Write the desc.
    serializer->emplace<StrikeSpec>(fContext->getTypeface()->uniqueID(), fPendingGlyphs.size(),
                                    fDiscardableHandleId);
    serializer->writeDescriptor(*fDesc.get());

    // Write FontMetrics.
    SkPaint::FontMetrics fontMetrics;
    fContext->getFontMetrics(&fontMetrics);
    serializer->write<SkPaint::FontMetrics>(fontMetrics);

    // Write Glyphs.
    for (const auto& glyphID : fPendingGlyphs) {
        auto glyph = serializer->emplace<SkGlyph>();
        glyph->initWithGlyphID(glyphID);
        fContext->getMetrics(glyph);
        auto imageSize = glyph->computeImageSize();
        glyph->fPathData = nullptr;
        glyph->fImage = nullptr;

        if (imageSize > 0) {
            // Since the allocateArray can move glyph, make one that stays in one place.
            SkGlyph stationaryGlyph = *glyph;
            stationaryGlyph.fImage = serializer->allocateArray<uint8_t>(imageSize);
            fContext->getImage(stationaryGlyph);
        }
    }

    // Note that we reset the context after serializing pending glyphs since we
    // don't want to extend the lifetime of the typeface.
    fPendingGlyphs.clear();
    fContext.reset();
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

SkStrikeClient::SkStrikeClient(sk_sp<DiscardableHandleManager> discardableManager)
        : fDiscardableHandleManager(std::move(discardableManager)) {}

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

        auto strike = SkStrikeCache::FindStrikeExclusive(*client_desc);
        if (strike == nullptr) {
            // Note that we don't need to deserialize the effects since we won't be generating any
            // glyphs here anyway, and the desc is still correct since it includes the serialized
            // effects.
            SkScalerContextEffects effects;
            auto scaler = SkStrikeCache::CreateScalerContext(*client_desc, effects, *tf);
            strike = SkStrikeCache::CreateStrikeExclusive(
                    *client_desc, std::move(scaler), &fontMetrics,
                    skstd::make_unique<DiscardableStrikePinner>(spec.discardableHandleId,
                                                                fDiscardableHandleManager));
        }

        for (size_t j = 0; j < spec.glyphCount; j++) {
            SkGlyph glyph;
            if (!deserializer.read<SkGlyph>(&glyph)) READ_FAILURE

            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph.getPackedID());
            *allocatedGlyph = glyph;

            ArraySlice<uint8_t> image;
            auto imageSize = glyph.computeImageSize();
            if (imageSize != 0) {
                image = deserializer.readArray<uint8_t>(imageSize);
                if (!image.data()) READ_FAILURE
                allocatedGlyph->allocImage(strike->getAlloc());
                memcpy(allocatedGlyph->fImage, image.data(), image.size());
            }
        }
    }

    return true;
}

sk_sp<SkTypeface> SkStrikeClient::deserializeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len != sizeof(wire)) return nullptr;

    memcpy(&wire, buf, sizeof(wire));
    return addTypeface(wire);
}

sk_sp<SkTypeface> SkStrikeClient::addTypeface(const WireTypeface& wire) {
    auto* typeface = fRemoteFontIdToTypeface.find(wire.typefaceID);
    if (typeface) return *typeface;

    auto newTypeface = sk_make_sp<SkTypefaceProxy>(wire.typefaceID, wire.glyphCount, wire.style,
                                                   wire.isFixed, this);
    fRemoteFontIdToTypeface.set(wire.typefaceID, newTypeface);
    return std::move(newTypeface);
}

void SkStrikeClient::generateFontMetrics(const SkTypefaceProxy& typefaceProxy,
                                         const SkScalerContextRec& rec,
                                         SkPaint::FontMetrics* metrics) {
    TRACE_EVENT1("skia", "generateFontMetrics", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    SkDebugf("generateFontMetrics: %s\n", rec.dump().c_str());
    SkStrikeCache::Dump();
    SkDEBUGFAIL("GlyphCacheMiss");
}

void SkStrikeClient::generateMetricsAndImage(const SkTypefaceProxy& typefaceProxy,
                                             const SkScalerContextRec& rec,
                                             SkArenaAlloc* alloc,
                                             SkGlyph* glyph) {
    TRACE_EVENT1("skia", "generateMetricsAndImage", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    SkDebugf("generateMetricsAndImage: %s\n", rec.dump().c_str());
    SkStrikeCache::Dump();
    SkDEBUGFAIL("GlyphCacheMiss");
}

void SkStrikeClient::generatePath(const SkTypefaceProxy& typefaceProxy,
                                  const SkScalerContextRec& rec,
                                  SkGlyphID glyphID,
                                  SkPath* path) {
    TRACE_EVENT1("skia", "generateMetricsAndImage", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    SkDebugf("generatePath: %s\n", rec.dump().c_str());
    SkStrikeCache::Dump();
    SkDEBUGFAIL("GlyphCacheMiss");
}

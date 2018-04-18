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
#include "SkTraceEvent.h"
#include "SkTypeface_remote.h"

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
    Deserializer(const char* memory, size_t memorySize)
            : fMemory(memory), fMemorySize(memorySize) {}

    template <typename T>
    T* read() {
        T* result = (T*)this->ensureAtLeast(sizeof(T), alignof(T));
        if (!result) return nullptr;

        fBytesRead += sizeof(T);
        return result;
    }

    SkDescriptor* readDescriptor() {
        SkDescriptor* result =
                (SkDescriptor*)this->ensureAtLeast(sizeof(SkDescriptor), alignof(SkDescriptor));
        if (!result) return nullptr;

        fBytesRead += result->getLength();
        return result;
    }

    template <typename T>
    ArraySlice<T> readArray(int count) {
        size_t size = count * sizeof(T);
        const T* base = (const T*)this->ensureAtLeast(size, alignof(T));
        if (!base) return ArraySlice<T>();

        ArraySlice<T> result = ArraySlice<T>{base, (uint32_t)count};
        fBytesRead += size;
        return result;
    }

    bool empty() const { return fBytesRead == fMemorySize; }

private:
    const char* ensureAtLeast(size_t size, size_t alignment) {
        size_t padded = pad(fBytesRead, alignment);

        // Not enough data
        if (padded + size < fMemorySize) return nullptr;

        fBytesRead = padded;
        return fMemory + fBytesRead;
    }

    const char* fMemory;
    const size_t fMemorySize;
    size_t fBytesRead = 0u;
};

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
                                                     SkScalerContextFlags flags,
                                                     SkStrikeServer* strikeSever)
        : SkNoDrawCanvas{new TrackLayerDevice{SkIRect::MakeWH(width, height), props}}
        , fDeviceMatrix{deviceMatrix}
        , fSurfaceProps{props}
        , fScalerContextFlags{flags}
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

void SkTextBlobCacheDiffCanvas::processGlyphRun(
        const SkPoint& position,
        const SkTextBlobRunIterator& it,
        const SkPaint& runPaint)
{

    if (runPaint.getTextEncoding() != SkPaint::TextEncoding::kGlyphID_TextEncoding) {
        TRACE_EVENT0("skia", "kGlyphID_TextEncoding");
        return;
    }

    // All other alignment modes need the glyph advances. Use the slow drawing mode.
    if (runPaint.getTextAlign() != SkPaint::kLeft_Align) {
        TRACE_EVENT0("skia", "kLeft_Align");
        return;
    }

    using PosFn = SkPoint(*)(int index, const SkScalar* pos);
    PosFn posFn;
    switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning: {
            // Default positioning needs advances. Can't do that.
            TRACE_EVENT0("skia", "kDefault_Positioning");
            return;
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
        return;
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

    // TODO(crbug.com/831354): The typeface proxy on the client does not perform replicate the
    // filtering done by the typeface on the server. Until this is resolved, also ignore this
    // filtering for recs generated on the server.
    const bool applyTypefaceFiltering = false;
    SkScalerContext::MakeRecAndEffects(runPaint, &fSurfaceProps, &runMatrix, fScalerContextFlags,
                                       &rec, &effects, applyTypefaceFiltering);

    TRACE_EVENT1("skia", "RecForDesc", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    auto desc = SkScalerContext::DescriptorGivenRecAndEffects(rec, effects);
    auto* glyphCacheState =
            static_cast<SkStrikeServerImpl*>(fStrikeServer)->getOrCreateCache(std::move(desc));
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
    StrikeSpec(SkFontID typefaceID_, int glyphCount_, SkDiscardableHandleId discardableHandleId_)
            : typefaceID{typefaceID_}
            , glyphCount{glyphCount_}
            , discardableHandleId(discardableHandleId_) {}
    SkFontID typefaceID;
    int glyphCount;
    SkDiscardableHandleId discardableHandleId;
    /* desc */
    /* n X (glyphs ids) */
};

struct WireTypeface {
    WireTypeface(SkFontID typeface_id, int glyph_count, SkFontStyle style, bool is_fixed)
            : typefaceID(typeface_id), glyphCount(glyph_count), style(style), isFixed(is_fixed) {}

    // std::thread::id thread_id;  // TODO:need to figure a good solution
    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
};

// SkStrikeServer -----------------------------------------

std::unique_ptr<SkStrikeServer> SkStrikeServer::Create(
        DiscardableHandleManager* discardableHandleManager) {
    return std::make_unique<SkStrikeServerImpl>(discardableHandleManager);
}

SkStrikeServerImpl::SkStrikeServerImpl(DiscardableHandleManager* discardableHandleManager)
        : fDiscardableHandleManager(discardableHandleManager) {
    SkASSERT(fDiscardableHandleManager);
}

SkStrikeServerImpl::~SkStrikeServerImpl() = default;

sk_sp<SkData> SkStrikeServerImpl::serializeTypeface(SkTypeface* tf) {
    SkFontID typeface_id = SkTypeface::UniqueID(tf);
    auto data = SkData::MakeWithCopy(&typeface_id, sizeof(typeface_id));
    if (fCachedTypefaces.contains(typeface_id)) return data;

    fTypefacesToSend.emplace_back(typeface_id, tf->countGlyphs(), tf->fontStyle(),
                                  tf->isFixedPitch());
    fCachedTypefaces.add(typeface_id);
    return data;
}

void SkStrikeServerImpl::writeStrikeData(std::vector<uint8_t>* memory) {
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
    fRemoteGlyphStateMap.clear();
}

SkStrikeServerImpl::SkGlyphCacheState* SkStrikeServerImpl::getOrCreateCache(
        std::unique_ptr<SkDescriptor> desc) {
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

    auto* desc_ptr = desc.get();
    auto new_handle = fDiscardableHandleManager->createHandle();
    auto cache_state = std::make_unique<SkGlyphCacheState>(std::move(desc), new_handle);
    auto* cache_state_ptr = cache_state.get();

    fLockedDescs.insert(desc_ptr);
    fRemoteGlyphStateMap[desc_ptr] = std::move(cache_state);
    return cache_state_ptr;
}

SkStrikeServerImpl::SkGlyphCacheState::SkGlyphCacheState(std::unique_ptr<SkDescriptor> desc,
                                                         uint32_t discardable_handle_id)
        : fDesc(std::move(desc)), fDiscardableHandleId(discardable_handle_id) {
    SkASSERT(fDesc);
}

SkStrikeServerImpl::SkGlyphCacheState::~SkGlyphCacheState() = default;

void SkStrikeServerImpl::SkGlyphCacheState::addGlyph(SkTypeface* typeface,
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

void SkStrikeServerImpl::SkGlyphCacheState::writePendingGlyphs(Serializer* serializer) {
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

sk_sp<SkStrikeClient> SkStrikeClient::Create(DiscardableHandleManager* discardableManager) {
    return sk_make_sp<SkStrikeClientImpl>(discardableManager);
}

SkStrikeClientImpl::SkStrikeClientImpl(DiscardableHandleManager* discardableManager)
        : fDiscardableHandleManager(discardableManager) {}

SkStrikeClientImpl::~SkStrikeClientImpl() = default;

bool SkStrikeClientImpl::readStrikeData(const void* memory, size_t memorySize) {
    Deserializer deserializer(static_cast<const char*>(memory), memorySize);

    auto* typefaceSize = deserializer.read<size_t>();
    if (!typefaceSize) return false;

    for (size_t i = 0; i < *typefaceSize; ++i) {
        auto* wire = deserializer.read<WireTypeface>();
        if (!wire) return false;

        // TODO(khushalsagar): The typeface no longer needs a reference to the
        // SkStrikeClient, since all needed glyphs must have been pushed before
        // raster.
        auto newTypeface = sk_make_sp<SkTypefaceProxy>(wire->typefaceID, wire->glyphCount,
                                                       wire->style, wire->isFixed, this);
        SkASSERT(!fRemoteFontIdToTypeface.find(wire->typefaceID));
        fRemoteFontIdToTypeface.set(wire->typefaceID, std::move(newTypeface));
    }

    auto* strikeCount = deserializer.read<size_t>();
    if (!strikeCount) return false;

    for (int i = 0; i < *strikeCount; ++i) {
        auto* has_glyphs = deserializer.read<bool>();
        if (!has_glyphs) return false;

        if (!*has_glyphs) continue;

        auto* spec = deserializer.read<StrikeSpec>();
        auto* desc = deserializer.readDescriptor();
        auto* fontMetrics = deserializer.read<SkPaint::FontMetrics>();
        if (!spec || !desc || !fontMetrics) return false;

        // Get the local typeface from remote fontID.
        auto* tf = fRemoteFontIdToTypeface.find(spec->typefaceID)->get();
        // Received strikes for a typeface which doesn't exist.
        if (!tf) return false;

        // Replace the ContextRec in the desc from the server to create the client
        // side descriptor.
        // TODO: Can we do this in-place and re-compute checksum? Instead of a complete copy.
        SkAutoDescriptor ad;
        auto* client_desc = SkScalerContext::AutoDescriptorFromDesc(desc, tf->uniqueID(), &ad);

        auto strike = SkStrikeCache::FindStrikeExclusive(*client_desc);
        if (strike == nullptr) {
            // TODO: Do we need to deserialize the effects from the desc to get the
            // real values? We won't be generating any glyphs here anyway, and the desc
            // is still correct since it includes the serialized effects.
            SkScalerContextEffects effects;
            auto scaler = SkStrikeCache::CreateScalerContext(*client_desc, effects, *tf);
            DiscardableHandle handle(this, spec->discardableHandleId);
            strike = SkStrikeCache::CreateStrikeExclusive(*client_desc, std::move(scaler),
                                                          fontMetrics, handle);
        }

        for (int j = 0; j < spec->glyphCount; j++) {
            auto* glyph = deserializer.read<SkGlyph>();
            if (!glyph) return false;

            ArraySlice<uint8_t> image;
            auto imageSize = glyph->computeImageSize();
            if (imageSize != 0) {
                image = deserializer.readArray<uint8_t>(imageSize);
                if (!image.data()) return false;
            }

            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph->getPackedID());
            *allocatedGlyph = *glyph;
            allocatedGlyph->allocImage(strike->getAlloc());
            memcpy(allocatedGlyph->fImage, image.data(), image.size());
        }
    }

    return true;
}

bool SkStrikeClientImpl::deleteHandle(SkDiscardableHandleId discardableHandleId) {
    return fDiscardableHandleManager->deleteHandle(discardableHandleId);
}

sk_sp<SkTypeface> SkStrikeClientImpl::deserializeTypeface(const void* buf, size_t len) {
    SkFontID font_id;
    if (len != sizeof(SkFontID)) return nullptr;

    memcpy(&font_id, buf, sizeof(font_id));
    auto* typeFace = fRemoteFontIdToTypeface.find(font_id);
    if (!typeFace) return nullptr;

    return *typeFace;
}

void SkStrikeClientImpl::generateFontMetrics(const SkTypefaceProxy& typefaceProxy,
                                             const SkScalerContextRec& rec,
                                             SkPaint::FontMetrics* metrics) {
    TRACE_EVENT1("skia", "generateFontMetrics", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    SkDebugf("generateFontMetrics: %s\n", rec.dump().c_str());
    SkStrikeCache::Dump();
}

void SkStrikeClientImpl::generateMetricsAndImage(const SkTypefaceProxy& typefaceProxy,
                                                 const SkScalerContextRec& rec,
                                                 SkArenaAlloc* alloc,
                                                 SkGlyph* glyph) {
    TRACE_EVENT1("skia", "generateMetricsAndImage", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    SkDebugf("generateMetricsAndImage: %s\n", rec.dump().c_str());
    SkStrikeCache::Dump();
}

void SkStrikeClientImpl::generatePath(const SkTypefaceProxy& typefaceProxy,
                                      const SkScalerContextRec& rec,
                                      SkGlyphID glyphID,
                                      SkPath* path) {
    TRACE_EVENT1("skia", "generateMetricsAndImage", "rec", TRACE_STR_COPY(rec.dump().c_str()));
    SkDebugf("generatePath: %s\n", rec.dump().c_str());
    SkStrikeCache::Dump();
}

SkStrikeClientImpl::DiscardableHandle::DiscardableHandle()
        : fStrikeClient(nullptr), fDiscardableHandleId(-1) {}

SkStrikeClientImpl::DiscardableHandle::DiscardableHandle(SkStrikeClientImpl* strikeClient,
                                                         SkDiscardableHandleId discardableHandleId)
        : fStrikeClient(strikeClient), fDiscardableHandleId(discardableHandleId) {
    fStrikeClient->weak_ref();
}

SkStrikeClientImpl::DiscardableHandle::~DiscardableHandle() {
    if (fStrikeClient) fStrikeClient->weak_unref();
}

bool SkStrikeClientImpl::DiscardableHandle::deleteHandle() {
    // Empty handle.
    if (!fStrikeClient) return true;

    bool deleted = true;
    if (fStrikeClient->try_ref()) {
        deleted = fStrikeClient->deleteHandle(fDiscardableHandleId);
        fStrikeClient->unref();
    }

    return deleted;
}

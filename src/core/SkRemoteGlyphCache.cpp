/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

struct WireTypeface {
    // std::thread::id thread_id;  // TODO:need to figure a good solution
    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
};

// -- Serializer ----------------------------------------------------------------------------------

class Serializer {
public:
    void startWrite() {
        fCursor = 0;
    }

    template <typename T>
    void startWrite(const T& data) {
        this->startWrite();
        this->write<T>(data);
    }

    template <typename T, typename... Args>
    T* startEmplace(Args&&... args) {
        this->startWrite();
        return this->emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* emplace(Args&&... args) {
        T* result = new (&fBuffer[fCursor]) T{std::forward<Args>(args)...};
        fCursor += sizeof(T);
        return result;
    }

    template <typename T>
    void write(const T& data) {
        // TODO: guard against bad T.
        memcpy(&fBuffer[fCursor], &data, sizeof(data));
        fCursor += sizeof(data);
    }

    void writeDescriptor(const SkDescriptor& desc) {
        memcpy(&fBuffer[fCursor], &desc, desc.getLength());
        fCursor += desc.getLength();
    }

    template <typename T>
    T* allocateArray(int count) {
        T* result = (T*)&fBuffer[fCursor];
        fCursor += count * sizeof(T);
        return result;
    }

    SkTransport::IOResult endWrite(SkTransport* transport) {
        return transport->write(fBuffer.get(), fCursor);
    }

private:
    static constexpr size_t kBufferSize = kPageSize * 2000;
    std::unique_ptr<uint8_t[]> fBuffer{new uint8_t[kBufferSize]};
    size_t fCursor{0};
    //size_t fEnd{0};
};

// -- Deserializer -------------------------------------------------------------------------------

class Deserializer {
public:
    void startRead(SkTransport* transport) {
        fCursor = 0;
        fEnd = 0;
        fTransport = transport;
    }

    template <typename T>
    T* startRead(SkTransport* transport) {
        this->startRead(transport);
        return this->read<T>();
    }

    template <typename T>
    T* read() {
        T* result = (T*)this->ensureAtLeast(sizeof(T));
        fCursor += sizeof(T);
        return result;
    }

    SkDescriptor* readDescriptor() {
        SkDescriptor* result = (SkDescriptor*)this->ensureAtLeast(sizeof(SkDescriptor));
        size_t size = result->getLength();
        this->ensureAtLeast(size);
        fCursor += size;
        return result;
    }

    template <typename T>
    SkArraySlice<T> readArray(int count) {
        size_t size = count * sizeof(T);
        const T* base = (const T*)this->ensureAtLeast(size);
        SkArraySlice<T> result = SkArraySlice<T>{base, (uint32_t)count};
        fCursor += size;
        return result;
    }

    size_t endRead() {
        fTransport = nullptr;
        return size();
    }

    size_t size() {return fCursor;}

private:
    void* ensureAtLeast(size_t size) {
        if (size > fEnd - fCursor) {
            if (readAtLeast(size) == SkTransport::IOResult::kFail) {
                return nullptr;
            }
        }
        return &fBuffer[fCursor];
    }

    SkTransport::IOResult readAtLeast(size_t size) {
        size_t readSoFar = 0;
        size_t bufferLeft = kBufferSize - fCursor;
        size_t needed = size - (fEnd - fCursor);
        while (readSoFar < needed) {
            SkTransport::IOResult result;
            size_t readSize;
            std::tie(readSize, result) =
                    fTransport->read(&fBuffer[fEnd+readSoFar], bufferLeft - readSoFar);
            if (result == SkTransport::kFail) {return result;}
            readSoFar += readSize;
        }
        fEnd += readSoFar;
        return SkTransport::kSuccess;
    }

    SkTransport* fTransport;

    static constexpr size_t kBufferSize = kPageSize * 2000;
    std::unique_ptr<uint8_t[]> fBuffer{new uint8_t[kBufferSize]};
    size_t fCursor{0};
    size_t fEnd{0};
};

// -- SkRemoteGlyphCacheRenderer -----------------------------------------------------------------

void SkRemoteGlyphCacheRenderer::prepareSerializeProcs(SkSerialProcs* procs) {
    auto encode = [](SkTypeface* tf, void* ctx) {
        return reinterpret_cast<SkRemoteGlyphCacheRenderer*>(ctx)->encodeTypeface(tf);
    };
    procs->fTypefaceProc = encode;
    procs->fTypefaceCtx = this;
}

SkScalerContext* SkRemoteGlyphCacheRenderer::generateScalerContext(
    const SkScalerContextRecDescriptor& desc, SkFontID typefaceId)
{

    auto scaler = fScalerContextMap.find(desc);
    if (scaler == nullptr) {
        auto typefaceIter = fTypefaceMap.find(typefaceId);
        if (typefaceIter == nullptr) {
            // TODO: handle this with some future fallback strategy.
            SK_ABORT("unknown type face");
            // Should never happen
            return nullptr;
        }
        auto tf = typefaceIter->get();
        // TODO: make effects really work.
        SkScalerContextEffects effects;
        auto mapSc = tf->createScalerContext(effects, &desc.desc(), false);
        scaler = fScalerContextMap.set(desc, std::move(mapSc));
    }
    return scaler->get();
}

sk_sp<SkData> SkRemoteGlyphCacheRenderer::encodeTypeface(SkTypeface* tf) {
    WireTypeface wire = {
        SkTypeface::UniqueID(tf),
        tf->countGlyphs(),
        tf->fontStyle(),
        tf->isFixedPitch()
    };
    auto typeFace = fTypefaceMap.find(SkTypeface::UniqueID(tf));
    if (typeFace == nullptr) {
        fTypefaceMap.set(SkTypeface::UniqueID(tf), sk_ref_sp(tf));
    }
    // Can this be done with no copy?
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

// -- SkRemoteGlyphCacheGPU ------------------------------------------------------------------------

SkRemoteGlyphCacheGPU::SkRemoteGlyphCacheGPU(
    std::unique_ptr<SkRemoteScalerContext> remoteScalerContext)
    : fRemoteScalerContext{std::move(remoteScalerContext)} { }

void SkRemoteGlyphCacheGPU::prepareDeserializeProcs(SkDeserialProcs* procs) {
    auto decode = [](const void* buf, size_t len, void* ctx) {
        return reinterpret_cast<SkRemoteGlyphCacheGPU*>(ctx)->decodeTypeface(buf, len);
    };
    procs->fTypefaceProc = decode;
    procs->fTypefaceCtx = this;
}

SkTypeface* SkRemoteGlyphCacheGPU::lookupTypeface(SkFontID id) {
    auto typeface = fMapIdToTypeface.find(id);
    SkASSERT(typeface != nullptr);
    return typeface->get();
}

sk_sp<SkTypeface> SkRemoteGlyphCacheGPU::decodeTypeface(const void* buf, size_t len) {
    WireTypeface wire;
    if (len < sizeof(wire)) {
        SK_ABORT("Incomplete transfer");
        return nullptr;
    }
    memcpy(&wire, buf, sizeof(wire));

    auto typeFace = fMapIdToTypeface.find(wire.typefaceID);
    if (typeFace == nullptr) {

        auto newTypeface = sk_make_sp<SkTypefaceProxy>(
            wire.typefaceID,
            wire.glyphCount,
            wire.style,
            wire.isFixed,
            fRemoteScalerContext.get());

        typeFace = fMapIdToTypeface.set(wire.typefaceID, newTypeface);
    }
    return *typeFace;
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

// -- SkStrikeCacheDifferenceSpec ------------------------------------------------------------------

SkStrikeCacheDifferenceSpec::StrikeDifferences::StrikeDifferences(
        SkFontID typefaceID, std::unique_ptr<SkDescriptor> desc)
        : fTypefaceID{typefaceID}
        , fDesc{std::move(desc)} { }

void SkStrikeCacheDifferenceSpec::StrikeDifferences::operator()(uint16_t glyphID, SkIPoint pos) {
    SkPackedGlyphID packedGlyphID{glyphID, pos.x(), pos.y()};
    fGlyphIDs->add(packedGlyphID);
}


SkStrikeCacheDifferenceSpec::StrikeDifferences&
SkStrikeCacheDifferenceSpec::findStrikeDifferences(const SkDescriptor& desc,
                                                   SkFontID typefaceID) {
    auto mapIter = fDescMap.find(&desc);
    if (mapIter == fDescMap.end()) {
        auto newDesc = desc.copy();
        auto newDescPtr = newDesc.get();
        StrikeDifferences strikeDiffs{typefaceID, std::move(newDesc)};

        mapIter = fDescMap.emplace_hint(mapIter, newDescPtr, std::move(strikeDiffs));
    }

    return mapIter->second;
}

void SkStrikeCacheDifferenceSpec::writeSpecToTransport(AllInOneTransport* transport) const {
    transport->emplace<Header>((int)fDescMap.size());
    for (auto& i : fDescMap) {
        auto strikeDiff = &i.second;
        transport->emplace<StrikeSpec>(
                strikeDiff->fTypefaceID, strikeDiff->fDesc->getLength(), strikeDiff->fGlyphIDs->count());
        transport->writeDescriptor(*strikeDiff->fDesc);
        strikeDiff->fGlyphIDs->foreach([&](SkPackedGlyphID id) {
            transport->write<SkPackedGlyphID>(id);
        });
    }
}

template <typename PerStrike, typename PerGlyph>
void SkStrikeCacheDifferenceSpec::iterateDifferences(PerStrike perStrike, PerGlyph perGlyph) const {
    for (auto& i : fDescMap) {
        auto strikeDiff = &i.second;
        perStrike(strikeDiff->fTypefaceID,
                  *strikeDiff->fDesc,
                  strikeDiff->fGlyphIDs->count());
        strikeDiff->fGlyphIDs->foreach([&](SkPackedGlyphID id) {
            perGlyph(id);
        });
    }
}

// -- SkTextBlobCacheDiffCanvas -------------------------------------------------------------------
SkTextBlobCacheDiffCanvas::SkTextBlobCacheDiffCanvas(
        int width, int height,
        const SkMatrix& deviceMatrix,
        const SkSurfaceProps& props,
        SkScalerContextFlags flags,
        SkStrikeCacheDifferenceSpec* strikeDiffs)
        : SkNoDrawCanvas{new TrackLayerDevice{SkIRect::MakeWH(width, height), props}}
        , fDeviceMatrix{deviceMatrix}
        , fSurfaceProps{props}
        , fScalerContextFlags{flags}
        , fStrikeCacheDiff{strikeDiffs} { }

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
        runPaint.setFlags(this->getTopDevice()->filterTextFlags(runPaint));
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
        return;
    }

    // All other alignment modes need the glyph advances. Use the slow drawing mode.
    if (runPaint.getTextAlign() != SkPaint::kLeft_Align) {
        return;
    }

    using PosFn = SkPoint(*)(int index, const SkScalar* pos);
    PosFn posFn;
    switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning:
            // Default positioning needs advances. Can't do that.
            return;

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

    SkAutoDescriptor ad;
    SkScalerContextRec rec;
    SkScalerContextEffects effects;

    SkScalerContext::MakeRecAndEffects(runPaint, &fSurfaceProps, &runMatrix,
                                       fScalerContextFlags, &rec, &effects);

    auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

    auto typefaceID = SkTypefaceProxy::DownCast(runPaint.getTypeface())->remoteTypefaceID();
    auto& addGlyph = fStrikeCacheDiff->findStrikeDifferences(*desc, typefaceID);

    auto cache = SkGlyphCache::FindStrikeExclusive(*desc);
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

        if (cache &&
            cache->isGlyphCached(glyphs[index], subPixelPos.x(), subPixelPos.y())) {
            continue;
        }

        addGlyph(glyphs[index], subPixelPos);
    }
}

struct Header {
    Header(int strikeCount_) : strikeCount{strikeCount_} {}
    const int strikeCount;
};
struct StrikeSpec {
    StrikeSpec(SkFontID typefaceID_, uint32_t descLength_, int glyphCount_)
            : typefaceID{typefaceID_}
            , descLength{descLength_}
            , glyphCount{glyphCount_} { }
    SkFontID typefaceID;
    uint32_t descLength;
    int glyphCount;
    /* desc */
    /* n X (glyphs ids) */
};

static void write_strikes_spec(const SkStrikeCacheDifferenceSpec &spec,
                               Serializer* serializer,
                               SkTransport* transport) {
    serializer->startEmplace<Op>(OpCode::kPrepopulateCache, SkFontID{0}, SkScalerContextRec{});

    serializer->emplace<Header>(spec.size());

    auto perStrike = [serializer](SkFontID typefaceID, const SkDescriptor& desc, int glyphCount) {
        serializer->emplace<StrikeSpec>(typefaceID, desc.getLength(), glyphCount);
        serializer->writeDescriptor(desc);
    };

    auto perGlyph = [serializer](SkPackedGlyphID glyphID) {
        serializer->write<SkPackedGlyphID>(glyphID);
    };

    spec.iterateDifferences(perStrike, perGlyph);

    serializer->endWrite(transport);
}

/*

     template <typename PerStrike, typename PerGlyph, typename FinishStrike>
    void readDataFromTransport(
            AllInOneTransport* transport, PerStrike perStrike, PerGlyph perGlyph, FinishStrike finishStrike) {
        auto header = transport->read<Header>();
        for (int i = 0; i < header->strikeCount; i++) {
            auto strike = transport->read<StrikeSpec>();
            auto desc = transport->readDescriptor();
            auto fontMetrics = transport->read<SkPaint::FontMetrics>();
            perStrike(strike, desc, fontMetrics);
            for (int j = 0; j < strike->glyphCount; j++) {
                auto glyph = transport->read<SkGlyph>();
                SkArraySlice<uint8_t> image = SkArraySlice<uint8_t>{};
                auto imageSize = glyph->computeImageSize();
                if (imageSize != 0) {
                    image = transport->readArray<uint8_t>(imageSize);
                }
                perGlyph(glyph, image);
            }
            finishStrike();
        }
    }

 */

static void read_strikes_data(SkRemoteGlyphCacheGPU* cache,
                              Deserializer* deserializer,
                              SkTransport* transport) {
    deserializer->startRead(transport);
    auto header = deserializer->read<Header>();
    for (int i = 0; i < header->strikeCount; i++) {
        auto spec = deserializer->read<StrikeSpec>();
        auto desc = deserializer->readDescriptor();
        auto fontMetrics = deserializer->read<SkPaint::FontMetrics>();
        auto tf = cache->lookupTypeface(spec->typefaceID);

        // TODO: implement effects handling.
        SkScalerContextEffects effects;
        auto strike = SkGlyphCache::FindStrikeExclusive(*desc);
        if (strike == nullptr) {
            auto scaler = SkGlyphCache::CreateScalerContext(*desc, effects, *tf);
            strike = SkGlyphCache::CreateStrikeExclusive(*desc, std::move(scaler), fontMetrics);
        }
        for (int j = 0; j < spec->glyphCount; j++) {
            auto glyph = deserializer->read<SkGlyph>();
            SkArraySlice<uint8_t> image;
            auto imageSize = glyph->computeImageSize();
            if (imageSize != 0) {
                image = deserializer->readArray<uint8_t>(imageSize);
            }
            SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph->getPackedID());
            *allocatedGlyph = *glyph;
            allocatedGlyph->allocImage(strike->getAlloc());
            memcpy(allocatedGlyph->fImage, image.data(), image.size());
        }
    }
    deserializer->endRead();
}

void SkPrepopulateCache(
        AllInOneTransport* transport,
        SkTransport* transport2,
        SkRemoteGlyphCacheGPU* cache,
        sk_sp<SkPicture> pic,
        SkIRect bounds, const SkSurfaceProps& props) {

    SkMatrix deviceMatrix = SkMatrix::I();

    Serializer serializer;

    SkStrikeCacheDifferenceSpec strikeDifference;
    SkTextBlobCacheDiffCanvas filter(
            bounds.width(), bounds.height(), deviceMatrix, props,
            SkScalerContextFlags::kFakeGammaAndBoostContrast,
            &strikeDifference);

    pic->playback(&filter);

    write_strikes_spec(strikeDifference, &serializer, transport2);

    Deserializer deserializer;

    read_strikes_data(cache, &deserializer, transport2);

    /*
    SkExclusiveStrikePtr strike;

    auto perStrike = [&strike, cache](SkTextBlobCacheDiffCanvas::StrikeSpec* spec,
                                      SkDescriptor* desc,
                                      SkPaint::FontMetrics* fontMetrics) {
        auto tf = cache->lookupTypeface(spec->typefaceID);
        // TODO: implement effects handling.
        SkScalerContextEffects effects;
        if ((strike = SkGlyphCache::FindStrikeExclusive(*desc)) == nullptr) {
            auto scaler = SkGlyphCache::CreateScalerContext(*desc, effects, *tf);
            strike = SkGlyphCache::CreateStrikeExclusive(*desc, std::move(scaler), fontMetrics);
        }
    };

    auto perGlyph = [&strike](SkGlyph* glyph, SkArraySlice<uint8_t> image) {
        SkGlyph* allocatedGlyph = strike->getRawGlyphByID(glyph->getPackedID());
        *allocatedGlyph = *glyph;
        allocatedGlyph->allocImage(strike->getAlloc());
        memcpy(allocatedGlyph->fImage, image.data(), image.size());
    };

    auto finishStrike = [&strike]() {
        strike.reset(nullptr);
    };

    // needed for font metrics mistake.
    AllInOneTransport in = AllInOneTransport::AllInOneTransport(*transport);

    in.startRead();
    filter.readDataFromTransport(&in, perStrike, perGlyph, finishStrike);
    in.endRead();
    */
}

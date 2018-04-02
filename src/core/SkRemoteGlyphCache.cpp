/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"

#include <iterator>
#include <memory>
#include <tuple>

#include "SkDevice.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkTypeface_remote.h"

static const size_t kPageSize = 4096;

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

// -- SkRemoteStrikeTransport ----------------------------------------------------------------------------------

SkRemoteStrikeTransport::IOResult SkRemoteStrikeTransport::writeSkData(const SkData& data) {
    size_t size = data.size();

    if (this->write(&size, sizeof(size)) == kFail) {
        return kFail;
    }

    if (this->write(data.data(), size) == kFail) {
        return kFail;
    }

    return kSuccess;
}
sk_sp<SkData> SkRemoteStrikeTransport::readSkData() {
    size_t size;
    if(std::get<1>(this->read(&size, sizeof(size))) == kFail) {
        return nullptr;
    }

    auto data = std::unique_ptr<uint8_t[]>{new uint8_t[size]};
    size_t totalRead = 0;
    while (totalRead < size) {
        size_t sizeRead;
        IOResult result;
        std::tie(sizeRead, result) = this->read(&data[totalRead], size - totalRead);
        if (result == kFail || sizeRead == 0) {
            return nullptr;
        }
        totalRead += sizeRead;
    }

    return SkData::MakeWithCopy(data.get(), size);
}

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

    template <typename T>
    T* allocate() {
        // TODO: guard against bad T.
        T* result = (T*)&fBuffer[fCursor];
        fCursor += sizeof(T);
        return result;
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

    SkRemoteStrikeTransport::IOResult endWrite(SkRemoteStrikeTransport* transport) {
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
    void startRead(SkRemoteStrikeTransport* transport) {
        fCursor = 0;
        fEnd = 0;
        fTransport = transport;
    }

    template <typename T>
    T* startRead(SkRemoteStrikeTransport* transport) {
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
    ArraySlice<T> readArray(int count) {
        size_t size = count * sizeof(T);
        const T* base = (const T*)this->ensureAtLeast(size);
        ArraySlice<T> result = ArraySlice<T>{base, (uint32_t)count};
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
            if (readAtLeast(size) == SkRemoteStrikeTransport::kFail) {
                return nullptr;
            }
        }
        return &fBuffer[fCursor];
    }

    SkRemoteStrikeTransport::IOResult readAtLeast(size_t size) {
        size_t readSoFar = 0;
        size_t bufferLeft = kBufferSize - fCursor;
        size_t needed = size - (fEnd - fCursor);
        while (readSoFar < needed) {
            SkRemoteStrikeTransport::IOResult result;
            size_t readSize;
            std::tie(readSize, result) =
                    fTransport->read(&fBuffer[fEnd+readSoFar], bufferLeft - readSoFar);
            if (result == SkRemoteStrikeTransport::kFail || readSize == 0) {return SkRemoteStrikeTransport::kFail;}
            readSoFar += readSize;
        }
        fEnd += readSoFar;
        return SkRemoteStrikeTransport::kSuccess;
    }

    SkRemoteStrikeTransport* fTransport;

    static constexpr size_t kBufferSize = kPageSize * 2000;
    std::unique_ptr<uint8_t[]> fBuffer{new uint8_t[kBufferSize]};
    size_t fCursor{0};
    size_t fEnd{0};
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
    auto mapIter = fDescriptorToDifferencesMap.find(&desc);
    if (mapIter == fDescriptorToDifferencesMap.end()) {
        auto newDesc = desc.copy();
        auto newDescPtr = newDesc.get();
        StrikeDifferences strikeDiffs{typefaceID, std::move(newDesc)};

        mapIter = fDescriptorToDifferencesMap.emplace_hint(mapIter, newDescPtr, std::move(strikeDiffs));
    }

    return mapIter->second;
}

template <typename PerStrike, typename PerGlyph>
void SkStrikeCacheDifferenceSpec::iterateDifferences(PerStrike perStrike, PerGlyph perGlyph) const {
    for (auto& i : fDescriptorToDifferencesMap) {
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

// Op code semantics:
// * FontMetrics - (SkFontID, SkDescriptor) -> SkPaint::FontMetrics
// * GlyphPath - (SkFontID, SkDescriptor, SkPackedGlyphID) -> SkPath
// * GlyphMetricsAndImage - (SkFontID, SkDescriptor, SkPackedGlyphID) -> (SkGlyph, <image bits>)
// * PrepopulateCache - StrikeCacheDifferenceSpec -> StrikeCacheDifferenceData

enum class OpCode : int32_t {
    kFontMetrics          = 0,
    kGlyphPath            = 1,
    kGlyphMetricsAndImage = 2,
    kPrepopulateCache     = 3,
};

class Op {
public:
    Op(OpCode opCode, SkFontID typefaceId, const SkScalerContextRec& rec)
            : opCode{opCode}
            , typefaceId{typefaceId}
            , descriptor{rec} { }
    const OpCode opCode;
    const SkFontID typefaceId;
    const SkScalerContextRecDescriptor descriptor;
    SkPackedGlyphID glyphID;
};

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

struct WireTypeface {
    // std::thread::id thread_id;  // TODO:need to figure a good solution
    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
};

static void write_strikes_spec(const SkStrikeCacheDifferenceSpec &spec,
                               Serializer* serializer,
                               SkRemoteStrikeTransport* transport) {
    serializer->startEmplace<Op>(OpCode::kPrepopulateCache, SkFontID{0}, SkScalerContextRec{});

    serializer->emplace<Header>(spec.strikeCount());

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

static void read_strikes_spec_write_strikes_data(
        Deserializer* deserializer, Serializer* serializer, SkRemoteStrikeTransport* transport,
        SkStrikeServer* rc)
{
    // Don't start because the op started this deserialization.
    auto header = deserializer->read<Header>();
    serializer->startWrite<Header>(*header);
    for (int i = 0; i < header->strikeCount; i++) {
        auto spec = deserializer->read<StrikeSpec>();
        auto desc = deserializer->readDescriptor();
        serializer->write<StrikeSpec>(*spec);
        serializer->writeDescriptor(*desc);
        SkScalerContextRecDescriptor recDesc{*desc};
        auto scaler = rc->generateScalerContext(recDesc, spec->typefaceID);
        SkPaint::FontMetrics fontMetrics;
        scaler->getFontMetrics(&fontMetrics);
        serializer->write<SkPaint::FontMetrics>(fontMetrics);
        auto glyphIDs = deserializer->readArray<SkPackedGlyphID>(spec->glyphCount);
        for (auto glyphID : glyphIDs) {
            auto glyph = serializer->allocate<SkGlyph>();
            glyph->initWithGlyphID(glyphID);
            scaler->getMetrics(glyph);
            auto imageSize = glyph->computeImageSize();
            glyph->fPathData = nullptr;
            glyph->fImage = nullptr;

            if (imageSize > 0) {
                glyph->fImage = serializer->allocateArray<uint8_t>(imageSize);
                scaler->getImage(*glyph);
            }
        }
    }
    deserializer->endRead();
    serializer->endWrite(transport);
}

static void update_caches_from_strikes_data(SkStrikeClient *client,
                                            Deserializer *deserializer,
                                            SkRemoteStrikeTransport *transport) {
    deserializer->startRead(transport);
    auto header = deserializer->read<Header>();
    for (int i = 0; i < header->strikeCount; i++) {
        auto spec = deserializer->read<StrikeSpec>();
        auto desc = deserializer->readDescriptor();
        auto fontMetrics = deserializer->read<SkPaint::FontMetrics>();
        auto tf = client->lookupTypeface(spec->typefaceID);

        // TODO: implement effects handling.
        SkScalerContextEffects effects;
        auto strike = SkGlyphCache::FindStrikeExclusive(*desc);
        if (strike == nullptr) {
            auto scaler = SkGlyphCache::CreateScalerContext(*desc, effects, *tf);
            strike = SkGlyphCache::CreateStrikeExclusive(*desc, std::move(scaler), fontMetrics);
        }
        for (int j = 0; j < spec->glyphCount; j++) {
            auto glyph = deserializer->read<SkGlyph>();
            ArraySlice<uint8_t> image;
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

// -- SkStrikeServer -------------------------------------------------------------------------------
SkStrikeServer::SkStrikeServer(SkRemoteStrikeTransport* transport)
    : fTransport{transport} { }

SkStrikeServer::~SkStrikeServer() {
    printf("Strike server - ops: %d\n", fOpCount);
}

int SkStrikeServer::serve() {

    auto serializer = skstd::make_unique<Serializer>();
    auto deserializer = skstd::make_unique<Deserializer>();;

    while (true) {
        Op* op = deserializer->startRead<Op>(fTransport);
        if (op == nullptr) { break; }

        fOpCount += 1;

        switch (op->opCode) {
            case OpCode::kFontMetrics : {
                auto sc = this->generateScalerContext(op->descriptor, op->typefaceId);
                SkPaint::FontMetrics metrics;
                sc->getFontMetrics(&metrics);
                serializer->startWrite<SkPaint::FontMetrics>(metrics);
                serializer->endWrite(fTransport);
                break;
            }
            case OpCode::kGlyphPath : {
                auto sc = this->generateScalerContext(op->descriptor, op->typefaceId);
                // TODO: check for buffer overflow.
                SkPath path;
                sc->getPath(op->glyphID, &path);
                size_t pathSize = path.writeToMemory(nullptr);
                serializer->startWrite<size_t>(pathSize);
                auto pathData = serializer->allocateArray<uint8_t>(pathSize);
                path.writeToMemory(pathData);
                serializer->endWrite(fTransport);
                break;
            }
            case OpCode::kGlyphMetricsAndImage : {
                auto sc = this->generateScalerContext(op->descriptor, op->typefaceId);

                serializer->startWrite();
                auto glyph = serializer->allocate<SkGlyph>();
                // TODO: check for buffer overflow.
                glyph->initWithGlyphID(op->glyphID);
                sc->getMetrics(glyph);
                auto imageSize = glyph->computeImageSize();
                glyph->fPathData = nullptr;
                glyph->fImage = nullptr;

                if (imageSize > 0) {
                    glyph->fImage = serializer->allocateArray<uint8_t>(imageSize);
                    sc->getImage(*glyph);
                }

                serializer->endWrite(fTransport);
                break;
            }
            case OpCode::kPrepopulateCache : {
                read_strikes_spec_write_strikes_data(
                        deserializer.get(), serializer.get(), fTransport, this);
                break;
            }

            default:
                SK_ABORT("Bad op");
        }
    }
    return 0;
}

void SkStrikeServer::prepareSerializeProcs(SkSerialProcs* procs) {
    auto encode = [](SkTypeface* tf, void* ctx) {
        return reinterpret_cast<SkStrikeServer*>(ctx)->encodeTypeface(tf);
    };
    procs->fTypefaceProc = encode;
    procs->fTypefaceCtx = this;
}

SkScalerContext* SkStrikeServer::generateScalerContext(
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

sk_sp<SkData> SkStrikeServer::encodeTypeface(SkTypeface* tf) {
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

// -- SkStrikeClient -------------------------------------------------------------------------------

static Op* start_op_write(
    OpCode opCode, const SkTypefaceProxy& tf, const SkScalerContextRec& rec, Serializer* serializer)
{
    return serializer->startEmplace<Op>(opCode, tf.remoteTypefaceID(), rec);
}

SkStrikeClient::SkStrikeClient(SkRemoteStrikeTransport* transport) : fTransport{transport} { }

void SkStrikeClient::generateFontMetrics(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkPaint::FontMetrics* metrics) {
    // Send generateFontMetrics
    {
        Serializer serializer;
        serializer.startEmplace<Op>(OpCode::kFontMetrics, typefaceProxy.remoteTypefaceID(), rec);
        start_op_write(OpCode::kFontMetrics, typefaceProxy, rec, &serializer);
        serializer.endWrite(fTransport);
    }

    // Receive generateFontMetrics
    {
        Deserializer deserializer;
        deserializer.startRead(fTransport);
        *metrics = *deserializer.read<SkPaint::FontMetrics>();
        deserializer.endRead();
    }
}

void SkStrikeClient::generateMetricsAndImage(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkArenaAlloc* alloc,
        SkGlyph* glyph) {
    SkScalerContextRecDescriptor rd{rec};

    {
        Serializer serializer;
        Op *op = serializer.startEmplace<Op>(
            OpCode::kGlyphMetricsAndImage, typefaceProxy.remoteTypefaceID(), rec);
        op->glyphID = glyph->getPackedID();
        serializer.endWrite(fTransport);
    }

    // Receive generateMetricsAndImage
    {
        Deserializer deserializer;
        *glyph = *deserializer.startRead<SkGlyph>(fTransport);
        auto imageSize = glyph->computeImageSize();
        glyph->fPathData = nullptr;
        glyph->fImage = nullptr;
        if (imageSize > 0) {
            auto image = deserializer.readArray<uint8_t>(imageSize);
            SkASSERT(imageSize == image.size());
            glyph->allocImage(alloc);
            memcpy(glyph->fImage, image.data(), imageSize);
        }
        deserializer.endRead();
    }

}
void SkStrikeClient::generatePath(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkGlyphID glyphID,
        SkPath* path) {
    {
        Serializer serializer;
        Op *op = serializer.startEmplace<Op>(
            OpCode::kGlyphPath, typefaceProxy.remoteTypefaceID(), rec);
        op->glyphID = glyphID;
        serializer.endWrite(fTransport);
    }

    {
        Deserializer deserializer;
        size_t pathSize = *deserializer.startRead<size_t>(fTransport);
        auto rawPath = deserializer.readArray<uint8_t>(pathSize);
        path->readFromMemory(rawPath.data(), rawPath.size());
        deserializer.endRead();
    }
}

void SkStrikeClient::primeStrikeCache(const SkStrikeCacheDifferenceSpec& strikeDifferences) {
    {
        auto serializer = skstd::make_unique<Serializer>();
        write_strikes_spec(strikeDifferences, serializer.get(), fTransport);
    }
    {
        auto deserializer = skstd::make_unique<Deserializer>();;
        update_caches_from_strikes_data(this, deserializer.get(), fTransport);
    }
}

void SkStrikeClient::prepareDeserializeProcs(SkDeserialProcs* procs) {
    auto decode = [](const void* buf, size_t len, void* ctx) {
        return reinterpret_cast<SkStrikeClient*>(ctx)->decodeTypeface(buf, len);
    };
    procs->fTypefaceProc = decode;
    procs->fTypefaceCtx = this;

}

SkTypeface* SkStrikeClient::lookupTypeface(SkFontID id) {
    auto typeface = fMapIdToTypeface.find(id);
    SkASSERT(typeface != nullptr);
    return typeface->get();
}

sk_sp<SkTypeface> SkStrikeClient::decodeTypeface(const void* buf, size_t len) {
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
                this);

        typeFace = fMapIdToTypeface.set(wire.typefaceID, newTypeface);
    }
    return *typeFace;
}

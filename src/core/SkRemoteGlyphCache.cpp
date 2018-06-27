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

// -- SkRemoteStrikeTransport ----------------------------------------------------------------------

static SkRemoteStrikeTransport::IOResult write_data(
    size_t size, const uint8_t* data, SkRemoteStrikeTransport* t) {

    if (t->write(&size, sizeof(size)) == SkRemoteStrikeTransport::kFail) {
        return SkRemoteStrikeTransport::kFail;
    }

    if (t->write(data, size) == SkRemoteStrikeTransport::kFail) {
        return SkRemoteStrikeTransport::kFail;
    }

    return SkRemoteStrikeTransport::kSuccess;
}

static SkRemoteStrikeTransport::IOResult read_data(
    size_t size, uint8_t* data, SkRemoteStrikeTransport* t) {

    size_t totalRead = 0;
    while (totalRead < size) {
        size_t sizeRead;
        SkRemoteStrikeTransport::IOResult result;
        std::tie(sizeRead, result) = t->read(&data[totalRead], size - totalRead);
        if (result == SkRemoteStrikeTransport::kFail || sizeRead == 0) {
            return SkRemoteStrikeTransport::kFail;
        }
        totalRead += sizeRead;
    }

    return SkRemoteStrikeTransport::kSuccess;
}

SkRemoteStrikeTransport::IOResult SkRemoteStrikeTransport::writeSkData(const SkData& data) {
    return write_data(data.size(), (uint8_t*)data.data(), this);
}

sk_sp<SkData> SkRemoteStrikeTransport::readSkData() {
    size_t size;
    if(std::get<1>(this->read(&size, sizeof(size))) == kFail) {
        return nullptr;
    }

    auto data = std::unique_ptr<uint8_t[]>{new uint8_t[size]};
    if (read_data(size, data.get(), this) == kFail) {
        return nullptr;
    }

    return SkData::MakeWithCopy(data.get(), size);
}

SkRemoteStrikeTransport::IOResult SkRemoteStrikeTransport::writeVector(
    const std::vector<uint8_t>& vector) {
    return write_data(vector.size(), vector.data(), this);
}

SkRemoteStrikeTransport::IOResult SkRemoteStrikeTransport::readVector(
    std::vector<uint8_t>* vector) {
    size_t vectorSize = 0;
    size_t readSize = 0;
    SkRemoteStrikeTransport::IOResult result;
    std::tie(readSize, result) = this->read(&vectorSize, sizeof(vectorSize));
    if(result == kFail || readSize == 0) {
        return kFail;
    }

    vector->resize(vectorSize);

    return read_data(vectorSize, vector->data(), this);
}

// -- Serializer ----------------------------------------------------------------------------------

static size_t pad(size_t size, size_t alignment) {
    return (size + (alignment - 1)) & ~(alignment - 1);
}

// N.B. pointers are only valid until the next call.
class Serializer {
public:
    Serializer(std::vector<uint8_t>* buffer) : fBuffer{buffer} { }

    template <typename T>
    T* push_back(const T& data) {
        auto result = allocate(sizeof(T), alignof(T));
        return new (result) T(data);
    }

    template <typename T, typename... Args>
    T* emplace_back(Args&& ... args) {
        auto result = allocate(sizeof(T), alignof(T));
        return new (result) T{std::forward<Args>(args)...};
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

class Deserializer {
public:
    Deserializer(const std::vector<uint8_t>& buffer) : fBuffer{buffer} { }

    template <typename T>
    T* read() {
        size_t padded = pad(fCursor, alignof(T));
        fCursor = padded + sizeof(T);
        return (T*)&fBuffer[padded];
    }

    SkDescriptor* readDescriptor() {
        size_t padded = pad(fCursor, alignof(SkDescriptor));
        SkDescriptor* result = (SkDescriptor*)&fBuffer[padded];
        fCursor = padded + result->getLength();
        return result;
    }

    template <typename T>
    ArraySlice<T> readArray(int count) {
        size_t padded = pad(fCursor, alignof(T));
        size_t size = count * sizeof(T);
        const T* base = (const T*)&fBuffer[padded];
        ArraySlice<T> result = ArraySlice<T>{base, (uint32_t)count};
        fCursor = padded + size;
        return result;
    }

    size_t size() {return fCursor;}

private:
    const std::vector<uint8_t>& fBuffer;
    size_t                      fCursor{0};
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

        mapIter = fDescriptorToDifferencesMap.emplace_hint(
            mapIter, newDescPtr, std::move(strikeDiffs));
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

// -- TrackLayerDevice -----------------------------------------------------------------------------
class TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props)
            : SkNoPixelsDevice(bounds, props) { }
    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps);
    }

    // Stolen from the SkBitmapDevice, but the SkGPUDevice is similar.
    bool onShouldDisableLCD(const SkPaint& paint) const override {
        if (paint.getPathEffect() ||
            paint.isFakeBoldText() ||
            paint.getStyle() != SkPaint::kFill_Style ||
            !paint.isSrcOver())
        {
            return true;
        }
        return false;
    }
};

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

struct StrikeDiffHeader {
    StrikeDiffHeader() {}
    StrikeDiffHeader(int strikeCount_) : strikeCount{strikeCount_} {}
    int strikeCount;
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
    SkFontID        typefaceID;
    int             glyphCount;
    SkFontStyle     style;
    bool            isFixed;
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
    union {
        // kGlyphPath and kGlyphMetricsAndImage
        SkPackedGlyphID glyphID;
        // kPrepopulateCache
        StrikeDiffHeader strikeSpecHeader;
    };
};

class Result {
    union {
        SkPaint::FontMetrics fontMetrics;
        SkGlyph glyph;
        StrikeDiffHeader strikeDiffHeader;
    };
};

size_t SkStrikeCacheDifferenceSpec::sizeBytes() const {
    size_t sum = sizeof(Op) + sizeof(StrikeDiffHeader);
    for (auto& pair : fDescriptorToDifferencesMap) {
        const auto& strike = pair.second;
        sum += sizeof(StrikeSpec)
               + strike.fDesc->getLength()
               + strike.fGlyphIDs->count() * sizeof(SkPackedGlyphID);
    }
    return sum;
}

static void write_strikes_spec(const SkStrikeCacheDifferenceSpec &spec,
                               Serializer* serializer) {
    serializer->emplace_back<Op>(OpCode::kPrepopulateCache, SkFontID{0}, SkScalerContextRec{});

    serializer->emplace_back<StrikeDiffHeader>(spec.strikeCount());

    auto perStrike = [serializer](SkFontID typefaceID, const SkDescriptor& desc, int glyphCount) {
        serializer->emplace_back<StrikeSpec>(typefaceID, desc.getLength(), glyphCount);
        serializer->writeDescriptor(desc);
    };

    auto perGlyph = [serializer](SkPackedGlyphID glyphID) {
        serializer->push_back<SkPackedGlyphID>(glyphID);
    };

    spec.iterateDifferences(perStrike, perGlyph);
}

static void read_strikes_spec_write_strikes_data(
        Deserializer* deserializer, Serializer* serializer, SkStrikeServer* server)
{
    // Don't start because the op started this deserialization.
    auto header = deserializer->read<StrikeDiffHeader>();
    serializer->push_back<StrikeDiffHeader>(*header);
    for (int i = 0; i < header->strikeCount; i++) {
        auto spec = deserializer->read<StrikeSpec>();
        auto desc = deserializer->readDescriptor();
        serializer->push_back<StrikeSpec>(*spec);
        serializer->writeDescriptor(*desc);
        SkScalerContextRecDescriptor recDesc{*desc};
        auto scaler = server->generateScalerContext(recDesc, spec->typefaceID);
        SkPaint::FontMetrics fontMetrics;
        scaler->getFontMetrics(&fontMetrics);
        serializer->push_back<SkPaint::FontMetrics>(fontMetrics);
        auto glyphIDs = deserializer->readArray<SkPackedGlyphID>(spec->glyphCount);
        for (auto glyphID : glyphIDs) {
            auto glyph = serializer->emplace_back<SkGlyph>();
            glyph->initWithGlyphID(glyphID);
            scaler->getMetrics(glyph);
            auto imageSize = glyph->computeImageSize();
            glyph->fPathData = nullptr;
            glyph->fImage = nullptr;

            if (imageSize > 0) {
                // Since the allocateArray can move glyph, make one that stays in one place.
                SkGlyph stationaryGlyph = *glyph;
                stationaryGlyph.fImage = serializer->allocateArray<uint8_t>(imageSize);
                scaler->getImage(stationaryGlyph);
            }
        }
    }
}

static void update_caches_from_strikes_data(SkStrikeClient *client,
                                            Deserializer *deserializer) {
    auto header = deserializer->read<StrikeDiffHeader>();
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
}

// -- SkStrikeServer -------------------------------------------------------------------------------
SkStrikeServer::SkStrikeServer(SkRemoteStrikeTransport* transport)
    : fTransport{transport} { }

SkStrikeServer::~SkStrikeServer() {
    printf("Strike server - ops: %d\n", fOpCount);
}

int SkStrikeServer::serve() {

    std::vector<uint8_t> inBuffer;
    std::vector<uint8_t> outBuffer;

    while (true) {
        inBuffer.clear();
        auto result = fTransport->readVector(&inBuffer);
        if (result == SkRemoteStrikeTransport::kFail) {
            break;
        }

        Deserializer deserializer{inBuffer};
        Op* op = deserializer.read<Op>();

        fOpCount += 1;

        outBuffer.clear();
        Serializer serializer{&outBuffer};

        switch (op->opCode) {
            case OpCode::kFontMetrics : {
                auto sc = this->generateScalerContext(op->descriptor, op->typefaceId);
                SkPaint::FontMetrics metrics;
                sc->getFontMetrics(&metrics);
                serializer.push_back<SkPaint::FontMetrics>(metrics);
                break;
            }
            case OpCode::kGlyphPath : {
                auto sc = this->generateScalerContext(op->descriptor, op->typefaceId);
                // TODO: check for buffer overflow.
                SkPath path;
                if (sc->getPath(op->glyphID, &path)) {
                    size_t pathSize = path.writeToMemory(nullptr);
                    serializer.push_back<size_t>(pathSize);
                    auto pathData = serializer.allocateArray<uint8_t>(pathSize);
                    path.writeToMemory(pathData);
                }
                break;
            }
            case OpCode::kGlyphMetricsAndImage : {
                auto scaler = this->generateScalerContext(op->descriptor, op->typefaceId);

                auto glyph = serializer.emplace_back<SkGlyph>();
                // TODO: check for buffer overflow.
                glyph->initWithGlyphID(op->glyphID);
                scaler->getMetrics(glyph);
                auto imageSize = glyph->computeImageSize();
                glyph->fPathData = nullptr;
                glyph->fImage = nullptr;

                if (imageSize > 0) {
                    // Since the allocateArray can move glyph, make one that stays in one place.
                    SkGlyph stationaryGlyph = *glyph;
                    stationaryGlyph.fImage = serializer.allocateArray<uint8_t>(imageSize);
                    scaler->getImage(stationaryGlyph);
                }
                break;
            }
            case OpCode::kPrepopulateCache : {
                read_strikes_spec_write_strikes_data(
                        &deserializer, &serializer, this);
                break;
            }

            default:
                SK_ABORT("Bad op");
        }

        fTransport->writeVector(outBuffer);
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

SkStrikeClient::SkStrikeClient(SkRemoteStrikeTransport* transport)
    : fTransport{transport} { }

void SkStrikeClient::generateFontMetrics(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkPaint::FontMetrics* metrics) {
    // Send generateFontMetrics
    {
        fBuffer.clear();
        Serializer serializer{&fBuffer};
        serializer.emplace_back<Op>(OpCode::kFontMetrics, typefaceProxy.remoteTypefaceID(), rec);
        fTransport->writeVector(fBuffer);
    }

    // Receive generateFontMetrics
    {
        fBuffer.clear();
        fTransport->readVector(&fBuffer);
        Deserializer deserializer(fBuffer);
        *metrics = *deserializer.read<SkPaint::FontMetrics>();
    }
}

void SkStrikeClient::generateMetricsAndImage(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkArenaAlloc* alloc,
        SkGlyph* glyph) {
    {
        fBuffer.clear();
        Serializer serializer(&fBuffer);
        Op *op = serializer.emplace_back<Op>(
            OpCode::kGlyphMetricsAndImage, typefaceProxy.remoteTypefaceID(), rec);
        op->glyphID = glyph->getPackedID();
        fTransport->writeVector(fBuffer);
    }

    // Receive generateMetricsAndImage
    {
        fBuffer.clear();
        fTransport->readVector(&fBuffer);
        Deserializer deserializer(fBuffer);
        *glyph = *deserializer.read<SkGlyph>();
        auto imageSize = glyph->computeImageSize();
        glyph->fPathData = nullptr;
        glyph->fImage    = nullptr;
        if (imageSize > 0) {
            auto image = deserializer.readArray<uint8_t>(imageSize);
            SkASSERT(imageSize == image.size());
            glyph->allocImage(alloc);
            memcpy(glyph->fImage, image.data(), imageSize);
        }
    }

}
bool SkStrikeClient::generatePath(
        const SkTypefaceProxy& typefaceProxy,
        const SkScalerContextRec& rec,
        SkGlyphID glyphID,
        SkPath* path) {
    {
        fBuffer.clear();
        Serializer serializer{&fBuffer};
        Op *op = serializer.emplace_back<Op>(
            OpCode::kGlyphPath, typefaceProxy.remoteTypefaceID(), rec);
        op->glyphID = glyphID;
        fTransport->writeVector(fBuffer);
    }

    {
        fBuffer.clear();
        fTransport->readVector(&fBuffer);
        Deserializer deserializer{fBuffer};
        size_t pathSize = *deserializer.read<size_t>();
        if (pathSize == 0) {
            return false;
        }
        auto rawPath = deserializer.readArray<uint8_t>(pathSize);
        path->readFromMemory(rawPath.data(), rawPath.size());
    }
    return true;
}

void SkStrikeClient::primeStrikeCache(const SkStrikeCacheDifferenceSpec& strikeDifferences) {
    {
        fBuffer.clear();
        fBuffer.reserve(strikeDifferences.sizeBytes());
        Serializer serializer{&fBuffer};
        write_strikes_spec(strikeDifferences, &serializer);
        fTransport->writeVector(fBuffer);
    }
    {
        fBuffer.clear();
        fTransport->readVector(&fBuffer);
        Deserializer deserializer{fBuffer};
        update_caches_from_strikes_data(this, &deserializer);
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

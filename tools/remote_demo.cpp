/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkGlyph.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "SkData.h"
#include "SkDescriptor.h"
#include "SkGraphics.h"
#include "SkNoDrawCanvas.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"
#include "SkTextBlobRunIterator.h"
#include "SkGlyphCache.h"
#include "SkDrawFilter.h"
#include "SkDevice.h"

#include <type_traits>
#include <chrono>
#include <ctype.h>
#include <err.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <iomanip>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <SkFindAndPlaceGlyph.h>
#include <SkDrawLooper.h>
#include "SkTypeface_remote.h"
#include "SkRemoteGlyphCache.h"
#include "SkMakeUnique.h"

static const size_t kPageSize = 4096;

static bool gUseGpu = true;
static bool gPurgeFontCaches = true;
static bool gUseProcess = true;

static int gFontMetrics;
static int gMetricsImage;
static int gPath;

enum direction : int {kRead = 0, kWrite = 1};

#define INSTRUMENT 0

template <typename T>
class SkArraySlice : public std::tuple<const T*, size_t> {
public:
    // Additional constructors as needed.
    SkArraySlice(const T* data, size_t size) : std::tuple<const T*, size_t>{data, size} { }
    SkArraySlice() : SkArraySlice<T>(nullptr, 0) { }
    friend const T* begin(const SkArraySlice<T>& slice) {
        return slice.data();
    }

    friend const T* end(const SkArraySlice<T>& slice) {
        return &slice.data()[slice.size()];
    }

    const T* data() const {
        return std::get<0>(*this);
    }

    size_t size() const {
        return std::get<1>(*this);
    }
};

// TODO: handle alignment
// TODO: handle overflow
class Transport {
public:
    enum IOResult : bool {kFail = false, kSuccess = true};

    Transport(Transport&& t)
        : fReadFd{t.fReadFd}
        , fWriteFd{t.fWriteFd}
        , fBuffer{std::move(t.fBuffer)}
        , fCloser{t.fCloser} { }

    Transport(const Transport& t)
        : fReadFd{t.fReadFd}
        , fWriteFd{t.fWriteFd}
        , fBuffer{new uint8_t[kBufferSize]}
        , fCloser{t.fCloser} { }

    Transport(int readFd, int writeFd)
        : fReadFd{readFd}
        , fWriteFd{writeFd}
        , fCloser{std::make_shared<Closer>(readFd, writeFd)} { }

    static Transport DoubleBuffer(const Transport& transport) {
        return Transport{transport};
    }

    struct Closer {
        Closer(int readFd, int writeFd) : fReadFd{readFd}, fWriteFd{writeFd} { }
        ~Closer() {
            close(fWriteFd);
            close(fReadFd);
        }
        int fReadFd,
            fWriteFd;
    };

    void startRead() {
        fCursor = 0;
        fEnd = 0;
    }

    template <typename T>
    T* startRead() {
        this->startRead();
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

    size_t endRead() {return size();}

    sk_sp<SkData> readEntireData() {
        size_t* size = this->startRead<size_t>();
        if (size == nullptr) {
            return nullptr;
        }
        const uint8_t* data = this->readArray<uint8_t>(*size).data();
        if (size == nullptr || data == nullptr) {
            this->endRead();
            return sk_sp<SkData>(nullptr);
        }
        auto result = SkData::MakeWithCopy(data, *size);
        this->endRead();
        return result;
    }

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

    IOResult endWrite() {
        ssize_t written;
        if((written = ::write(fWriteFd, fBuffer.get(), fCursor)) < 0) {
            return kFail;
        }
        return kSuccess;
    }

    IOResult writeEntireData(const SkData& data) {
        size_t size = data.size();
        iovec vec[2];
        vec[0].iov_base = &size;
        vec[0].iov_len = sizeof(size);
        vec[1].iov_base = (void *)data.data();
        vec[1].iov_len = size;

        if(::writev(fWriteFd, vec, 2) < 0) {
            return kFail;
        }
        return kSuccess;
    }

    size_t size() {return fCursor;}

private:
    void* ensureAtLeast(size_t size) {
        if (size > fEnd - fCursor) {
            if (readAtLeast(size) == kFail) {
                return nullptr;
            }
        }
        return &fBuffer[fCursor];
    }

    IOResult readAtLeast(size_t size) {
        size_t readSoFar = 0;
        size_t bufferLeft = kBufferSize - fCursor;
        size_t needed = size - (fEnd - fCursor);
        while (readSoFar < needed) {
            ssize_t readSize;
            if ((readSize = ::read(fReadFd, &fBuffer[fEnd+readSoFar], bufferLeft - readSoFar)) <= 0) {
                if (readSize != 0) {
                    err(1,"Failed read %zu", size);
                }
                return kFail;
            }
            readSoFar += readSize;
        }
        fEnd += readSoFar;
        return kSuccess;
    }

    static constexpr size_t kBufferSize = kPageSize * 2000;
    const int fReadFd,
              fWriteFd;

    std::unique_ptr<uint8_t[]> fBuffer{new uint8_t[kBufferSize]};
    std::shared_ptr<Closer> fCloser;

    size_t fCursor{0};
    size_t fEnd{0};
};

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
    union {
        // op 0
        SkPaint::FontMetrics fontMetrics;
        // op 1, 2, and 4
        SkGlyph glyph;
        // op 3
        struct {
            SkGlyphID glyphId;
            size_t pathSize;
        };
    };
};


class TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props)
            : SkNoPixelsDevice(bounds, props) { }
    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override {
        const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return new TrackLayerDevice(this->getGlobalBounds(), surfaceProps);
    }
};


class TextBlobFilterCanvas : public SkNoDrawCanvas {
public:
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

    struct Header {
        Header(int strikeCount_) : strikeCount{strikeCount_} {}
        const int strikeCount;
    };

    TextBlobFilterCanvas(int width, int height,
                         const SkMatrix& deviceMatrix,
                         const SkSurfaceProps& props,
                         SkScalerContextFlags flags)
        : SkNoDrawCanvas{new TrackLayerDevice{SkIRect::MakeWH(width, height), props}}
        , fDeviceMatrix{deviceMatrix}
        , fSurfaceProps{props}
        , fScalerContextFlags{flags} { }

    void writeSpecToTransport(Transport* transport) {
        transport->emplace<Header>((int)fDescMap.size());
        for (auto& i : fDescMap) {
            auto accum = &i.second;
            transport->emplace<StrikeSpec>(
                accum->typefaceID, accum->desc->getLength(), accum->glyphIDs->count());
            transport->writeDescriptor(*accum->desc);
            accum->glyphIDs->foreach([&](SkPackedGlyphID id) {
                transport->write<SkPackedGlyphID>(id);
            });
        }
    }

    static void WriteDataToTransport(
        Transport* in, Transport* out, SkRemoteGlyphCacheRenderer* rc) {
        auto perHeader = [out](Header* header) {
          out->write<Header>(*header);
        };

        struct {
            SkScalerContext* scaler{nullptr};
        } strikeData;

        auto perStrike = [out, &strikeData, rc](StrikeSpec* spec, SkDescriptor* desc) {
            out->write<StrikeSpec>(*spec);
            out->writeDescriptor(*desc);
            SkScalerContextRecDescriptor recDesc{*desc};
            strikeData.scaler = rc->generateScalerContext(recDesc, spec->typefaceID);
            SkPaint::FontMetrics fontMetrics;
            strikeData.scaler->getFontMetrics(&fontMetrics);
            out->write<SkPaint::FontMetrics>(fontMetrics);
        };

        auto perGlyph = [out, &strikeData](SkPackedGlyphID glyphID) {
            SkGlyph glyph;
            glyph.initWithGlyphID(glyphID);
            strikeData.scaler->getMetrics(&glyph);
            auto imageSize = glyph.computeImageSize();
            glyph.fImage = nullptr;
            glyph.fPathData = nullptr;
            out->write<SkGlyph>(glyph);

            if (imageSize > 0) {
                glyph.fImage = out->allocateArray<uint8_t>(imageSize);
                strikeData.scaler->getImage(glyph);
            }
        };

        ReadSpecFromTransport(in, perHeader, perStrike, perGlyph);
    }

    template <typename PerHeader, typename PerStrike, typename PerGlyph>
    static void ReadSpecFromTransport(Transport* transport,
                               PerHeader perHeader,
                               PerStrike perStrike,
                               PerGlyph perGlyph) {
        auto header = transport->read<TextBlobFilterCanvas::Header>();
        perHeader(header);
        for (int i = 0; i < header->strikeCount; i++) {
            auto strike = transport->read<TextBlobFilterCanvas::StrikeSpec>();
            auto desc = transport->readDescriptor();
            //desc->assertChecksum();
            perStrike(strike, desc);
            auto glyphIDs = transport->readArray<SkPackedGlyphID>(strike->glyphCount);
            for (auto glyphID : glyphIDs) {
                perGlyph(glyphID);
            }
        }
    }

    template <typename PerStrike, typename PerGlyph, typename FinishStrike>
    void readDataFromTransport(
        Transport* transport, PerStrike perStrike, PerGlyph perGlyph, FinishStrike finishStrike) {
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


protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        return kFullLayer_SaveLayerStrategy;
    }

    void onDrawTextBlob(
        const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) override
    {
        SkPoint position{x, y};

        SkPaint runPaint{paint};
        SkTextBlobRunIterator it(blob);
        for (;!it.done(); it.next()) {
            // applyFontToPaint() always overwrites the exact same attributes,
            // so it is safe to not re-seed the paint for this reason.
            it.applyFontToPaint(&runPaint);
            runPaint.setFlags(this->getTopDevice()->filterTextFlags(runPaint));
            if (auto looper = runPaint.getLooper()) {
                this->processLooper(position, it, runPaint, looper, this);
            } else {
                this->processGlyphRun(position, it, runPaint);
            }
        }
    }

    void onDrawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&) override {
        SK_ABORT("DrawText");
    }
    void onDrawPosText(const void*, size_t, const SkPoint[], const SkPaint&) override {
        SK_ABORT("DrawPosText");
    }
    void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) override {
        SK_ABORT("DrawPosTextH");
    }

private:
    using PosFn = SkPoint(*)(int index, const SkScalar* pos);
    using MapFn = SkPoint(*)(const SkMatrix& m, SkPoint pt);

    struct CacheAccum {
        SkFontID typefaceID;
        SkDescriptor* desc;
        //std::vector<SkPackedGlyphID> glyphIDs;
        std::unique_ptr<SkTHashSet<SkPackedGlyphID>> glyphIDs;
    };

    void processLooper(
        const SkPoint& position,
        const SkTextBlobRunIterator& it,
        const SkPaint& origPaint,
        SkDrawLooper* looper,
        SkCanvas* canvas)
    {
        SkSTArenaAlloc<48> alloc;
        auto context = looper->makeContext(canvas, &alloc);
        SkPaint runPaint = origPaint;
        while (context->next(this, &runPaint)) {
            canvas->save();
            this->processGlyphRun(position, it, runPaint);
            canvas->restore();
            runPaint = origPaint;
        }
    }

    void processGlyphRun(
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

        SkAxisAlignment axisAlignment = SkAxisAlignment::kNone_SkAxisAlignment;
        if (it.positioning() == SkTextBlob::kHorizontal_Positioning) {
            axisAlignment = rec.computeAxisAlignmentForHText();
        }

        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto mapIter = fDescMap.find(desc);
        if (mapIter == fDescMap.end()) {
            auto newDesc = desc->copy();
            auto newDescPtr = newDesc.get();
            fUniqueDescriptors.emplace_back(std::move(newDesc));
            CacheAccum newAccum;
            newAccum.desc = newDescPtr;

            newAccum.typefaceID =
                SkTypefaceProxy::DownCast(runPaint.getTypeface())->fontID();

            newAccum.glyphIDs = skstd::make_unique<SkTHashSet<SkPackedGlyphID>>();
            mapIter = fDescMap.emplace_hint(mapIter, newDescPtr, std::move(newAccum));
        }

        auto accum = &mapIter->second;

        auto cache = SkGlyphCache::FindStrikeExclusive(*desc);
        bool isSubpixel = SkToBool(rec.fFlags & SkScalerContext::kSubpixelPositioning_Flag);

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

            SkPackedGlyphID glyphID{glyphs[index], subPixelPos.x(), subPixelPos.y()};
            accum->glyphIDs->add(glyphID);
        }
    }

    const SkMatrix fDeviceMatrix;
    const SkSurfaceProps fSurfaceProps;
    const SkScalerContextFlags fScalerContextFlags;

    struct DescHash {
        size_t operator()(const SkDescriptor* key) const {
            return key->getChecksum();
        }
    };

    struct DescEq {
        bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
            return lhs->getChecksum() == rhs->getChecksum();
        }
    };

    using DescMap = std::unordered_map<SkDescriptor*, CacheAccum, DescHash, DescEq>;
    DescMap fDescMap{16, DescHash(), DescEq()};
    std::vector<std::unique_ptr<SkDescriptor>> fUniqueDescriptors;

    std::vector<SkPackedGlyphID> fTempGlyphs;
    std::vector<SkPackedGlyphID> runGlyphs;
};


class RemoteScalerContextFIFO : public SkRemoteScalerContext {
public:
    explicit RemoteScalerContextFIFO(Transport* transport)
        : fTransport{transport} { }
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             SkPaint::FontMetrics* metrics) override {
        gFontMetrics += 1;

        //SK_ABORT("generateFontMetrics should not be called.");
        // Send generateFontMetrics
        Op* op = this->startOpWrite(OpCode::kFontMetrics, tf, rec);
        fTransport->endWrite();

#if INSTRUMENT
        SkScalerContextRecDescriptor rd{rec};
        std::cout << " metrics font op rec tf: " << rec.fFontID
                  << " tf id: " << tf.fontID()
                  << " rec: " << rd.desc().getChecksum()
                  << rec.dump().c_str() << std::endl;
#endif
        // Receive generateFontMetrics
        op = fTransport->startRead<Op>();
        *metrics = op->fontMetrics;
        fTransport->endRead();
    }

    void generateMetricsAndImage(const SkTypefaceProxy& tf,
                                 const SkScalerContextRec& rec,
                                 SkArenaAlloc* alloc,
                                 SkGlyph* glyph) override {
        gMetricsImage += 1;
        //SK_ABORT("generateMetricsAndImage should not be called.");
        // Send generateMetricsAndImage
        SkScalerContextRecDescriptor rd{rec};

#if INSTRUMENT
        std::cout << " metrics image op rec tf: " << rec.fFontID
                  << " tf id: " << tf.fontID()
                  << " rec: " << rd.desc().getChecksum()
                  << " glyphid: " << glyph->getPackedID().getPackedID() << "\n"
                  << rec.dump().c_str() << std::endl;
#endif
        Op* op = this->startOpWrite(OpCode::kGlyphMetricsAndImage, tf, rec);
        op->glyph = *glyph;
        fTransport->endWrite();

        // Receive generateMetricsAndImage
        op = fTransport->startRead<Op>();
        *glyph = op->glyph;
        auto imageSize = op->glyph.computeImageSize();
        glyph->fPathData = nullptr;
        if (imageSize > 0) {
            auto image = fTransport->readArray<uint8_t>(imageSize);
            SkASSERT(imageSize == image.size());
            glyph->allocImage(alloc);
            memcpy(glyph->fImage, image.data(), imageSize);
        } else {
            glyph->fImage = nullptr;
        }
        fTransport->endRead();
    }

    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      SkGlyphID glyph, SkPath* path) override {
        gPath += 1;
        // Send generatePath
        SkScalerContextRecDescriptor rd{rec};

        std::cout << " path op rec tf: " << rec.fFontID
                  << " tf id: " << tf.fontID()
                  << " rec: " << rd.desc().getChecksum()
                  << " glyphid: " << glyph << std::endl;
        Op* op = this->startOpWrite(OpCode::kGlyphPath, tf, rec);
        op->glyphId = glyph;
        fTransport->endWrite();

        op = fTransport->startRead<Op>();
        auto rawPath = fTransport->readArray<uint8_t>(op->pathSize);
        path->readFromMemory(rawPath.data(), rawPath.size());
        fTransport->endRead();
    }

private:
    Op* startOpWrite(OpCode opCode, const SkTypefaceProxy& tf,
                     const SkScalerContextRec& rec) {
        return fTransport->startEmplace<Op>(opCode, tf.fontID(), rec);
    }

    Transport* const fTransport;
};

static void prepopulate_cache(
    Transport* transport,
    SkRemoteGlyphCacheGPU* cache,
    sk_sp<SkPicture> pic,
    TextBlobFilterCanvas* filter) {

    pic->playback(filter);

    transport->startEmplace<Op>(OpCode::kPrepopulateCache, SkFontID{0},
                                SkScalerContextRec{});
    filter->writeSpecToTransport(transport);
    transport->endWrite();

    SkExclusiveStrikePtr strike;

    auto perStrike = [&strike, cache](TextBlobFilterCanvas::StrikeSpec* spec,
                                          SkDescriptor* desc,
                                          SkPaint::FontMetrics* fontMetrics) {
        auto tf = cache->lookupTypeface(spec->typefaceID);
        // TODO: implement effects handling.
        SkScalerContextEffects effects;
        if ((strike = SkGlyphCache::FindStrikeExclusive(*desc)) == nullptr) {
            auto scaler = SkGlyphCache::CreateScalerContext(*desc, effects, *tf);
            strike = SkGlyphCache::CreateStrikeExclusive(*desc, std::move(scaler), fontMetrics);
        }
#if INSTRUMENT
        std::cout << std::hex << "prepop cache " << (intptr_t)cache
                  << " desc: " << desc->getChecksum()
                  << " typeface id: " << tf->uniqueID()
                  << " glyph count: " << spec->glyphCount << std::endl;
        auto rec = (SkScalerContextRec*)desc->findEntry(kRec_SkDescriptorTag, nullptr);
        SkDebugf("%s\n", rec->dump().c_str());
#endif

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
    Transport in = Transport::DoubleBuffer(*transport);
#if INSTRUMENT
    SkDebugf("========= Sending prep cache ========\n");
#endif

    in.startRead();
    filter->readDataFromTransport(&in, perStrike, perGlyph, finishStrike);
    in.endRead();
}

std::string gSkpName;
static void final_draw(std::string outFilename,
                       Transport* transport,
                       SkDeserialProcs* procs,
                       SkData* picData,
                       SkRemoteGlyphCacheGPU* cache) {

    auto pic = SkPicture::MakeFromData(picData, procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();

    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());
    auto c = s->getCanvas();
    auto picUnderTest = SkPicture::MakeFromData(picData, procs);

    SkMatrix deviceMatrix = SkMatrix::I();
    // kFakeGammaAndBoostContrast
    TextBlobFilterCanvas filter(
        r.width(), r.height(), deviceMatrix, s->props(),
        SkScalerContextFlags::kFakeGammaAndBoostContrast);

    if (cache != nullptr) {
        for (int i = 0; i < 0; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            prepopulate_cache(transport, cache, picUnderTest, &filter);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            (void)elapsed_seconds;
            if (i == 0) {
                std::cout << "filter time: " << elapsed_seconds.count() * 1e6
                          << "us size: " << transport->size() << std::endl;
            }
        }
    }

    std::chrono::duration<double> total_seconds{0.0};
    for (int i = 0; i < 100; i++) { // 20
        if (gPurgeFontCaches) {
            SkGraphics::PurgeFontCache();
        }
        auto start = std::chrono::high_resolution_clock::now();
        if (cache != nullptr) {
            prepopulate_cache(transport, cache, picUnderTest, &filter);
        }
        c->drawPicture(picUnderTest);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        total_seconds += elapsed_seconds;
    }

    std::cout << "useProcess: " << gUseProcess
              << " useGPU: " << gUseGpu
              << " purgeCache: " << gPurgeFontCaches << std::endl;
    fprintf(stderr, "%s use GPU %s elapsed time %8.6f s\n", gSkpName.c_str(),
            gUseGpu ? "true" : "false", total_seconds.count());
    /*std::cerr << gSkpName << " use GPU " << std::boolalpha << gUseGpu << " elapsed time: "
              << std::fixed << std::setw( 6 ) << std::setprecision( 1 )
              << total_seconds.count() << " s\n";*/

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f(outFilename.c_str());
    f.write(data->data(), data->size());
}

static void gpu(int readFd, int writeFd) {

    Transport transport{readFd, writeFd};

    auto picData = transport.readEntireData();
    if (picData == nullptr) {
        return;
    }

    SkRemoteGlyphCacheGPU rc{
        skstd::make_unique<RemoteScalerContextFIFO>(&transport)
    };
    SkDeserialProcs procs;
    rc.prepareDeserializeProcs(&procs);

    final_draw("test.png", &transport, &procs, picData.get(), &rc);

    if (gFontMetrics + gMetricsImage + gPath > 0) {
        fprintf(stderr, "exceptions - fm: %d mi: %d p: %d\n", gFontMetrics, gMetricsImage, gPath);
    }
}

static int renderer(
    const std::string& skpName, int readFd, int writeFd)
{
    Transport transport{readFd, writeFd};

    auto skpData = SkData::MakeFromFileName(skpName.c_str());
    std::cout << "skp stream is " << skpData->size() << " bytes long " << std::endl;

    SkRemoteGlyphCacheRenderer rc;
    SkSerialProcs procs;
    sk_sp<SkData> stream;
    if (gUseGpu) {
        auto pic = SkPicture::MakeFromData(skpData.get());
        rc.prepareSerializeProcs(&procs);
        stream = pic->serialize(&procs);
    } else {
        stream = skpData;
    }

    std::cout << "stream is " << stream->size() << " bytes long" << std::endl;

    if (!gUseGpu) {
        final_draw("test-direct.png", &transport, nullptr, stream.get(), nullptr);
        return 0;
    }

    if (transport.writeEntireData(*stream) == Transport::kFail) {
        return 1;
    }

    std::cout << "Waiting for scaler context ops." << std::endl;

    while (true) {

        // Share the buffer between read and write.
        Op* op = transport.startRead<Op>();
        if (op == nullptr) { std::cout << "Exit op loop" << std::endl; break;}

            switch (op->opCode) {
                case OpCode::kFontMetrics : {
                    auto sc = rc.generateScalerContext(op->descriptor, op->typefaceId);
                    sc->getFontMetrics(&op->fontMetrics);
                    transport.endWrite();
                    break;
                }
                case OpCode::kGlyphPath : {
                    auto sc = rc.generateScalerContext(op->descriptor, op->typefaceId);
                    // TODO: check for buffer overflow.
                    SkPath path;
                    sc->getPath(op->glyphId, &path);
                    size_t pathSize = path.writeToMemory(nullptr);
                    auto pathData = transport.allocateArray<uint8_t>(pathSize);
                    op->pathSize = path.writeToMemory(pathData);
                    transport.endWrite();
                    break;
                }
                case OpCode::kGlyphMetricsAndImage : {
                    auto sc = rc.generateScalerContext(op->descriptor, op->typefaceId);

                    // TODO: check for buffer overflow.
                    auto glyphId = op->glyph.getPackedID();
                    op->glyph.initWithGlyphID(glyphId);
                    sc->getMetrics(&op->glyph);
                    auto imageSize = op->glyph.computeImageSize();
                    op->glyph.fPathData = nullptr;

                    if (imageSize > 0) {
                        op->glyph.fImage = transport.allocateArray<uint8_t>(imageSize);
                        sk_bzero(op->glyph.fImage, imageSize);
                        sc->getImage(op->glyph);
                    } else {
                        op->glyph.fImage = nullptr;
                    }
                    transport.endWrite();
                    break;
                }
                case OpCode::kPrepopulateCache : {

                    Transport& in = transport;
                    Transport out = Transport::DoubleBuffer(transport);

                    out.startWrite();
                    TextBlobFilterCanvas::WriteDataToTransport(&in ,&out, &rc);
                    out.endWrite();
                    in.endRead();

                    //std::cout << "read prepopulate spec size: " << in.size() << std::endl;
                    //std::cout << "write prepopulate data size: " << out.size() << std::endl;
                    break;
                }
                default:
                    SK_ABORT("Bad op");
            }
    }

    std::cout << "Returning from render" << std::endl;

    return 0;
}

static void start_gpu(int render_to_gpu[2], int gpu_to_render[2]) {
    std::cout << "gpu - Starting GPU" << std::endl;
    close(gpu_to_render[kRead]);
    close(render_to_gpu[kWrite]);
    gpu(render_to_gpu[kRead], gpu_to_render[kWrite]);
}

static void start_render(std::string& skpName, int render_to_gpu[2], int gpu_to_render[2]) {
    std::cout << "renderer - Starting Renderer" << std::endl;
    close(render_to_gpu[kRead]);
    close(gpu_to_render[kWrite]);
    renderer(skpName, gpu_to_render[kRead], render_to_gpu[kWrite]);
}

int main(int argc, char** argv) {
    std::string skpName = argc > 1 ? std::string{argv[1]} : std::string{"skps/desk_nytimes.skp"};
    int mode = argc > 2 ? atoi(argv[2]) : -1;
    printf("skp: %s\n", skpName.c_str());

    gSkpName = skpName;

    int render_to_gpu[2],
        gpu_to_render[2];

    for (int m = 0; m < 8; m++) {
        int r = pipe(render_to_gpu);
        if (r < 0) {
            perror("Can't write picture from render to GPU ");
            return 1;
        }
        r = pipe(gpu_to_render);
        if (r < 0) {
            perror("Can't write picture from render to GPU ");
            return 1;
        }

        gPurgeFontCaches = (m & 4) == 4;
        gUseGpu = (m & 2) == 2;
        gUseProcess = (m & 1) == 1;

        if (mode >= 0 && mode < 8 && mode != m) {
            continue;
        }

        if (gUseProcess) {
            pid_t child = fork();
            SkGraphics::Init();

            if (child == 0) {
                start_gpu(render_to_gpu, gpu_to_render);
            } else {
                start_render(skpName, render_to_gpu, gpu_to_render);
                waitpid(child, nullptr, 0);
            }
        } else {
            SkGraphics::Init();
            std::thread(gpu, render_to_gpu[kRead], gpu_to_render[kWrite]).detach();
            renderer(skpName, gpu_to_render[kRead], render_to_gpu[kWrite]);
        }
    }

    return 0;
}


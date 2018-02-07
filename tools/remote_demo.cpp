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

#include <type_traits>
#include <chrono>
#include <ctype.h>
#include <err.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <unordered_map>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <SkFindAndPlaceGlyph.h>
#include "SkTypeface_remote.h"
#include "SkRemoteGlyphCache.h"
#include "SkMakeUnique.h"

static const size_t kPageSize = 4096;

static bool gUseGpu = true;
static bool gPurgeFontCaches = true;
static bool gUseProcess = true;

enum direction : int {kRead = 0, kWrite = 1};

class Transport {
public:
    enum IOResult : bool {kFail = false, kSuccess = true};

    Transport(int readFd, int writeFd) : fReadFd{readFd}, fWriteFd{writeFd} {}

    ~Transport() {
        close(fWriteFd);
        close(fWriteFd);
    }

    template <typename T>
    T* startRead() {
        fCursor = 0;
        fEnd = 0;
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
    T* readArray(int count) {
        size_t size = count * sizeof(T);
        T* result = (T*)this->ensureAtLeast(size);
        fCursor += size;
        return result;
    }

    void endRead() {}

    sk_sp<SkData> readEntireData() {
        size_t* size = this->startRead<size_t>();
        uint8_t* data = this->readArray<uint8_t>(*size);
        if (size == nullptr || data == nullptr) {
            this->endRead();
            return sk_sp<SkData>(nullptr);
        }
        auto result = SkData::MakeWithCopy(data, *size);
        this->endRead();
        return result;
    }

    template <typename T>
    void startWrite(const T& data) {
        fCursor = 0;
        this->write<T>(data);
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

    IOResult endWrite() {
        if(::write(fWriteFd, fBuffer.get(), fCursor) < 0) {
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
                return kFail;
            }
            readSoFar += readSize;
        }
        fEnd += readSoFar;
        return kSuccess;
    }

    static constexpr size_t kBufferSize = kPageSize * 512;
    const int fReadFd,
              fWriteFd;

    std::unique_ptr<uint8_t[]> fBuffer{new uint8_t[kBufferSize]};
    size_t fCursor{0};
    size_t fEnd{0};
};

enum class OpCode : int32_t {
    kFontMetrics          = 0,
    kGlyphMetrics         = 1,
    kGlyphImage           = 2,
    kGlyphPath            = 3,
    kGlyphMetricsAndImage = 4,
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
        // op 1 and 2
        SkGlyph glyph;
        // op 3
        struct {
            SkGlyphID glyphId;
            size_t pathSize;
        };
    };
};

class RemoteScalerContextFIFO : public SkRemoteScalerContext {
public:
    explicit RemoteScalerContextFIFO(int readFd, int writeFd)
        : fReadFd{readFd}
        , fWriteFd{writeFd} { }
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             SkPaint::FontMetrics* metrics) override {
        Op* op = this->createOp(OpCode::kFontMetrics, tf, rec);
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        memcpy(metrics, &op->fontMetrics, sizeof(op->fontMetrics));
        op->~Op();
    }

    void generateMetrics(const SkTypefaceProxy& tf,
                         const SkScalerContextRec& rec,
                         SkGlyph* glyph) override {
        Op* op = this->createOp(OpCode::kGlyphMetrics, tf, rec);
        memcpy(&op->glyph, glyph, sizeof(*glyph));
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        memcpy(glyph, &op->glyph, sizeof(op->glyph));
        op->~Op();
    }

    void generateImage(const SkTypefaceProxy& tf,
                       const SkScalerContextRec& rec,
                       const SkGlyph& glyph) override {
        SK_ABORT("generateImage should not be called.");
        Op* op = this->createOp(OpCode::kGlyphImage, tf, rec);
        memcpy(&op->glyph, &glyph, sizeof(glyph));
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        memcpy(glyph.fImage, fBuffer + sizeof(Op), glyph.rowBytes() * glyph.fHeight);
        op->~Op();
    }

    void generateMetricsAndImage(const SkTypefaceProxy& tf,
                                 const SkScalerContextRec& rec,
                                 SkArenaAlloc* alloc,
                                 SkGlyph* glyph) override {
        Op* op = this->createOp(OpCode::kGlyphMetricsAndImage, tf, rec);
        memcpy(&op->glyph, glyph, sizeof(op->glyph));
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        memcpy(glyph, &op->glyph, sizeof(*glyph));
        glyph->allocImage(alloc);
        memcpy(glyph->fImage, fBuffer + sizeof(Op), glyph->rowBytes() * glyph->fHeight);
        op->~Op();
    }

    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      SkGlyphID glyph, SkPath* path) override {
        Op* op = this->createOp(OpCode::kGlyphPath, tf, rec);
        op->glyphId = glyph;
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        path->readFromMemory(fBuffer + sizeof(Op), op->pathSize);
        op->~Op();
    }

private:
    Op* createOp(OpCode opCode, const SkTypefaceProxy& tf,
                 const SkScalerContextRec& rec) {
        Op* op = new (fBuffer) Op(opCode, tf.fontID(), rec);

        return op;
    }

    const int fReadFd,
              fWriteFd;
    uint8_t   fBuffer[1024 * kPageSize];
};

class TextBlobFilterCanvas : public SkNoDrawCanvas {
public:
    TextBlobFilterCanvas(int width, int height,
                         const SkMatrix& deviceMatrix,
                         const SkSurfaceProps& props,
                         SkScalerContextFlags flags)
        : SkNoDrawCanvas(width, height)
        , fDeviceMatrix{deviceMatrix}
        , fSurfaceProps{props}
        , fScalerContextFlags{flags} { }

    size_t prepareForBufferFill()  {
        size_t wireSize = sizeof(Header);
        for (auto& i : fDescMap) {
            size_t strikeSize = sizeof(Strike);
            auto accum = &i.second;
            auto strike = &accum->strike;
            auto glyphCount = accum->glyphIDs->count();
            strikeSize += glyphCount * sizeof(SkPackedGlyphID);
            strike->glyphCount = glyphCount;
            auto descLength = accum->desc->getLength();
            strikeSize += descLength;
            strike->descLength = descLength;
            strike->size = strikeSize;
            wireSize += strikeSize;
        }
        fHeader.size = wireSize;
        fHeader.strikeCount = fDescMap.size();
        return wireSize;
    }

    void fillInBuffer(uint8_t* buffer, size_t size) {
        SkASSERT(fHeader.size > 0);
        SkASSERT(fHeader.size <= size);
        auto cursor = buffer;
        cursor = write(fHeader, cursor);
        for (auto& i : fDescMap) {
            auto accum = &i.second;
            auto strike = &accum->strike;
            cursor = write(*strike, cursor);
            cursor = write(*accum->desc, cursor);
            accum->glyphIDs->foreach([&](SkPackedGlyphID id) {
                cursor = write(id, cursor);
            });
        }
    }

protected:
    void onDrawTextBlob(
        const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) override
    {
        SkMatrix blobMatrix{fDeviceMatrix};
        blobMatrix.preConcat(this->getTotalMatrix());
        if (blobMatrix.hasPerspective()) {return;}
        blobMatrix.preTranslate(x, y);

        SkTextBlobRunIterator it(blob);
        for (;!it.done(); it.next()) {
            this->processGlyphRun(blobMatrix, paint, it);
        }
    }

private:
    using PosFn = SkPoint(*)(int index, const SkScalar* pos);
    using MapFn = SkPoint(*)(const SkMatrix& m, SkPoint pt);

    struct Strike {
        size_t size;
        SkFontID typefaceID;
        uint32_t descLength;
        int glyphCount;
        /* desc */
        /* glyphs */
    };

    struct Header {
        size_t size;
        int strikeCount;
    };

    struct CacheAccum {
        Strike strike;
        SkDescriptor* desc;
        //std::vector<SkPackedGlyphID> glyphIDs;
        std::unique_ptr<SkTHashSet<SkPackedGlyphID>> glyphIDs;
    };

    template <typename T>
    uint8_t* write(const std::vector<T>& v, uint8_t* cursor) {
        size_t sizeBytes = v.size() * sizeof(T);
        memcpy(cursor, v.data(), sizeBytes);
        return cursor + sizeBytes;
    }

    uint8_t* write(const SkDescriptor& desc, uint8_t* cursor) {
        memcpy(cursor, &desc, desc.getLength());
        return cursor + desc.getLength();
    }

    template <typename T>
    uint8_t* write(const T& data, uint8_t* cursor) {
        // TODO: guard against bad T.
        memcpy(cursor, &data, sizeof(data));
        return cursor + sizeof(data);
    }

    size_t strikeSize(const CacheAccum& accum) const {
        size_t wireSize = sizeof(Strike);
        wireSize += accum.desc->getLength();
        wireSize += accum.glyphIDs->count() * sizeof(SkPackedGlyphID);
        return wireSize;
    }

    void processGlyphRun(
        const SkMatrix& blobMatrix,
        const SkPaint& paint,
        const SkTextBlobRunIterator& it) {

        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        SkPaint runPaint{paint};
        it.applyFontToPaint(&runPaint);

        // All other alignment modes need the glyph advances. Use the slow drawing mode.
        if (runPaint.getTextAlign() != SkPaint::kLeft_Align) {
            return;
        }

        SkMatrix runMatrix{blobMatrix};
        runMatrix.preTranslate(it.offset().x(), it.offset().y());

        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;

        SkScalerContext::MakeRecAndEffects(runPaint, &fSurfaceProps, &runMatrix,
                                           fScalerContextFlags, &rec, &effects);

        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto cache = SkGlyphCache::DetatchCacheOrNull(*desc);

        PosFn posFn;
        SkAxisAlignment axisAlignment = SkAxisAlignment::kNone_SkAxisAlignment;
        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                // Default positioning needs advances. Can't do that.
                return;

            case SkTextBlob::kHorizontal_Positioning:
                posFn = [](int index, const SkScalar* pos) {
                    return SkPoint{pos[index], 0};
                };
                axisAlignment = SkScalerContext::ComputeAxisAlignmentForHText(rec);
                break;

            case SkTextBlob::kFull_Positioning:
                posFn = [](int index, const SkScalar* pos) {
                    return SkPoint{pos[2*index], pos[2*index + 1]};
                };
                break;

            default:
                posFn = nullptr;
                SK_ABORT("unhandled positioning mode");
        }

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
            case SkMatrix::kScale_Mask|SkMatrix::kTranslate_Mask:
                mapFn = [](const SkMatrix& m, SkPoint pt) {
                    return SkPoint{pt.x() * m.getScaleX() + m.getTranslateX(),
                                   pt.y() * m.getScaleY() + m.getTranslateY()};
                };
                break;
            case SkMatrix::kAffine_Mask|SkMatrix::kScale_Mask:
            case SkMatrix::kAffine_Mask|SkMatrix::kScale_Mask|SkMatrix::kTranslate_Mask:
                mapFn = [](const SkMatrix& m, SkPoint pt) {
                    return SkPoint{
                        pt.x() * m.getScaleX() + pt.y() * m.getSkewX()  + m.getTranslateX(),
                        pt.x() * m.getSkewY()  + pt.y() * m.getScaleY() + m.getTranslateY()};
                };
                break;
            default:
                mapFn = nullptr;
                SK_ABORT("Bad matrix.");
        }

        auto iter = fDescMap.find(desc);
        if (iter == fDescMap.end()) {
            auto newDesc = desc->copy();
            auto newDescPtr = newDesc.get();
            fUniqueDescriptors.emplace_back(std::move(newDesc));
            CacheAccum newAccum;
            newAccum.desc = newDescPtr;
            newAccum.strike.typefaceID =
                SkTypefaceProxy::DownCast(runPaint.getTypeface())->fontID();
            newAccum.glyphIDs = skstd::make_unique<SkTHashSet<SkPackedGlyphID>>();
            iter = fDescMap.emplace_hint(iter, newDescPtr, std::move(newAccum));
        }

        auto accum = &iter->second;

        auto pos = it.pos();
        const uint16_t* glyphs = it.glyphs();
        for (uint32_t index = 0; index < it.glyphCount(); index++) {
            SkIPoint subPixelPos{0, 0};
            if (runPaint.isAntiAlias()) {
                SkPoint glyphPos = mapFn(runMatrix, posFn(index, pos));
                subPixelPos = SkFindAndPlaceGlyph::SubpixelAlignment(axisAlignment, glyphPos);
            }

            if (cache && cache->isGlyphIdCached(glyphs[index], subPixelPos.x(), subPixelPos.y())) {
                continue;
            }

            SkPackedGlyphID glyphID{glyphs[index], subPixelPos.x(), subPixelPos.y()};
            accum->glyphIDs->add(glyphID);
        }


        if (cache) {
            SkGlyphCache::AttachCache(cache);
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

    Header fHeader{0, -1};
    std::vector<SkPackedGlyphID> fTempGlyphs;
    std::vector<SkPackedGlyphID> runGlyphs;
};

static void final_draw(std::string outFilename,
                       SkDeserialProcs* procs,
                       SkData* picData) {

    auto pic = SkPicture::MakeFromData(picData, procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();

    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());
    auto c = s->getCanvas();
    auto picUnderTest = SkPicture::MakeFromData(picData, procs);

    SkMatrix deviceMatrix = SkMatrix::I();
    TextBlobFilterCanvas filter(
        r.width(), r.height(), deviceMatrix, s->props(), SkScalerContextFlags::kNone);

    for (int i = 0; i < 1; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        filter.drawPicture(picUnderTest);
        filter.prepareForBufferFill();
        size_t size = filter.prepareForBufferFill();
        (void)size;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        (void)elapsed_seconds;
        if (i == 0) {
            std::cout << "filter time: " << elapsed_seconds.count() * 1e6
                      << " size: " << size << std::endl;
        }
    }

    std::chrono::duration<double> total_seconds{0.0};
    for (int i = 0; i < 20; i++) {
        if (gPurgeFontCaches) {
            SkGraphics::PurgeFontCache();
        }
        auto start = std::chrono::high_resolution_clock::now();
        c->drawPicture(picUnderTest);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        total_seconds += elapsed_seconds;
    }

    std::cout << "useProcess: " << gUseProcess
              << " useGPU: " << gUseGpu
              << " purgeCache: " << gPurgeFontCaches << std::endl;
    std::cerr << "elapsed time: " << total_seconds.count() << "s\n";

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f(outFilename.c_str());
    f.write(data->data(), data->size());
}

static void gpu(int readFd, int writeFd) {

    Transport transport{readFd, writeFd};

    auto picData = transport.readEntireData();
    /*
    size_t picSize = 0;
    ssize_t r = read(readFd, &picSize, sizeof(picSize));
    if (r > 0) {

        static constexpr size_t kBufferSize = 10 * 1024 * kPageSize;
        std::unique_ptr<uint8_t[]> picBuffer{new uint8_t[kBufferSize]};

        size_t readSoFar = 0;
        while (readSoFar < picSize) {
            ssize_t readSize;
            if ((readSize = read(readFd, &picBuffer[readSoFar], kBufferSize - readSoFar)) <= 0) {
                if (readSize == 0) return;
                err(1, "gpu pic read error %d", errno);
            }
            readSoFar += readSize;
        }





        auto picData = SkData::MakeWithCopy(picBuffer.get(), picSize);
        */

    SkRemoteGlyphCacheGPU rc{
        skstd::make_unique<RemoteScalerContextFIFO>(readFd, writeFd)
    };
    SkDeserialProcs procs;
    rc.prepareDeserializeProcs(&procs);

    final_draw("test.png", &procs, picData.get());

}

static int renderer(
    const std::string& skpName, int readFd, int writeFd)
{
    Transport transport{readFd, writeFd};
    std::string prefix{"skps/"};
    std::string fileName{prefix + skpName + ".skp"};

    auto skpData = SkData::MakeFromFileName(fileName.c_str());
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
        final_draw("test-direct.png", nullptr, stream.get());
        return 0;
    }

    if (transport.writeEntireData(*stream) == Transport::kFail) {
        return 1;
    }
    /*
    {
        size_t size = stream->size();
        write(writeFd, &size, sizeof(size));
    }

    size_t writeSoFar = 0;
    char* buffer = (char*)stream->data();
    while (writeSoFar < stream->size()) {
        ssize_t writeSize = write(writeFd, &buffer[writeSoFar], stream->size() - writeSoFar);
        if (writeSize <= 0) {
            if (writeSize == 0) {
                std::cout << "Exit" << std::endl;
                return 1;
            }
            perror("Can't write picture from render to GPU ");
            return 1;
        }
        writeSoFar += writeSize;
    }
    */
    std::cout << "Waiting for scaler context ops." << std::endl;

    static constexpr size_t kBufferSize = 1024 * kPageSize;
    std::unique_ptr<uint8_t[]> glyphBuffer{new uint8_t[kBufferSize]};

    Op* op = (Op*)glyphBuffer.get();
    while (true) {
        ssize_t size = read(readFd, glyphBuffer.get(), sizeof(*op));
        if (size <= 0) { std::cout << "Exit op loop" << std::endl; break;}
        size_t writeSize = sizeof(*op);

            auto sc = rc.generateScalerContext(op->descriptor, op->typefaceId);
            switch (op->opCode) {
                case OpCode::kFontMetrics : {
                    sc->getFontMetrics(&op->fontMetrics);
                    break;
                }
                case OpCode::kGlyphMetrics : {
                    sc->getMetrics(&op->glyph);
                    break;
                }
                case OpCode::kGlyphImage : {
                    // TODO: check for buffer overflow.
                    op->glyph.fImage = &glyphBuffer[sizeof(Op)];
                    sc->getImage(op->glyph);
                    writeSize += op->glyph.rowBytes() * op->glyph.fHeight;
                    break;
                }
                case OpCode::kGlyphPath : {
                    // TODO: check for buffer overflow.
                    SkPath path;
                    sc->getPath(op->glyphId, &path);
                    op->pathSize = path.writeToMemory(&glyphBuffer[sizeof(Op)]);
                    writeSize += op->pathSize;
                    break;
                }
                case OpCode::kGlyphMetricsAndImage : {
                    // TODO: check for buffer overflow.
                    sc->getMetrics(&op->glyph);
                    if (op->glyph.fWidth <= 0 || op->glyph.fWidth >= kMaxGlyphWidth) {
                        op->glyph.fImage = nullptr;
                        break;
                    }
                    op->glyph.fImage = &glyphBuffer[sizeof(Op)];
                    sc->getImage(op->glyph);
                    writeSize += op->glyph.rowBytes() * op->glyph.fHeight;
                    break;
                }
                default:
                    SK_ABORT("Bad op");
            }

        write(writeFd, glyphBuffer.get(), writeSize);
    }

    //close(readFd);
    //close(writeFd);

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
    std::string skpName = argc > 1 ? std::string{argv[1]} : std::string{"desk_nytimes"};
    int mode = argc > 2 ? atoi(argv[2]) : -1;
    printf("skp: %s\n", skpName.c_str());

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


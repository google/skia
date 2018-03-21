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
#include <SkFindAndPlaceGlyph.h>
#include <SkDrawLooper.h>
#include "SkTypeface_remote.h"
#include "SkRemoteGlyphCache.h"
#include "SkMakeUnique.h"

static bool gUseGpu = true;
static bool gPurgeFontCaches = true;
static bool gUseProcess = true;

static int gFontMetrics;
static int gMetricsImage;
static int gPath;

enum direction : int {kRead = 0, kWrite = 1};

#define INSTRUMENT 0

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
                  << " tf id: " << tf.remoteTypefaceID()
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
        return fTransport->startEmplace<Op>(opCode, tf.remoteTypefaceID(), rec);
    }

    Transport* const fTransport;
};

static void prepopulate_cache(
    Transport* transport,
    SkRemoteGlyphCacheGPU* cache,
    sk_sp<SkPicture> pic,
    SkIRect bounds, const SkSurfaceProps& props) {

    SkMatrix deviceMatrix = SkMatrix::I();

    SkStrikeCacheDifferenceSpec strikeDifference;
    SkTextBlobCacheDiffCanvas filter(
            bounds.width(), bounds.height(), deviceMatrix, props,
            SkScalerContextFlags::kFakeGammaAndBoostContrast,
            &strikeDifference);

    pic->playback(&filter);

    transport->startEmplace<Op>(OpCode::kPrepopulateCache, SkFontID{0},
                                SkScalerContextRec{});
    strikeDifference.writeSpecToTransport(transport);
    transport->endWrite();

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
    filter.readDataFromTransport(&in, perStrike, perGlyph, finishStrike);
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

    SkStrikeCacheDifferenceSpec strikeDifference;
    SkTextBlobCacheDiffCanvas filter(
        r.width(), r.height(), deviceMatrix, s->props(),
        SkScalerContextFlags::kFakeGammaAndBoostContrast,
        &strikeDifference);

    if (cache != nullptr) {
        for (int i = 0; i < 0; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            prepopulate_cache(transport, cache, picUnderTest, r, s->props());

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
            prepopulate_cache(transport, cache, picUnderTest, r, s->props());
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
                    SkTextBlobCacheDiffCanvas::WriteDataToTransport(&in ,&out, &rc);
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


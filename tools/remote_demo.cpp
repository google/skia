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

class ReadWriteTransport : public SkTransport {
public:
    ReadWriteTransport(int readFd, int writeFd) : fReadFd{readFd}, fWriteFd{writeFd} {}
    ~ReadWriteTransport() override {
        // TODO: turn this back on after old transport is gone.
        //close(fWriteFd);
        //close(fReadFd);
    }
    IOResult write(void* buffer, size_t size) override {
        ssize_t writeSize = ::write(fWriteFd, buffer, size);
        if (writeSize < 0) {
            err(1,"Failed write %zu", size);
            return kFail;
        }
        return kSuccess;
    }

    std::tuple<size_t, IOResult> read(void* buffer, size_t size) override {
        ssize_t readSize = ::read(fReadFd, buffer, size);
        if (readSize < 0) {
            err(1,"Failed read %zu", size);
            return {size, kFail};
        }
        return {readSize, kSuccess};
    }

private:
    const int fReadFd,
              fWriteFd;
};

class RemoteScalerContextRW : public SkRemoteScalerContext {
public:
    explicit RemoteScalerContextRW(AllInOneTransport* transport)
        : fTransport{transport} { }
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             SkPaint::FontMetrics* metrics) override {
        gFontMetrics += 1;

        //SK_ABORT("generateFontMetrics should not be called.");
        // Send generateFontMetrics
        this->startOpWrite(OpCode::kFontMetrics, tf, rec);
        fTransport->endWrite();

        // Receive generateFontMetrics
        *metrics = *fTransport->startRead<SkPaint::FontMetrics>();
        fTransport->endRead();
    }

    void generateMetricsAndImage(const SkTypefaceProxy& tf,
                                 const SkScalerContextRec& rec,
                                 SkArenaAlloc* alloc,
                                 SkGlyph* glyph) override {
        gMetricsImage += 1;
        SkScalerContextRecDescriptor rd{rec};

        Op* op = this->startOpWrite(OpCode::kGlyphMetricsAndImage, tf, rec);
        op->glyph = *glyph;
        fTransport->endWrite();

        // Receive generateMetricsAndImage
        *glyph = *fTransport->startRead<SkGlyph>();
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

        Op* op = this->startOpWrite(OpCode::kGlyphPath, tf, rec);
        op->glyphId = glyph;
        fTransport->endWrite();

        size_t pathSize = *fTransport->startRead<size_t>();
        auto rawPath = fTransport->readArray<uint8_t>(pathSize);
        path->readFromMemory(rawPath.data(), rawPath.size());
        fTransport->endRead();
    }

private:
    Op* startOpWrite(OpCode opCode, const SkTypefaceProxy& tf,
                     const SkScalerContextRec& rec) {
        return fTransport->startEmplace<Op>(opCode, tf.remoteTypefaceID(), rec);
    }

    AllInOneTransport* const fTransport;
};

std::string gSkpName;
static void final_draw(std::string outFilename,
                       SkTransport* transport,
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
            SkPrepopulateCache(transport, cache, picUnderTest, r, s->props());

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            (void)elapsed_seconds;
            if (i == 0) {
                std::cout << "filter time: " << elapsed_seconds.count() * 1e6 << std::endl;
            }
        }
    }

    std::chrono::duration<double> total_seconds{0.0};
    for (int i = 0; i < 1; i++) { // 20
        if (gPurgeFontCaches) {
            SkGraphics::PurgeFontCache();
        }
        auto start = std::chrono::high_resolution_clock::now();
        if (cache != nullptr) {
            SkPrepopulateCache(transport, cache, picUnderTest, r, s->props());
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

    AllInOneTransport transport{readFd, writeFd};
    ReadWriteTransport rwTransport{readFd, writeFd};


    auto picData = transport.readEntireData();
    if (picData == nullptr) {
        return;
    }

    SkRemoteGlyphCacheGPU rc{
        skstd::make_unique<RemoteScalerContextRW>(&transport)
    };

    SkDeserialProcs procs;
    rc.prepareDeserializeProcs(&procs);

    final_draw("test.png", &rwTransport, &procs, picData.get(), &rc);

    if (gFontMetrics + gMetricsImage + gPath > 0) {
        fprintf(stderr, "exceptions - fm: %d mi: %d p: %d\n", gFontMetrics, gMetricsImage, gPath);
    }

    close(writeFd);
    close(readFd);
    printf("GPU is exiting\n");
}

static int renderer(
    const std::string& skpName, int readFd, int writeFd)
{
    AllInOneTransport transport{readFd, writeFd};
    ReadWriteTransport rwTransport{readFd, writeFd};
    SkStrikeServer server{&rwTransport};

    auto skpData = SkData::MakeFromFileName(skpName.c_str());
    std::cout << "skp stream is " << skpData->size() << " bytes long " << std::endl;

    SkSerialProcs procs;
    sk_sp<SkData> stream;
    if (gUseGpu) {
        auto pic = SkPicture::MakeFromData(skpData.get());
        server.prepareSerializeProcs(&procs);
        stream = pic->serialize(&procs);
    } else {
        stream = skpData;
    }

    std::cout << "stream is " << stream->size() << " bytes long" << std::endl;

    if (!gUseGpu) {
        final_draw("test-direct.png", &rwTransport, nullptr, stream.get(), nullptr);
        return 0;
    }

    if (transport.writeEntireData(*stream) == AllInOneTransport::kFail) {
        return 1;
    }

    std::cout << "Waiting for scaler context ops." << std::endl;

    return server.serve();
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


/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <chrono>
#include <err.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "SkRemoteGlyphCache.h"
#include "SkGraphics.h"
#include "SkSurface.h"

static std::string gSkpName;
static bool gUseGpu = true;
static bool gPurgeFontCaches = true;
static bool gUseProcess = true;


class ReadWriteTransport : public SkTransport {
public:
    ReadWriteTransport(int readFd, int writeFd) : fReadFd{readFd}, fWriteFd{writeFd} {}
    ~ReadWriteTransport() override {
        close(fWriteFd);
        close(fReadFd);
    }
    IOResult write(const void* buffer, size_t size) override {
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

static void prime_cache_spec(const SkIRect& bounds,
                        const SkSurfaceProps& props,
                        const SkPicture& pic,
                        SkStrikeCacheDifferenceSpec* strikeDifference) {
    SkMatrix deviceMatrix = SkMatrix::I();

    SkTextBlobCacheDiffCanvas filter(
            bounds.width(), bounds.height(), deviceMatrix, props,
            SkScalerContextFlags::kFakeGammaAndBoostContrast,
            strikeDifference);

    pic.playback(&filter);
}

static void final_draw(std::string outFilename,
                       SkTransport* transport,
                       SkDeserialProcs* procs,
                       SkData* picData,
                       SkStrikeClient* client) {

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

    if (client != nullptr) {
        for (int i = 0; i < 0; i++) {
            auto start = std::chrono::high_resolution_clock::now();


            SkStrikeCacheDifferenceSpec strikeDifference;

            prime_cache_spec(r, s->props(), *picUnderTest, &strikeDifference);

            client->primeStrikeCache(strikeDifference);

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
        if (client != nullptr) {
            SkStrikeCacheDifferenceSpec strikeDifference;
            prime_cache_spec(r, s->props(), *picUnderTest, &strikeDifference);
            client->primeStrikeCache(strikeDifference);
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

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f(outFilename.c_str());
    f.write(data->data(), data->size());
}

static void gpu(int readFd, int writeFd) {

    ReadWriteTransport rwTransport{readFd, writeFd};

    auto picData = rwTransport.readSkData();
    if (picData == nullptr) {
        return;
    }

    SkStrikeClient client{&rwTransport};

    SkDeserialProcs procs;
    client.prepareDeserializeProcs(&procs);

    final_draw("test.png", &rwTransport, &procs, picData.get(), &client);

    printf("GPU is exiting\n");
}

static int renderer(
    const std::string& skpName, int readFd, int writeFd)
{
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

        if (rwTransport.writeSkData(*stream) == SkTransport::kFail) {
            return 1;
        }

        std::cout << "Waiting for scaler context ops." << std::endl;

        return server.serve();
    } else {
        stream = skpData;
        final_draw("test-correct.png", &rwTransport, nullptr, stream.get(), nullptr);
        return 0;
    }
}


int main(int argc, char** argv) {
    std::string skpName = argc > 1 ? std::string{argv[1]} : std::string{"skps/desk_nytimes.skp"};
    int mode = argc > 2 ? atoi(argv[2]) : -1;
    printf("skp: %s\n", skpName.c_str());

    gSkpName = skpName;

    enum direction : int {kRead = 0, kWrite = 1};


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
                close(gpu_to_render[kRead]);
                close(render_to_gpu[kWrite]);
                gpu(render_to_gpu[kRead], gpu_to_render[kWrite]);
            } else {
                close(render_to_gpu[kRead]);
                close(gpu_to_render[kWrite]);
                renderer(skpName, gpu_to_render[kRead], render_to_gpu[kWrite]);
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


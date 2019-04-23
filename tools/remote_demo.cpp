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

#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "src/core/SkRemoteGlyphCache.h"
#include "src/core/SkScalerContext.h"

static std::string gSkpName;
static bool gUseGpu = true;
static bool gPurgeFontCaches = true;
static bool gUseProcess = true;

class ServerDiscardableManager : public SkStrikeServer::DiscardableHandleManager {
public:
    ServerDiscardableManager() = default;
    ~ServerDiscardableManager() override = default;

    SkDiscardableHandleId createHandle() override { return ++nextHandleId; }
    bool lockHandle(SkDiscardableHandleId handleId) override {
        return handleId > lastPurgedHandleId;
    }
    void purgeAll() { lastPurgedHandleId = nextHandleId; }

private:
    SkDiscardableHandleId nextHandleId = 0u;
    SkDiscardableHandleId lastPurgedHandleId = 0u;
};

class ClientDiscardableManager : public SkStrikeClient::DiscardableHandleManager {
public:
    class ScopedPurgeCache {
    public:
        ScopedPurgeCache(ClientDiscardableManager* manager) : fManager(manager) {
            if (fManager) fManager->allowPurging = true;
        }
        ~ScopedPurgeCache() {
            if (fManager) fManager->allowPurging = false;
        }

    private:
        ClientDiscardableManager* fManager;
    };

    ClientDiscardableManager() = default;
    ~ClientDiscardableManager() override = default;

    bool deleteHandle(SkDiscardableHandleId) override { return allowPurging; }

private:
    bool allowPurging = false;
};

static bool write_SkData(int fd, const SkData& data) {
    size_t size = data.size();
    ssize_t bytesWritten = ::write(fd, &size, sizeof(size));
    if (bytesWritten < 0) {
        err(1,"Failed write %zu", size);
        return false;
    }

    bytesWritten = ::write(fd, data.data(), data.size());
    if (bytesWritten < 0) {
        err(1,"Failed write %zu", size);
        return false;
    }

    return true;
}

static sk_sp<SkData> read_SkData(int fd) {
    size_t size;
    ssize_t readSize = ::read(fd, &size, sizeof(size));
    if (readSize <= 0) {
        if (readSize < 0) {
            err(1, "Failed read %zu", size);
        }
        return nullptr;
    }

    auto out = SkData::MakeUninitialized(size);
    auto data = (uint8_t*)out->data();

    size_t totalRead = 0;
    while (totalRead < size) {
        ssize_t sizeRead;
        sizeRead = ::read(fd, &data[totalRead], size - totalRead);
        if (sizeRead <= 0) {
            if (readSize < 0) {
                err(1, "Failed read %zu", size);
            }
            return nullptr;
        }
        totalRead += sizeRead;
    }

    return out;
}

class Timer {
public:
    void start() {
        fStart = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        auto end = std::chrono::high_resolution_clock::now();
        fElapsedSeconds += end - fStart;
    }

    double elapsedSeconds() {
        return fElapsedSeconds.count();
    }

private:
    decltype(std::chrono::high_resolution_clock::now()) fStart;
    std::chrono::duration<double>                       fElapsedSeconds{0.0};
};

static bool push_font_data(const SkPicture& pic, SkStrikeServer* strikeServer,
                           sk_sp<SkColorSpace> colorSpace, int writeFd) {
    const SkIRect bounds = pic.cullRect().round();
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas filter(bounds.width(), bounds.height(), props,
                                     strikeServer, std::move(colorSpace));
    pic.playback(&filter);

    std::vector<uint8_t> fontData;
    strikeServer->writeStrikeData(&fontData);
    auto data = SkData::MakeWithoutCopy(fontData.data(), fontData.size());
    return write_SkData(writeFd, *data);
}

static void final_draw(std::string outFilename, SkData* picData, SkStrikeClient* client,
                       ClientDiscardableManager* discardableManager, int readFd, int writeFd) {
    SkDeserialProcs procs;
    auto decode = [](const void* data, size_t length, void* ctx) -> sk_sp<SkTypeface> {
        return reinterpret_cast<SkStrikeClient*>(ctx)->deserializeTypeface(data, length);
    };
    procs.fTypefaceProc = decode;
    procs.fTypefaceCtx = client;

    auto pic = SkPicture::MakeFromData(picData, &procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();

    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());
    auto c = s->getCanvas();
    auto picUnderTest = SkPicture::MakeFromData(picData, &procs);

    Timer drawTime;
    auto randomData = SkData::MakeUninitialized(1u);
    for (int i = 0; i < 100; i++) {
        if (gPurgeFontCaches) {
            ClientDiscardableManager::ScopedPurgeCache purge(discardableManager);
            SkGraphics::PurgeFontCache();
            SkASSERT(SkGraphics::GetFontCacheUsed() == 0u);
        }

        drawTime.start();
        if (client != nullptr) {
            // Kick the renderer to send us the fonts.
            write_SkData(writeFd, *randomData);
            auto fontData = read_SkData(readFd);
            if (fontData && !fontData->isEmpty()) {
                if (!client->readStrikeData(fontData->data(), fontData->size()))
                    SK_ABORT("Bad serialization");
            }
        }
        c->drawPicture(picUnderTest);
        drawTime.stop();
    }

    std::cout << "useProcess: " << gUseProcess
              << " useGPU: " << gUseGpu
              << " purgeCache: " << gPurgeFontCaches << std::endl;
    fprintf(stderr, "%s use GPU %s elapsed time %8.6f s\n", gSkpName.c_str(),
            gUseGpu ? "true" : "false", drawTime.elapsedSeconds());

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f(outFilename.c_str());
    f.write(data->data(), data->size());
}

static void gpu(int readFd, int writeFd) {

    if (gUseGpu) {
        auto picData = read_SkData(readFd);
        if (picData == nullptr) {
            return;
        }

        sk_sp<ClientDiscardableManager> discardableManager = sk_make_sp<ClientDiscardableManager>();
        SkStrikeClient strikeClient(discardableManager);

        final_draw("test.png", picData.get(), &strikeClient, discardableManager.get(), readFd,
                   writeFd);
    }

    ::close(writeFd);
    ::close(readFd);

    printf("GPU is exiting\n");
}

static int renderer(
    const std::string& skpName, int readFd, int writeFd)
{
    ServerDiscardableManager discardableManager;
    SkStrikeServer server(&discardableManager);
    auto closeAll = [readFd, writeFd]() {
        ::close(writeFd);
        ::close(readFd);
    };

    auto skpData = SkData::MakeFromFileName(skpName.c_str());
    std::cout << "skp stream is " << skpData->size() << " bytes long " << std::endl;

    sk_sp<SkData> stream;
    if (gUseGpu) {
        auto pic = SkPicture::MakeFromData(skpData.get());
        auto colorSpace = SkColorSpace::MakeSRGB();
        SkSerialProcs procs;
        auto encode = [](SkTypeface* tf, void* ctx) -> sk_sp<SkData> {
            return reinterpret_cast<SkStrikeServer*>(ctx)->serializeTypeface(tf);
        };
        procs.fTypefaceProc = encode;
        procs.fTypefaceCtx = &server;

        stream = pic->serialize(&procs);

        if (!write_SkData(writeFd, *stream)) {
            closeAll();
            return 1;
        }

        while (true) {
            auto inBuffer = read_SkData(readFd);
            if (inBuffer == nullptr) {
                closeAll();
                return 0;
            }
            if (gPurgeFontCaches) discardableManager.purgeAll();
            push_font_data(*pic.get(), &server, colorSpace, writeFd);
        }
    } else {
        stream = skpData;
        final_draw("test-correct.png", stream.get(), nullptr, nullptr, -1, -1);
        closeAll();
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


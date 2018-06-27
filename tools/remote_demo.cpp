/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "SkData.h"
#include "SkDescriptor.h"
#include "SkGraphics.h"
#include "SkSemaphore.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

#include <chrono>
#include <ctype.h>
#include <err.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <unordered_map>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include "SkTypeface_remote.h"
#include "SkRemoteGlyphCache.h"
#include "SkMakeUnique.h"

static const size_t kPageSize = 4096;

static bool gUseGpu = true;
static bool gPurgeFontCaches = true;
static bool gUseProcess = true;

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

static void final_draw(std::string outFilename,
                       SkDeserialProcs* procs,
                       uint8_t* picData,
                       size_t picSize) {

    auto pic = SkPicture::MakeFromData(picData, picSize, procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();

    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());
    auto c = s->getCanvas();
    auto picUnderTest = SkPicture::MakeFromData(picData, picSize, procs);


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

        SkRemoteGlyphCacheGPU rc{
            skstd::make_unique<RemoteScalerContextFIFO>(readFd, writeFd)
        };

        SkDeserialProcs procs;
        rc.prepareDeserializeProcs(&procs);

        final_draw("test.png", &procs, picBuffer.get(), picSize);

    }

    close(writeFd);
    close(readFd);
}

static int renderer(
    const std::string& skpName, int readFd, int writeFd)
{
    std::string prefix{"skps/"};
    std::string fileName{prefix + skpName + ".skp"};

    auto skp = SkData::MakeFromFileName(fileName.c_str());
    std::cout << "skp stream is " << skp->size() << " bytes long " << std::endl;

    SkRemoteGlyphCacheRenderer rc;
    SkSerialProcs procs;
    sk_sp<SkData> stream;
    if (gUseGpu) {
        auto pic = SkPicture::MakeFromData(skp.get());
        rc.prepareSerializeProcs(&procs);
        stream = pic->serialize(&procs);
    } else {
        stream = skp;
    }

    std::cout << "stream is " << stream->size() << " bytes long" << std::endl;

    size_t picSize = stream->size();
    uint8_t* picBuffer = (uint8_t*) stream->data();

    if (!gUseGpu) {
        final_draw("test-direct.png", nullptr, picBuffer, picSize);
        close(writeFd);
        close(readFd);
        return 0;
    }

    write(writeFd, &picSize, sizeof(picSize));

    size_t writeSoFar = 0;
    while (writeSoFar < picSize) {
        ssize_t writeSize = write(writeFd, &picBuffer[writeSoFar], picSize - writeSoFar);
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

    close(readFd);
    close(writeFd);

    std::cout << "Returning from render" << std::endl;

    return 0;
}

enum direction : int {kRead = 0, kWrite = 1};

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


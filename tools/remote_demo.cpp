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

static const size_t kPageSize = 4096;

struct WireTypeface {
    std::thread::id thread_id;
    SkFontID        typeface_id;
    SkFontStyle     style;
    bool            is_fixed;
};

class ScalerContextRecDescriptor {
public:
    explicit ScalerContextRecDescriptor(const SkScalerContextRec& rec) {
        auto desc = reinterpret_cast<SkDescriptor*>(&fDescriptor);
        desc->init();
        desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    }

    const SkDescriptor& desc() const {
        return *reinterpret_cast<const SkDescriptor*>(&fDescriptor);
    }

    struct Hash {
        size_t operator()(ScalerContextRecDescriptor const& s) const {
            return SkOpts::hash_fn(&s.desc(), sizeof(s), 0);
        }
    };

    struct Equal {
        bool operator()( const ScalerContextRecDescriptor& lhs,
                         const ScalerContextRecDescriptor& rhs ) const {
            return lhs.desc() == rhs.desc();
        }
    };

private:
    // The system only passes descriptors without effects. That is why it uses a fixed size
    // descriptor. storageFor is needed because some of the constructors below are private.
    template <typename T>
    using storageFor = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    struct {
        storageFor<SkDescriptor>        dummy1;
        storageFor<SkDescriptor::Entry> dummy2;
        storageFor<SkScalerContextRec>  dummy3;
    } fDescriptor;
};

class Op {
public:
    explicit Op(const SkScalerContextRec& rec) : descriptor{rec} {}
    int32_t op;
    SkFontID typeface_id;
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
    ScalerContextRecDescriptor descriptor;
};

class RemoteScalerContextPassThread : public SkRemoteScalerContext {
public:
    explicit RemoteScalerContextPassThread(int readFd, int writeFd)
        : fReadFd{readFd}
        , fWriteFd{writeFd} { }
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             SkPaint::FontMetrics* metrics) override {
        Op* op = this->createOp(0, tf, rec);
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        memcpy(metrics, &op->fontMetrics, sizeof(op->fontMetrics));
        op->~Op();
    }

    void generateMetrics(const SkTypefaceProxy& tf,
                         const SkScalerContextRec& rec,
                         SkGlyph* glyph) override {
        Op* op = this->createOp(1, tf, rec);
        memcpy(&op->glyph, glyph, sizeof(*glyph));
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        memcpy(glyph, &op->glyph, sizeof(op->glyph));
        op->~Op();
    }

    void generateImage(const SkTypefaceProxy& tf,
                       const SkScalerContextRec& rec,
                       const SkGlyph& glyph) override {
        Op* op = this->createOp(2, tf, rec);
        memcpy(&op->glyph, &glyph, sizeof(glyph));
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        //memcpy((SkGlyph *)&glyph, &op->glyph, sizeof(op->glyph));
        //((SkGlyph*)&glyph)->fImage = oldImage;
        memcpy(glyph.fImage, fBuffer + sizeof(Op), glyph.rowBytes() * glyph.fHeight);
        op->~Op();
    }

    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      SkGlyphID glyph, SkPath* path) override {
        Op* op = this->createOp(3, tf, rec);
        op->glyphId = glyph;
        write(fWriteFd, fBuffer, sizeof(*op));
        read(fReadFd, fBuffer, sizeof(fBuffer));
        path->readFromMemory(fBuffer + sizeof(Op), op->pathSize);
        op->~Op();
    }

private:
    Op* createOp(uint32_t opID, const SkTypefaceProxy& tf,
                 const SkScalerContextRec& rec) {
        Op* op = new (fBuffer) Op(rec);
        op->op = opID;
        op->typeface_id = tf.fontID();

        return op;
    }

    const int fReadFd,
              fWriteFd;
    uint8_t   fBuffer[1024 * kPageSize];
};

static sk_sp<SkTypeface> gpu_from_renderer_by_ID(const void* buf, size_t len, void* ctx) {
    WireTypeface wire;
    if (len >= sizeof(wire)) {
        memcpy(&wire, buf, sizeof(wire));
        return sk_sp<SkTypeface>(
                new SkTypefaceProxy(
                        wire.typeface_id,
                        wire.thread_id,
                        wire.style,
                        wire.is_fixed,
                        (SkRemoteScalerContext*) ctx));
    }
    return nullptr;
}

std::unordered_map<SkFontID, sk_sp<SkTypeface>> gTypefaceMap;


// TODO: Figure out how to manage the entries.
std::unordered_map<ScalerContextRecDescriptor,
                   std::unique_ptr<SkScalerContext>,
                   ScalerContextRecDescriptor::Hash,
                   ScalerContextRecDescriptor::Equal>
    gScalerContextMap(16,
                      ScalerContextRecDescriptor::Hash(),
                      ScalerContextRecDescriptor::Equal());

static SkScalerContext* scaler_context_from_op(Op* op) {

    SkScalerContext* sc;
    auto j = gScalerContextMap.find(op->descriptor);
    if (j != gScalerContextMap.end()) {
        sc = j->second.get();
    } else {
        auto i = gTypefaceMap.find(op->typeface_id);
        if (i == gTypefaceMap.end()) {
            std::cerr << "bad typeface id: " << op->typeface_id << std::endl;
            SK_ABORT("unknown type face");
        }
        auto tf = i->second;
        SkScalerContextEffects effects;
        auto mapSc = tf->createScalerContext(effects, &op->descriptor.desc(), false);
        sc = mapSc.get();
        gScalerContextMap.emplace_hint(j, op->descriptor, std::move(mapSc));
    }
    return sc;

}

static sk_sp<SkData> renderer_to_gpu_by_ID(SkTypeface* tf, void* ctx) {
    WireTypeface wire = {
            std::this_thread::get_id(),
            SkTypeface::UniqueID(tf),
            tf->fontStyle(),
            tf->isFixedPitch()
    };
    auto i = gTypefaceMap.find(SkTypeface::UniqueID(tf));
    if (i == gTypefaceMap.end()) {
        gTypefaceMap.insert({SkTypeface::UniqueID(tf), sk_ref_sp(tf)});
    }
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

static void final_draw(std::string outFilename,
                       SkDeserialProcs* procs,
                       uint8_t* picData,
                       size_t picSize) {
    auto start = std::chrono::high_resolution_clock::now();

    auto pic = SkPicture::MakeFromData(picData, picSize, procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();
    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());

    auto c = s->getCanvas();
    c->drawPicture(pic);

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f(outFilename.c_str());
    f.write(data->data(), data->size());
}

static void gpu(int readFd, int writeFd) {

    size_t picSize = 0;
    read(readFd, &picSize, sizeof(picSize));

    static constexpr size_t kBufferSize = 10 * 1024 * kPageSize;
    std::unique_ptr<uint8_t[]> picBuffer{new uint8_t[kBufferSize]};

    size_t readSoFar = 0;
    while (readSoFar < picSize) {
        ssize_t readSize;
        if((readSize = read(readFd, &picBuffer[readSoFar], kBufferSize - readSoFar)) <= 0) {
            if (readSize == 0) return;
            err(1, "gpu pic read error %d", errno);
        }
        readSoFar += readSize;
    }

    SkDeserialProcs procs;
    std::unique_ptr<SkRemoteScalerContext> rsc{
            new RemoteScalerContextPassThread{readFd, writeFd}};
    procs.fTypefaceProc = gpu_from_renderer_by_ID;
    procs.fTypefaceCtx = rsc.get();
    final_draw("test.png", &procs, picBuffer.get(), picSize);
    /*
    auto pic = SkPicture::MakeFromData(picBuffer.get(), picSize, &procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();
    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());

    auto c = s->getCanvas();
    c->drawPicture(pic);

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f("test.png");
    f.write(data->data(), data->size());
     */
    close(writeFd);
    close(readFd);
}

static int renderer(const std::string& skpName, int readFd, int writeFd) {
    std::string prefix{"skps/"};
    std::string fileName{prefix + skpName + ".skp"};

    auto skp = SkData::MakeFromFileName(fileName.c_str());
    auto pic = SkPicture::MakeFromData(skp.get());

    bool toGpu = true;

    SkSerialProcs procs;
    if (toGpu) {
        procs.fTypefaceProc = renderer_to_gpu_by_ID;
    }

    auto stream = pic->serialize(&procs);

    std::cerr << "stream is " << stream->size() << " bytes long" << std::endl;

    size_t picSize = stream->size();
    uint8_t* picBuffer = (uint8_t*) stream->data();

    if (!toGpu) {
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
                std::cerr << "Exit" << std::endl;
                return 1;
            }
            perror("Can't write picture from render to GPU ");
            return 1;
        }
        writeSoFar += writeSize;
    }
    std::cerr << "Waiting for scaler context ops." << std::endl;

    static constexpr size_t kBufferSize = 1024 * kPageSize;
    std::unique_ptr<uint8_t[]> glyphBuffer{new uint8_t[kBufferSize]};

    Op* op = (Op*)glyphBuffer.get();
    while (true) {
        ssize_t size = read(readFd, glyphBuffer.get(), sizeof(*op));
        if (size <= 0) { std::cerr << "Exit op loop" << std::endl; break;}
        size_t writeSize = sizeof(*op);

            auto sc = scaler_context_from_op(op);
            switch (op->op) {
                case 0: {
                    sc->getFontMetrics(&op->fontMetrics);
                    break;
                }
                case 1: {
                    sc->getMetrics(&op->glyph);
                    break;
                }
                case 2: {
                    // TODO: check for buffer overflow.
                    op->glyph.fImage = &glyphBuffer[sizeof(Op)];
                    sc->getImage(op->glyph);
                    writeSize += op->glyph.rowBytes() * op->glyph.fHeight;
                    break;
                }
                case 3: {
                    // TODO: check for buffer overflow.
                    SkPath path;
                    sc->getPath(op->glyphId, &path);
                    op->pathSize = path.writeToMemory(&glyphBuffer[sizeof(Op)]);
                    writeSize += op->pathSize;
                    break;
                }
                default:
                    SkASSERT("Bad op");
            }
        write(writeFd, glyphBuffer.get(), writeSize);
    }

    close(readFd);
    close(writeFd);

    std::cerr << "Returning from render" << std::endl;

    return 0;
}

int main(int argc, char** argv) {
    std::string skpName = argc > 1 ? std::string{argv[1]} : std::string{"desk_nytimes"};
    printf("skp: %s\n", skpName.c_str());

    int render_to_gpu[2],
        gpu_to_render[2];

    enum direction : int {kRead = 0, kWrite = 1};

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

    pid_t child = fork();
    SkGraphics::Init();

    if (child == 0) {
        // The child - renderer
        // Close unused pipe ends.
        close(render_to_gpu[kRead]);
        close(gpu_to_render[kWrite]);
        std::cerr << "Starting renderer" << std::endl;
        printf("skp: %s\n", skpName.c_str());
        //renderer(skpName, gpu_to_render[kRead], render_to_gpu[kWrite]);
        gpu(gpu_to_render[kRead], render_to_gpu[kWrite]);
    } else {
        // The parent - GPU
        // Close unused pipe ends.
        std::cerr << "child id - " << child << std::endl;
        close(gpu_to_render[kRead]);
        close(render_to_gpu[kWrite]);
        //gpu(render_to_gpu[kRead], gpu_to_render[kWrite]);
        renderer(skpName, render_to_gpu[kRead], gpu_to_render[kWrite]);

        std::cerr << "Waiting for renderer." << std::endl;
        waitpid(child, nullptr, 0);
    }

    return 0;
}


/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkData.h"
#include "SkDescriptor.h"
#include "SkGraphics.h"
#include "SkSemaphore.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"
#include "SkWriteBuffer.h"

#include "gm.h"
#include <ctype.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <unordered_map>

#include <unistd.h>
#include <sys/mman.h>

// Stolen from SkSemaphore.cpp
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <mach/mach.h>

// We've got to teach TSAN that there is a happens-before edge beteween
// semaphore_signal() and semaphore_wait().
#if __has_feature(thread_sanitizer)
extern "C" void AnnotateHappensBefore(const char*, int, void*);
        extern "C" void AnnotateHappensAfter (const char*, int, void*);
#else
static void AnnotateHappensBefore(const char*, int, void*) {}
static void AnnotateHappensAfter (const char*, int, void*) {}
#endif

struct OSSemaphore {
    semaphore_t fSemaphore;
    std::string fName;

    OSSemaphore(const std::string& name) : fName{name} {
        kern_return_t r = semaphore_create(mach_task_self(), &fSemaphore, SYNC_POLICY_LIFO, 0/*initial count*/);
        if (r != KERN_SUCCESS) {
            fprintf(stderr, "%s - semaphore create failed. Error <%d, %s>", fName.c_str(), r,
                    mach_error_string
                    (r));
            //exit(1);
        }
    }
    ~OSSemaphore() { semaphore_destroy(mach_task_self(), fSemaphore); }

    void signal(int n) {
        while (n --> 0) {
            AnnotateHappensBefore(__FILE__, __LINE__, &fSemaphore);
            kern_return_t r = semaphore_signal(fSemaphore);
            if (r != KERN_SUCCESS) {
                fprintf(stderr, "%s - semaphore signal failed. Error <%d, %s>", fName.c_str(), r, mach_error_string
                        (r));
                //exit(1);
            }
        }
    }
    void wait() {
        kern_return_t r = semaphore_wait(fSemaphore);
        if (r != KERN_SUCCESS) {
            fprintf(stderr, "%s - semaphore wait failed. Error <%d, %s>", fName.c_str(), r, mach_error_string(r));
            //exit(1);
        }
        AnnotateHappensAfter(__FILE__, __LINE__, &fSemaphore);
    }
};
#elif defined(SK_BUILD_FOR_WIN32)
struct OSSemaphore {
        HANDLE fSemaphore;

        OSSemaphore()  {
            fSemaphore = CreateSemaphore(nullptr    /*security attributes, optional*/,
                                         0       /*initial count*/,
                                         MAXLONG /*max count*/,
                                         nullptr    /*name, optional*/);
        }
        ~OSSemaphore() { CloseHandle(fSemaphore); }

        void signal(int n) {
            ReleaseSemaphore(fSemaphore, n, nullptr/*returns previous count, optional*/);
        }
        void wait() { WaitForSingleObject(fSemaphore, INFINITE/*timeout in ms*/); }
    };
#else
    // It's important we test for Mach before this.  This code will compile but not work there.
    #include <errno.h>
    #include <semaphore.h>
    struct OSSemaphore {
        sem_t fSemaphore;

        OSSemaphore()  { sem_init(&fSemaphore, 0/*cross process?*/, 0/*initial count*/); }
        ~OSSemaphore() { sem_destroy(&fSemaphore); }

        void signal(int n) { while (n --> 0) { sem_post(&fSemaphore); } }
        void wait() {
            // Try until we're not interrupted.
            while(sem_wait(&fSemaphore) == -1 && errno == EINTR);
        }
    };
#endif

struct WireTypeface {
    std::thread::id thread_id;
    SkFontID        typeface_id;
    SkFontStyle     style;
    bool            is_fixed;
};

// TODO: Move this to a common place. Original in SkPaint.
template <typename A>
static auto create_desc_for_scaler_context(
        const SkScalerContextRec& rec,
        const SkScalerContextEffects& effects,
        A alloc) -> decltype(alloc((size_t)0)) {

    SkBinaryWriteBuffer peBuffer, mfBuffer, raBuffer;
    int entryCount = 1;
    size_t descSize = sizeof(rec);

    if (effects.fPathEffect) {
        effects.fPathEffect->flatten(peBuffer);
        descSize += peBuffer.bytesWritten();
        entryCount += 1;
    }
    if (effects.fMaskFilter) {
        effects.fMaskFilter->flatten(mfBuffer);
        descSize += mfBuffer.bytesWritten();
        entryCount += 1;
    }
    if (effects.fRasterizer) {
        effects.fRasterizer->flatten(raBuffer);
        descSize += raBuffer.bytesWritten();
        entryCount += 1;
    }

    descSize += SkDescriptor::ComputeOverhead(entryCount);

    auto desc = alloc(descSize);

    desc->init();
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);

    auto add = [&desc](uint32_t tag, SkBinaryWriteBuffer* buffer) {
        buffer->writeToMemory(desc->addEntry(tag, buffer->bytesWritten(), nullptr));
    };

    if (effects.fPathEffect) {
        add(kPathEffect_SkDescriptorTag, &peBuffer);
    }
    if (effects.fMaskFilter) {
        add(kMaskFilter_SkDescriptorTag, &mfBuffer);
    }
    if (effects.fRasterizer) {
        add(kRasterizer_SkDescriptorTag, &raBuffer);
    }

    desc->computeChecksum();
    return desc;
}


class RemoteScalerContextPassThread : public RemoteScalerContext {
public:
    explicit RemoteScalerContextPassThread(
            uint8_t* buffer,
            OSSemaphore* renderToGpu,
            OSSemaphore* gpuToRender)
        : fBuffer{buffer}
        , fRenderToGpu{renderToGpu}
        , fGpuToRender{gpuToRender} { }
    void generateFontMetrics(const SkTypefaceProxy& tf,
                             const SkScalerContextRec& rec,
                             const SkScalerContextEffects& effects,
                             SkPaint::FontMetrics* metrics) override {
        Op* op = this->createOp(0, tf, rec, effects);
        std::cerr << "Sending generateFontMetrics" << std::endl;
        fGpuToRender->signal(1);
        fRenderToGpu->wait();
        std::cerr << "Receiving generateFontMetrics" << std::endl;
        memcpy(metrics, &op->fontMetrics, sizeof(op->fontMetrics));
        op->~Op();
    }

    void generateMetrics(const SkTypefaceProxy& tf,
                         const SkScalerContextRec& rec,
                         const SkScalerContextEffects& effects,
                         SkGlyph* glyph) override {
        Op* op = this->createOp(1, tf, rec, effects);
        memcpy(&op->glyph, glyph, sizeof(*glyph));
        fGpuToRender->signal(1);
        fRenderToGpu->wait();
        memcpy(glyph, &op->glyph, sizeof(op->glyph));
        op->~Op();
    }

    void generateImage(const SkTypefaceProxy& tf,
                       const SkScalerContextRec& rec,
                       const SkScalerContextEffects& effects,
                       const SkGlyph& glyph) override {
        Op* op = this->createOp(2, tf, rec, effects);
        //void* oldImage = glyph.fImage;
        memcpy(&op->glyph, &glyph, sizeof(glyph));
        fGpuToRender->signal(1);
        fRenderToGpu->wait();
        //memcpy((SkGlyph *)&glyph, &op->glyph, sizeof(op->glyph));
        //((SkGlyph*)&glyph)->fImage = oldImage;
        std::cout << "rb: " << glyph.rowBytes() << " h: " << glyph.fHeight << std::endl;
        memcpy(glyph.fImage, fBuffer + sizeof(Op), glyph.rowBytes() * glyph.fHeight);
        op->~Op();
    }

    // TODO: punt on path serialization
    void generatePath(const SkTypefaceProxy& tf,
                      const SkScalerContextRec& rec,
                      const SkScalerContextEffects& effects,
                      SkGlyphID glyph, SkPath* path) override {
        Op* op = this->createOp(3, tf, rec, effects);
        op->glyphId = glyph;
        op->path = path;
        fGpuToRender->signal(1);
        fRenderToGpu->wait();
        op->~Op();
    }

private:
    Op* createOp(uint32_t opID, const SkTypefaceProxy& tf,
                 const SkScalerContextRec& rec,
                 const SkScalerContextEffects& effects) {
        Op* op = new (fBuffer) Op();
        op->op = opID;
        op->typeface_id = tf.fontID();

        auto ad = &op->ad;

        auto alloc = [ad](size_t size) {
            ad->reset(size);
            return ad->getDesc();
        };
        create_desc_for_scaler_context(rec, effects, alloc);
        return op;
    }

    uint8_t*     const fBuffer;
    OSSemaphore* const fRenderToGpu;
    OSSemaphore* const fGpuToRender;
};

static sk_sp<SkTypeface> gpu_from_renderer_by_ID(const void* buf, size_t len, void* ctx) {
    WireTypeface wire;
    if (len >= sizeof(wire)) {
        memcpy(&wire, buf, sizeof(wire));
        std::cerr << wire.thread_id << "  " << wire.typeface_id << std::endl;
        return sk_sp<SkTypeface>(
                new SkTypefaceProxy(
                        wire.typeface_id,
                        wire.thread_id,
                        wire.style,
                        wire.is_fixed,
                        (RemoteScalerContext*) ctx));
    }
    return nullptr;
}

std::unordered_map<SkFontID, sk_sp<SkTypeface>> gTypefaceMap;

static std::unique_ptr<SkScalerContext> scaler_context_from_op(Op* op) {

    auto i = gTypefaceMap.find(op->typeface_id);
    if (i == gTypefaceMap.end()) {
        std::cout << "bad typeface id: " <<  op->typeface_id << std::endl;
        SK_ABORT("unknown type face");
    }
    auto tf = i->second;
    SkScalerContextEffects effects;
    return tf->createScalerContext(effects, op->ad.getDesc(), false);
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
        std::cerr << "font id table - inserting: " << SkTypeface::UniqueID(tf) << std::endl;
        gTypefaceMap.insert({SkTypeface::UniqueID(tf), sk_ref_sp(tf)});
    }
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

static void gpu(void* picBuffer, void* fontBuffer, OSSemaphore* renderer_to_gpu, OSSemaphore*
        gpu_to_renderer) {
    renderer_to_gpu->wait();
    std::cerr << "gpu - Receiving picture" << std::endl;
    SkDeserialProcs procs;
    std::unique_ptr<RemoteScalerContext> rsc{
            new RemoteScalerContextPassThread{
                    (uint8_t*)fontBuffer, renderer_to_gpu, gpu_to_renderer}};
    procs.fTypefaceProc = gpu_from_renderer_by_ID;
    procs.fTypefaceCtx = rsc.get();
    auto pic = SkPicture::MakeFromData(picBuffer, 10 * 1024 * 4096, &procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();
    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());

    auto c = s->getCanvas();
    c->drawPicture(pic);

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f("test.png");
    f.write(data->data(), data->size());
}

static void renderer(
        const std::string& skpName,
        uint8_t* picBuffer, uint8_t* fontBuffer, OSSemaphore* renderer_to_gpu, OSSemaphore*
gpu_to_renderer) {
    std::string prefix{"skps/"};
    auto skp = SkData::MakeFromFileName((prefix + skpName + ".skp").c_str());
    auto pic = SkPicture::MakeFromData(skp.get());

    SkSerialProcs procs;
    procs.fTypefaceProc = renderer_to_gpu_by_ID;
    auto stream = pic->serialize(&procs);

    std::cerr << "stream is " << skp->size() << " bytes long" << std::endl;

    memcpy(picBuffer, stream->data(), stream->size());
    std::cerr << "render - Sending stream." << std::endl;
    renderer_to_gpu->signal(1);
    std::cerr << "Waiting for scaler context ops." << std::endl;

    Op* op = (Op*)fontBuffer;
    while (true) {
        gpu_to_renderer->wait();
        std::cerr << "op: " << op << " op->op: " << op->op << std::endl;

            auto sc = scaler_context_from_op(op);
            switch (op->op) {
                case 0:
                    sc->getFontMetrics(&op->fontMetrics);
                    break;
                case 1:
                    sc->getMetrics(&op->glyph);
                    break;
                case 2:
                    // TODO: check for buffer overflow.
                    op->glyph.fImage = fontBuffer + sizeof(Op);
                    sc->getImage(op->glyph);
                    break;
                case 3:
                    sc->getPath(op->glyphId, op->path);
                    break;
                default:
                    SkASSERT("Bad op");
            }

        renderer_to_gpu->signal(1);
    }
}

static const size_t kPageSize = 4096;

int main(int argc, char** argv) {
    std::string skpName = argc > 1 ? std::string{argv[1]} : std::string{"desk_nytimes"};
    printf("skp: %s\n", skpName.c_str());

    uint8_t* picBuffer =
            (uint8_t*)mmap(
                    nullptr, 10 * 1024 * kPageSize,
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_SHARED,
                    -1, 0);

    uint8_t* fontBuffer =
            (uint8_t*)mmap(
                    nullptr, 1024 * kPageSize,
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_SHARED,
                    -1, 0);

    std::cout << "picBuffer: " << ((intptr_t)picBuffer) << std::endl;
    std::cout << "fontBuffer: " << ((intptr_t)fontBuffer) << std::endl;
    memset(fontBuffer, 0xff, 100);

    OSSemaphore renderer_to_gpu("render to gpu"),
                gpu_to_renderer("gpu to render");

    pid_t child = fork();
    SkGraphics::Init();

    if (child == 0) {
        // The child

        renderer(skpName, picBuffer, fontBuffer, &renderer_to_gpu, &gpu_to_renderer);
    } else {
        // The parent
        gpu(picBuffer, fontBuffer, &renderer_to_gpu, &gpu_to_renderer);
        std::cerr << "Waiting for renderer." << std::endl;
        waitpid(child, nullptr, 0);
    }


    return 0;
}

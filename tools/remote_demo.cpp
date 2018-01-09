/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkGraphics.h"
#include "SkSemaphore.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"
#include "gm.h"
#include <ctype.h>
#include <memory>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <unordered_map>

extern std::unordered_map<SkFontID, sk_sp<SkTypeface>> gTypefaceMap;

char gRemoteBuffer[1024*1024*1024];
SkSemaphore renderer_to_gpu,
            gpu_to_renderer;

struct WireTypeface {
    std::thread::id thread_id;
    SkFontID        typeface_id;
    SkFontStyle     style;
    bool            is_fixed;
};

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

static sk_sp<SkTypeface> gpu_from_renderer_by_ID(const void* buf, size_t len, void* ctx) {
    WireTypeface wire;
    if (len >= sizeof(wire)) {
        memcpy(&wire, buf, sizeof(wire));
        //std::cout << wire.thread_id << "  " << wire.typeface_id << std::endl;
        return sk_sp<SkTypeface>(
                new SkTypefaceProxy(wire.typeface_id, wire.thread_id, wire.style, wire.is_fixed));
    }
    return nullptr;
}

static std::unique_ptr<SkScalerContext> scaler_context_from_op(Op* op) {
    //std::cerr << "op: " << op << " op->op: " << op->op << std::endl;

    auto i = gTypefaceMap.find(op->typeface_id);
    if (i == gTypefaceMap.end()) {
        std::cout << "bad typeface id: " <<  op->typeface_id << std::endl;
        SK_ABORT("unknown type face");
    }
    auto tf = i->second;
    SkScalerContextEffects effects;
    return tf->createScalerContext(effects, op->ad.getDesc(), false);
}

static void renderer(skiagm::GM* gm) {
    auto skp = SkData::MakeFromFileName(std::string("skps/desk_nytimes.skp").c_str());
    auto pic = SkPicture::MakeFromData(skp.get());
    //SkPictureRecorder rec;
    //rec.beginRecording(SkRect::Make(SkIRect::MakeSize(gm->getISize())));
    //    gm->draw(rec.getRecordingCanvas());
    //auto pic = rec.finishRecordingAsPicture();

    SkSerialProcs procs;
    procs.fTypefaceProc = renderer_to_gpu_by_ID;
    auto stream = pic->serialize(&procs);

    printf("stream is %zu bytes long\n", skp->size());

    memcpy(gRemoteBuffer, stream->data(), stream->size());
    renderer_to_gpu.signal(1);

    Op* op = (Op*)gRemoteBuffer;

    while (true) {
        gpu_to_renderer.wait();
        //std::cout << "op: " << op << " op->op: " << op->op << std::endl;

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
                op->glyph.fImage = gRemoteBuffer + sizeof(Op);
                sc->getImage(op->glyph);
                break;
            case 3:
                sc->getPath(op->glyphId, op->path);
                break;
            default:
                SkASSERT("Bad op");
        }

        renderer_to_gpu.signal(1);
    }
}

static void gpu() {
    renderer_to_gpu.wait();

    SkDeserialProcs procs;
    procs.fTypefaceProc = gpu_from_renderer_by_ID;
    auto pic = SkPicture::MakeFromData(gRemoteBuffer, sizeof(gRemoteBuffer), &procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();
    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());

    auto c = s->getCanvas();
    c->drawPicture(pic);

    auto i = s->makeImageSnapshot();
    auto data = i->encodeToData();
    SkFILEWStream f("test.png");
    f.write(data->data(), data->size());
    /*
    printf("%s\n", msg);

    for (char* c = msg; *c != 0; c++) {
        gRemoteBuffer[0] = *c;
        gpu_to_renderer.signal(1);

        renderer_to_gpu.wait();
        printf("%c", gRemoteBuffer[0]);
    }
    printf("\n");
    */
}

int main(int argc, char** argv) {
    const char* name = argc > 1 ? argv[1] : "textblob";

    SkGraphics::Init();

    std::unique_ptr<skiagm::GM> gm;
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        gm.reset(r->factory()(nullptr));
        if (strcmp(gm->getName(), name) == 0) {
            break;
        }
    }

    printf("%s\n", gm->getName());

    std::thread(renderer, gm.get()).detach();
    gpu();
    return 0;
}

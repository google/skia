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

static char buffer[1024*1024*1024];
static SkSemaphore renderer_to_gpu,
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
    return SkData::MakeWithCopy(&wire, sizeof(wire));
}

static sk_sp<SkTypeface> gpu_from_renderer_by_ID(const void* buf, size_t len, void* ctx) {
    WireTypeface wire;
    if (len >= sizeof(wire)) {
        memcpy(&wire, buf, sizeof(wire));
        std::cout << wire.thread_id << "  " << wire.typeface_id << std::endl;
        return sk_sp<SkTypeface>(
                new SkTypefaceProxy(wire.typeface_id, wire.thread_id, wire.style, wire.is_fixed));
    }
    return nullptr;
}

struct GlyphRequest {
    SkFontID fontId;

};

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

    memcpy(buffer, stream->data(), stream->size());
    renderer_to_gpu.signal(1);

    while (true) {
        gpu_to_renderer.wait();
        buffer[0] = toupper(buffer[0]);
        renderer_to_gpu.signal(1);
    }
}

static void gpu() {
    renderer_to_gpu.wait();

    SkDeserialProcs procs;
    procs.fTypefaceProc = gpu_from_renderer_by_ID;
    auto pic = SkPicture::MakeFromData(buffer, sizeof(buffer), &procs);

    auto cullRect = pic->cullRect();
    auto r = cullRect.round();
    auto s = SkSurface::MakeRasterN32Premul(r.width(), r.height());

    auto c = s->getCanvas();
    c->drawPicture(pic);

    /*
    printf("%s\n", msg);

    for (char* c = msg; *c != 0; c++) {
        buffer[0] = *c;
        gpu_to_renderer.signal(1);

        renderer_to_gpu.wait();
        printf("%c", buffer[0]);
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

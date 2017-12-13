/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "Resources.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkSerialProcs.h"
#include "SkSurface.h"

static sk_sp<SkImage> picture_to_image(sk_sp<SkPicture> pic) {
    SkIRect r = pic->cullRect().round();
    auto surf = SkSurface::MakeRasterN32Premul(r.width(), r.height());
    surf->getCanvas()->drawPicture(pic);
    return surf->makeImageSnapshot();
}

struct State {
    const char* fStr;
    SkImage*    fImg;
};

DEF_TEST(serial_procs_image, reporter) {
    auto src_img = GetResourceAsImage("images/mandrill_128.png");
    const char magic_str[] = "magic signature";

    const SkSerialImageProc sprocs[] = {
        [](SkImage* img, void* ctx) -> sk_sp<SkData> { return nullptr; },
        [](SkImage* img, void* ctx) { return img->encodeToData(); },
        [](SkImage* img, void* ctx) { return SkData::MakeWithCString(((State*)ctx)->fStr); },
    };
    const SkDeserialImageProc dprocs[] = {
        [](const void* data, size_t length, void*) -> sk_sp<SkImage> {
            SK_ABORT("should not get called");
            return nullptr;
        },
        [](const void* data, size_t length, void*) {
            return SkImage::MakeFromEncoded(SkData::MakeWithCopy(data, length));
        },
        [](const void* data, size_t length, void* ctx) -> sk_sp<SkImage> {
            State* state = (State*)ctx;
            if (length != strlen(state->fStr)+1 || memcmp(data, state->fStr, length)) {
                return nullptr;
            }
            return sk_ref_sp(state->fImg);
        },
    };

    sk_sp<SkPicture> pic;
    {
        SkPictureRecorder rec;
        SkCanvas* canvas = rec.beginRecording(128, 128);
        canvas->drawImage(src_img, 0, 0, nullptr);
        pic = rec.finishRecordingAsPicture();
    }

    State state = { magic_str, src_img.get() };

    SkSerialProcs sproc;
    sproc.fImageCtx  = &state;
    SkDeserialProcs dproc;
    dproc.fImageCtx  = &state;

    for (size_t i = 0; i < SK_ARRAY_COUNT(sprocs); ++i) {
        sproc.fImageProc = sprocs[i];
        auto data = pic->serialize(sproc);
        REPORTER_ASSERT(reporter, data);

        dproc.fImageProc = dprocs[i];
        auto new_pic = SkPicture::MakeFromData(data, dproc);
        REPORTER_ASSERT(reporter, data);

        auto dst_img = picture_to_image(new_pic);
        REPORTER_ASSERT(reporter, sk_tool_utils::equal_pixels(src_img.get(), dst_img.get()));
    }
}


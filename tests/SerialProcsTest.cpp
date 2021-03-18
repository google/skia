/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

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
            return nullptr;
        },
        [](const void* data, size_t length, void*) {
            return SkImage::MakeFromEncoded(SkData::MakeWithCopy(data, length));
        },
        [](const void* data, size_t length, void* ctx) -> sk_sp<SkImage> {
            State* state = (State*)ctx;
            if (length != strlen(state->fStr)+1 || 0 != memcmp(data, state->fStr, length)) {
                return nullptr;
            }
            return sk_ref_sp(state->fImg);
        },
    };

    sk_sp<SkPicture> pic;
    {
        SkPictureRecorder rec;
        SkCanvas* canvas = rec.beginRecording(128, 128);
        canvas->drawImage(src_img, 0, 0);
        pic = rec.finishRecordingAsPicture();
    }

    State state = { magic_str, src_img.get() };

    SkSerialProcs sproc;
    sproc.fImageCtx  = &state;
    SkDeserialProcs dproc;
    dproc.fImageCtx  = &state;

    for (size_t i = 0; i < SK_ARRAY_COUNT(sprocs); ++i) {
        sproc.fImageProc = sprocs[i];
        auto data = pic->serialize(&sproc);
        REPORTER_ASSERT(reporter, data);

        dproc.fImageProc = dprocs[i];
        auto new_pic = SkPicture::MakeFromData(data.get(), &dproc);
        REPORTER_ASSERT(reporter, data);

        auto dst_img = picture_to_image(new_pic);
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(src_img.get(), dst_img.get()));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkPicture> make_pic(const std::function<void(SkCanvas*)>& drawer) {
    SkPictureRecorder rec;
    drawer(rec.beginRecording(128, 128));
    return rec.finishRecordingAsPicture();
}

static SkSerialProcs makes(SkSerialPictureProc proc, void* ctx = nullptr) {
    SkSerialProcs procs;
    procs.fPictureProc = proc;
    procs.fPictureCtx = ctx;
    return procs;
}

static SkDeserialProcs maked(SkDeserialPictureProc proc, const void* ctx = nullptr) {
    SkDeserialProcs procs;
    procs.fPictureProc = proc;
    procs.fPictureCtx = const_cast<void*>(ctx);
    return procs;
}

// packages the picture's point in the skdata, and records it in the ctx as an array
struct Context {
    SkTDArray<SkPicture*>   fArray;
    SkPicture*              fSkipMe = nullptr;
};

static sk_sp<SkData> array_serial_proc(SkPicture* pic, void* ctx) {
    Context* c = (Context*)ctx;
    if (c->fSkipMe == pic) {
        return nullptr;
    }
    *c->fArray.append() = pic;
    return SkData::MakeWithCopy(&pic, sizeof(pic));
}

static sk_sp<SkPicture> array_deserial_proc(const void* data, size_t size, void* ctx) {
    SkASSERT(sizeof(SkPicture*) == size);

    Context* c = (Context*)ctx;
    SkPicture* pic;
    memcpy(&pic, data, size);

    int index = c->fArray.find(pic);
    SkASSERT(index >= 0);
    c->fArray.removeShuffle(index);

    return sk_ref_sp(pic);
}

static void test_pictures(skiatest::Reporter* reporter, sk_sp<SkPicture> p0, int count,
                          bool skipRoot) {
    Context ctx;
    if (skipRoot) {
        ctx.fSkipMe = p0.get();
    }

    SkSerialProcs sprocs = makes(array_serial_proc, &ctx);
    auto d0 = p0->serialize(&sprocs);
    REPORTER_ASSERT(reporter, ctx.fArray.count() == count);
    SkDeserialProcs dprocs = maked(array_deserial_proc, &ctx);
    p0 = SkPicture::MakeFromData(d0.get(), &dprocs);
    REPORTER_ASSERT(reporter, ctx.fArray.count() == 0);
}

DEF_TEST(serial_procs_picture, reporter) {

    auto p1 = make_pic([](SkCanvas* c) {
        // need to be large enough that drawPictures doesn't "unroll" us
        for (int i = 0; i < 20; ++i) {
            c->drawColor(SK_ColorRED);
        }
    });

    // now use custom serialization
    auto p0 = make_pic([](SkCanvas* c) { c->drawColor(SK_ColorBLUE); });
    test_pictures(reporter, p0, 1, false);

    // test inside effect
    p0 = make_pic([p1](SkCanvas* c) {
        SkPaint paint;
        SkTileMode tm = SkTileMode::kClamp;
        paint.setShader(p1->makeShader(tm, tm, SkFilterMode::kNearest));
        c->drawPaint(paint);
    });
    test_pictures(reporter, p0, 1, true);

    // test nested picture
    p0 = make_pic([p1](SkCanvas* c) {
        c->drawColor(SK_ColorRED);
        c->drawPicture(p1);
        c->drawColor(SK_ColorBLUE);
    });
    test_pictures(reporter, p0, 1, true);
}

static sk_sp<SkPicture> make_picture(sk_sp<SkTypeface> tf0, sk_sp<SkTypeface> tf1) {
    SkPictureRecorder rec;
    SkCanvas* canvas = rec.beginRecording(100, 100);
    SkPaint paint;
    SkFont font;
    font.setTypeface(tf0); canvas->drawString("hello", 0, 0, font, paint);
    font.setTypeface(tf1); canvas->drawString("hello", 0, 0, font, paint);
    font.setTypeface(tf0); canvas->drawString("hello", 0, 0, font, paint);
    font.setTypeface(tf1); canvas->drawString("hello", 0, 0, font, paint);
    return rec.finishRecordingAsPicture();
}

DEF_TEST(serial_typeface, reporter) {
    auto tf0 = MakeResourceAsTypeface("fonts/hintgasp.ttf");
    auto tf1 = MakeResourceAsTypeface("fonts/Roboto2-Regular_NoEmbed.ttf");
    if (!tf0 || !tf1 || tf0.get() == tf1.get()) {
        return; // need two different typefaces for this test to make sense.
    }

    auto pic = make_picture(tf0, tf1);

    int counter = 0;
    SkSerialProcs procs;
    procs.fTypefaceProc = [](SkTypeface* tf, void* ctx) -> sk_sp<SkData> {
        *(int*)ctx += 1;
        return nullptr;
    };
    procs.fTypefaceCtx = &counter;
    auto data = pic->serialize(&procs);

    // The picture has 2 references to each typeface, but we want the serialized picture to
    // only have written the data 1 time per typeface.
    REPORTER_ASSERT(reporter, counter == 2);
}


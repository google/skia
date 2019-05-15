/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/ToolUtils.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixdesc.h"
}

#include "include/core/SkImage.h"
#include "include/core/SkYUVAIndex.h"

static SkCanvas* gCanvas;

template <typename T> class CPtr {
public:
    CPtr(T* value, void (*deleter)(T*)) : fV(value), fD(deleter) {}
    ~CPtr() { if (fV) fD(fV); }

    T* get() const { return fV; }
    T* operator->() const { return fV; }

private:
    T* fV;
    void (*fD)(T*);
};

static void check_err(int err) {
    if (err >= 0) return;

    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0) {
        errbuf_ptr = strerror(AVUNERROR(err));
    }
    SkDebugf("%s\n", errbuf_ptr);
    SkASSERT(err >= 0);
}

static void test_yuv_420(int w, int h, uint8_t* const data[], int const strides[]) {
    SkImageInfo info[3];
    info[0] = SkImageInfo::Make(w, h, kGray_8_SkColorType, kOpaque_SkAlphaType);
    info[1] = SkImageInfo::Make(w/2, h/2, kGray_8_SkColorType, kOpaque_SkAlphaType);
    info[2] = SkImageInfo::Make(w/2, h/2, kGray_8_SkColorType, kOpaque_SkAlphaType);

    SkPixmap pm[4];
    for (int i = 0; i < 3; ++i) {
        pm[i] = SkPixmap(info[i], data[i], strides[i]);
    }
    pm[3].reset();  // no alpha

    SkYUVAIndex indices[4];
    indices[SkYUVAIndex::kY_Index] = {0, SkColorChannel::kR};
    indices[SkYUVAIndex::kU_Index] = {1, SkColorChannel::kR};
    indices[SkYUVAIndex::kV_Index] = {2, SkColorChannel::kR};
    indices[SkYUVAIndex::kA_Index] = {-1, SkColorChannel::kR};

    GrContext* gr = gCanvas->getGrContext();
    if (gr) {
        auto img = SkImage::MakeFromYUVAPixmaps(gr, kRec709_SkYUVColorSpace, pm, indices, {w, h},
                                                kTopLeft_GrSurfaceOrigin, false, false);
        gCanvas->drawImage(img, 50, 50, nullptr);
    }
}

static void test_frame(const AVFrame* frame) {
    SkDebugf("frame %d x %d format=%d %s\n", frame->width, frame->height,
             frame->format, av_get_pix_fmt_name((AVPixelFormat)frame->format));

    switch (frame->format) {
        case AV_PIX_FMT_YUV420P:
            test_yuv_420(frame->width, frame->height, frame->data, frame->linesize);
            break;
        default:
            SkDebugf("unsupported format (for now)\n");
    }
}

static void test_decoder(AVFormatContext* fmt, int stream_index, AVCodecContext* dec, AVStream* strm) {
    AVFrame* frame = av_frame_alloc();
    SkASSERT(frame);

    AVPacket pkt;
    av_init_packet(&pkt);

    int counter = 0;
    while (!av_read_frame(fmt, &pkt)) {
        if (pkt.stream_index != stream_index) {
            continue;
        }

        int ret = avcodec_send_packet(dec, &pkt);
        if (ret == AVERROR(EAGAIN)) {
            ret = 0;
        }
        SkDebugf("send_packet %d\n", ret);

        ret = avcodec_receive_frame(dec, frame);
        if (ret >= 0) {
            test_frame(frame);
            if (++counter > 10) {
                break;
            }
        }
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
        }
        SkDebugf("receive_frame %d\n", ret);
    }
}

static void test_ffmpeg(SkCanvas* canvas) {
    gCanvas = canvas;

    const char* filename = "/skia/ice.mp4";
    auto data = SkData::MakeFromFileName(filename);
    SkDebugf("movie size %zu\n", data->size());

    avcodec_register_all();

    AVFormatContext* fmt_ctx = nullptr;
    int err = avformat_open_input(&fmt_ctx, filename, nullptr, nullptr);
    check_err(err);

//    av_dump_format(fmt_ctx, 0, filename, 0);
    AVCodec* codec;
    int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (stream_index >= 0) {
        SkASSERT(codec);
        AVStream* strm = fmt_ctx->streams[stream_index];

        SkDebugf("dimensions: %d x %d\n", strm->codecpar->width, strm->codecpar->height);

        AVCodecContext* cd_ctx = avcodec_alloc_context3(codec);
        SkASSERT(cd_ctx);

        // needed?
        check_err(avcodec_parameters_to_context(cd_ctx, strm->codecpar));

        check_err(avcodec_open2(cd_ctx, codec, nullptr));

        test_decoder(fmt_ctx, stream_index, cd_ctx, strm);

        avcodec_free_context(&cd_ctx);
    }
    avformat_close_input(&fmt_ctx);
}

static void do_draw(SkCanvas* canvas, const SkRect& r) {
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(0x800000FF);
    canvas->drawRect(r, paint);
}

/**
 *  Exercise SkCanvasPriv::kDontClipToLayer_SaveLayerFlag flag, which does not limit the clip to the
 *  layer's bounds. Thus when a draw occurs, it can (depending on "where" it is) draw into the layer
 *  and/or draw onto the surrounding portions of the canvas, or both.
 *
 *  This GM has a 100x100 rectangle (r), which its going to draw. However first it creates a layer
 *  with this flag covering 1/2 of the rectangle (upper half). Then it draws the rect in SRC mode.
 *
 *  The portion of the draw that intersects the layer should see the SRC draw, apply it to the layer
 *  and then during restore, it will SRC_OVER that layer onto the canvas (SRC_OVER since the layer
 *  has no paint, so it gets the default xfermode during restore).
 *
 *  Note: when we talk about drawing directly into the "canvas", in fact we are drawing into an
 *        "outer" layer we created (filled with red). This is a testing detail, so that our final
 *        output image is itself opaque, otherwise we make it harder to view the GM as a PNG.
 *
 *  The portion of the draw below the layer draws directly into the canvas. Since it is in SRC mode,
 *  it will write 0x80 to the canvas' alpha, overwriting the "red", which then gets blended with
 *  the GM's white background.
 *
 *  The portion in the layer, will end up SRC_OVERing the 0x80 layer pixels onto the canvas' red
 *  pixels, making magenta.
 *
 *  Thus the expected result is the upper half to be magenta 0xFF7F0080, and the lower half to be
 *  light blue 0xFF7F7FFF.
 */
DEF_SIMPLE_GM(dont_clip_to_layer, canvas, 120, 120) {
    const SkRect r { 10, 10, 110, 110 };

    // Wrap the entire test inside a red layer, so we don't punch the actual gm's alpha with
    // kSrc_Mode, which makes it hard to view (we like our GMs to have opaque pixels).
    canvas->saveLayer(&r, nullptr);
    canvas->drawColor(SK_ColorRED);

    SkRect r0 = { 20, 20, 100, 55 };
    SkRect r1 = { 20, 65, 100, 100 };

    SkCanvas::SaveLayerRec rec;
    rec.fPaint = nullptr;
    rec.fBounds = &r0;
    rec.fBackdrop = nullptr;
    rec.fSaveLayerFlags = SkCanvasPriv::kDontClipToLayer_SaveLayerFlag;
    canvas->saveLayer(rec);
    rec.fBounds = &r1;
    canvas->saveLayer(rec);
    do_draw(canvas, r);
    canvas->restore();
    canvas->restore();

    canvas->restore();  // red-layer
}

/** Draw a 2px border around the target, then red behind the target;
    set the clip to match the target, then draw >> the target in blue.
*/

static void draw(SkCanvas* canvas, SkRect& target, int x, int y) {
    SkPaint borderPaint;
    borderPaint.setColor(SkColorSetRGB(0x0, 0xDD, 0x0));
    borderPaint.setAntiAlias(true);
    SkPaint backgroundPaint;
    backgroundPaint.setColor(SkColorSetRGB(0xDD, 0x0, 0x0));
    backgroundPaint.setAntiAlias(true);
    SkPaint foregroundPaint;
    foregroundPaint.setColor(SkColorSetRGB(0x0, 0x0, 0xDD));
    foregroundPaint.setAntiAlias(true);

    canvas->save();
    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
    target.inset(SkIntToScalar(-2), SkIntToScalar(-2));
    canvas->drawRect(target, borderPaint);
    target.inset(SkIntToScalar(2), SkIntToScalar(2));
    canvas->drawRect(target, backgroundPaint);
    canvas->clipRect(target, true);
    target.inset(SkIntToScalar(-4), SkIntToScalar(-4));
    canvas->drawRect(target, foregroundPaint);
    canvas->restore();
}

static void draw_square(SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_column(SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(1 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_bar(SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 1 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_rect_tests(SkCanvas* canvas) {
    draw_square(canvas, 10, 10);
    draw_column(canvas, 30, 10);
    draw_bar(canvas, 10, 30);
}

/**
   Test a set of clipping problems discovered while writing blitAntiRect,
   and test all the code paths through the clipping blitters.
   Each region should show as a blue center surrounded by a 2px green
   border, with no red.
*/
DEF_SIMPLE_GM(aaclip, canvas, 240, 120) {
    test_ffmpeg(canvas);

    // Initial pixel-boundary-aligned draw
        draw_rect_tests(canvas);

        // Repeat 4x with .2, .4, .6, .8 px offsets
        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);
}

/////////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_MAC

#include "include/utils/mac/SkCGUtils.h"
#include "src/core/SkMakeUnique.h"

static std::unique_ptr<SkCanvas> make_canvas(const SkBitmap& bm) {
    const SkImageInfo& info = bm.info();
    if (info.bytesPerPixel() == 4) {
        return SkCanvas::MakeRasterDirectN32(info.width(), info.height(),
                                             (SkPMColor*)bm.getPixels(),
                                             bm.rowBytes());
    } else {
        return skstd::make_unique<SkCanvas>(bm);
    }
}

static void test_image(SkCanvas* canvas, const SkImageInfo& info) {
    SkBitmap bm;
    bm.allocPixels(info);

    if (info.isOpaque()) {
        bm.eraseColor(SK_ColorGREEN);
    } else {
        bm.eraseColor(0);
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);
    make_canvas(bm)->drawCircle(50, 50, 49, paint);
    canvas->drawBitmap(bm, 10, 10);

    CGImageRef image = SkCreateCGImageRefWithColorspace(bm, nullptr);

    SkBitmap bm2;
    SkCreateBitmapFromCGImage(&bm2, image);
    canvas->drawBitmap(bm2, 10, 120);
    canvas->drawImage(SkMakeImageFromCGImage(image), 10, 120 + bm2.height() + 10);

    CGImageRelease(image);
}

DEF_SIMPLE_GM(cgimage, canvas, 800, 250) {
        const struct {
            SkColorType fCT;
            SkAlphaType fAT;
        } rec[] = {
            { kRGB_565_SkColorType, kOpaque_SkAlphaType },

            { kRGBA_8888_SkColorType, kPremul_SkAlphaType },
            { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType },
            { kRGBA_8888_SkColorType, kOpaque_SkAlphaType },

            { kBGRA_8888_SkColorType, kPremul_SkAlphaType },
            { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType },
            { kBGRA_8888_SkColorType, kOpaque_SkAlphaType },
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
            SkImageInfo info = SkImageInfo::Make(100, 100, rec[i].fCT, rec[i].fAT);
            test_image(canvas, info);
            canvas->translate(info.width() + 10, 0);
        }
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

// https://bug.skia.org/3716
class ClipCubicGM : public skiagm::GM {
    const SkScalar W = 100;
    const SkScalar H = 240;

    SkPath fVPath, fHPath;
public:
    ClipCubicGM() {
        fVPath.moveTo(W, 0);
        fVPath.cubicTo(W, H-10, 0, 10, 0, H);

        SkMatrix pivot;
        pivot.setRotate(90, W/2, H/2);
        fVPath.transform(pivot, &fHPath);
    }

protected:
    SkString onShortName() override {
        return SkString("clipcubic");
    }

    SkISize onISize() override {
        return SkISize::Make(400, 410);
    }

    void doDraw(SkCanvas* canvas, const SkPath& path) {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(0xFFCCCCCC);
        canvas->drawPath(path, paint);

        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, paint);
    }

    void drawAndClip(SkCanvas* canvas, const SkPath& path, SkScalar dx, SkScalar dy) {
        SkAutoCanvasRestore acr(canvas, true);

        SkRect r = SkRect::MakeXYWH(0, H/4, W, H/2);
        SkPaint paint;
        paint.setColor(ToolUtils::color_to_565(0xFF8888FF));

        canvas->drawRect(r, paint);
        this->doDraw(canvas, path);

        canvas->translate(dx, dy);

        canvas->drawRect(r, paint);
        canvas->clipRect(r);
        this->doDraw(canvas, path);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(80, 10);
        this->drawAndClip(canvas, fVPath, 200, 0);
        canvas->translate(0, 200);
        this->drawAndClip(canvas, fHPath, 200, 0);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM(return new ClipCubicGM;)

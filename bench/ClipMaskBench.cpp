/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkImage.h"
#include "SkPictureRecorder.h"
#include "SkString.h"
#include "SkSurface.h"

static void DrawMask(SkCanvas* canvas) {
    sk_tool_utils::draw_checkerboard(canvas, SK_ColorTRANSPARENT, SK_ColorGREEN, 10);
}

class ClipMaskBench : public Benchmark {
public:
    using MaskMakerFunc = sk_sp<SkImage> (*)(int);

    ClipMaskBench(const char name[], const MaskMakerFunc maskMaker)
        : fName(SkStringPrintf("clipmask_%s", name))
        , fClip(maskMaker(kSize)) {}

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkCanvas::SaveLayerRec rec(nullptr, nullptr, nullptr, fClip.get(), nullptr, 0);

        for (int i = 0; i < loops; ++i) {
            canvas->saveLayer(rec);
            canvas->drawColor(SK_ColorBLUE);
            canvas->restore();
        }
    }

private:
    static constexpr int kSize = 400;

    SkString       fName;
    sk_sp<SkImage> fClip;
};

DEF_BENCH(return new ClipMaskBench("a8", [](int size) -> sk_sp<SkImage> {
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(SkImageInfo::MakeA8(size, size));
    DrawMask(surface->getCanvas());
    return surface->makeImageSnapshot();
});)

DEF_BENCH(return new ClipMaskBench("8888", [](int size) -> sk_sp<SkImage> {
    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(size, size);
    DrawMask(surface->getCanvas());
    return surface->makeImageSnapshot();
});)

DEF_BENCH(return new ClipMaskBench("picture", [](int size) -> sk_sp<SkImage> {
    SkPictureRecorder recorder;
    DrawMask(recorder.beginRecording(size, size));
    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(), SkISize::Make(size, size),
                                    nullptr, nullptr, SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
});)


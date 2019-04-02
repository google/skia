/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPictureRecorder.h"
#include "SkPictureShader.h"
#include "SkShaderMaskFilter.h"
#include "SkSurface.h"

static sk_sp<SkShader> make_bitmap_shader() {
    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setAntiAlias(true);

    auto surface = SkSurface::MakeRasterN32Premul(100, 100);
    surface->getCanvas()->drawCircle(50, 50, 50, p);

    return surface->makeImageSnapshot()->makeShader(SkShader::kRepeat_TileMode,
                                                    SkShader::kRepeat_TileMode);
}

static sk_sp<SkShader> make_picture_shader() {
    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setAntiAlias(true);

    SkPictureRecorder recorder;
    recorder.beginRecording(100, 100)->drawCircle(50, 50, 50, p);

    return recorder.finishRecordingAsPicture()->makeShader(SkTileMode::kRepeat,
                                                           SkTileMode::kRepeat);
}

class ShaderMFBench final : public Benchmark {

public:
    using ShaderMaker = sk_sp<SkShader>(*)();

    ShaderMFBench(const char* nm, bool opaque, const ShaderMaker& maker) {
        fMaskFilter = SkShaderMaskFilter::Make(maker());
        fColor = opaque ? 0xff00ff00 : 0x8000ff00;
        fName = SkStringPrintf("shadermaskfilter_%s_%x", nm, SkColorGetA(fColor));
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint maskPaint;
        maskPaint.setMaskFilter(fMaskFilter);

        for (int i = 0; i < loops; ++i) {
            SkAutoCanvasRestore arc(canvas, false);
            canvas->saveLayer(nullptr, &maskPaint);
            canvas->drawColor(fColor);
        }
    }

private:
    SkString            fName;
    sk_sp<SkMaskFilter> fMaskFilter;
    SkColor  fColor;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new ShaderMFBench("bitmap" , true , make_bitmap_shader ); )
DEF_BENCH( return new ShaderMFBench("bitmap" , false, make_bitmap_shader ); )
DEF_BENCH( return new ShaderMFBench("picture", true , make_picture_shader); )
DEF_BENCH( return new ShaderMFBench("picture", false, make_picture_shader); )

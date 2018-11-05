// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm.h"

#include "SkSurface.h"

struct Result {
    int64_t fTotalError = 0;
    int fBadPixelCount = 0;
    int fMaxError = 0;
};

static inline uint32_t color(const SkPixmap& pm, SkIPoint p) {
    return *pm.addr32(p.x(), p.y());
}

static inline bool inside(SkIPoint point, SkISize dimensions) {
    return (unsigned)point.x() < (unsigned)dimensions.width() &&
           (unsigned)point.y() < (unsigned)dimensions.height();
}

Result Check(const SkPixmap& minImg, const SkPixmap& maxImg, const SkPixmap& img,
             unsigned tolerance, SkBitmap* errorOut) {
    Result result = {0, 0, 0};
    SkISize dim = img.info().dimensions();
    SkASSERT(minImg.info().dimensions() == dim);
    SkASSERT(maxImg.info().dimensions() == dim);
    static const SkIPoint kNeighborhood[9] = {
        { 0,  0},
        {-1,  0}, { 1,  0}, { 0, -1}, { 0,  1},
        {-1, -1}, { 1, -1}, {-1,  1}, { 1,  1},
    };
    for (int y = 0; y < dim.height(); ++y) {
        for (int x = 0; x < dim.width(); ++x) {
            SkIPoint xy{x, y};
            uint32_t c = color(img, xy);
            int error = INT_MAX;
            // loop over neighborhood (halo);
            for (SkIPoint delta : kNeighborhood) {
                SkIPoint point = xy + delta;
                if (inside(point, dim)) {
                    int err = 0;
                    // loop over four color channels
                    for (int j : {0, 8, 16, 24}) {
                        uint8_t v    = (c                    >> j) & 0xFF,
                                vmin = (color(minImg, point) >> j) & 0xFF,
                                vmax = (color(maxImg, point) >> j) & 0xFF;
                        err = SkMax32(err, SkMax32((int)v - (int)vmax, (int)vmin - (int)v));
                    }
                    error = SkMin32(error, err);
                }
            }
            if (error > (int)tolerance) {
                ++result.fBadPixelCount;
                result.fTotalError += error;
                result.fMaxError = SkMax32(error, result.fMaxError);
                if (errorOut) {
                    if (!errorOut->getPixels()) {
                        errorOut->allocPixels(SkImageInfo::Make(
                                    dim.width(), dim.height(),
                                    kBGRA_8888_SkColorType,
                                    kOpaque_SkAlphaType));
                        errorOut->eraseColor(SK_ColorWHITE);
                    }
                    SkASSERT((unsigned)error < 256);
                    *(errorOut->getAddr32(x, y)) = SkColorSetARGB(0xFF, (uint8_t)error, 0, 0);
                }
            }
        }
    }
    return result;
}

SkPixmap do_gm(SkSurface* s, skiagm::GM* gm) {
    SkPixmap pixmap;
    SkCanvas* canvas = s->getCanvas();
    canvas->clear(SK_ColorWHITE);
    gm->draw(canvas);
    canvas->flush();
    SkAssertResult(s->peekPixels(&pixmap));
    return pixmap;
}

bool test_jitter(skiagm::GM* gm) {
    SkISize size = gm->getISize();
    auto orig = SkSurface::MakeRasterN32Premul(size.width(), size.height());
    auto s = SkSurface::MakeRasterN32Premul(size.width(), size.height());
    SkPixmap originalPixmap = do_gm(orig.get(), gm);

    constexpr uint8_t kSkiaSkqpGlobalErrorTolerance = 8;
    constexpr int N = 7;
    constexpr float phase = 0.3f;
    constexpr float magnitude = 0.03125f;
    for (int i = 0; i < N; ++i) {
        SkAutoCanvasRestore autoCanvasRestore(s->getCanvas(), true);
        float angle = i * (6.2831853f / N) + phase;
        s->getCanvas()->translate(magnitude * cosf(angle), magnitude * sinf(angle));
        SkPixmap testPixmap = do_gm(s.get(), gm);
        Result result = Check(originalPixmap, originalPixmap, testPixmap,
                              kSkiaSkqpGlobalErrorTolerance, nullptr);
        if (result.fTotalError > 0) {
            return false;
        }
    }
    return true;
}


int main() {
    int goodcount = 0;
    int badcount = 0;
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(factory(nullptr));
        if (0 == strcmp("p3", gm->getName())) {
            continue;
        }
        bool j = test_jitter(gm.get());
        if (j) {  ++goodcount; } else { ++badcount; }
        SkDebugf("%d %s\n", j, gm->getName());
    }
    SkDebugf("good = %d\nbad = %d\n\n", goodcount, badcount);
}

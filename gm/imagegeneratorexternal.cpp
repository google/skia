/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkImage.h"
#include "SkImageGenerator.h"
#include "SkMutex.h"
#include "SkSurface.h"
#include "SkTArray.h"

namespace {

class ExternalGenerator : public SkImageGenerator {
public:
    ExternalGenerator(const SkISize size)
        : INHERITED(SkImageInfo::MakeN32Premul(size.width(), size.height())) {

        int level = 0;
        for (int size = kMaxSize; size; size /= 2) {
            sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(size, size);
            DrawRings(surface->getCanvas(), 0xff008000, level++);
            fMips.emplace_back(surface->makeImageSnapshot());
        }
    }

    virtual ~ExternalGenerator() {}

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     SkPMColor[], int*) override {
        sk_sp<SkSurface> s = SkSurface::MakeRasterDirect(info, pixels, rowBytes);
        s->getCanvas()->clear(SK_ColorTRANSPARENT);
        DrawRings(s->getCanvas(), SK_ColorRED);
        return true;
    }

    bool onAccessScaledImage(const SkRect& src, const SkMatrix& matrix, SkFilterQuality,
                             ScaledImageRec* rec) override {
        // Not strictly needed for this immutable class.
        SkAutoExclusive lock(fMutex);

        SkSize scaleSize;
        if (!matrix.decomposeScale(&scaleSize, nullptr)) {
            return false;
        }
        scaleSize.set(scaleSize.width()  * this->getInfo().width()  / kMaxSize,
                      scaleSize.height() * this->getInfo().height() / kMaxSize);

        const SkScalar scale = SkTMin(scaleSize.width(), scaleSize.height());
        const int lvl = SkScalarFloorToInt(-SkScalarLog2(scale));

        rec->fImage = fMips[SkTPin(lvl, 0, fMips.count())];

        const SkRect origBounds = SkRect::Make(this->getInfo().bounds());
        const SkRect  newBounds = SkRect::Make(rec->fImage->bounds());

        SkMatrix srcMap = SkMatrix::MakeScale(newBounds.width()  / origBounds.width(),
                                              newBounds.height() / origBounds.height());
        srcMap.preTranslate(src.x(), src.y());
        srcMap.mapRect(&rec->fSrcRect, SkRect::MakeWH(src.width(), src.height()));

        rec->fQuality = kLow_SkFilterQuality;

        return true;
    }

private:
    static void DrawRings(SkCanvas* c, SkColor color, int lvl = 0) {
        static constexpr SkScalar kStep = 0.2f;

        SkRect rect = SkRect::MakeWH(1, 1);

        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(0.02f);
        p.setAntiAlias(true);
        p.setColor(color);

        c->concat(SkMatrix::MakeRectToRect(SkRect::MakeWH(1, 1),
                                           SkRect::MakeIWH(c->imageInfo().width(),
                                                           c->imageInfo().height()),
                                           SkMatrix::kFill_ScaleToFit));
        while (!rect.isEmpty()) {
            c->drawRect(rect, p);
            rect.inset(kStep, kStep);
        }

        static constexpr SkScalar kTxtSize = 0.2f;
        SkASSERT(lvl >= 0 && lvl <= 9);
        const char label = '0' + lvl;
        p.setTextSize(kTxtSize);
        p.setLinearText(true);
        p.setStyle(SkPaint::kFill_Style);
        SkRect labelBounds;
        p.measureText(&label, 1, &labelBounds);

        c->drawText(&label, 1, 0.5f - labelBounds.width() / 2, 0.5f + labelBounds.height() / 2, p);
    }

    SkMutex fMutex;

    static constexpr int kMaxSize = 512;
    SkTArray<sk_sp<SkImage>> fMips;

    typedef SkImageGenerator INHERITED;
};

} // anonymous ns

class ImageGenExternalGM : public skiagm::GM {
public:
    explicit ImageGenExternalGM(bool useShader) : fUseShader(useShader) {}

protected:
    SkString onShortName() override {
        return SkStringPrintf("ImageGeneratorExternal%s", fUseShader ? "_shader" : "_rect");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onOnceBeforeDraw() override {
        fImage = SkImage::MakeFromGenerator(new ExternalGenerator(SkISize::Make(kGeneratorSize,
                                                                                kGeneratorSize)));
    }

    void onDraw(SkCanvas* canvas) override {
        static const SkRect gSubsets[] = {
            SkRect::MakeLTRB(0    , 0    , 1    , 1    ),
            SkRect::MakeLTRB(0    , 0    , 0.5f , 0.5f ),
            SkRect::MakeLTRB(0.5f , 0    , 1    , 0.5f ),
            SkRect::MakeLTRB(0.5f , 0.5f , 1    , 1    ),
            SkRect::MakeLTRB(0    , 0.5f , 0.5f , 1    ),
            SkRect::MakeLTRB(0.25f, 0.25f, 0.75f, 0.75f),
        };

        SkPaint p;
        p.setFilterQuality(kLow_SkFilterQuality);

        for (int i = 1; i <= 4; ++i) {
            const SkRect dst = SkRect::MakeIWH(kGeneratorSize / i, kGeneratorSize / i);

            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gSubsets); ++j) {
                SkRect subset = gSubsets[j];
                subset.set(kGeneratorSize * subset.left(),
                           kGeneratorSize * subset.top(),
                           kGeneratorSize * subset.right(),
                           kGeneratorSize * subset.bottom());
                this->drawSubset(canvas, subset, dst, p);
                canvas->translate(kGeneratorSize * 1.1f, 0);
            }
            canvas->restore();
            canvas->translate(0, dst.height() * 1.2f);
        }
    }

private:
    void drawSubset(SkCanvas* canvas, const SkRect& src, const SkRect& dst,
                    const SkPaint& paint) const {
        if (fUseShader) {
            SkPaint p(paint);
            SkMatrix localMatrix = SkMatrix::MakeRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
            p.setShader(fImage->makeShader(SkShader::kClamp_TileMode,
                                           SkShader::kClamp_TileMode,
                                           &localMatrix));
            canvas->drawRect(dst, p);
        } else {
            canvas->drawImageRect(fImage, src, dst, &paint);
        }
    }

    static constexpr int kGeneratorSize = 200;
    sk_sp<SkImage> fImage;
    bool           fUseShader;

    typedef skiagm::GM INHERITED;
};

DEF_GM( return new ImageGenExternalGM(false); )
DEF_GM( return new ImageGenExternalGM(true); )

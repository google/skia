/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleDrawable.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "modules/particles/include/SkParticleData.h"
#include "src/core/SkAutoMalloc.h"

static sk_sp<SkImage> make_circle_image(int radius) {
    auto surface = SkSurface::MakeRasterN32Premul(radius * 2, radius * 2);
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    surface->getCanvas()->drawCircle(radius, radius, radius, paint);
    return surface->makeImageSnapshot();
}

static inline SkRSXform make_rsxform(SkPoint ofs,
                                     float posX, float posY, float dirX, float dirY, float scale) {
    const float s = dirX * scale;
    const float c = -dirY * scale;
    return SkRSXform::Make(c, s,
                           posX + -c * ofs.fX + s * ofs.fY,
                           posY + -s * ofs.fX + -c * ofs.fY);
}

struct DrawAtlasArrays {
    DrawAtlasArrays(const SkParticles& particles, int count, SkPoint center)
            : fXforms(count)
            , fRects(count)
            , fColors(count) {
        float* c[] = {
            particles.fData[SkParticles::kColorR].get(),
            particles.fData[SkParticles::kColorG].get(),
            particles.fData[SkParticles::kColorB].get(),
            particles.fData[SkParticles::kColorA].get(),
        };

        float* pos[] = {
            particles.fData[SkParticles::kPositionX].get(),
            particles.fData[SkParticles::kPositionY].get(),
        };
        float* dir[] = {
            particles.fData[SkParticles::kHeadingX].get(),
            particles.fData[SkParticles::kHeadingY].get(),
        };
        float* scale = particles.fData[SkParticles::kScale].get();

        for (int i = 0; i < count; ++i) {
            fXforms[i] = make_rsxform(center, pos[0][i], pos[1][i], dir[0][i], dir[1][i], scale[i]);
            fColors[i] = SkColor4f{ c[0][i], c[1][i], c[2][i], c[3][i] }.toSkColor();
        }
    }

    SkAutoTMalloc<SkRSXform> fXforms;
    SkAutoTMalloc<SkRect>    fRects;
    SkAutoTMalloc<SkColor>   fColors;
};

class SkCircleDrawable : public SkParticleDrawable {
public:
    SkCircleDrawable(int radius = 1)
            : fRadius(radius) {
        this->rebuild();
    }

    REFLECTED(SkCircleDrawable, SkParticleDrawable)

    void draw(SkCanvas* canvas, const SkParticles& particles, int count,
              const SkPaint* paint) override {
        SkPoint center = { SkIntToScalar(fRadius), SkIntToScalar(fRadius) };
        DrawAtlasArrays arrays(particles, count, center);
        for (int i = 0; i < count; ++i) {
            arrays.fRects[i].set(0.0f, 0.0f, fImage->width(), fImage->height());
        }
        canvas->drawAtlas(fImage, arrays.fXforms.get(), arrays.fRects.get(), arrays.fColors.get(),
                          count, SkBlendMode::kModulate, nullptr, paint);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Radius", fRadius);
        this->rebuild();
    }

private:
    int fRadius;

    void rebuild() {
        fRadius = SkTMax(fRadius, 1);
        if (!fImage || fImage->width() != 2 * fRadius) {
            fImage = make_circle_image(fRadius);
        }
    }

    // Cached
    sk_sp<SkImage> fImage;
};

class SkImageDrawable : public SkParticleDrawable {
public:
    SkImageDrawable(const SkString& path = SkString(), int cols = 1, int rows = 1)
            : fPath(path)
            , fCols(cols)
            , fRows(rows) {
        this->rebuild();
    }

    REFLECTED(SkImageDrawable, SkParticleDrawable)

    void draw(SkCanvas* canvas, const SkParticles& particles, int count,
              const SkPaint* paint) override {
        SkRect baseRect = getBaseRect();
        SkPoint center = { baseRect.width() * 0.5f, baseRect.height() * 0.5f };
        DrawAtlasArrays arrays(particles, count, center);

        int frameCount = fCols * fRows;
        float* spriteFrames = particles.fData[SkParticles::kSpriteFrame].get();
        for (int i = 0; i < count; ++i) {
            int frame = static_cast<int>(spriteFrames[i] * frameCount + 0.5f);
            frame = SkTPin(frame, 0, frameCount - 1);
            int row = frame / fCols;
            int col = frame % fCols;
            arrays.fRects[i] = baseRect.makeOffset(col * baseRect.width(), row * baseRect.height());
        }
        canvas->drawAtlas(fImage, arrays.fXforms.get(), arrays.fRects.get(), arrays.fColors.get(),
                          count, SkBlendMode::kModulate, nullptr, paint);
    }

    void visitFields(SkFieldVisitor* v) override {
        SkString oldPath = fPath;

        v->visit("Path", fPath);
        v->visit("Columns", fCols);
        v->visit("Rows", fRows);

        fCols = SkTMax(fCols, 1);
        fRows = SkTMax(fRows, 1);
        if (oldPath != fPath) {
            this->rebuild();
        }
    }

private:
    SkString fPath;
    int      fCols;
    int      fRows;

    SkRect getBaseRect() const {
        return SkRect::MakeWH(static_cast<float>(fImage->width()) / fCols,
                              static_cast<float>(fImage->height() / fRows));
    }

    void rebuild() {
        fImage = SkImage::MakeFromEncoded(SkData::MakeFromFileName(fPath.c_str()));
        if (!fImage) {
            if (!fPath.isEmpty()) {
                SkDebugf("Could not load image \"%s\"\n", fPath.c_str());
            }
            fImage = make_circle_image(1);
        }
    }

    // Cached
    sk_sp<SkImage> fImage;
};

void SkParticleDrawable::RegisterDrawableTypes() {
    REGISTER_REFLECTED(SkParticleDrawable);
    REGISTER_REFLECTED(SkCircleDrawable);
    REGISTER_REFLECTED(SkImageDrawable);
}

sk_sp<SkParticleDrawable> SkParticleDrawable::MakeCircle(int radius) {
    return sk_sp<SkParticleDrawable>(new SkCircleDrawable(radius));
}

sk_sp<SkParticleDrawable> SkParticleDrawable::MakeImage(const SkString& path, int cols, int rows) {
    return sk_sp<SkParticleDrawable>(new SkImageDrawable(path, cols, rows));
}

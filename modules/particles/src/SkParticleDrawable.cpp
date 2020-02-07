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
#include "modules/skresources/include/SkResources.h"
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
    SkCircleDrawable(int radius = 1) : fRadius(radius) {}

    REFLECTED(SkCircleDrawable, SkParticleDrawable)

    void draw(SkCanvas* canvas, const SkParticles& particles, int count,
              const SkPaint& paint) override {
        int r = std::max(fRadius, 1);
        SkPoint center = { SkIntToScalar(r), SkIntToScalar(r) };
        DrawAtlasArrays arrays(particles, count, center);
        for (int i = 0; i < count; ++i) {
            arrays.fRects[i].setIWH(fImage->width(), fImage->height());
        }
        canvas->drawAtlas(fImage, arrays.fXforms.get(), arrays.fRects.get(), arrays.fColors.get(),
                          count, SkBlendMode::kModulate, nullptr, &paint);
    }

    void prepare(const skresources::ResourceProvider*) override {
        int r = std::max(fRadius, 1);
        if (!fImage || fImage->width() != 2 * r) {
            fImage = make_circle_image(r);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Radius", fRadius);
    }

private:
    int fRadius;

    // Cached
    sk_sp<SkImage> fImage;
};

class SkImageDrawable : public SkParticleDrawable {
public:
    SkImageDrawable(const char* imagePath = "", const char* imageName = "",
                    int cols = 1, int rows = 1)
            : fPath(imagePath)
            , fName(imageName)
            , fCols(cols)
            , fRows(rows) {}

    REFLECTED(SkImageDrawable, SkParticleDrawable)

    void draw(SkCanvas* canvas, const SkParticles& particles, int count,
              const SkPaint& paint) override {
        int cols = std::max(fCols, 1),
            rows = std::max(fRows, 1);
        SkRect baseRect = SkRect::MakeWH(static_cast<float>(fImage->width()) / cols,
                                         static_cast<float>(fImage->height()) / rows);
        SkPoint center = { baseRect.width() * 0.5f, baseRect.height() * 0.5f };
        DrawAtlasArrays arrays(particles, count, center);

        int frameCount = cols * rows;
        float* spriteFrames = particles.fData[SkParticles::kSpriteFrame].get();
        for (int i = 0; i < count; ++i) {
            int frame = static_cast<int>(spriteFrames[i] * frameCount + 0.5f);
            frame = SkTPin(frame, 0, frameCount - 1);
            int row = frame / cols;
            int col = frame % cols;
            arrays.fRects[i] = baseRect.makeOffset(col * baseRect.width(), row * baseRect.height());
        }
        canvas->drawAtlas(fImage, arrays.fXforms.get(), arrays.fRects.get(), arrays.fColors.get(),
                          count, SkBlendMode::kModulate, nullptr, &paint);
    }

    void prepare(const skresources::ResourceProvider* resourceProvider) override {
        fImage.reset();
        if (auto asset = resourceProvider->loadImageAsset(fPath.c_str(), fName.c_str(), nullptr)) {
            fImage = asset->getFrame(0);
        }
        if (!fImage) {
            SkDebugf("Could not load image \"%s:%s\"\n", fPath.c_str(), fName.c_str());
            fImage = make_circle_image(1);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Path", fPath);
        v->visit("Name", fName);
        v->visit("Columns", fCols);
        v->visit("Rows", fRows);
    }

private:
    SkString fPath;
    SkString fName;
    int      fCols;
    int      fRows;

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

sk_sp<SkParticleDrawable> SkParticleDrawable::MakeImage(const char* imagePath,
                                                        const char* imageName,
                                                        int cols, int rows) {
    return sk_sp<SkParticleDrawable>(new SkImageDrawable(imagePath, imageName, cols, rows));
}

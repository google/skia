/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tools/gpu/ProtectedUtils.h"
#include "tools/viewer/Slide.h"

class ProtectedSlide : public Slide {
public:
    ProtectedSlide() { fName = "Protected"; }

    SkISize getDimensions() const override { return {kSize, 2*kSize}; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorDKGRAY);

#if defined(SK_GANESH)
        GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            canvas->clear(SK_ColorGREEN);
            return;
        }

        if (fCachedContext != dContext) {
            fCachedContext = dContext;

            // Intentionally leak these. The issue is that, on Android, Viewer keeps recreating
            // the context w/o signaling the slides.
            (void) fProtectedImage.release();
            (void) fUnProtectedImage.release();

            if (ProtectedUtils::ContextSupportsProtected(dContext)) {
                fProtectedImage = ProtectedUtils::CreateProtectedSkImage(dContext, { kSize, kSize },
                                                                         SkColors::kRed,
                                                                         /* isProtected= */ true);
            }

            fUnProtectedImage = ProtectedUtils::CreateProtectedSkImage(dContext, { kSize, kSize },
                                                                       SkColors::kRed,
                                                                       /* isProtected= */ false);
        }

        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(2);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setShader(fProtectedImage ? fProtectedImage->makeShader({}) : nullptr);
        paint.setImageFilter(SkImageFilters::Blur(10, 10, nullptr));

        canvas->drawRect(SkRect::MakeWH(kSize, kSize), paint);
        canvas->drawRect(SkRect::MakeWH(kSize, kSize), stroke);

        paint.setShader(fUnProtectedImage->makeShader({}));
        canvas->drawRect(SkRect::MakeXYWH(0, kSize, kSize, kSize), paint);
        canvas->drawRect(SkRect::MakeXYWH(0, kSize, kSize, kSize), stroke);
#endif // SK_GANESH
    }

private:
    static const int kSize = 128;

    GrDirectContext* fCachedContext = nullptr;
    sk_sp<SkImage> fProtectedImage;
    sk_sp<SkImage> fUnProtectedImage;
};

DEF_SLIDE( return new ProtectedSlide(); )


/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint3.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkCamera.h"
#include "include/utils/SkShadowUtils.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"
#include "src/utils/SkUTF.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

////////////////////////////////////////////////////////////////////////////

class MaterialShadowsView : public Sample {
    SkPath    fCirclePath;
    SkPath    fCapsulePath;
    SkPath    fLargeRRPath;
    SkPath    fSmallRRPath;

    SkPoint3  fLightPos;

    void onOnceBeforeDraw() override {
        fCirclePath.addCircle(0, 0, 56/2);
        fCapsulePath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-64, -24, 128, 48), 24, 24));
        fLargeRRPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-64, -64, 128, 128), 4, 4));
        fSmallRRPath.addRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(-40, -40, 80, 80), 4, 4));

        fLightPos = SkPoint3::Make(0, -700, 700);
    }

    SkString name() override { return SkString("MaterialShadows"); }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path,
                          const SkPoint3& zPlaneParams,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightRadius, SkScalar spotAlpha) {
        uint32_t flags = 0;
        flags |= SkShadowFlags::kDirectionalLight_ShadowFlag;

        SkColor ambientColor = SkColorSetARGB(ambientAlpha * 255, 0, 0, 0);
        SkColor spotColor = SkColorSetARGB(spotAlpha * 255, 0, 0, 0);
        SkShadowUtils::DrawShadow(canvas, path, zPlaneParams, lightPos, lightRadius,
                                  ambientColor, spotColor, flags);

        canvas->drawPath(path, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(0xFFFFFFFF);

        const SkScalar kLightRadius = 1.1f;
        const SkScalar kAmbientAlpha = 0.05f;
        const SkScalar kSpotAlpha = 0.35f;

        const SkScalar elevations[] = { 1, 3, 6, 8, 12, 24 };

        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint3 lightPos = fLightPos;
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, 0);

        paint.setColor(SK_ColorWHITE);
        canvas->save();
        canvas->translate(80, 80);
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(elevations); ++i) {
            zPlaneParams.fZ = elevations[i];
            this->drawShadowedPath(canvas, fCirclePath, zPlaneParams, paint, kAmbientAlpha,
                                   lightPos, kLightRadius, kSpotAlpha);
            canvas->translate(80, 0);
        }
        canvas->restore();

        canvas->save();
        canvas->translate(120, 175);
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(elevations); ++i) {
            zPlaneParams.fZ = elevations[i];
            this->drawShadowedPath(canvas, fCapsulePath, zPlaneParams, paint, kAmbientAlpha,
                                   lightPos, kLightRadius, kSpotAlpha);
            canvas->translate(160, 0);
        }
        canvas->restore();

        canvas->save();
        canvas->translate(120, 320);
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(elevations); ++i) {
            zPlaneParams.fZ = elevations[i];
            this->drawShadowedPath(canvas, fLargeRRPath, zPlaneParams, paint, kAmbientAlpha,
                                   lightPos, kLightRadius, kSpotAlpha);
            canvas->translate(160, 0);
        }
        canvas->restore();

        canvas->save();
        canvas->translate(100, 475);
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(elevations); ++i) {
            zPlaneParams.fZ = elevations[i];
            this->drawShadowedPath(canvas, fSmallRRPath, zPlaneParams, paint, kAmbientAlpha,
                                   lightPos, kLightRadius, kSpotAlpha);
            canvas->translate(160, 0);
        }
        canvas->restore();

        canvas->save();
        canvas->translate(100, 600);
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(elevations); ++i) {
            canvas->save();
            zPlaneParams.fZ = elevations[i];
            canvas->rotate(10);
            this->drawShadowedPath(canvas, fSmallRRPath, zPlaneParams, paint, kAmbientAlpha,
                                   lightPos, kLightRadius, kSpotAlpha);
            canvas->restore();
            canvas->translate(160, 0);
        }
        canvas->restore();

        canvas->save();
        canvas->translate(100, 725);
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(elevations); ++i) {
            canvas->save();
            zPlaneParams.fZ = elevations[i];
            canvas->rotate(45);
            this->drawShadowedPath(canvas, fSmallRRPath, zPlaneParams, paint, kAmbientAlpha,
                                   lightPos, kLightRadius, kSpotAlpha);
            canvas->restore();
            canvas->translate(160, 0);
        }
        canvas->restore();

    }

private:
    using INHERITED = Sample;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new MaterialShadowsView(); )

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "SkCanvas.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"
#include "SkSurfaceProps.h"
#include "Test.h"

class SurfaceParameters {
public:
    static const int kNumParams = 8;

    SurfaceParameters()
            : fWidth(64)
            , fHeight(64)
            , fOrigin(kTopLeft_GrSurfaceOrigin)
            , fColorType(kRGBA_8888_SkColorType)
            , fColorSpace(SkColorSpace::MakeSRGB())
            , fSampleCount(0)
            , fSurfaceProps(0x0, kUnknown_SkPixelGeometry) {
    }

    // Modify the SurfaceParameters in just one way
    void modify(int i) {
        switch (i) {
        case 0:
            fWidth = 63;
            break;
        case 1:
            fHeight = 63;
            break;
        case 2:
            fOrigin = kBottomLeft_GrSurfaceOrigin;
            break;
        case 3:
            fColorType = kAlpha_8_SkColorType;
            break;
        case 4:
            fColorSpace = SkColorSpace::MakeSRGBLinear();
            break;
        case 5:
            fSampleCount = 4;
            break;
        case 6:
            fSurfaceProps = SkSurfaceProps(0x0, kRGB_H_SkPixelGeometry);
            break;
        case 7:
            fSurfaceProps = SkSurfaceProps(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                                           kUnknown_SkPixelGeometry);
            break;
        }
    }

    // Create the surface with the current set of parameters
    sk_sp<SkSurface> make(GrContext* context) const {
        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        return SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, fSampleCount,
                                           fOrigin, &fSurfaceProps);
    }

private:
    int                 fWidth;
    int                 fHeight;
    GrSurfaceOrigin     fOrigin;
    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
    int                 fSampleCount;
    SkSurfaceProps      fSurfaceProps;
};

// This tests SkSurfaceCharacterization/SkSurface compatibility
DEF_GPUTEST_FOR_ALL_CONTEXTS(SkSurfaceCharacterization, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    std::unique_ptr<SkDeferredDisplayList> ddl;

    // First, create a DDL using the stock SkSurface parameters
    {
        SurfaceParameters params;

        sk_sp<SkSurface> s = params.make(context);
        if (!s) {
            return;
        }

        SkSurfaceCharacterization c;
        SkAssertResult(s->characterize(&c));

        SkDeferredDisplayListRecorder r(c);
        SkCanvas* canvas = r.getCanvas();
        canvas->drawRect(SkRect::MakeXYWH(10, 10, 10, 10), SkPaint());
        ddl = r.detach();

        REPORTER_ASSERT(reporter, s->draw(ddl.get()));
    }

    // Then, alter each parameter in turn and check that the DDL & surface are incompatible
    for (int i = 0; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params;
        params.modify(i);

        sk_sp<SkSurface> s = params.make(context);
        if (!s) {
            continue;
        }

        REPORTER_ASSERT(reporter, !s->draw(ddl.get()));
    }
}

#endif

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkSurface.h"
#include "Test.h"

static void test_drawPathEmpty(skiatest::Reporter*, SkCanvas* canvas) {
    // Filling an empty path should not crash.
    SkPaint paint;
    canvas->drawRect(SkRect(), paint);
    canvas->drawPath(SkPath(), paint);
    canvas->drawOval(SkRect(), paint);
    canvas->drawRect(SkRect(), paint);
    canvas->drawRRect(SkRRect(), paint);

    // Stroking an empty path should not crash.
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorGRAY);
    paint.setStrokeWidth(SkIntToScalar(20));
    paint.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawRect(SkRect(), paint);
    canvas->drawPath(SkPath(), paint);
    canvas->drawOval(SkRect(), paint);
    canvas->drawRect(SkRect(), paint);
    canvas->drawRRect(SkRRect(), paint);
}


DEF_GPUTEST(GpuDrawPath, reporter, factory) {
    return;

    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);

        GrContext* grContext = factory->get(glType);
        if (NULL == grContext) {
            continue;
        }
        static const int sampleCounts[] = { 0, 4, 16 };

        for (size_t i = 0; i < SK_ARRAY_COUNT(sampleCounts); ++i) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(255, 255);
            
            SkAutoTUnref<SkSurface> surface(
                SkSurface::NewRenderTarget(grContext, SkSurface::kNo_Budgeted, info,
                                           sampleCounts[i], NULL));
            test_drawPathEmpty(reporter, surface->getCanvas());
        }
    }
}

#endif

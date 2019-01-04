/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "SkDeferredDisplayList.h"
#include "SkDeferredDisplayListPriv.h"
#include "SkDeferredDisplayListRecorder.h"

#include "SkSurface.h"

#include "Test.h"

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(DDLFoo, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    SkSurfaceCharacterization characterization;
    SkAssertResult(s->characterize(&characterization));

    std::unique_ptr<SkDeferredDisplayList> ddl1, ddl2;

    {
        SkDeferredDisplayListRecorder recorder(characterization);

        SkCanvas* canvas = recorder.getCanvas();

        canvas->clear(SK_ColorRED);

        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(8, 8, 16, 16));

        ddl1 = recorder.detach();
    }

    {
        SkDeferredDisplayListRecorder recorder(characterization);

        SkCanvas* canvas = recorder.getCanvas();

        SkPaint p;
        p.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeWH(32, 32), p);

        ddl2 = recorder.detach();
    }

    REPORTER_ASSERT(reporter, ddl1->priv().lazyProxyData());
    REPORTER_ASSERT(reporter, ddl2->priv().lazyProxyData());

    // The lazy proxy data being different ensures that the SkSurface, SkCanvas and backing-
    // lazy proxy are all different between the two DDLs
    REPORTER_ASSERT(reporter, ddl1->priv().lazyProxyData() != ddl2->priv().lazyProxyData());

    s->draw(ddl1.get());
    s->draw(ddl2.get());

#if 0
    // Make sure the clipRect from DDL1 didn't percolate into DDL2
    s->readPixels(ii, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            REPORTER_ASSERT(reporter, bitmap.getColor(x, y) == SK_ColorGREEN);
            if (bitmap.getColor(x, y) != SK_ColorGREEN) {
                return; // we only really need to report the error once
            }
        }
    }
#endif
}


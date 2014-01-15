
/*
 * Copyright 2012 Google Inc.
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
#include "SkGpuDevice.h"
#include "SkPaint.h"
#include "SkPixelRef.h"
#include "SkRect.h"
#include "Test.h"

static const char* boolStr(bool value) {
    return value ? "true" : "false";
}

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "8888"
};

struct Pair {
    SkBitmap::Config    fConfig;
    const char*         fValid;
};

/**
 *  Check to ensure that copying a GPU-backed SkBitmap behaved as expected.
 *  @param reporter Used to report failures.
 *  @param desiredConfig Config being copied to. If the copy succeeded, dst must have this Config.
 *  @param success True if the copy succeeded.
 *  @param src A GPU-backed SkBitmap that had copyTo or deepCopyTo called on it.
 *  @param dst SkBitmap that was copied to.
 *  @param expectSameGenID Whether the genIDs should be the same if success is true.
 */
static void TestIndividualCopy(skiatest::Reporter* reporter, const SkBitmap::Config desiredConfig,
                               const bool success, const SkBitmap& src, const SkBitmap& dst,
                               const bool expectSameGenID) {
    if (success) {
        REPORTER_ASSERT(reporter, src.width() == dst.width());
        REPORTER_ASSERT(reporter, src.height() == dst.height());
        REPORTER_ASSERT(reporter, dst.config() == desiredConfig);
        if (src.config() == dst.config()) {
            if (expectSameGenID) {
                REPORTER_ASSERT(reporter, src.getGenerationID() == dst.getGenerationID());
            } else {
                REPORTER_ASSERT(reporter, src.getGenerationID() != dst.getGenerationID());
            }
            REPORTER_ASSERT(reporter, src.pixelRef() != NULL && dst.pixelRef() != NULL);

            // Do read backs and make sure that the two are the same.
            SkBitmap srcReadBack, dstReadBack;
            {
                SkASSERT(src.getTexture() != NULL);
                const SkIPoint origin = src.pixelRefOrigin();
                const SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY,
                                                         src.width(), src.height());
                bool readBack = src.pixelRef()->readPixels(&srcReadBack, &subset);
                REPORTER_ASSERT(reporter, readBack);
            }
            if (dst.getTexture() != NULL) {
                const SkIPoint origin = dst.pixelRefOrigin();
                const SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY,
                                                         dst.width(), dst.height());
                bool readBack = dst.pixelRef()->readPixels(&dstReadBack, &subset);
                REPORTER_ASSERT(reporter, readBack);
            } else {
                // If dst is not a texture, do a copy instead, to the same config as srcReadBack.
                bool copy = dst.copyTo(&dstReadBack, srcReadBack.config());
                REPORTER_ASSERT(reporter, copy);
            }

            SkAutoLockPixels srcLock(srcReadBack);
            SkAutoLockPixels dstLock(dstReadBack);
            REPORTER_ASSERT(reporter, srcReadBack.readyToDraw() && dstReadBack.readyToDraw());

            const char* srcP = static_cast<const char*>(srcReadBack.getAddr(0, 0));
            const char* dstP = static_cast<const char*>(dstReadBack.getAddr(0, 0));
            REPORTER_ASSERT(reporter, srcP != dstP);

            REPORTER_ASSERT(reporter, !memcmp(srcP, dstP, srcReadBack.getSize()));
        } else {
            REPORTER_ASSERT(reporter, src.getGenerationID() != dst.getGenerationID());
        }
    } else {
        // dst should be unchanged from its initial state
        REPORTER_ASSERT(reporter, dst.config() == SkBitmap::kNo_Config);
        REPORTER_ASSERT(reporter, dst.width() == 0);
        REPORTER_ASSERT(reporter, dst.height() == 0);
    }

}

// Stripped down version of TestBitmapCopy that checks basic fields (width, height, config, genID)
// to ensure that they were copied properly.
DEF_GPUTEST(GpuBitmapCopy, reporter, factory) {
#ifdef SK_BUILD_FOR_ANDROID // https://code.google.com/p/skia/issues/detail?id=753
    return;
#endif
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);
        if (!GrContextFactory::IsRenderingGLContext(glType)) {
            continue;
        }

        GrContext* grContext = factory->get(glType);
        if (NULL == grContext) {
            continue;
        }


        if (NULL == grContext) {
            return;
        }
        static const Pair gPairs[] = {
            { SkBitmap::kNo_Config,         "00"  },
            { SkBitmap::kARGB_8888_Config,  "01"  },
        };

        const int W = 20;
        const int H = 33;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
            SkBitmap src, dst;

            SkGpuDevice* device = SkNEW_ARGS(SkGpuDevice, (grContext, gPairs[i].fConfig, W, H));
            SkAutoUnref aur(device);
            src = device->accessBitmap(false);
            device->clear(SK_ColorWHITE);

            // Draw something different to the same portion of the bitmap that we will extract as a
            // subset, so that comparing the pixels of the subset will be meaningful.
            SkIRect subsetRect = SkIRect::MakeLTRB(W/2, H/2, W, H);
            SkCanvas drawingCanvas(device);
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            drawingCanvas.drawRect(SkRect::Make(subsetRect), paint);

            // Extract a subset. If this succeeds we will test copying the subset.
            SkBitmap subset;
            const bool extracted = src.extractSubset(&subset, subsetRect);

            for (size_t j = 0; j < SK_ARRAY_COUNT(gPairs); j++) {
                dst.reset();
                bool success = src.deepCopyTo(&dst, gPairs[j].fConfig);
                bool expected = gPairs[i].fValid[j] != '0';
                if (success != expected) {
                    ERRORF(reporter, "SkBitmap::deepCopyTo from %s to %s. "
                           "expected %s returned %s", gConfigName[i],
                           gConfigName[j], boolStr(expected),
                           boolStr(success));
                }

                bool canSucceed = src.canCopyTo(gPairs[j].fConfig);
                if (success != canSucceed) {
                    ERRORF(reporter, "SkBitmap::deepCopyTo from %s to %s "
                           "returned %s, but canCopyTo returned %s",
                           gConfigName[i], gConfigName[j], boolStr(success),
                           boolStr(canSucceed));
                }

                TestIndividualCopy(reporter, gPairs[j].fConfig, success, src, dst, true);

                // Test copying the subset bitmap, using both copyTo and deepCopyTo.
                if (extracted) {
                    SkBitmap subsetCopy;
                    success = subset.copyTo(&subsetCopy, gPairs[j].fConfig);
                    REPORTER_ASSERT(reporter, success == expected);
                    REPORTER_ASSERT(reporter, success == canSucceed);
                    TestIndividualCopy(reporter, gPairs[j].fConfig, success, subset, subsetCopy,
                                       true);

                    // Reset the bitmap so that a failed copyTo will leave it in the expected state.
                    subsetCopy.reset();
                    success = subset.deepCopyTo(&subsetCopy, gPairs[j].fConfig);
                    REPORTER_ASSERT(reporter, success == expected);
                    REPORTER_ASSERT(reporter, success == canSucceed);
                    TestIndividualCopy(reporter, gPairs[j].fConfig, success, subset, subsetCopy,
                                       true);

                    // Now set a bitmap to be a subset that will share the same pixelref.
                    // This allows testing another case of cloning the genID. When calling copyTo
                    // on a bitmap representing a subset of its pixelref, the resulting pixelref
                    // should not share the genID, since we only copied the subset.
                    SkBitmap trueSubset;
                    // FIXME: Once https://codereview.chromium.org/109023008/ lands, call
                    // trueSubset.installPixelRef(src.pixelRef(), subset);
                    trueSubset.setConfig(gPairs[i].fConfig, W/2, H/2);
                    trueSubset.setPixelRef(src.pixelRef(), W/2, H/2);

                    subsetCopy.reset();
                    success = trueSubset.copyTo(&subsetCopy, gPairs[j].fConfig);
                    REPORTER_ASSERT(reporter, success == expected);
                    REPORTER_ASSERT(reporter, success == canSucceed);
                    TestIndividualCopy(reporter, gPairs[j].fConfig, success, trueSubset, subsetCopy,
                                       false);

                    // deepCopyTo copies the entire pixelref, even if the bitmap only represents
                    // a subset. Therefore, the result should share the same genID.
                    subsetCopy.reset();
                    success = trueSubset.deepCopyTo(&subsetCopy, gPairs[j].fConfig);
                    REPORTER_ASSERT(reporter, success == expected);
                    REPORTER_ASSERT(reporter, success == canSucceed);
                    TestIndividualCopy(reporter, gPairs[j].fConfig, success, trueSubset, subsetCopy,
                                       true);
                }
            } // for (size_t j = ...
        } // for (size_t i = ...
    } // GrContextFactory::GLContextType
}

#endif

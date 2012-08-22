
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "SkBitmap.h"
#include "SkGpuDevice.h"
#include "SkPixelRef.h"
#include "SkRect.h"
#include "Test.h"

static const char* boolStr(bool value) {
    return value ? "true" : "false";
}

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "4444", "8888"
};

struct Pair {
    SkBitmap::Config    fConfig;
    const char*         fValid;
};

// Stripped down version of TestBitmapCopy that checks basic fields (width, height, config, genID)
// to ensure that they were copied properly.
static void TestGpuBitmapCopy(skiatest::Reporter* reporter, GrContext* grContext) {
    if (NULL == grContext) {
        return;
    }
    static const Pair gPairs[] = {
        { SkBitmap::kNo_Config,         "000"  },
        { SkBitmap::kARGB_4444_Config,  "011"  },
        { SkBitmap::kARGB_8888_Config,  "011"  },
    };

    const int W = 20;
    const int H = 33;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        for (size_t j = 0; j < SK_ARRAY_COUNT(gPairs); j++) {
            SkBitmap src, dst;

            SkGpuDevice* device = SkNEW_ARGS(SkGpuDevice, (grContext, gPairs[i].fConfig, W, H));
            SkAutoUnref aur(device);
            src = device->accessBitmap(false);
            device->clear(SK_ColorWHITE);

            bool success = src.deepCopyTo(&dst, gPairs[j].fConfig);
            bool expected = gPairs[i].fValid[j] != '0';
            if (success != expected) {
                SkString str;
                str.printf("SkBitmap::deepCopyTo from %s to %s. expected %s returned %s",
                           gConfigName[i], gConfigName[j], boolStr(expected),
                           boolStr(success));
                reporter->reportFailed(str);
            }

            bool canSucceed = src.canCopyTo(gPairs[j].fConfig);
            if (success != canSucceed) {
                SkString str;
                str.printf("SkBitmap::deepCopyTo from %s to %s returned %s,"
                           "but canCopyTo returned %s",
                           gConfigName[i], gConfigName[j], boolStr(success),
                           boolStr(canSucceed));
                reporter->reportFailed(str);
            }

            if (success) {
                REPORTER_ASSERT(reporter, src.width() == dst.width());
                REPORTER_ASSERT(reporter, src.height() == dst.height());
                REPORTER_ASSERT(reporter, dst.config() == gPairs[j].fConfig);
                if (src.config() == dst.config()) {
                    REPORTER_ASSERT(reporter, src.getGenerationID() == dst.getGenerationID());
                    // Do read backs and make sure that the two are the same.
                    SkBitmap srcReadBack, dstReadBack;
                    REPORTER_ASSERT(reporter, src.pixelRef() != NULL
                                    && dst.pixelRef() != NULL);
                    src.pixelRef()->readPixels(&srcReadBack);
                    dst.pixelRef()->readPixels(&dstReadBack);
                    SkAutoLockPixels srcLock(srcReadBack);
                    SkAutoLockPixels dstLock(dstReadBack);
                    REPORTER_ASSERT(reporter, srcReadBack.readyToDraw()
                                    && dstReadBack.readyToDraw());
                    const char* srcP = (const char*)srcReadBack.getAddr(0, 0);
                    const char* dstP = (const char*)dstReadBack.getAddr(0, 0);
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
        } // for (size_t j = ...
    }
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GpuBitmapCopy", TestGpuBitmapCopyClass, TestGpuBitmapCopy)


/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkColorMatrixFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkRect.h"

class ImageFilterTest {
public:

    static SkImageFilter* make_scale(float amount, SkImageFilter* input = NULL) {
        SkScalar s = SkFloatToScalar(amount);
        SkScalar matrix[20] = { s, 0, 0, 0, 0,
                                0, s, 0, 0, 0,
                                0, 0, s, 0, 0,
                                0, 0, 0, s, 0 };
        SkAutoTUnref<SkColorFilter> filter(new SkColorMatrixFilter(matrix));
        return SkColorFilterImageFilter::Create(filter, input);
    }

    static SkImageFilter* make_grayscale(SkImageFilter* input = NULL, const SkIRect* cropRect = NULL) {
        SkScalar matrix[20];
        memset(matrix, 0, 20 * sizeof(SkScalar));
        matrix[0] = matrix[5] = matrix[10] = SkFloatToScalar(0.2126f);
        matrix[1] = matrix[6] = matrix[11] = SkFloatToScalar(0.7152f);
        matrix[2] = matrix[7] = matrix[12] = SkFloatToScalar(0.0722f);
        matrix[18] = SkFloatToScalar(1.0f);
        SkAutoTUnref<SkColorFilter> filter(new SkColorMatrixFilter(matrix));
        return SkColorFilterImageFilter::Create(filter, input, cropRect);
    }

    static SkImageFilter* make_mode_blue(SkImageFilter* input = NULL) {
        SkAutoTUnref<SkColorFilter> filter(
            SkColorFilter::CreateModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
        return SkColorFilterImageFilter::Create(filter, input);
    }

    static void Test(skiatest::Reporter* reporter) {
        {
            // Check that two non-clipping color matrices concatenate into a single filter.
            SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f));
            SkAutoTUnref<SkImageFilter> quarterBrightness(make_scale(0.5f, halfBrightness));
            REPORTER_ASSERT(reporter, NULL == quarterBrightness->getInput(0));
        }

        {
            // Check that a clipping color matrix followed by a grayscale does not concatenate into a single filter.
            SkAutoTUnref<SkImageFilter> doubleBrightness(make_scale(2.0f));
            SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f, doubleBrightness));
            REPORTER_ASSERT(reporter, NULL != halfBrightness->getInput(0));
        }

        {
            // Check that a color filter image filter without a crop rect can be
            // expressed as a color filter.
            SkAutoTUnref<SkImageFilter> gray(make_grayscale());
            REPORTER_ASSERT(reporter, true == gray->asColorFilter(NULL));
        }

        {
            // Check that a color filter image filter with a crop rect cannot
            // be expressed as a color filter.
            SkIRect cropRect = SkIRect::MakeXYWH(0, 0, 100, 100);
            SkAutoTUnref<SkImageFilter> grayWithCrop(make_grayscale(NULL, &cropRect));
            REPORTER_ASSERT(reporter, false == grayWithCrop->asColorFilter(NULL));
        }
    }
};


#include "TestClassDef.h"
DEFINE_TESTCLASS("ImageFilterTest", ImageFilterTestClass, ImageFilterTest::Test)

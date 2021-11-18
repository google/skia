/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/Tessellation.h"
#include "tests/Test.h"

DEF_TEST(PreChopPathCurves, reporter) {
    // These particular test cases can get stuck in infinite recursion due to limited fp32
    // precision. (Although they will not with the provided tessellationPrecision values; we had to
    // lower precision in order to avoid the "viewport size" assert in PreChopPathCurves.) Bump the
    // tessellationPreciion up to 4 and run these tests in order to verify our bail condition for
    // infinite recursion caused by bad fp32 precision. If the test completes, it passed.
    SkPath p = SkPath().moveTo(11.171727877046647f, -11.78621173228717f)
                       .quadTo(11.171727877046647f, -11.78621173228717f,
                               8.33583747124031f, 77.27177002747368f)
                       .cubicTo(8.33583747124031f, 77.27177002747368f,
                                8.33583747124031f, 77.27177002747368f,
                                11.171727877046647f, -11.78621173228717f)
                       .conicTo(11.171727877046647f, -11.78621173228717f,
                                8.33583747124031f, 77.27177002747368f,
                                1e-6f)
                       .conicTo(8.33583747124031f, 77.27177002747368f,
                                11.171727877046647f, -11.78621173228717f,
                                1e6f);

    SkMatrix m = SkMatrix::Scale(138.68622826903837f, 74192976757580.44189f);
    skgpu::PreChopPathCurves(1/16.f, p, m, {1000, -74088852800000.f, 3000, -74088852700000.f});

    m = SkMatrix::Scale(138.68622826903837f, 74192976757580.44189f*.3f);
    skgpu::PreChopPathCurves(.25f, p, m, {1000, -22226658140000.f, 3000, -22226658130000.f});

    m = SkMatrix::Scale(138.68622826903837f, 74192976757580.44189f/4);
    skgpu::PreChopPathCurves(.25f, p, m, {1000, -18522213200000.f, 3000, -18522213100000.f});
}

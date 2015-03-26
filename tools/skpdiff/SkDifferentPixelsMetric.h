/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDifferentPixelsMetric_DEFINED
#define SkDifferentPixelsMetric_DEFINED

#include "SkTDArray.h"

#if SK_SUPPORT_OPENCL
#include "SkCLImageDiffer.h"
#else
#include "SkImageDiffer.h"
#endif

/**
 * A differ that measures the percentage of different corresponding pixels. If the two images are
 * not the same size or have no pixels, the result will always be zero.
 */
class SkDifferentPixelsMetric :
#if SK_SUPPORT_OPENCL
    public SkCLImageDiffer {
#else
    public SkImageDiffer {
#endif
public:
    const char* getName() const override;
    virtual bool diff(SkBitmap* baseline, SkBitmap* test,
                      const BitmapsToCreate& bitmapsToCreate,
                      Result* result) const override;

protected:
#if SK_SUPPORT_OPENCL
    bool onInit() override;
#endif

private:
#if SK_SUPPORT_OPENCL
    cl_kernel fKernel;

    typedef SkCLImageDiffer INHERITED;
#else
    typedef SkImageDiffer INHERITED;
#endif
};

#endif

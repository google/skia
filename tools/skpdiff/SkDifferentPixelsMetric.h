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
    virtual const char* getName() const SK_OVERRIDE;
    virtual bool diff(SkBitmap* baseline, SkBitmap* test, bool computeMask,
                      Result* result) const SK_OVERRIDE;

protected:
#if SK_SUPPORT_OPENCL
    virtual bool onInit() SK_OVERRIDE;
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

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
    virtual const char* getName() SK_OVERRIDE;
    virtual int queueDiff(SkBitmap* baseline, SkBitmap* test) SK_OVERRIDE;
    virtual void deleteDiff(int id) SK_OVERRIDE;
    virtual bool isFinished(int id) SK_OVERRIDE;
    virtual double getResult(int id) SK_OVERRIDE;
    virtual int getPointsOfInterestCount(int id) SK_OVERRIDE;
    virtual SkIPoint* getPointsOfInterest(int id) SK_OVERRIDE;

protected:
#if SK_SUPPORT_OPENCL
    virtual bool onInit() SK_OVERRIDE;
#endif

private:
    struct QueuedDiff;

    SkTDArray<QueuedDiff> fQueuedDiffs;

#if SK_SUPPORT_OPENCL
    cl_kernel fKernel;

    typedef SkCLImageDiffer INHERITED;
#else
    typedef SkImageDiffer INHERITED;
#endif
};

#endif

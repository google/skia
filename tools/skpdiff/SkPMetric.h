/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPMetric_DEFINED
#define SkPMetric_DEFINED

#include "SkTArray.h"
#include "SkTDArray.h"

#include "SkImageDiffer.h"

/**
 * An image differ that uses the pdiff image metric to compare images.
 */
class SkPMetric : public SkImageDiffer {
public:
    virtual const char* getName() SK_OVERRIDE;
    virtual int queueDiff(SkBitmap* baseline, SkBitmap* test) SK_OVERRIDE;
    virtual void deleteDiff(int id) SK_OVERRIDE;
    virtual bool isFinished(int id) SK_OVERRIDE;
    virtual double getResult(int id) SK_OVERRIDE;
    virtual int getPointsOfInterestCount(int id) SK_OVERRIDE;
    virtual SkIPoint* getPointsOfInterest(int id) SK_OVERRIDE;

private:
    struct QueuedDiff {
        bool finished;
        double result;
        SkTDArray<SkIPoint> poi;
    };

    SkTArray<QueuedDiff> fQueuedDiffs;

    typedef SkImageDiffer INHERITED;
};


#endif

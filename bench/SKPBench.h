/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPBench_DEFINED
#define SKPBench_DEFINED

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it scaled inside a device clip.
 */
class SKPBench : public Benchmark {
public:
    SKPBench(const char* name, const SkPicture*, const SkIRect& devClip, SkScalar scale);

protected:
    virtual const char* onGetName() SK_OVERRIDE;
    virtual const char* onGetUniqueName() SK_OVERRIDE;
    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE;
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE;
    virtual SkIPoint onGetSize() SK_OVERRIDE;

private:
    SkAutoTUnref<const SkPicture> fPic;
    const SkIRect fClip;
    const SkScalar fScale;
    SkString fName;
    SkString fUniqueName;

    typedef Benchmark INHERITED;
};

#endif

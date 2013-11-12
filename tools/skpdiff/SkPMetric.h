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
    virtual const char* getName() const SK_OVERRIDE { return "perceptual"; }
    virtual bool diff(SkBitmap* baseline, SkBitmap* test, bool computeMask,
                      Result* result) const SK_OVERRIDE;

private:
    typedef SkImageDiffer INHERITED;
};


#endif

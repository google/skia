/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPLSGeometryProcessor_DEFINED
#define GrPLSGeometryProcessor_DEFINED

#include "GrGeometryProcessor.h"

/**
 * A minor extension to GrGeometryProcessor that adds bounds tracking for pixel local storage
 * purposes.
 */
class GrPLSGeometryProcessor : public GrGeometryProcessor {
public:
    GrPixelLocalStorageState getPixelLocalStorageState() const override {
        return GrPixelLocalStorageState::kDraw_GrPixelLocalStorageState;
    }

    const SkRect& getBounds() const {
       return fBounds;
    }

    void setBounds(SkRect& bounds) {
       fBounds = bounds;
    }

private:
    SkRect fBounds;
};

#endif

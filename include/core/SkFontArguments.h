/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontAgruments_DEFINED
#define SkFontAgruments_DEFINED

#include "SkScalar.h"
#include "SkTypes.h"

/** Represents a set of actual arguments for a font. */
struct SkFontArguments {
    struct Axis {
        SkFourByteTag fTag;
        SkScalar fStyleValue;
    };

    struct VariationCoordinate {
        SkFourByteTag axis;
        SkScalar value;
    };
    SkFontArguments() : fCollectionIndex(0), fAxisCount(0), fAxes(nullptr) {}

    /** Specify the index of the desired font.
     *
     *  Font formats like ttc, dfont, cff, cid, pfr, t42, t1, and fon may actually be indexed
     *  collections of fonts.
     */
    SkFontArguments& setCollectionIndex(int collectionIndex) {
        fCollectionIndex = collectionIndex;
        return *this;
    }

    /** Specify the GX variation axis values.
     *
     *  Any axes not specified will use the default value. Specified axes not present in the
     *  font will be ignored.
     *
     *  @param axes not copied. This pointer must remain valid for life of SkFontArguments.
     */
    SkFontArguments& setAxes(const Axis* axes, int axisCount) {
        fAxisCount = axisCount;
        fAxes = axes;
        return *this;
    }

    int getCollectionIndex() const {
        return fCollectionIndex;
    }
    const Axis* getAxes(int* axisCount) const {
        *axisCount = fAxisCount;
        return fAxes;
    }
private:
    int fCollectionIndex;
    int fAxisCount;
    const Axis* fAxes;
};

#endif

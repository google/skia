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

    struct VariationPosition {
        struct Coordinate {
            SkFourByteTag axis;
            SkScalar value;
        };
        Coordinate* coordinates;
        int coordinateCount;
    };
    SkFontArguments() : fCollectionIndex(0), fVariationDesignPosition{nullptr, 0} {}

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
        fVariationDesignPosition.coordinateCount = axisCount;
        fVariationDesignPosition.coordinates =
            const_cast<VariationPosition::Coordinate*>(
                reinterpret_cast<const VariationPosition::Coordinate*>(axes));
        return *this;
    }
    SkFontArguments& setVariationDesignPosition(VariationPosition position) {
        fVariationDesignPosition = position;
        return *this;
    }

    int getCollectionIndex() const {
        return fCollectionIndex;
    }
    const Axis* getAxes(int* axisCount) const {
        *axisCount = fVariationDesignPosition.coordinateCount;
        return reinterpret_cast<const Axis*>(fVariationDesignPosition.coordinates);
    }
    const VariationPosition getVariationDesignPosition() {
        return fVariationDesignPosition;
    }
private:
    int fCollectionIndex;
    VariationPosition fVariationDesignPosition;
};

#endif

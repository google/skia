/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontArguments_DEFINED
#define SkFontArguments_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"

/** Represents a set of actual arguments for a font. */
struct SkFontArguments {
    struct VariationPosition {
        struct Coordinate {
            SkFourByteTag axis;
            float value;
        };
        const Coordinate* coordinates;
        int coordinateCount;
    };
    // deprecated, use VariationPosition::Coordinate instead
    struct Axis {
       SkFourByteTag fTag;
       float fStyleValue;
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

    // deprecated, use setVariationDesignPosition instead.
    SkFontArguments& setAxes(const Axis* axes, int axisCount) {
        fVariationDesignPosition.coordinates =
                reinterpret_cast<const VariationPosition::Coordinate*>(axes);
        fVariationDesignPosition.coordinateCount = axisCount;
        return *this;
    }

    /** Specify a position in the variation design space.
     *
     *  Any axis not specified will use the default value.
     *  Any specified axis not actually present in the font will be ignored.
     *
     *  @param position not copied. The value must remain valid for life of SkFontArguments.
     */
    SkFontArguments& setVariationDesignPosition(VariationPosition position) {
        fVariationDesignPosition.coordinates = position.coordinates;
        fVariationDesignPosition.coordinateCount = position.coordinateCount;
        return *this;
    }

    int getCollectionIndex() const {
        return fCollectionIndex;
    }
    // deprecated, use getVariationDesignPosition instead.
    const Axis* getAxes(int* axisCount) const {
        *axisCount = fVariationDesignPosition.coordinateCount;
        return reinterpret_cast<const Axis*>(fVariationDesignPosition.coordinates);
    }
    VariationPosition getVariationDesignPosition() const {
        return fVariationDesignPosition;
    }
private:
    int fCollectionIndex;
    VariationPosition fVariationDesignPosition;
};

#endif

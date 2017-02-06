/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowTessellator_DEFINED
#define SkShadowTessellator_DEFINED

#include "SkTDArray.h"
#include "SkRefCnt.h"
#include "SkPoint.h"

#include "SkColor.h"

class SkMatrix;
class SkPath;

class SkShadowVertices : public SkRefCnt {
public:
    /**
     * This function generates an ambient shadow mesh for a path by walking the path, outsetting by
     * the radius, and setting inner and outer colors to umbraColor and penumbraColor, respectively.
     * If transparent is true, then the center of the ambient shadow will be filled in.
     */
    static sk_sp<SkShadowVertices> MakeAmbient(const SkPath& path, const SkMatrix& ctm,
                                               SkScalar radius, SkColor umbraColor,
                                               SkColor penumbraColor, bool transparent);

    /**
     * This function generates a spot shadow mesh for a path by walking the transformed path,
     * further transforming by the scale and translation, and outsetting and insetting by a radius.
     * The center will be clipped against the original path unless transparent is true.
     */
    static sk_sp<SkShadowVertices> MakeSpot(const SkPath& path, const SkMatrix& ctm,
                                            SkScalar scale, const SkVector& translate,
                                            SkScalar radius, SkColor umbraColor,
                                            SkColor penumbraColor, bool transparent);

    int vertexCount() const { return fVertexCnt; }
    const SkPoint* positions() const { return fPositions.get(); }
    const SkColor* colors() const { return fColors.get(); }

    int indexCount() const { return fIndexCnt; }
    const uint16_t* indices() const { return fIndices.get(); }

    size_t size() const {
        return sizeof(*this) + fVertexCnt * (sizeof(SkPoint) + sizeof(SkColor)) +
               fIndexCnt * sizeof(uint16_t);
    }

private:
    template<typename T> using Deleter = SkTDArray<SkPoint>::Deleter;
    template<typename T> using UniqueArray = std::unique_ptr<const T[], Deleter<T>>;

    SkShadowVertices(UniqueArray<SkPoint>&& positions, UniqueArray<SkColor>&& colors,
                     UniqueArray<uint16_t>&& indices, int vertexCnt, int indexCnt)
            : fVertexCnt(vertexCnt)
            , fIndexCnt(indexCnt)
            , fPositions(std::move(positions))
            , fColors(std::move(colors))
            , fIndices(std::move(indices)) {
        SkASSERT(SkToBool(fPositions) && SkToBool(fColors) && SkToBool(vertexCnt) &&
                 SkToBool(fIndices) && SkToBool(indexCnt));
    }

    int fVertexCnt;
    int fIndexCnt;
    UniqueArray<SkPoint> fPositions;
    UniqueArray<SkColor> fColors;
    UniqueArray<uint16_t> fIndices;
};

#endif

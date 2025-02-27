/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDefaultGeoProcFactory_DEFINED
#define GrDefaultGeoProcFactory_DEFINED

#include "include/private/base/SkAssert.h"
#include "src/core/SkColorData.h"

#include <cstdint>

class GrGeometryProcessor;
class SkArenaAlloc;
class SkMatrix;

/*
 * A factory for creating default Geometry Processors which simply multiply position by the uniform
 * view matrix and wire through color, coverage, UV coords if requested.
 */
namespace GrDefaultGeoProcFactory {
    struct Color {
        enum Type {
            kPremulGrColorUniform_Type,
            kPremulGrColorAttribute_Type,
            kPremulWideColorAttribute_Type,
        };
        explicit Color(const SkPMColor4f& color)
                : fType(kPremulGrColorUniform_Type)
                , fColor(color) {}
        Color(Type type)
                : fType(type)
                , fColor(SK_PMColor4fILLEGAL) {
            SkASSERT(type != kPremulGrColorUniform_Type);
        }

        Type fType;
        SkPMColor4f fColor;
    };

    struct Coverage {
        enum Type {
            kSolid_Type,
            kUniform_Type,
            kAttribute_Type,
            kAttributeTweakAlpha_Type,
            kAttributeUnclamped_Type,  // Fragment shader will call saturate(coverage) before using.
                                       // (Not compatible with kAttributeTweakAlpha_Type.)
        };
        explicit Coverage(uint8_t coverage) : fType(kUniform_Type), fCoverage(coverage) {}
        Coverage(Type type) : fType(type), fCoverage(0xff) {
            SkASSERT(type != kUniform_Type);
        }

        Type fType;
        uint8_t fCoverage;
    };

    struct LocalCoords {
        enum Type {
            kUnused_Type,
            kUsePosition_Type,
            kHasExplicit_Type,
        };
        LocalCoords(Type type) : fType(type), fMatrix(nullptr) {}
        LocalCoords(Type type, const SkMatrix* matrix) : fType(type), fMatrix(matrix) {
            SkASSERT(kUnused_Type != type);
        }
        bool hasLocalMatrix() const { return nullptr != fMatrix; }

        Type fType;
        const SkMatrix* fMatrix;
    };

    GrGeometryProcessor* Make(SkArenaAlloc*,
                              const Color&,
                              const Coverage&,
                              const LocalCoords&,
                              const SkMatrix& viewMatrix);

    /*
     * Use this factory to create a GrGeometryProcessor that expects a device space vertex position
     * attribute. The view matrix must still be provided to compute correctly transformed
     * coordinates for GrFragmentProcessors. It may fail if the view matrix is not invertible.
     */
    GrGeometryProcessor* MakeForDeviceSpace(SkArenaAlloc*,
                                            const Color&,
                                            const Coverage&,
                                            const LocalCoords&,
                                            const SkMatrix& viewMatrix);
}  // namespace GrDefaultGeoProcFactory

#endif

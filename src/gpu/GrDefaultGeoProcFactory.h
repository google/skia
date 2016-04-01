/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDefaultGeoProcFactory_DEFINED
#define GrDefaultGeoProcFactory_DEFINED

#include "GrGeometryProcessor.h"

/*
 * A factory for creating default Geometry Processors which simply multiply position by the uniform
 * view matrix and wire through color, coverage, UV coords if requested.  Right now this is only
 * used in the creation of optimized draw states because adding default GPs to the drawstate can
 * interfere with batching due to updating the drawstate.
 */
namespace GrDefaultGeoProcFactory {
    // Structs for adding vertex attributes
    struct PositionAttr {
        SkPoint fPosition;
    };

    struct PositionCoverageAttr {
        SkPoint fPosition;
        GrColor fCoverage;
    };

    struct PositionColorAttr {
        SkPoint fPosition;
        SkColor fColor;
    };

    struct PositionColorCoverageAttr {
        SkPoint fPosition;
        SkColor fColor;
        GrColor fCoverage;
    };

    struct PositionLocalCoordAttr {
        SkPoint fPosition;
        SkPoint fLocalCoord;
    };

    struct PositionLocalCoordCoverageAttr {
        SkPoint fPosition;
        SkPoint fLocalCoord;
        GrColor fCoverage;
    };

    struct PositionColorLocalCoordAttr {
        SkPoint fPosition;
        GrColor fColor;
        SkPoint fLocalCoord;
    };

    struct PositionColorLocalCoordCoverage {
        SkPoint fPosition;
        GrColor fColor;
        SkPoint fLocalCoord;
        GrColor fCoverage;
    };

    struct Color {
        enum Type {
            kNone_Type,
            kUniform_Type,
            kAttribute_Type,
        };
        Color(GrColor color) : fType(kUniform_Type), fColor(color) {}
        Color(Type type) : fType(type), fColor(GrColor_ILLEGAL) {
            SkASSERT(type != kUniform_Type);

            // TODO This is temporary
            if (kAttribute_Type == type) {
                fColor = GrColor_WHITE;
            }
        }

        Type fType;
        GrColor fColor;
    };

    struct Coverage {
        enum Type {
            kNone_Type,
            kSolid_Type,
            kUniform_Type,
            kAttribute_Type,
        };
        Coverage(uint8_t coverage) : fType(kUniform_Type), fCoverage(coverage) {}
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
            kHasTransformed_Type,
        };
        LocalCoords(Type type) : fType(type), fMatrix(nullptr) {}
        LocalCoords(Type type, const SkMatrix* matrix) : fType(type), fMatrix(matrix) {
            SkASSERT(kUnused_Type != type);
        }
        bool hasLocalMatrix() const { return nullptr != fMatrix; }

        Type fType;
        const SkMatrix* fMatrix;
    };

    const GrGeometryProcessor* Create(const Color&,
                                      const Coverage&,
                                      const LocalCoords&,
                                      const SkMatrix& viewMatrix);

    /*
     * Use this factory to create a GrGeometryProcessor that expects a device space vertex position
     * attribute. The view matrix must still be provided to compute correctly transformed
     * coordinates for GrFragmentProcessors. It may fail if the view matrix is not invertible.
     */
    const GrGeometryProcessor* CreateForDeviceSpace(const Color&,
                                                    const Coverage&,
                                                    const LocalCoords&,
                                                    const SkMatrix& viewMatrix);

    inline size_t DefaultVertexStride() { return sizeof(PositionAttr); }
};

#endif

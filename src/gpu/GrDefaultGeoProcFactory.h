/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDefaultGeoProcFactory_DEFINED
#define GrDefaultGeoProcFactory_DEFINED

#include "GrGeometryProcessor.h"

class GrDrawState;

/*
 * A factory for creating default Geometry Processors which simply multiply position by the uniform
 * view matrix and wire through color, coverage, UV coords if requested.  Right now this is only
 * used in the creation of optimized draw states because adding default GPs to the drawstate can
 * interfere with batching due to updating the drawstate.
 */
class GrDefaultGeoProcFactory {
public:
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

    enum GPType {
        kPosition_GPType = 0x0, // we ALWAYS have position
        kColor_GPType = 0x01,
        kLocalCoord_GPType = 0x02,
        kCoverage_GPType= 0x04,
        kLastGPType = kCoverage_GPType
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
            kNone_Type,
            kUsePosition_Type,
            kHasExplicit_Type,
        };
    };

    static const GrGeometryProcessor* Create(const Color&,
                                             const Coverage&,
                                             LocalCoords::Type,
                                             const SkMatrix& viewMatrix = SkMatrix::I(),
                                             const SkMatrix& localMatrix = SkMatrix::I());

    /*
     * The following functions are used to create default GPs. If you just need to create
     * attributes separately from creating the default GP, use the SetAttribs function followed
     * by the Create function. Otherwise use CreateAndSetAttribs to do both at once.
     *
     * You must unref the return from Create.
     */
    // TODO clean this up
    static const GrGeometryProcessor* Create(uint32_t gpTypeFlags,
                                             GrColor,
                                             bool localCoordsWillBeRead,
                                             bool coverageWillBeIgnored,
                                             const SkMatrix& viewMatrix = SkMatrix::I(),
                                             const SkMatrix& localMatrix = SkMatrix::I(),
                                             uint8_t coverage = 0xff);

    static size_t DefaultVertexStride() { return sizeof(PositionAttr); }
};

#endif

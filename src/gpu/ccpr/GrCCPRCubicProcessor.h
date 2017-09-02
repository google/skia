/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRCubicProcessor_DEFINED
#define GrCCPRCubicProcessor_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

class GrGLSLGeometryBuilder;

/**
 * This class renders the coverage of convex closed cubic segments using the techniques outlined in
 * "Resolution Independent Curve Rendering using Programmable Graphics Hardware" by Charles Loop and
 * Jim Blinn:
 *
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
 *
 * The caller is expected to chop cubics at the KLM roots (a.k.a. inflection points and loop
 * intersection points, resulting in necessarily convex segments) before feeding them into this
 * processor. (Use GrCCPRGeometry.)
 *
 * The curves are rendered in two passes:
 *
 * Pass 1: Draw the (convex) bezier quadrilateral, inset by 1/2 pixel all around, and use the
 *         gradient-based AA technique outlined in the Loop/Blinn paper to compute coverage.
 *
 * Pass 2: Draw a border around the previous inset, up to the bezier quadrilatral's conservative
 *         raster hull, and compute coverage using pseudo MSAA. This pass is necessary because the
 *         gradient approach does not work near the L and M lines.
 *
 * FIXME: The pseudo MSAA border is slow and ugly. We should investigate an alternate solution of
 * just approximating the curve with straight lines for short distances across the problem points
 * instead.
 */
class GrCCPRCubicProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    enum class CubicType {
        kSerpentine,
        kLoop
    };

    GrCCPRCubicProcessor(CubicType cubicType)
            : INHERITED(CoverageType::kShader)
            , fCubicType(cubicType)
            , fKLMMatrix("klm_matrix", kMat33f_GrSLType, GrShaderVar::kNonArray,
                         kHigh_GrSLPrecision)
            , fKLMDerivatives("klm_derivatives", kVec2f_GrSLType, 3, kHigh_GrSLPrecision)
            , fEdgeDistanceEquation("edge_distance_equation", kVec3f_GrSLType,
                                    GrShaderVar::kNonArray, kHigh_GrSLPrecision)
            , fKLMD(kVec4f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        varyingHandler->addVarying("klmd", &fKLMD, kHigh_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const TexelBufferHandle& pointsBuffer, const char* atlasOffset,
                            const char* rtAdjust, GrGPArgs*) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* rtAdjust, const char* outputWind) const final;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjust) const final;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const final;

protected:
    virtual void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                   const char* wind, const char* rtAdjust) const = 0;
    virtual void onEmitPerVertexGeometryCode(SkString* fnBody) const = 0;

    const CubicType   fCubicType;
    GrShaderVar       fKLMMatrix;
    GrShaderVar       fKLMDerivatives;
    GrShaderVar       fEdgeDistanceEquation;
    GrGLSLGeoToFrag   fKLMD;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

class GrCCPRCubicHullProcessor : public GrCCPRCubicProcessor {
public:
    GrCCPRCubicHullProcessor(CubicType cubicType)
            : INHERITED(cubicType)
            , fGradMatrix(kMat22f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("grad_matrix", &fGradMatrix, kHigh_GrSLPrecision);
    }

    void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                           const char* wind, const char* rtAdjust) const override;
    void onEmitPerVertexGeometryCode(SkString* fnBody) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

protected:
    GrGLSLGeoToFrag   fGradMatrix;

    typedef GrCCPRCubicProcessor INHERITED;
};

class GrCCPRCubicCornerProcessor : public GrCCPRCubicProcessor {
public:
    GrCCPRCubicCornerProcessor(CubicType cubicType)
            : INHERITED(cubicType)
            , fEdgeDistanceDerivatives("edge_distance_derivatives", kVec2f_GrSLType,
                                        GrShaderVar::kNonArray, kHigh_GrSLPrecision)
            , fdKLMDdx(kVec4f_GrSLType)
            , fdKLMDdy(kVec4f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addFlatVarying("dklmddx", &fdKLMDdx, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("dklmddy", &fdKLMDdy, kHigh_GrSLPrecision);
    }

    void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                           const char* wind, const char* rtAdjust) const override;
    void onEmitPerVertexGeometryCode(SkString* fnBody) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

protected:
    GrShaderVar        fEdgeDistanceDerivatives;
    GrGLSLGeoToFrag    fdKLMDdx;
    GrGLSLGeoToFrag    fdKLMDdy;

    typedef GrCCPRCubicProcessor INHERITED;
};

#endif

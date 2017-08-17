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
 * processor.
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
    enum class Type {
        kSerpentine,
        kLoop
    };

    GrCCPRCubicProcessor(Type type)
            : INHERITED(CoverageType::kShader)
            , fType(type)
            , fInset(kVec3f_GrSLType)
            , fTS(kFloat_GrSLType)
            , fKLMMatrix("klm_matrix", kMat33f_GrSLType, GrShaderVar::kNonArray,
                         kHigh_GrSLPrecision)
            , fKLMDerivatives("klm_derivatives", kVec2f_GrSLType, 3, kHigh_GrSLPrecision) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        varyingHandler->addVarying("insets", &fInset, kHigh_GrSLPrecision);
        varyingHandler->addVarying("ts", &fTS, kHigh_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const TexelBufferHandle& pointsBuffer, const char* atlasOffset,
                            const char* rtAdjust, GrGPArgs*) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* rtAdjust, const char* outputWind) const final;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjust) const final;

protected:
    virtual void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                   const char* wind, const char* rtAdjust) const = 0;

    const Type        fType;
    GrGLSLVertToGeo   fInset;
    GrGLSLVertToGeo   fTS;
    GrShaderVar       fKLMMatrix;
    GrShaderVar       fKLMDerivatives;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

class GrCCPRCubicInsetProcessor : public GrCCPRCubicProcessor {
public:
    GrCCPRCubicInsetProcessor(Type type)
            : INHERITED(type)
            , fKLM(kVec3f_GrSLType)
            , fGradMatrix(kMat22f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("klm", &fKLM, kHigh_GrSLPrecision);
        varyingHandler->addVarying("grad_matrix", &fGradMatrix, kHigh_GrSLPrecision);
    }

    void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                           const char* wind, const char* rtAdjust) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

protected:
    GrGLSLGeoToFrag   fKLM;
    GrGLSLGeoToFrag   fGradMatrix;

    typedef GrCCPRCubicProcessor INHERITED;
};

class GrCCPRCubicBorderProcessor : public GrCCPRCubicProcessor {
public:
    GrCCPRCubicBorderProcessor(Type type)
            : INHERITED(type)
            , fEdgeDistanceEquation("edge_distance_equation", kVec3f_GrSLType,
                                    GrShaderVar::kNonArray, kHigh_GrSLPrecision)
            , fEdgeDistanceDerivatives("edge_distance_derivatives", kVec2f_GrSLType,
                                        GrShaderVar::kNonArray, kHigh_GrSLPrecision)
            , fEdgeSpaceTransform("edge_space_transform", kVec4f_GrSLType, GrShaderVar::kNonArray,
                                  kHigh_GrSLPrecision)
            , fKLMD(kVec4f_GrSLType)
            , fdKLMDdx(kVec4f_GrSLType)
            , fdKLMDdy(kVec4f_GrSLType)
            , fEdgeSpaceCoord(kVec2f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("klmd", &fKLMD, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("dklmddx", &fdKLMDdx, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("dklmddy", &fdKLMDdy, kHigh_GrSLPrecision);
        varyingHandler->addVarying("edge_space_coord", &fEdgeSpaceCoord, kHigh_GrSLPrecision);
    }

    void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                           const char* wind, const char* rtAdjust) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

protected:
    GrShaderVar        fEdgeDistanceEquation;
    GrShaderVar        fEdgeDistanceDerivatives;
    GrShaderVar        fEdgeSpaceTransform;
    GrGLSLGeoToFrag    fKLMD;
    GrGLSLGeoToFrag    fdKLMDdx;
    GrGLSLGeoToFrag    fdKLMDdy;
    GrGLSLGeoToFrag    fEdgeSpaceCoord;

    typedef GrCCPRCubicProcessor INHERITED;
};

#endif

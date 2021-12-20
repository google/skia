/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrStrokeTessellationShader.h"

#include "src/gpu/KeyBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/StrokeTessellator.h"

GrStrokeTessellationShader::GrStrokeTessellationShader(const GrShaderCaps& shaderCaps,
                                                       Mode mode,
                                                       PatchAttribs attribs,
                                                       const SkMatrix& viewMatrix,
                                                       const SkStrokeRec& stroke,
                                                       SkPMColor4f color,
                                                       int8_t maxParametricSegments_log2)
        : GrTessellationShader(kTessellate_GrStrokeTessellationShader_ClassID,
                               (mode == Mode::kHardwareTessellation)
                                       ? GrPrimitiveType::kPatches
                                       : GrPrimitiveType::kTriangleStrip,
                               (mode == Mode::kHardwareTessellation) ? 1 : 0, viewMatrix, color)
        , fMode(mode)
        , fPatchAttribs(attribs)
        , fStroke(stroke)
        , fMaxParametricSegments_log2(maxParametricSegments_log2) {
    // We should use explicit curve type when, and only when, there isn't infinity support.
    // Otherwise the GPU can infer curve type based on infinity.
    SkASSERT(shaderCaps.infinitySupport() != (attribs & PatchAttribs::kExplicitCurveType));
    if (fMode == Mode::kHardwareTessellation) {
        // Explicit curve type is not implemented for tessellation shaders.
        SkASSERT(!(attribs & PatchAttribs::kExplicitCurveType));
    }
    if (fMode == Mode::kHardwareTessellation) {
        // A join calculates its starting angle using prevCtrlPtAttr.
        fAttribs.emplace_back("prevCtrlPtAttr", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
        // with w=p3.x.
        //
        // If p0 == prevCtrlPtAttr, then no join is emitted.
        //
        // pts=[p0, p3, p3, p3] is a reserved pattern that means this patch is a join only,
        // whose start and end tangents are (p0 - inputPrevCtrlPt) and (p3 - p0).
        //
        // pts=[p0, p0, p0, p3] is a reserved pattern that means this patch is a "bowtie", or
        // double-sided round join, anchored on p0 and rotating from (p0 - prevCtrlPtAttr) to
        // (p3 - p0).
        fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
    } else {
        // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
        // with w=p3.x.
        //
        // An empty stroke (p0==p1==p2==p3) is a special case that denotes a circle, or
        // 180-degree point stroke.
        fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fMode == Mode::kLog2Indirect) {
            // argsAttr.xy contains the lastControlPoint for setting up the join.
            //
            // "argsAttr.z=numTotalEdges" tells the shader the literal number of edges in the
            // triangle strip being rendered (i.e., it should be vertexCount/2). If
            // numTotalEdges is negative and the join type is "kRound", it also instructs the
            // shader to only allocate one segment the preceding round join.
            fAttribs.emplace_back("argsAttr", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
        } else {
            SkASSERT(fMode == Mode::kFixedCount);
            // argsAttr contains the lastControlPoint for setting up the join.
            fAttribs.emplace_back("argsAttr", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
    }
    if (fPatchAttribs & PatchAttribs::kStrokeParams) {
        fAttribs.emplace_back("dynamicStrokeAttr", kFloat2_GrVertexAttribType,
                              kFloat2_GrSLType);
    }
    if (fPatchAttribs & PatchAttribs::kColor) {
        fAttribs.emplace_back("dynamicColorAttr",
                              (fPatchAttribs & PatchAttribs::kWideColorIfEnabled)
                                      ? kFloat4_GrVertexAttribType
                                      : kUByte4_norm_GrVertexAttribType,
                              kHalf4_GrSLType);
    }
    if (fPatchAttribs & PatchAttribs::kExplicitCurveType) {
        // A conic curve is written out with p3=[w,Infinity], but GPUs that don't support
        // infinity can't detect this. On these platforms we write out an extra float with each
        // patch that explicitly tells the shader what type of curve it is.
        fAttribs.emplace_back("curveTypeAttr", kFloat_GrVertexAttribType, kFloat_GrSLType);
    }
    if (fMode == Mode::kHardwareTessellation) {
        this->setVertexAttributesWithImplicitOffsets(fAttribs.data(), fAttribs.count());
        SkASSERT(this->vertexStride() == sizeof(SkPoint) * 5 + PatchAttribsStride(fPatchAttribs));
    } else {
        this->setInstanceAttributesWithImplicitOffsets(fAttribs.data(), fAttribs.count());
        SkASSERT(this->instanceStride() == sizeof(SkPoint) * 5 + PatchAttribsStride(fPatchAttribs));
        if (!shaderCaps.vertexIDSupport()) {
            constexpr static Attribute kVertexAttrib("edgeID", kFloat_GrVertexAttribType,
                                                     kFloat_GrSLType);
            this->setVertexAttributesWithImplicitOffsets(&kVertexAttrib, 1);
        }
    }
    SkASSERT(fAttribs.count() <= kMaxAttribCount);
}

const char* GrStrokeTessellationShader::Impl::kCosineBetweenVectorsFn = R"(
float cosine_between_vectors(float2 a, float2 b) {
    // FIXME(crbug.com/800804,skbug.com/11268): This can overflow if we don't normalize exponents.
    float ab_cosTheta = dot(a,b);
    float ab_pow2 = dot(a,a) * dot(b,b);
    return (ab_pow2 == 0.0) ? 1.0 : clamp(ab_cosTheta * inversesqrt(ab_pow2), -1.0, 1.0);
})";

// Extends the middle radius to either the miter point, or the bevel edge if we surpassed the miter
// limit and need to revert to a bevel join.
const char* GrStrokeTessellationShader::Impl::kMiterExtentFn = R"(
float miter_extent(float cosTheta, float miterLimit) {
    float x = fma(cosTheta, .5, .5);
    return (x * miterLimit * miterLimit >= 1.0) ? inversesqrt(x) : sqrt(x);
})";

// Returns the number of radial segments required for each radian of rotation, in order for the
// curve to appear "smooth" as defined by the parametricPrecision.
const char* GrStrokeTessellationShader::Impl::kNumRadialSegmentsPerRadianFn = R"(
float num_radial_segments_per_radian(float parametricPrecision, float strokeRadius) {
    return .5 / acos(max(1.0 - 1.0/(parametricPrecision * strokeRadius), -1.0));
})";

// Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
// precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// We override this result anyway when t==1 so it shouldn't be a problem.
const char* GrStrokeTessellationShader::Impl::kUncheckedMixFn = R"(
float unchecked_mix(float a, float b, float T) {
    return fma(b - a, T, a);
}
float2 unchecked_mix(float2 a, float2 b, float T) {
    return fma(b - a, float2(T), a);
}
float4 unchecked_mix(float4 a, float4 b, float4 T) {
    return fma(b - a, T, a);
})";

void GrStrokeTessellationShader::Impl::emitTessellationCode(
        const GrStrokeTessellationShader& shader, SkString* code, GrGPArgs* gpArgs,
        const GrShaderCaps& shaderCaps) const {
    // The subclass is responsible to define the following symbols before calling this method:
    //
    //     // Functions.
    //     float2 unchecked_mix(float2, float2, float);
    //     float unchecked_mix(float, float, float);
    //
    //     // Values provided by either uniforms or attribs.
    //     float2 p0, p1, p2, p3;
    //     float w;
    //     float STROKE_RADIUS;
    //     float 2x2 AFFINE_MATRIX;
    //     float2 TRANSLATE;
    //
    //     // Values calculated by the specific subclass.
    //     float combinedEdgeID;
    //     bool isFinalEdge;
    //     float numParametricSegments;
    //     float radsPerSegment;
    //     float2 tan0;
    //     float2 tan1;
    //     float strokeOutset;
    //
    code->appendf(R"(
    float2 tangent, strokeCoord;
    if (combinedEdgeID != 0 && !isFinalEdge) {
        // Compute the location and tangent direction of the stroke edge with the integral id
        // "combinedEdgeID", where combinedEdgeID is the sorted-order index of parametric and radial
        // edges. Start by finding the tangent function's power basis coefficients. These define a
        // tangent direction (scaled by some uniform value) as:
        //                                                 |T^2|
        //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
        //                                    |.   .  .|   |1  |
        float2 A, B, C = p1 - p0;
        float2 D = p3 - p0;
        if (w >= 0.0) {
            // P0..P2 represent a conic and P3==P2. The derivative of a conic has a cumbersome
            // order-4 denominator. However, this isn't necessary if we are only interested in a
            // vector in the same *direction* as a given tangent line. Since the denominator scales
            // dx and dy uniformly, we can throw it out completely after evaluating the derivative
            // with the standard quotient rule. This leaves us with a simpler quadratic function
            // that we use to find a tangent.
            C *= w;
            B = .5*D - C;
            A = (w - 1.0) * D;
            p1 *= w;
        } else {
            float2 E = p2 - p1;
            B = E - C;
            A = fma(float2(-3), E, D);
        }
        // FIXME(crbug.com/800804,skbug.com/11268): Consider normalizing the exponents in A,B,C at
        // this point in order to prevent fp32 overflow.

        // Now find the coefficients that give a tangent direction from a parametric edge ID:
        //
        //                                                                 |parametricEdgeID^2|
        //     Tangent_Direction(parametricEdgeID) = dx,dy = |A  B_  C_| * |parametricEdgeID  |
        //                                                   |.   .   .|   |1                 |
        //
        float2 B_ = B * (numParametricSegments * 2.0);
        float2 C_ = C * (numParametricSegments * numParametricSegments);

        // Run a binary search to determine the highest parametric edge that is located on or before
        // the combinedEdgeID. A combined ID is determined by the sum of complete parametric and
        // radial segments behind it. i.e., find the highest parametric edge where:
        //
        //    parametricEdgeID + floor(numRadialSegmentsAtParametricT) <= combinedEdgeID
        //
        float lastParametricEdgeID = 0.0;
        float maxParametricEdgeID = min(numParametricSegments - 1.0, combinedEdgeID);
        // FIXME(crbug.com/800804,skbug.com/11268): This normalize() can overflow.
        float2 tan0norm = normalize(tan0);
        float negAbsRadsPerSegment = -abs(radsPerSegment);
        float maxRotation0 = (1.0 + combinedEdgeID) * abs(radsPerSegment);
        for (int exp = %i - 1; exp >= 0; --exp) {
            // Test the parametric edge at lastParametricEdgeID + 2^exp.
            float testParametricID = lastParametricEdgeID + exp2(float(exp));
            if (testParametricID <= maxParametricEdgeID) {
                float2 testTan = fma(float2(testParametricID), A, B_);
                testTan = fma(float2(testParametricID), testTan, C_);
                float cosRotation = dot(normalize(testTan), tan0norm);
                float maxRotation = fma(testParametricID, negAbsRadsPerSegment, maxRotation0);
                maxRotation = min(maxRotation, PI);
                // Is rotation <= maxRotation? (i.e., is the number of complete radial segments
                // behind testT, + testParametricID <= combinedEdgeID?)
                if (cosRotation >= cos(maxRotation)) {
                    // testParametricID is on or before the combinedEdgeID. Keep it!
                    lastParametricEdgeID = testParametricID;
                }
            }
        }

        // Find the T value of the parametric edge at lastParametricEdgeID.
        float parametricT = lastParametricEdgeID / numParametricSegments;

        // Now that we've identified the highest parametric edge on or before the
        // combinedEdgeID, the highest radial edge is easy:
        float lastRadialEdgeID = combinedEdgeID - lastParametricEdgeID;

        // Find the angle of tan0, or the angle between tan0norm and the positive x axis.
        float angle0 = acos(clamp(tan0norm.x, -1.0, 1.0));
        angle0 = tan0norm.y >= 0.0 ? angle0 : -angle0;

        // Find the tangent vector on the edge at lastRadialEdgeID.
        float radialAngle = fma(lastRadialEdgeID, radsPerSegment, angle0);
        tangent = float2(cos(radialAngle), sin(radialAngle));
        float2 norm = float2(-tangent.y, tangent.x);

        // Find the T value where the tangent is orthogonal to norm. This is a quadratic:
        //
        //     dot(norm, Tangent_Direction(T)) == 0
        //
        //                         |T^2|
        //     norm * |A  2B  C| * |T  | == 0
        //            |.   .  .|   |1  |
        //
        float a=dot(norm,A), b_over_2=dot(norm,B), c=dot(norm,C);
        float discr_over_4 = max(b_over_2*b_over_2 - a*c, 0.0);
        float q = sqrt(discr_over_4);
        if (b_over_2 > 0.0) {
            q = -q;
        }
        q -= b_over_2;

        // Roots are q/a and c/q. Since each curve section does not inflect or rotate more than 180
        // degrees, there can only be one tangent orthogonal to "norm" inside 0..1. Pick the root
        // nearest .5.
        float _5qa = -.5*q*a;
        float2 root = (abs(fma(q,q,_5qa)) < abs(fma(a,c,_5qa))) ? float2(q,a) : float2(c,q);
        float radialT = (root.t != 0.0) ? root.s / root.t : 0.0;
        radialT = clamp(radialT, 0.0, 1.0);

        if (lastRadialEdgeID == 0.0) {
            // The root finder above can become unstable when lastRadialEdgeID == 0 (e.g., if
            // there are roots at exatly 0 and 1 both). radialT should always == 0 in this case.
            radialT = 0.0;
        }

        // Now that we've identified the T values of the last parametric and radial edges, our final
        // T value for combinedEdgeID is whichever is larger.
        float T = max(parametricT, radialT);

        // Evaluate the cubic at T. Use De Casteljau's for its accuracy and stability.
        float2 ab = unchecked_mix(p0, p1, T);
        float2 bc = unchecked_mix(p1, p2, T);
        float2 cd = unchecked_mix(p2, p3, T);
        float2 abc = unchecked_mix(ab, bc, T);
        float2 bcd = unchecked_mix(bc, cd, T);
        float2 abcd = unchecked_mix(abc, bcd, T);

        // Evaluate the conic weight at T.
        float u = unchecked_mix(1.0, w, T);
        float v = w + 1 - u;  // == mix(w, 1, T)
        float uv = unchecked_mix(u, v, T);

        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        if (T != radialT) {
            tangent = (w >= 0.0) ? bc*u - ab*v : bcd - abc;
        }

        strokeCoord = (w >= 0.0) ? abc/uv : abcd;
    } else {
        // Edges at the beginning and end of the strip use exact endpoints and tangents. This
        // ensures crack-free seaming between instances.
        tangent = (combinedEdgeID == 0) ? tan0 : tan1;
        strokeCoord = (combinedEdgeID == 0) ? p0 : p3;
    })", shader.maxParametricSegments_log2() /* Parametric/radial sort loop count. */);

    code->append(R"(
    // FIXME(crbug.com/800804,skbug.com/11268): This normalize() can overflow.
    float2 ortho = normalize(float2(tangent.y, -tangent.x));
    strokeCoord += ortho * (STROKE_RADIUS * strokeOutset);)");

    if (!shader.stroke().isHairlineStyle()) {
        // Normal case. Do the transform after tessellation.
        code->append(R"(
        float2 devCoord = AFFINE_MATRIX * strokeCoord + TRANSLATE;)");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
        gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "strokeCoord");
    } else {
        // Hairline case. The scale and skew already happened before tessellation.
        code->append(R"(
        float2 devCoord = strokeCoord + TRANSLATE;
        float2 localCoord = inverse(AFFINE_MATRIX) * strokeCoord;)");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
        gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localCoord");
    }
}

void GrStrokeTessellationShader::Impl::emitFragmentCode(const GrStrokeTessellationShader& shader,
                                                        const EmitArgs& args) {
    if (!shader.hasDynamicColor()) {
        // The fragment shader just outputs a uniform color.
        const char* colorUniformName;
        fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                         kHalf4_GrSLType, "color",
                                                         &colorUniformName);
        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, colorUniformName);
    } else {
        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor,
                                       fDynamicColorName.c_str());
    }
    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
}

void GrStrokeTessellationShader::Impl::setData(const GrGLSLProgramDataManager& pdman,
                                               const GrShaderCaps&,
                                               const GrGeometryProcessor& geomProc) {
    const auto& shader = geomProc.cast<GrStrokeTessellationShader>();
    const auto& stroke = shader.stroke();

    if (!shader.hasDynamicStroke()) {
        // Set up the tessellation control uniforms.
        skgpu::StrokeTolerances tolerances;
        if (!stroke.isHairlineStyle()) {
            tolerances = skgpu::StrokeTolerances::MakeNonHairline(shader.viewMatrix().getMaxScale(),
                                                                  stroke.getWidth());
        } else {
            // In the hairline case we transform prior to tessellation. Set up tolerances for an
            // identity viewMatrix and a strokeWidth of 1.
            tolerances = skgpu::StrokeTolerances::MakeNonHairline(1, 1);
        }
        float strokeRadius = (stroke.isHairlineStyle()) ? .5f : stroke.getWidth() * .5;
        pdman.set4f(fTessControlArgsUniform,
                    tolerances.fParametricPrecision,  // PARAMETRIC_PRECISION
                    tolerances.fNumRadialSegmentsPerRadian,  // NUM_RADIAL_SEGMENTS_PER_RADIAN
                    skgpu::GetJoinType(stroke),  // JOIN_TYPE
                    strokeRadius);  // STROKE_RADIUS
    } else {
        SkASSERT(!stroke.isHairlineStyle());
        float maxScale = shader.viewMatrix().getMaxScale();
        pdman.set1f(fTessControlArgsUniform,
                    skgpu::StrokeTolerances::CalcParametricPrecision(maxScale));
    }

    if (shader.mode() == GrStrokeTessellationShader::Mode::kFixedCount) {
        SkASSERT(shader.fixedCountNumTotalEdges() != 0);
        pdman.set1f(fEdgeCountUniform, (float)shader.fixedCountNumTotalEdges());
    }

    // Set up the view matrix, if any.
    const SkMatrix& m = shader.viewMatrix();
    pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
    pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(),
                m.getScaleY());

    if (!shader.hasDynamicColor()) {
        pdman.set4fv(fColorUniform, 1, shader.color().vec());
    }
}

void GrStrokeTessellationShader::addToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const {
    bool keyNeedsJoin = (fMode != Mode::kHardwareTessellation) &&
                        !(fPatchAttribs & PatchAttribs::kStrokeParams);
    SkASSERT((int)fMode >> 2 == 0);
    SkASSERT(fStroke.getJoin() >> 2 == 0);
    // Attribs get worked into the key automatically during GrGeometryProcessor::getAttributeKey().
    // When color is in a uniform, it's always wide. kWideColor doesn't need to be considered here.
    uint32_t key = (uint32_t)(fPatchAttribs & ~PatchAttribs::kColor);
    key = (key << 2) | (uint32_t)fMode;
    key = (key << 2) | ((keyNeedsJoin) ? fStroke.getJoin() : 0);
    key = (key << 1) | (uint32_t)fStroke.isHairlineStyle();
    key = (key << 8) | fMaxParametricSegments_log2;
    b->add32(key);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrStrokeTessellationShader::makeProgramImpl(
        const GrShaderCaps&) const {
    switch (fMode) {
        case Mode::kHardwareTessellation:
            return std::make_unique<HardwareImpl>();
        case Mode::kLog2Indirect:
        case Mode::kFixedCount:
            return std::make_unique<InstancedImpl>();
    }
    SkUNREACHABLE;
}

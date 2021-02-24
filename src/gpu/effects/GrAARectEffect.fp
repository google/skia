/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;
layout(key) in GrClipEdgeType edgeType;
layout(ctype=SkRect) in float4 rect;
layout(ctype=SkRect) float4 prevRect = float4(-1);
uniform float4 rectUniform;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
     kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

half4 main() {
    half coverage;
    @switch (edgeType) {
        case GrClipEdgeType::kFillBW: // fall through
        case GrClipEdgeType::kInverseFillBW:
            // non-AA
            coverage = all(greaterThan(float4(sk_FragCoord.xy, rectUniform.zw),
                                       float4(rectUniform.xy, sk_FragCoord.xy))) ? 1 : 0;
            break;
        default:
            // compute coverage relative to left and right edges, add, then subtract 1 to account
            // for double counting. And similar for top/bottom.
            half4 dists4 = clamp(half4(1, 1, -1, -1) *
                                 half4(sk_FragCoord.xyxy - rectUniform), 0, 1);
            half2 dists2 = dists4.xy + dists4.zw - 1;
            coverage = dists2.x * dists2.y;
    }

    @if (edgeType == GrClipEdgeType::kInverseFillBW || edgeType == GrClipEdgeType::kInverseFillAA) {
        coverage = 1.0 - coverage;
    }
    return sample(inputFP) * coverage;
}

@setData(pdman) {
    SkASSERT(rect.isSorted());
    // The AA math in the shader evaluates to 0 at the uploaded coordinates, so outset by 0.5
    // to interpolate from 0 at a half pixel inset and 1 at a half pixel outset of rect.
    const SkRect& newRect = GrProcessorEdgeTypeIsAA(edgeType) ?
                            rect.makeOutset(.5f, .5f) : rect;
    if (newRect != prevRect) {
        pdman.set4f(rectUniform, newRect.fLeft, newRect.fTop, newRect.fRight, newRect.fBottom);
        prevRect = newRect;
    }
}

@test(d) {
    SkRect rect = SkRect::MakeLTRB(d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1());
    rect.sort();
    GrClipEdgeType edgeType = static_cast<GrClipEdgeType>(
            d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));

    return GrAARectEffect::Make(d->inputFP(), edgeType, rect);
}

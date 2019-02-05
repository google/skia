/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(key) in GrClipEdgeType edgeType;
layout(ctype=SkRect) in float4 rect;
layout(ctype=SkRect) float4 prevRect = float4(-1);
uniform float4 rectUniform;

@optimizationFlags { kCompatibleWithCoverageAsAlpha_OptimizationFlag }

void main() {
    half alpha;
    @switch (edgeType) {
        case GrClipEdgeType::kFillBW: // fall through
        case GrClipEdgeType::kInverseFillBW:
            // non-AA
            alpha = all(greaterThan(float4(sk_FragCoord.xy, rectUniform.zw),
                                    float4(rectUniform.xy, sk_FragCoord.xy))) ? 1 : 0;
            break;
        default:
            // The amount of coverage removed in x and y by the edges is computed as a pair of
            // negative numbers, xSub and ySub.
            half xSub, ySub;
            xSub = min(half(sk_FragCoord.x - rectUniform.x), 0.0);
            xSub += min(half(rectUniform.z - sk_FragCoord.x), 0.0);
            ySub = min(half(sk_FragCoord.y - rectUniform.y), 0.0);
            ySub += min(half(rectUniform.w - sk_FragCoord.y), 0.0);
            // Now compute coverage in x and y and multiply them to get the fraction of the pixel
            // covered.
            alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));
    }

    @if (edgeType == GrClipEdgeType::kInverseFillBW || edgeType == GrClipEdgeType::kInverseFillAA) {
        alpha = 1.0 - alpha;
    }
    sk_OutColor = sk_InColor * alpha;
}

@setData(pdman) {
    const SkRect& newRect = GrProcessorEdgeTypeIsAA(edgeType) ?
                            rect.makeInset(.5f, .5f) : rect;
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
    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        GrClipEdgeType edgeType = static_cast<GrClipEdgeType>(
                d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));

        fp = GrAARectEffect::Make(edgeType, rect);
    } while (nullptr == fp);
    return fp;
}

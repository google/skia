const int kMaxEdges = 8;
layout(key) in GrClipEdgeType edgeType;
layout(key) in int edgeCount;
in half inEdges[kMaxEdges * 3];
uniform half3 edges[edgeCount];
half3 prevEdges[kMaxEdges * 3];

@header {
    #include "GrAARectEffect.h"
    #include "GrConstColorProcessor.h"
    #include "SkPathPriv.h"
}

@optimizationFlags {
    kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

@class {
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType type, const SkPath& path) {
        if (GrClipEdgeType::kHairlineAA == type) {
            return nullptr;
        }
        if (path.getSegmentMasks() != SkPath::kLine_SegmentMask || !path.isConvex()) {
            return nullptr;
        }

        SkPathPriv::FirstDirection dir;
        // The only way this should fail is if the clip is effectively an infinitely thin line. In
        // that case nothing is inside the clip. It'd be nice to detect this at a higher level and
        // either skip the draw or omit the clip element.
        if (!SkPathPriv::CheapComputeFirstDirection(path, &dir)) {
            if (GrProcessorEdgeTypeIsInverseFill(type)) {
                return GrConstColorProcessor::Make(GrColor4f::OpaqueWhite(),
                                                   GrConstColorProcessor::InputMode::kModulateRGBA);
            }
            // This could use kIgnore instead of kModulateRGBA but it would trigger a debug print
            // about a coverage processor not being compatible with the alpha-as-coverage
            // optimization. We don't really care about this unlikely case so we just use
            // kModulateRGBA to suppress the print.
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::InputMode::kModulateRGBA);
        }

        SkScalar        edges[3 * kMaxEdges];
        SkPoint         pts[4];
        SkPath::Verb    verb;
        SkPath::Iter    iter(path, true);

        // SkPath considers itself convex so long as there is a convex contour within it,
        // regardless of any degenerate contours such as a string of moveTos before it.
        // Iterate here to consume any degenerate contours and only process the points
        // on the actual convex contour.
        int n = 0;
        while ((verb = iter.next(pts, true, true)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    SkASSERT(n == 0);
                case SkPath::kClose_Verb:
                    break;
                case SkPath::kLine_Verb: {
                    if (n >= kMaxEdges) {
                        return nullptr;
                    }
                    SkVector v = pts[1] - pts[0];
                    v.normalize();
                    if (SkPathPriv::kCCW_FirstDirection == dir) {
                        edges[3 * n] = v.fY;
                        edges[3 * n + 1] = -v.fX;
                    } else {
                        edges[3 * n] = -v.fY;
                        edges[3 * n + 1] = v.fX;
                    }
                    // Outset the edges by 0.5 so that a pixel with center on an edge is 50% covered
                    // in the AA case and 100% covered in the non-AA case.
                    edges[3 * n + 2] = -(edges[3 * n] * pts[1].fX + edges[3 * n + 1] * pts[1].fY) +
                                     SK_ScalarHalf;
                    ++n;
                    break;
                }
                default:
                    return nullptr;
            }
        }

        if (path.isInverseFillType()) {
            type = GrInvertProcessorEdgeType(type);
        }
        return Make(type, n, edges);
    }

    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType edgeType, const SkRect& rect) {
        if (GrClipEdgeType::kHairlineAA == edgeType){
            return nullptr;
        }
        return GrAARectEffect::Make(edgeType, rect);
    }
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType edgeType, int edgeCount,
                                                     SkScalar* inEdges) {
        SkScalar edges[kMaxEdges * 3];
        size_t size = edgeCount * 3 * sizeof(SkScalar);
        memcpy(edges, inEdges, size);
        memset(size + (char*) edges, 0, sizeof(edges) - size);
        return std::unique_ptr<GrFragmentProcessor>(new GrConvexPolyEffect(edgeType, edgeCount,
                                                                           edges));
    }
}

@setData(pdman) {
    size_t byteSize = 3 * edgeCount * sizeof(SkScalar);
    if (memcmp(prevEdges, inEdges, byteSize)) {
        pdman.set3fv(edges, edgeCount, (SkScalar*) inEdges);
        memcpy(prevEdges, inEdges, byteSize);
    }
}

void main() {
/*    half alpha = 1.0;
    half edge;
    for (int i = 0; i < edgeCount; ++i) {
        edge = dot(edges[i], half3(sk_FragCoord.x, sk_FragCoord.y, 1));
        @switch (edgeType) {
            case GrClipEdgeType::kFillBW: // fall through
            case GrClipEdgeType::kInverseFillBW:
                // non-AA
                edge = edge >= 0.5 ? 1.0 : 0.0;
                break;
            default:
                // AA
                edge = clamp(edge, 0.0, 1.0);
        }
        alpha *= edge;
    }

    @if (edgeType == GrClipEdgeType::kInverseFillBW || edgeType == GrClipEdgeType::kInverseFillAA) {
        alpha = 1.0 - alpha;
    }
    sk_OutColor = sk_InColor * alpha;*/
    sk_OutColor = float4(1, 0, 0, 1);
}

@test(d) {
    int count = d->fRandom->nextULessThan(kMaxEdges) + 1;
    SkScalar edges[kMaxEdges * 3];
    for (int i = 0; i < 3 * count; ++i) {
        edges[i] = d->fRandom->nextSScalar1();
    }

    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        GrClipEdgeType edgeType = static_cast<GrClipEdgeType>(
                d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));
        fp = GrConvexPolyEffect::Make(edgeType, count, edges);
    } while (nullptr == fp);
    return fp;
}
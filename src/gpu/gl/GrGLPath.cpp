
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPath.h"
#include "GrGLPathRendering.h"
#include "GrGLGpu.h"

namespace {
inline GrGLubyte verb_to_gl_path_cmd(SkPath::Verb verb) {
    static const GrGLubyte gTable[] = {
        GR_GL_MOVE_TO,
        GR_GL_LINE_TO,
        GR_GL_QUADRATIC_CURVE_TO,
        GR_GL_CONIC_CURVE_TO,
        GR_GL_CUBIC_CURVE_TO,
        GR_GL_CLOSE_PATH,
    };
    GR_STATIC_ASSERT(0 == SkPath::kMove_Verb);
    GR_STATIC_ASSERT(1 == SkPath::kLine_Verb);
    GR_STATIC_ASSERT(2 == SkPath::kQuad_Verb);
    GR_STATIC_ASSERT(3 == SkPath::kConic_Verb);
    GR_STATIC_ASSERT(4 == SkPath::kCubic_Verb);
    GR_STATIC_ASSERT(5 == SkPath::kClose_Verb);

    SkASSERT(verb >= 0 && (size_t)verb < SK_ARRAY_COUNT(gTable));
    return gTable[verb];
}

#ifdef SK_DEBUG
inline int num_coords(SkPath::Verb verb) {
    static const int gTable[] = {
        2, // move
        2, // line
        4, // quad
        5, // conic
        6, // cubic
        0, // close
    };
    GR_STATIC_ASSERT(0 == SkPath::kMove_Verb);
    GR_STATIC_ASSERT(1 == SkPath::kLine_Verb);
    GR_STATIC_ASSERT(2 == SkPath::kQuad_Verb);
    GR_STATIC_ASSERT(3 == SkPath::kConic_Verb);
    GR_STATIC_ASSERT(4 == SkPath::kCubic_Verb);
    GR_STATIC_ASSERT(5 == SkPath::kClose_Verb);

    SkASSERT(verb >= 0 && (size_t)verb < SK_ARRAY_COUNT(gTable));
    return gTable[verb];
}
#endif

inline GrGLenum join_to_gl_join(SkPaint::Join join) {
    static GrGLenum gSkJoinsToGrGLJoins[] = {
        GR_GL_MITER_REVERT,
        GR_GL_ROUND,
        GR_GL_BEVEL
    };
    return gSkJoinsToGrGLJoins[join];
    GR_STATIC_ASSERT(0 == SkPaint::kMiter_Join);
    GR_STATIC_ASSERT(1 == SkPaint::kRound_Join);
    GR_STATIC_ASSERT(2 == SkPaint::kBevel_Join);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gSkJoinsToGrGLJoins) == SkPaint::kJoinCount);
}

inline GrGLenum cap_to_gl_cap(SkPaint::Cap cap) {
    static GrGLenum gSkCapsToGrGLCaps[] = {
        GR_GL_FLAT,
        GR_GL_ROUND,
        GR_GL_SQUARE
    };
    return gSkCapsToGrGLCaps[cap];
    GR_STATIC_ASSERT(0 == SkPaint::kButt_Cap);
    GR_STATIC_ASSERT(1 == SkPaint::kRound_Cap);
    GR_STATIC_ASSERT(2 == SkPaint::kSquare_Cap);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gSkCapsToGrGLCaps) == SkPaint::kCapCount);
}

inline void points_to_coords(const SkPoint points[], size_t first_point, size_t amount,
                             GrGLfloat coords[]) {
    for (size_t i = 0;  i < amount; ++i) {
        coords[i * 2] =  SkScalarToFloat(points[first_point + i].fX);
        coords[i * 2 + 1] = SkScalarToFloat(points[first_point + i].fY);
    }
}
}

void GrGLPath::InitPathObject(GrGLGpu* gpu,
                              GrGLuint pathID,
                              const SkPath& skPath,
                              const GrStrokeInfo& stroke) {
    SkASSERT(!stroke.isDashed());
    if (!skPath.isEmpty()) {
        int verbCnt = skPath.countVerbs();
        int pointCnt = skPath.countPoints();
        int minCoordCnt = pointCnt * 2;

        SkSTArray<16, GrGLubyte, true> pathCommands(verbCnt);
        SkSTArray<16, GrGLfloat, true> pathCoords(minCoordCnt);

        SkDEBUGCODE(int numCoords = 0);

        if ((skPath.getSegmentMasks() & SkPath::kConic_SegmentMask) == 0) {
            // This branch does type punning, converting SkPoint* to GrGLfloat*.
            SK_COMPILE_ASSERT(sizeof(SkPoint) == sizeof(GrGLfloat) * 2, sk_point_not_two_floats);
            // This branch does not convert with SkScalarToFloat.
#ifndef SK_SCALAR_IS_FLOAT
#error Need SK_SCALAR_IS_FLOAT.
#endif
            pathCommands.resize_back(verbCnt);
            pathCoords.resize_back(minCoordCnt);
            skPath.getPoints(reinterpret_cast<SkPoint*>(&pathCoords[0]), pointCnt);
            skPath.getVerbs(&pathCommands[0], verbCnt);
            for (int i = 0; i < verbCnt; ++i) {
                SkPath::Verb v = static_cast<SkPath::Verb>(pathCommands[i]);
                pathCommands[i] = verb_to_gl_path_cmd(v);
                SkDEBUGCODE(numCoords += num_coords(v));
            }
        } else {
            SkPoint points[4];
            SkPath::RawIter iter(skPath);
            SkPath::Verb verb;
            while ((verb = iter.next(points)) != SkPath::kDone_Verb) {
                pathCommands.push_back(verb_to_gl_path_cmd(verb));
                GrGLfloat coords[6];
                int coordsForVerb;
                switch (verb) {
                    case SkPath::kMove_Verb:
                        points_to_coords(points, 0, 1, coords);
                        coordsForVerb = 2;
                        break;
                    case SkPath::kLine_Verb:
                        points_to_coords(points, 1, 1, coords);
                        coordsForVerb = 2;
                        break;
                    case SkPath::kConic_Verb:
                        points_to_coords(points, 1, 2, coords);
                        coords[4] = SkScalarToFloat(iter.conicWeight());
                        coordsForVerb = 5;
                        break;
                    case SkPath::kQuad_Verb:
                        points_to_coords(points, 1, 2, coords);
                        coordsForVerb = 4;
                        break;
                    case SkPath::kCubic_Verb:
                        points_to_coords(points, 1, 3, coords);
                        coordsForVerb = 6;
                        break;
                    case SkPath::kClose_Verb:
                        continue;
                    default:
                        SkASSERT(false);  // Not reached.
                        continue;
                }
                SkDEBUGCODE(numCoords += num_coords(verb));
                pathCoords.push_back_n(coordsForVerb, coords);
            }
        }

        SkASSERT(verbCnt == pathCommands.count());
        SkASSERT(numCoords == pathCoords.count());

        GR_GL_CALL(gpu->glInterface(), PathCommands(pathID, pathCommands.count(), &pathCommands[0],
                   pathCoords.count(), GR_GL_FLOAT, &pathCoords[0]));
    } else {
        GR_GL_CALL(gpu->glInterface(), PathCommands(pathID, 0, NULL, 0, GR_GL_FLOAT, NULL));
    }

    if (stroke.needToApply()) {
        SkASSERT(!stroke.isHairlineStyle());
        GR_GL_CALL(gpu->glInterface(),
            PathParameterf(pathID, GR_GL_PATH_STROKE_WIDTH, SkScalarToFloat(stroke.getWidth())));
        GR_GL_CALL(gpu->glInterface(),
            PathParameterf(pathID, GR_GL_PATH_MITER_LIMIT, SkScalarToFloat(stroke.getMiter())));
        GrGLenum join = join_to_gl_join(stroke.getJoin());
        GR_GL_CALL(gpu->glInterface(), PathParameteri(pathID, GR_GL_PATH_JOIN_STYLE, join));
        GrGLenum cap = cap_to_gl_cap(stroke.getCap());
        GR_GL_CALL(gpu->glInterface(), PathParameteri(pathID, GR_GL_PATH_END_CAPS, cap));
        GR_GL_CALL(gpu->glInterface(), PathParameterf(pathID, GR_GL_PATH_STROKE_BOUND, 0.02f));
    }
}

GrGLPath::GrGLPath(GrGLGpu* gpu, const SkPath& origSkPath, const GrStrokeInfo& origStroke)
    : INHERITED(gpu, origSkPath, origStroke),
      fPathID(gpu->glPathRendering()->genPaths(1)) {
    // Convert a dashing to either a stroke or a fill.
    const SkPath* skPath = &origSkPath;
    SkTLazy<SkPath> tmpPath;
    const GrStrokeInfo* stroke = &origStroke;
    GrStrokeInfo tmpStroke(SkStrokeRec::kFill_InitStyle);

    if (stroke->isDashed()) {
        if (stroke->applyDashToPath(tmpPath.init(), &tmpStroke, *skPath)) {
            skPath = tmpPath.get();
            stroke = &tmpStroke;
        }
    }

    InitPathObject(gpu, fPathID, *skPath, *stroke);

    fShouldStroke = stroke->needToApply();
    fShouldFill = stroke->isFillStyle() ||
            stroke->getStyle() == SkStrokeRec::kStrokeAndFill_Style;

    if (fShouldStroke) {
        // FIXME: try to account for stroking, without rasterizing the stroke.
        fBounds.outset(stroke->getWidth(), stroke->getWidth());
    }

    this->registerWithCache();
}

void GrGLPath::onRelease() {
    if (0 != fPathID && this->shouldFreeResources()) {
        static_cast<GrGLGpu*>(this->getGpu())->glPathRendering()->deletePaths(fPathID, 1);
        fPathID = 0;
    }

    INHERITED::onRelease();
}

void GrGLPath::onAbandon() {
    fPathID = 0;

    INHERITED::onAbandon();
}

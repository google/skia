
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPath.h"
#include "GrGLPathRendering.h"
#include "GrGpuGL.h"

namespace {
inline GrGLubyte verb_to_gl_path_cmd(SkPath::Verb verb) {
    static const GrGLubyte gTable[] = {
        GR_GL_MOVE_TO,
        GR_GL_LINE_TO,
        GR_GL_QUADRATIC_CURVE_TO,
        0xFF, // conic
        GR_GL_CUBIC_CURVE_TO,
        GR_GL_CLOSE_PATH,
    };
    GR_STATIC_ASSERT(0 == SkPath::kMove_Verb);
    GR_STATIC_ASSERT(1 == SkPath::kLine_Verb);
    GR_STATIC_ASSERT(2 == SkPath::kQuad_Verb);
    GR_STATIC_ASSERT(4 == SkPath::kCubic_Verb);
    GR_STATIC_ASSERT(5 == SkPath::kClose_Verb);

    SkASSERT(verb >= 0 && (size_t)verb < SK_ARRAY_COUNT(gTable));
    return gTable[verb];
}

#ifdef SK_DEBUG
inline int num_pts(SkPath::Verb verb) {
    static const int gTable[] = {
        1, // move
        1, // line
        2, // quad
        2, // conic
        3, // cubic
        0, // close
    };
    GR_STATIC_ASSERT(0 == SkPath::kMove_Verb);
    GR_STATIC_ASSERT(1 == SkPath::kLine_Verb);
    GR_STATIC_ASSERT(2 == SkPath::kQuad_Verb);
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

}

static const bool kIsWrapped = false; // The constructor creates the GL path object.

void GrGLPath::InitPathObject(GrGpuGL* gpu,
                              GrGLuint pathID,
                              const SkPath& skPath,
                              const SkStrokeRec& stroke) {
    if (!skPath.isEmpty()) {
        SkSTArray<16, GrGLubyte, true> pathCommands;
        SkSTArray<16, SkPoint, true> pathPoints;

        int verbCnt = skPath.countVerbs();
        int pointCnt = skPath.countPoints();
        pathCommands.resize_back(verbCnt);
        pathPoints.resize_back(pointCnt);

        // TODO: Direct access to path points since we could pass them on directly.
        skPath.getPoints(&pathPoints[0], pointCnt);
        skPath.getVerbs(&pathCommands[0], verbCnt);

        SkDEBUGCODE(int numPts = 0);
        for (int i = 0; i < verbCnt; ++i) {
            SkPath::Verb v = static_cast<SkPath::Verb>(pathCommands[i]);
            pathCommands[i] = verb_to_gl_path_cmd(v);
            SkDEBUGCODE(numPts += num_pts(v));
        }
        SkASSERT(pathPoints.count() == numPts);

        GR_GL_CALL(gpu->glInterface(),
                   PathCommands(pathID, verbCnt, &pathCommands[0],
                                2 * pointCnt, GR_GL_FLOAT, &pathPoints[0]));
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
        GR_GL_CALL(gpu->glInterface(),
            PathParameteri(pathID, GR_GL_PATH_JOIN_STYLE, join));
        GrGLenum cap = cap_to_gl_cap(stroke.getCap());
        GR_GL_CALL(gpu->glInterface(),
            PathParameteri(pathID, GR_GL_PATH_INITIAL_END_CAP, cap));
        GR_GL_CALL(gpu->glInterface(),
            PathParameteri(pathID, GR_GL_PATH_TERMINAL_END_CAP, cap));
    }
}

GrGLPath::GrGLPath(GrGpuGL* gpu, const SkPath& path, const SkStrokeRec& stroke)
    : INHERITED(gpu, kIsWrapped, path, stroke),
      fPathID(gpu->glPathRendering()->genPaths(1)) {

    InitPathObject(gpu, fPathID, fSkPath, stroke);

    if (stroke.needToApply()) {
        // FIXME: try to account for stroking, without rasterizing the stroke.
        fBounds.outset(stroke.getWidth(), stroke.getWidth());
    }
    this->registerWithCache();
}

GrGLPath::~GrGLPath() {
    this->release();
}

void GrGLPath::onRelease() {
    if (0 != fPathID && !this->isWrapped()) {
        static_cast<GrGpuGL*>(this->getGpu())->glPathRendering()->deletePaths(fPathID, 1);
        fPathID = 0;
    }

    INHERITED::onRelease();
}

void GrGLPath::onAbandon() {
    fPathID = 0;

    INHERITED::onAbandon();
}

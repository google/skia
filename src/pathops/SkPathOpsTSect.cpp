/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsTSect.h"

#if PATH_OP_COMPILE_FOR_SIZE

int SkIntersections::intersect(const SkDQuad& q1, const SkDQuad& q2) {
    SkTQuad quad1(q1);
    SkTQuad quad2(q2);
    SkTSect<SkTCurve, SkTCurve> sect1(quad1
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkTCurve, SkTCurve> sect2(quad2
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkTCurve, SkTCurve>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDConic& c, const SkDQuad& q) {
    SkTConic conic(c);
    SkTQuad quad(q);
    SkTSect<SkTCurve, SkTCurve> sect1(conic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkTCurve, SkTCurve> sect2(quad
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkTCurve, SkTCurve>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDConic& c1, const SkDConic& c2) {
    SkTConic conic1(c1);
    SkTConic conic2(c2);
    SkTSect<SkTCurve, SkTCurve> sect1(conic1
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkTCurve, SkTCurve> sect2(conic2
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkTCurve, SkTCurve>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDCubic& c, const SkDQuad& q) {
    SkTCubic cubic(c);
    SkTQuad quad(q);
    SkTSect<SkTCurve, SkTCurve> sect1(cubic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkTCurve, SkTCurve> sect2(quad
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkTCurve, SkTCurve>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDCubic& cu, const SkDConic& co) {
    SkTCubic cubic(cu);
    SkTConic conic(co);
    SkTSect<SkTCurve, SkTCurve> sect1(cubic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkTCurve, SkTCurve> sect2(conic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkTCurve, SkTCurve>::BinarySearch(&sect1, &sect2, this);
    return used();

}

int SkIntersections::intersect(const SkDCubic& c1, const SkDCubic& c2) {
    SkTCubic cubic1(c1);
    SkTCubic cubic2(c2);
    SkTSect<SkTCurve, SkTCurve> sect1(cubic1
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkTCurve, SkTCurve> sect2(cubic2
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkTCurve, SkTCurve>::BinarySearch(&sect1, &sect2, this);
    return used();
}

#else

int SkIntersections::intersect(const SkDQuad& quad1, const SkDQuad& quad2) {
    SkTSect<SkDQuad, SkDQuad> sect1(quad1
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDQuad> sect2(quad2
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkDQuad, SkDQuad>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDConic& conic, const SkDQuad& quad) {
    SkTSect<SkDConic, SkDQuad> sect1(conic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDConic> sect2(quad
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkDConic, SkDQuad>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDConic& conic1, const SkDConic& conic2) {
    SkTSect<SkDConic, SkDConic> sect1(conic1
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDConic> sect2(conic2
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkDConic, SkDConic>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDCubic& cubic, const SkDQuad& quad) {
    SkTSect<SkDCubic, SkDQuad> sect1(cubic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDQuad, SkDCubic> sect2(quad
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkDCubic, SkDQuad>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDCubic& cubic, const SkDConic& conic) {
    SkTSect<SkDCubic, SkDConic> sect1(cubic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDConic, SkDCubic> sect2(conic
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkDCubic, SkDConic>::BinarySearch(&sect1, &sect2, this);
    return used();
}

int SkIntersections::intersect(const SkDCubic& cubic1, const SkDCubic& cubic2) {
    SkTSect<SkDCubic, SkDCubic> sect1(cubic1
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(1));
    SkTSect<SkDCubic, SkDCubic> sect2(cubic2
        SkDEBUGPARAMS(globalState())  PATH_OPS_DEBUG_T_SECT_PARAMS(2));
    SkTSect<SkDCubic, SkDCubic>::BinarySearch(&sect1, &sect2, this);
    return used();
}

#endif


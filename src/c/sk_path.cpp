/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkParsePath.h"

#include "include/c/sk_path.h"

#include "src/c/sk_types_priv.h"

void sk_path_rmove_to(sk_path_t* cpath, float dx, float dy) {
    AsPath(cpath)->rMoveTo(dx, dy);
}

void sk_path_rline_to(sk_path_t* cpath, float dx, float dy) {
    AsPath(cpath)->rLineTo(dx, dy);
}

void sk_path_rquad_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1) {
    AsPath(cpath)->rQuadTo(dx0, dy0, dx1, dy1);
}

void sk_path_rconic_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1, float w) {
    AsPath(cpath)->rConicTo(dx0, dy0, dx1, dy1, w);
}

void sk_path_rcubic_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1, float dx2, float dy2) {
    AsPath(cpath)->rCubicTo(dx0, dy0, dx1, dy1, dx2, dy2);
}

void sk_path_add_rect_start(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir, uint32_t startIndex) {
    AsPath(cpath)->addRect(*AsRect(crect), (SkPath::Direction)cdir, startIndex);
}

void sk_path_add_arc(sk_path_t* cpath, const sk_rect_t* crect, float startAngle, float sweepAngle) {
    AsPath(cpath)->addArc(*AsRect(crect), startAngle, sweepAngle);
}

void sk_path_set_filltype(sk_path_t* cpath, sk_path_filltype_t cfilltype) {
    AsPath(cpath)->setFillType((SkPath::FillType)cfilltype);
}

sk_path_filltype_t sk_path_get_filltype(sk_path_t *cpath) {
    return (sk_path_filltype_t)AsPath(cpath)->getFillType();
}

void sk_path_transform(sk_path_t* cpath, const sk_matrix_t* cmatrix) {
    return AsPath(cpath)->transform(AsMatrix(cmatrix));
}

sk_path_t* sk_path_clone(const sk_path_t* cpath) {
    return ToPath(new SkPath(*AsPath(cpath)));
}

void sk_path_rewind (sk_path_t* cpath) {
    AsPath (cpath)->rewind ();
}

void sk_path_reset (sk_path_t* cpath) {
    AsPath (cpath)->reset ();
}

sk_path_iterator_t* sk_path_create_iter (sk_path_t *cpath, int forceClose) {
    return ToPathIter(new SkPath::Iter(*AsPath(cpath), forceClose));
}

sk_path_verb_t sk_path_iter_next (sk_path_iterator_t *iterator, sk_point_t points [4], int doConsumeDegenerates, int exact) {
    return (sk_path_verb_t)AsPathIter(iterator)->next(AsPoint(points), doConsumeDegenerates, exact);
}

float sk_path_iter_conic_weight (sk_path_iterator_t *iterator) {
    return AsPathIter(iterator)->conicWeight ();
}

int sk_path_iter_is_close_line (sk_path_iterator_t *iterator) {
    return AsPathIter(iterator)->isCloseLine ();
}

int sk_path_iter_is_closed_contour (sk_path_iterator_t *iterator) {
    return AsPathIter(iterator)->isClosedContour ();
}

void sk_path_iter_destroy (sk_path_iterator_t *iterator) {
    delete AsPathIter (iterator);
}

sk_path_rawiterator_t* sk_path_create_rawiter (sk_path_t *cpath) {
    return ToPathRawIter(new SkPath::RawIter(*AsPath(cpath)));
}

sk_path_verb_t sk_path_rawiter_next (sk_path_rawiterator_t *iterator, sk_point_t points [4]) {
    return (sk_path_verb_t)AsPathRawIter(iterator)->next(AsPoint(points));
}

sk_path_verb_t sk_path_rawiter_peek (sk_path_rawiterator_t *iterator) {
    return (sk_path_verb_t)AsPathRawIter(iterator)->peek ();
}

float sk_path_rawiter_conic_weight (sk_path_rawiterator_t *iterator) {
    return AsPathRawIter(iterator)->conicWeight ();
}

void sk_path_rawiter_destroy (sk_path_rawiterator_t *iterator) {
    delete AsPathRawIter (iterator);
}

void sk_path_add_path_offset (sk_path_t* cpath, sk_path_t* other, float dx, float dy, sk_path_add_mode_t add_mode) {
    AsPath (cpath)->addPath (AsPath (*other), dx, dy, (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_matrix (sk_path_t* cpath, sk_path_t* other, sk_matrix_t *matrix, sk_path_add_mode_t add_mode) {
    AsPath (cpath)->addPath (AsPath (*other), AsMatrix(matrix), (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path (sk_path_t* cpath, sk_path_t* other, sk_path_add_mode_t add_mode) {
    AsPath (cpath)->addPath (AsPath (*other), (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_reverse (sk_path_t* cpath, sk_path_t* other) {
    AsPath (cpath)->reverseAddPath (AsPath (*other));
}

sk_path_t* sk_path_new() {
    return ToPath(new SkPath());
}

void sk_path_delete(sk_path_t* cpath) {
    delete AsPath(cpath);
}

void sk_path_move_to(sk_path_t* cpath, float x, float y) {
    AsPath(cpath)->moveTo(x, y);
}

void sk_path_line_to(sk_path_t* cpath, float x, float y) {
    AsPath(cpath)->lineTo(x, y);
}

void sk_path_quad_to(sk_path_t* cpath, float x0, float y0, float x1, float y1) {
    AsPath(cpath)->quadTo(x0, y0, x1, y1);
}

void sk_path_conic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float w) {
    AsPath(cpath)->conicTo(x0, y0, x1, y1, w);
}

void sk_path_cubic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float x2, float y2) {
    AsPath(cpath)->cubicTo(x0, y0, x1, y1, x2, y2);
}

void sk_path_arc_to(sk_path_t* cpath, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y) {
    AsPath(cpath)->arcTo(rx, ry, xAxisRotate, (SkPath::ArcSize)largeArc, (SkPath::Direction)sweep, x, y);
}

void sk_path_rarc_to(sk_path_t* cpath, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y) {
    AsPath(cpath)->rArcTo(rx, ry, xAxisRotate, (SkPath::ArcSize)largeArc, (SkPath::Direction)sweep, x, y);
}

void sk_path_arc_to_with_oval(sk_path_t* cpath, const sk_rect_t* oval, float startAngle, float sweepAngle, bool forceMoveTo) {
    AsPath(cpath)->arcTo(*AsRect(oval), startAngle, sweepAngle, forceMoveTo);
}

void sk_path_arc_to_with_points(sk_path_t* cpath, float x1, float y1, float x2, float y2, float radius) {
    AsPath(cpath)->arcTo(x1, y1, x2, y2, radius);
}

void sk_path_close(sk_path_t* cpath) {
    AsPath(cpath)->close();
}

void sk_path_add_rect(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    AsPath(cpath)->addRect(*AsRect(crect), (SkPath::Direction)cdir);
}

void sk_path_add_rrect(sk_path_t* cpath, const sk_rrect_t* crect, sk_path_direction_t cdir) {
    AsPath(cpath)->addRRect(*AsRRect(crect), (SkPath::Direction)cdir);
}

void sk_path_add_rrect_start(sk_path_t* cpath, const sk_rrect_t* crect, sk_path_direction_t cdir, uint32_t start) {
    AsPath(cpath)->addRRect(*AsRRect(crect), (SkPath::Direction)cdir, start);
}

void sk_path_add_rounded_rect(sk_path_t* cpath, const sk_rect_t* crect, float rx, float ry, sk_path_direction_t cdir) {
    AsPath(cpath)->addRoundRect(*AsRect(crect), rx, ry, (SkPath::Direction)cdir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    AsPath(cpath)->addOval(*AsRect(crect), (SkPath::Direction)cdir);
}

void sk_path_add_circle(sk_path_t* cpath, float x, float y, float radius, sk_path_direction_t dir) {
    AsPath(cpath)->addCircle(x, y, radius, (SkPath::Direction)dir);
}

void sk_path_get_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    *crect = ToRect(AsPath(cpath)->getBounds());
}

void sk_path_compute_tight_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    *crect = ToRect(AsPath(cpath)->computeTightBounds());
}

int sk_path_count_points(const sk_path_t* cpath) {
    return AsPath(cpath)->countPoints();
}

int sk_path_count_verbs(const sk_path_t* cpath) {
    return AsPath(cpath)->countVerbs();
}

void sk_path_get_point(const sk_path_t* cpath, int index, sk_point_t* cpoint) {
    *cpoint = ToPoint(AsPath(cpath)->getPoint(index));
}

int sk_path_get_points(const sk_path_t* cpath, sk_point_t* cpoints, int max) {
    return AsPath(cpath)->getPoints(AsPoint(cpoints), max);
}

bool sk_path_contains (const sk_path_t* cpath, float x, float y) {
    return AsPath(cpath)->contains(x, y);
}

sk_path_convexity_t sk_path_get_convexity (const sk_path_t* cpath) {
    return (sk_path_convexity_t)AsPath(cpath)->getConvexity();
}

void sk_path_set_convexity (sk_path_t* cpath, sk_path_convexity_t convexity) {
    AsPath(cpath)->setConvexity((SkPath::Convexity)convexity);
}

bool sk_path_parse_svg_string (sk_path_t* cpath, const char* str) {
    return SkParsePath::FromSVGString(str, AsPath(cpath));
}

void sk_path_to_svg_string (const sk_path_t* cpath, sk_string_t* str) {
    SkParsePath::ToSVGString(*AsPath(cpath), AsString(str));
}

bool sk_path_get_last_point (const sk_path_t* cpath, sk_point_t* point) {
    return AsPath(cpath)->getLastPt(AsPoint(point));
}

bool sk_pathop_op(const sk_path_t* one, const sk_path_t* two, sk_pathop_t op, sk_path_t* result) {
    return Op(*AsPath(one), *AsPath(two), (SkPathOp)op, AsPath(result));
}

bool sk_pathop_simplify(const sk_path_t* path, sk_path_t* result) {
    return Simplify(*AsPath(path), AsPath(result));
}

bool sk_pathop_tight_bounds(const sk_path_t* path, sk_rect_t* result) {
    return TightBounds(*AsPath(path), AsRect(result));
}

sk_opbuilder_t* sk_opbuilder_new() {
    return ToOpBuilder(new SkOpBuilder());
}

void sk_opbuilder_destroy(sk_opbuilder_t* builder) {
    delete AsOpBuilder(builder);
}

void sk_opbuilder_add(sk_opbuilder_t* builder, const sk_path_t* path, sk_pathop_t op) {
    AsOpBuilder(builder)->add(*AsPath(path), (SkPathOp)op);
}

bool sk_opbuilder_resolve(sk_opbuilder_t* builder, sk_path_t* result) {
    return AsOpBuilder(builder)->resolve(AsPath(result));
}

int sk_path_convert_conic_to_quads(const sk_point_t* p0, const sk_point_t* p1, const sk_point_t* p2, float w, sk_point_t* pts, int pow2) {
    return SkPath::ConvertConicToQuads(*AsPoint(p0), *AsPoint(p1), *AsPoint(p2), w, AsPoint(pts), pow2);
}

sk_pathmeasure_t* sk_pathmeasure_new() {
    return ToPathMeasure(new SkPathMeasure());
}

sk_pathmeasure_t* sk_pathmeasure_new_with_path(const sk_path_t* path, bool forceClosed, float resScale) {
    return ToPathMeasure(new SkPathMeasure(*AsPath(path), forceClosed, resScale));
}

void sk_pathmeasure_destroy(sk_pathmeasure_t* pathMeasure) {
    delete AsPathMeasure(pathMeasure);
}

void sk_pathmeasure_set_path(sk_pathmeasure_t* pathMeasure, const sk_path_t* path, bool forceClosed) {
    AsPathMeasure(pathMeasure)->setPath(AsPath(path), forceClosed);
}

float sk_pathmeasure_get_length(sk_pathmeasure_t* pathMeasure) {
    return AsPathMeasure(pathMeasure)->getLength();
}

bool sk_pathmeasure_get_pos_tan(sk_pathmeasure_t* pathMeasure, float distance, sk_point_t* position, sk_vector_t* tangent) {
    return AsPathMeasure(pathMeasure)->getPosTan(distance, AsPoint(position), AsPoint(tangent));
}

bool sk_pathmeasure_get_matrix(sk_pathmeasure_t* pathMeasure, float distance, sk_matrix_t* matrix, sk_pathmeasure_matrixflags_t flags) {
    SkMatrix skmatrix;
    bool result = AsPathMeasure(pathMeasure)->getMatrix(distance, &skmatrix, (SkPathMeasure::MatrixFlags)flags);
    *matrix = ToMatrix(&skmatrix);
    return result;
}

bool sk_pathmeasure_get_segment(sk_pathmeasure_t* pathMeasure, float start, float stop, sk_path_t* dst, bool startWithMoveTo) {
    return AsPathMeasure(pathMeasure)->getSegment(start, stop, AsPath(dst), startWithMoveTo);
}

bool sk_pathmeasure_is_closed(sk_pathmeasure_t* pathMeasure) {
    return AsPathMeasure(pathMeasure)->isClosed();
}

bool sk_pathmeasure_next_contour(sk_pathmeasure_t* pathMeasure) {
    return AsPathMeasure(pathMeasure)->nextContour();
}

void sk_path_add_poly(sk_path_t* cpath, const sk_point_t* points, int count, bool close) {
    AsPath(cpath)->addPoly(AsPoint(points), count, close);
}

uint32_t sk_path_get_segment_masks(sk_path_t* cpath) {
    return AsPath(cpath)->getSegmentMasks();
}

bool sk_path_is_oval(sk_path_t* cpath, sk_rect_t* bounds) {
    return AsPath(cpath)->isOval(AsRect(bounds));
}

bool sk_path_is_rrect(sk_path_t* cpath, sk_rrect_t* bounds) {
    return AsPath(cpath)->isRRect(AsRRect(bounds));
}

bool sk_path_is_line(sk_path_t* cpath, sk_point_t line [2]) {
    return AsPath(cpath)->isLine(AsPoint(line));
}

bool sk_path_is_rect(sk_path_t* cpath, sk_rect_t* rect, bool* isClosed, sk_path_direction_t* direction) {
    return AsPath(cpath)->isRect(AsRect(rect), isClosed, (SkPath::Direction*)direction);
}

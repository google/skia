/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPath.h"

#include "sk_path.h"

#include "sk_types_priv.h"

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
    AsPath(cpath)->addRect(AsRect(*crect), (SkPath::Direction)cdir, startIndex);
}

void sk_path_add_arc(sk_path_t* cpath, const sk_rect_t* crect, float startAngle, float sweepAngle) {
    AsPath(cpath)->addArc(AsRect(*crect), startAngle, sweepAngle);
}

void sk_path_set_filltype(sk_path_t* cpath, sk_path_filltype_t cfilltype) {
    AsPath(cpath)->setFillType((SkPath::FillType)cfilltype);
}

sk_path_filltype_t sk_path_get_filltype(sk_path_t *cpath) {
    return (sk_path_filltype_t)AsPath(cpath)->getFillType();
}

void sk_path_transform(sk_path_t* cpath, const sk_matrix_t* cmatrix)
{
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    return AsPath(cpath)->transform(matrix);
}

sk_path_t* sk_path_clone(const sk_path_t* cpath)
{
    return (sk_path_t*)new SkPath(AsPath(*cpath));
}

void sk_path_rewind (sk_path_t* cpath)
{
    AsPath (cpath)->rewind ();
}

void sk_path_reset (sk_path_t* cpath)
{
    AsPath (cpath)->reset ();
}

sk_path_iterator_t* sk_path_create_iter (sk_path_t *cpath, int forceClose)
{
    SkPath::Iter* iter = new SkPath::Iter(AsPath(*cpath), forceClose);
    return ToPathIter(iter);
}

sk_path_verb_t sk_path_iter_next (sk_path_iterator_t *iterator, sk_point_t points [4], int doConsumeDegenerates, int exact)
{
    SkPath::Iter *iter = AsPathIter(iterator);
    SkPoint *pts = AsPoint(points);
    SkPath::Verb verb = iter->next(pts, doConsumeDegenerates, exact);
    return (sk_path_verb_t)verb;
}

float sk_path_iter_conic_weight (sk_path_iterator_t *iterator)
{
    return AsPathIter(iterator)->conicWeight ();
}

int sk_path_iter_is_close_line (sk_path_iterator_t *iterator)
{
    return AsPathIter(iterator)->isCloseLine ();
}

int sk_path_iter_is_closed_contour (sk_path_iterator_t *iterator)
{
    return AsPathIter(iterator)->isClosedContour ();
}

void sk_path_iter_destroy (sk_path_iterator_t *iterator)
{
    delete AsPathIter (iterator);
}

sk_path_rawiterator_t* sk_path_create_rawiter (sk_path_t *cpath)
{
    SkPath::RawIter* iter = new SkPath::RawIter(AsPath(*cpath));
    return ToPathRawIter(iter);
}

sk_path_verb_t sk_path_rawiter_next (sk_path_rawiterator_t *iterator, sk_point_t points [4])
{
    SkPath::RawIter *iter = AsPathRawIter(iterator);
    SkPoint *pts = AsPoint(points);
    SkPath::Verb verb = iter->next(pts);
    return (sk_path_verb_t)verb;
}

sk_path_verb_t sk_path_rawiter_peek (sk_path_rawiterator_t *iterator)
{
    return (sk_path_verb_t) AsPathRawIter(iterator)->peek ();
}

float sk_path_rawiter_conic_weight (sk_path_rawiterator_t *iterator)
{
    return AsPathRawIter(iterator)->conicWeight ();
}

void sk_path_rawiter_destroy (sk_path_rawiterator_t *iterator)
{
    delete AsPathRawIter (iterator);
}

void sk_path_add_path_offset (sk_path_t* cpath, sk_path_t* other, float dx, float dy, sk_path_add_mode_t add_mode)
{
    AsPath (cpath)->addPath (AsPath (*other), dx, dy, (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_matrix (sk_path_t* cpath, sk_path_t* other, sk_matrix_t *matrix, sk_path_add_mode_t add_mode)
{
    SkMatrix skmatrix;
    from_c(matrix, &skmatrix);
    AsPath (cpath)->addPath (AsPath (*other), skmatrix, (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path (sk_path_t* cpath, sk_path_t* other, sk_path_add_mode_t add_mode)
{
    AsPath (cpath)->addPath (AsPath (*other), (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_reverse (sk_path_t* cpath, sk_path_t* other)
{
    AsPath (cpath)->reverseAddPath (AsPath (*other));
}

sk_path_t* sk_path_new() { return (sk_path_t*)new SkPath; }

void sk_path_delete(sk_path_t* cpath) { delete AsPath(cpath); }

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

void sk_path_close(sk_path_t* cpath) {
    AsPath(cpath)->close();
}

void sk_path_add_rect(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    AsPath(cpath)->addRect(AsRect(*crect), (SkPath::Direction)cdir);
}

void sk_path_add_rounded_rect(sk_path_t* cpath, const sk_rect_t* crect, float rx, float ry, sk_path_direction_t cdir) {
    AsPath(cpath)->addRoundRect(AsRect(*crect), rx, ry, (SkPath::Direction)cdir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    AsPath(cpath)->addOval(AsRect(*crect), (SkPath::Direction)cdir);
}

void sk_path_add_circle(sk_path_t* cpath, float x, float y, float radius, sk_path_direction_t dir) {
    AsPath(cpath)->addCircle(x, y, radius, (SkPath::Direction)dir);
}

bool sk_path_get_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    const SkPath& path = AsPath(*cpath);

    if (path.isEmpty()) {
        if (crect) {
            *crect = ToRect(SkRect::MakeEmpty());
        }
        return false;
    }

    if (crect) {
        *crect = ToRect(path.getBounds());
    }
    return true;
}

int sk_path_count_points(const sk_path_t* cpath) {
    const SkPath& path = AsPath(*cpath);
    return path.countPoints();
}

void sk_path_get_point(const sk_path_t* cpath, int index, sk_point_t* cpoint) {
    const SkPath& path = AsPath(*cpath);
    if (cpoint) {
        SkPoint point = path.getPoint(index);
        *cpoint = ToPoint(point);
    }
}

int sk_path_get_points(const sk_path_t* cpath, sk_point_t* cpoints, int max) {
    const SkPath& path = AsPath(*cpath);
    return path.getPoints(AsPoint(cpoints), max);
}
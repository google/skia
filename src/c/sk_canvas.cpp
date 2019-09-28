/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAnnotation.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkOverdrawCanvas.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "include/utils/SkNWayCanvas.h"

#include "include/c/sk_canvas.h"

#include "src/c/sk_types_priv.h"

void sk_canvas_destroy(sk_canvas_t* ccanvas) {
    delete AsCanvas(ccanvas);
}

void sk_canvas_clear(sk_canvas_t* ccanvas, sk_color_t color) {
    AsCanvas(ccanvas)->clear(color);
}

void sk_canvas_discard(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->discard();
}

int sk_canvas_get_save_count(sk_canvas_t* ccanvas) {
    return AsCanvas(ccanvas)->getSaveCount();
}

void sk_canvas_restore_to_count(sk_canvas_t* ccanvas, int saveCount) {
    AsCanvas(ccanvas)->restoreToCount(saveCount);
}

void sk_canvas_draw_color(sk_canvas_t* ccanvas, sk_color_t color, sk_blendmode_t cmode) {
    AsCanvas(ccanvas)->drawColor(color, (SkBlendMode)cmode);
}

void sk_canvas_draw_points(sk_canvas_t* ccanvas, sk_point_mode_t pointMode, size_t count, const sk_point_t points [], const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPoints ((SkCanvas::PointMode)pointMode, count, AsPoint(points), *AsPaint(cpaint));
}

void sk_canvas_draw_point(sk_canvas_t* ccanvas, float x, float y, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPoint (x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_line(sk_canvas_t* ccanvas, float x0, float y0, float x1, float y1, sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawLine(x0, y0, x1, y1, *AsPaint(cpaint));
}

void sk_canvas_draw_text (sk_canvas_t* ccanvas, const char *text, size_t byteLength, float x, float y, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawText(text, byteLength, x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_pos_text (sk_canvas_t* ccanvas, const char *text, size_t byteLength, const sk_point_t pos[], const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPosText(text, byteLength, AsPoint(pos), *AsPaint(cpaint));
}

void sk_canvas_draw_text_on_path (sk_canvas_t* ccanvas, const char *text, size_t byteLength, const sk_path_t* path, float hOffset, float vOffset, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawTextOnPathHV(text, byteLength, *AsPath(path), hOffset, vOffset, *AsPaint(cpaint));
}

void sk_canvas_draw_text_blob (sk_canvas_t* ccanvas, sk_textblob_t* text, float x, float y, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawTextBlob(AsTextBlob(text), x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_bitmap(sk_canvas_t* ccanvas, const sk_bitmap_t* cbitmap, float x, float y, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawBitmap(*AsBitmap(cbitmap), x, y, AsPaint(cpaint));
}

void sk_canvas_draw_bitmap_rect(sk_canvas_t* ccanvas, const sk_bitmap_t* cbitmap, const sk_rect_t* csrcR, const sk_rect_t* cdstR, const sk_paint_t* cpaint) {
    if (csrcR) {
        AsCanvas(ccanvas)->drawBitmapRect(*AsBitmap(cbitmap), *AsRect(csrcR), *AsRect(cdstR), AsPaint(cpaint));
    }
    else {
        AsCanvas(ccanvas)->drawBitmapRect(*AsBitmap(cbitmap), *AsRect(cdstR), AsPaint(cpaint));
    }
}

void sk_canvas_reset_matrix(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->resetMatrix();
}

void sk_canvas_set_matrix(sk_canvas_t* ccanvas, const sk_matrix_t* cmatrix) {
    AsCanvas(ccanvas)->setMatrix(AsMatrix(cmatrix));
}

void sk_canvas_get_total_matrix(sk_canvas_t* ccanvas, sk_matrix_t* cmatrix) {
    *cmatrix = ToMatrix(&AsCanvas(ccanvas)->getTotalMatrix());
}

void sk_canvas_draw_round_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, float rx, float ry, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRoundRect(*AsRect(crect), rx, ry, *AsPaint(cpaint));
}

void sk_canvas_clip_rect_with_operation(sk_canvas_t* ccanvas, const sk_rect_t* crect, sk_clipop_t op, bool doAA) {
    AsCanvas(ccanvas)->clipRect(*AsRect(crect), (SkClipOp)op, doAA);
}

void sk_canvas_clip_path_with_operation(sk_canvas_t* ccanvas, const sk_path_t* cpath, sk_clipop_t op, bool doAA) {
    AsCanvas(ccanvas)->clipPath(*AsPath(cpath), (SkClipOp)op, doAA);
}

void sk_canvas_clip_rrect_with_operation(sk_canvas_t* ccanvas, const sk_rrect_t* crect, sk_clipop_t op, bool doAA) {
    AsCanvas(ccanvas)->clipRRect(*AsRRect(crect), (SkClipOp)op, doAA);
}

bool sk_canvas_get_local_clip_bounds(sk_canvas_t* ccanvas, sk_rect_t* cbounds) {
    return AsCanvas(ccanvas)->getLocalClipBounds(AsRect(cbounds));
}

bool sk_canvas_get_device_clip_bounds(sk_canvas_t* ccanvas, sk_irect_t* cbounds) {
    return AsCanvas(ccanvas)->getDeviceClipBounds(AsIRect(cbounds));
}

int sk_canvas_save(sk_canvas_t* ccanvas) {
    return AsCanvas(ccanvas)->save();
}

int sk_canvas_save_layer(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    return AsCanvas(ccanvas)->saveLayer(AsRect(crect), AsPaint(cpaint));
}

void sk_canvas_restore(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->restore();
}

void sk_canvas_translate(sk_canvas_t* ccanvas, float dx, float dy) {
    AsCanvas(ccanvas)->translate(dx, dy);
}

void sk_canvas_scale(sk_canvas_t* ccanvas, float sx, float sy) {
    AsCanvas(ccanvas)->scale(sx, sy);
}

void sk_canvas_rotate_degrees(sk_canvas_t* ccanvas, float degrees) {
    AsCanvas(ccanvas)->rotate(degrees);
}

void sk_canvas_rotate_radians(sk_canvas_t* ccanvas, float radians) {
    AsCanvas(ccanvas)->rotate(SkRadiansToDegrees(radians));
}

void sk_canvas_skew(sk_canvas_t* ccanvas, float sx, float sy) {
    AsCanvas(ccanvas)->skew(sx, sy);
}

void sk_canvas_concat(sk_canvas_t* ccanvas, const sk_matrix_t* cmatrix) {
    AsCanvas(ccanvas)->concat(AsMatrix(cmatrix));
}

bool sk_canvas_quick_reject(sk_canvas_t* ccanvas, const sk_rect_t* crect) {
    return AsCanvas(ccanvas)->quickReject(*AsRect(crect));
}

void sk_canvas_clip_region(sk_canvas_t* ccanvas, const sk_region_t* region, sk_clipop_t op) {
    AsCanvas(ccanvas)->clipRegion(*AsRegion(region), (SkClipOp)op);
}

void sk_canvas_draw_paint(sk_canvas_t* ccanvas, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPaint(*AsPaint(cpaint));
}

void sk_canvas_draw_region(sk_canvas_t* ccanvas, const sk_region_t* cregion, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRegion(*AsRegion(cregion), *AsPaint(cpaint));
}

void sk_canvas_draw_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(*AsRect(crect), *AsPaint(cpaint));
}

void sk_canvas_draw_rrect(sk_canvas_t* ccanvas, const sk_rrect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRRect(*AsRRect(crect), *AsPaint(cpaint));
}

void sk_canvas_draw_circle(sk_canvas_t* ccanvas, float cx, float cy, float rad, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawCircle(cx, cy, rad, *AsPaint(cpaint));
}

void sk_canvas_draw_oval(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawOval(*AsRect(crect), *AsPaint(cpaint));
}

void sk_canvas_draw_path(sk_canvas_t* ccanvas, const sk_path_t* cpath, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPath(*AsPath(cpath), *AsPaint(cpaint));
}

void sk_canvas_draw_image(sk_canvas_t* ccanvas, const sk_image_t* cimage, float x, float y, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawImage(AsImage(cimage), x, y, AsPaint(cpaint));
}

void sk_canvas_draw_image_rect(sk_canvas_t* ccanvas, const sk_image_t* cimage, const sk_rect_t* csrcR, const sk_rect_t* cdstR, const sk_paint_t* cpaint) {
    if (csrcR) {
        AsCanvas(ccanvas)->drawImageRect(AsImage(cimage), *AsRect(csrcR), *AsRect(cdstR), AsPaint(cpaint));
    } else {
        AsCanvas(ccanvas)->drawImageRect(AsImage(cimage), *AsRect(cdstR), AsPaint(cpaint));
    }
}

void sk_canvas_draw_picture(sk_canvas_t* ccanvas, const sk_picture_t* cpicture, const sk_matrix_t* cmatrix, const sk_paint_t* cpaint) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    AsCanvas(ccanvas)->drawPicture(AsPicture(cpicture), cmatrix ? &m : nullptr, AsPaint(cpaint));
}

void sk_canvas_draw_drawable(sk_canvas_t* ccanvas, sk_drawable_t* cdrawable, const sk_matrix_t* cmatrix) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    AsCanvas(ccanvas)->drawDrawable(AsDrawable(cdrawable), cmatrix ? &m : nullptr);
}

void sk_canvas_flush(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->flush();
}

sk_canvas_t* sk_canvas_new_from_bitmap(const sk_bitmap_t* bitmap) {
    return ToCanvas(new SkCanvas(*AsBitmap(bitmap)));
}

void sk_canvas_draw_annotation(sk_canvas_t* t, const sk_rect_t* rect, const char* key, sk_data_t* value) {
    AsCanvas(t)->drawAnnotation(*AsRect(rect), key, AsData(value));
}

void sk_canvas_draw_url_annotation(sk_canvas_t* t, const sk_rect_t* rect, sk_data_t* value) {
    SkAnnotateRectWithURL(AsCanvas(t), *AsRect(rect), AsData(value));
}

void sk_canvas_draw_named_destination_annotation(sk_canvas_t* t, const sk_point_t* point, sk_data_t* value) {
    SkAnnotateNamedDestination(AsCanvas(t), *AsPoint(point), AsData(value));
}

void sk_canvas_draw_link_destination_annotation(sk_canvas_t* t, const sk_rect_t* rect, sk_data_t* value) {
    SkAnnotateLinkToDestination(AsCanvas(t), *AsRect(rect), AsData(value));
}

void sk_canvas_draw_bitmap_lattice(sk_canvas_t* ccanvas, const sk_bitmap_t* bitmap, const sk_lattice_t* lattice, const sk_rect_t* dst, const sk_paint_t* paint) {
    AsCanvas(ccanvas)->drawBitmapLattice(*AsBitmap(bitmap), *AsLattice(lattice), *AsRect(dst), AsPaint(paint));
}

void sk_canvas_draw_image_lattice(sk_canvas_t* ccanvas, const sk_image_t* image, const sk_lattice_t* lattice, const sk_rect_t* dst, const sk_paint_t* paint) {
    AsCanvas(ccanvas)->drawImageLattice(AsImage(image), *AsLattice(lattice), *AsRect(dst), AsPaint(paint));
}

void sk_canvas_draw_bitmap_nine(sk_canvas_t* ccanvas, const sk_bitmap_t* bitmap, const sk_irect_t* center, const sk_rect_t* dst, const sk_paint_t* paint) {
    AsCanvas(ccanvas)->drawBitmapNine(*AsBitmap(bitmap), *AsIRect(center), *AsRect(dst), AsPaint(paint));
}

void sk_canvas_draw_image_nine(sk_canvas_t* ccanvas, const sk_image_t* image, const sk_irect_t* center, const sk_rect_t* dst, const sk_paint_t* paint) {
    AsCanvas(ccanvas)->drawImageNine(AsImage(image), *AsIRect(center), *AsRect(dst), AsPaint(paint));
}

void sk_canvas_draw_vertices(sk_canvas_t* ccanvas, sk_vertices_t* vertices, sk_blendmode_t mode, const sk_paint_t* paint) {
    AsCanvas(ccanvas)->drawVertices(AsVertices(vertices), (SkBlendMode)mode, *AsPaint(paint));
}

sk_nodraw_canvas_t* sk_nodraw_canvas_new(int width, int height) {
    return ToNoDrawCanvas(new SkNoDrawCanvas(width, height));
}

void sk_nodraw_canvas_destroy(sk_nodraw_canvas_t* t) {
    delete AsNoDrawCanvas(t);
}

sk_nway_canvas_t* sk_nway_canvas_new(int width, int height) {
    return ToNWayCanvas(new SkNWayCanvas(width, height));
}

void sk_nway_canvas_destroy(sk_nway_canvas_t* t) {
    delete AsNWayCanvas(t);
}

void sk_nway_canvas_add_canvas(sk_nway_canvas_t* t, sk_canvas_t* canvas) {
    AsNWayCanvas(t)->addCanvas(AsCanvas(canvas));
}

void sk_nway_canvas_remove_canvas(sk_nway_canvas_t* t, sk_canvas_t* canvas) {
    AsNWayCanvas(t)->removeCanvas(AsCanvas(canvas));
}

void sk_nway_canvas_remove_all(sk_nway_canvas_t* t) {
    AsNWayCanvas(t)->removeAll();
}

sk_overdraw_canvas_t* sk_overdraw_canvas_new(sk_canvas_t* canvas) {
    return ToOverdrawCanvas(new SkOverdrawCanvas(AsCanvas(canvas)));
}

void sk_overdraw_canvas_destroy(sk_overdraw_canvas_t* canvas) {
    delete AsOverdrawCanvas(canvas);
}

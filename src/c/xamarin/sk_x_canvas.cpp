/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"

#include "xamarin/sk_x_canvas.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

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

void sk_canvas_draw_color(sk_canvas_t* ccanvas, sk_color_t color, sk_xfermode_mode_t cmode) {
    SkXfermode::Mode mode;
    if (find_sk(cmode, &mode)) {
        AsCanvas(ccanvas)->drawColor(color, mode);
    }
}

void sk_canvas_draw_points(sk_canvas_t* ccanvas, sk_point_mode_t pointMode, size_t count, const sk_point_t points [], const sk_paint_t* cpaint)
{
    SkCanvas* canvas = AsCanvas(ccanvas);
    SkCanvas::PointMode mode = MapPointMode(pointMode);

    canvas->drawPoints (mode, count, AsPoint(points), *AsPaint(cpaint));
}

void sk_canvas_draw_point(sk_canvas_t* ccanvas, float x, float y, const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawPoint (x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_point_color(sk_canvas_t* ccanvas, float x, float y, sk_color_t color)
{
    AsCanvas(ccanvas)->drawPoint (x, y, color);
}

void sk_canvas_draw_line(sk_canvas_t* ccanvas, float x0, float y0, float x1, float y1, sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawLine(x0, y0, x1, y1, *AsPaint(cpaint));
}

void sk_canvas_draw_text (sk_canvas_t* ccanvas, const char *text, size_t byteLength, float x, float y, const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawText(text, byteLength, x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_pos_text (sk_canvas_t* ccanvas, const char *text, size_t byteLength, const sk_point_t pos[], const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawPosText(text, byteLength, AsPoint(pos), *AsPaint(cpaint));
}

void sk_canvas_draw_text_on_path (sk_canvas_t* ccanvas, const char *text, size_t byteLength, const sk_path_t* path,
                  float hOffset, float vOffset, const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawTextOnPathHV(text, byteLength, AsPath(*path), hOffset, vOffset, *AsPaint(cpaint));
}

void sk_canvas_draw_bitmap(sk_canvas_t* ccanvas, const sk_bitmap_t* cbitmap, float x, float y, const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawBitmap(*AsBitmap(cbitmap), x, y, AsPaint(cpaint));
}

void sk_canvas_draw_bitmap_rect(sk_canvas_t* ccanvas, const sk_bitmap_t* cbitmap, const sk_rect_t* csrcR, const sk_rect_t* cdstR, const sk_paint_t* cpaint)
{
    SkCanvas* canvas = AsCanvas(ccanvas);
    const SkBitmap& bitmap = *AsBitmap(cbitmap);
    const SkRect& dst = AsRect(*cdstR);
    const SkPaint* paint = AsPaint(cpaint);

    if (csrcR) {
        canvas->drawBitmapRect(bitmap, AsRect(*csrcR), dst, paint);
    }
    else {
        canvas->drawBitmapRect(bitmap, dst, paint);
    }
}

void sk_canvas_reset_matrix(sk_canvas_t* ccanvas)
{
    AsCanvas(ccanvas)->resetMatrix();
}

void sk_canvas_set_matrix(sk_canvas_t* ccanvas, const sk_matrix_t* cmatrix)
{
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    AsCanvas(ccanvas)->setMatrix(matrix);
}

void sk_canvas_get_total_matrix(sk_canvas_t* ccanvas, sk_matrix_t* cmatrix)
{
    SkMatrix matrix = AsCanvas(ccanvas)->getTotalMatrix();
    from_sk(&matrix, cmatrix);
}

void sk_canvas_draw_round_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, float rx, float ry, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRoundRect(AsRect(*crect), rx, ry, AsPaint(*cpaint));
}

void sk_canvas_clip_rect_with_operation(sk_canvas_t* ccanvas, const sk_rect_t* crect, sk_region_op_t op, bool doAA) {
    SkRegion::Op operation = SkRegion::kIntersect_Op;
    find_sk(op, &operation);
    AsCanvas(ccanvas)->clipRect(AsRect(*crect), operation, doAA);
}

void sk_canvas_clip_path_with_operation(sk_canvas_t* ccanvas, const sk_path_t* cpath, sk_region_op_t op, bool doAA) {
    SkRegion::Op operation = SkRegion::kIntersect_Op;
    find_sk(op, &operation);
    AsCanvas(ccanvas)->clipPath(AsPath(*cpath), operation, doAA);
}

bool sk_canvas_get_clip_bounds(sk_canvas_t* ccanvas, sk_rect_t* cbounds) {
    return AsCanvas(ccanvas)->getClipBounds(AsRect(cbounds));
}

bool sk_canvas_get_clip_device_bounds(sk_canvas_t* ccanvas, sk_irect_t* cbounds) {
    return AsCanvas(ccanvas)->getClipDeviceBounds(AsIRect(cbounds));
}

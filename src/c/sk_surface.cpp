/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

#include "sk_canvas.h"
#include "sk_data.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_picture.h"
#include "sk_surface.h"
#include "sk_types_priv.h"

static void from_c_matrix(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
                   cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
                   cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_colortype_t sk_colortype_get_default_8888() {
    sk_colortype_t ct;
    if (!find_c(kN32_SkColorType, &ct)) {
        ct = UNKNOWN_SK_COLORTYPE;
    }
    return ct;
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t* cinfo, const void* pixels,
                                     size_t rowBytes) {
    SkImageInfo info;
    if (!find_sk(*cinfo, &info)) {
        return NULL;
    }
    return (sk_image_t*)SkImage::NewRasterCopy(info, pixels, rowBytes);
}

sk_image_t* sk_image_new_from_encoded(const sk_data_t* cdata, const sk_irect_t* subset) {
    return ToImage(SkImage::NewFromEncoded(AsData(cdata),
                                           reinterpret_cast<const SkIRect*>(subset)));
}

sk_data_t* sk_image_encode(const sk_image_t* cimage) {
    return ToData(AsImage(cimage)->encode());
}

void sk_image_ref(const sk_image_t* cimage) {
    AsImage(cimage)->ref();
}

void sk_image_unref(const sk_image_t* cimage) {
    AsImage(cimage)->unref();
}

int sk_image_get_width(const sk_image_t* cimage) {
    return AsImage(cimage)->width();
}

int sk_image_get_height(const sk_image_t* cimage) {
    return AsImage(cimage)->height();
}

uint32_t sk_image_get_unique_id(const sk_image_t* cimage) {
    return AsImage(cimage)->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_path_t* sk_path_new() { return (sk_path_t*)new SkPath; }

void sk_path_delete(sk_path_t* cpath) { delete as_path(cpath); }

void sk_path_move_to(sk_path_t* cpath, float x, float y) {
    as_path(cpath)->moveTo(x, y);
}

void sk_path_line_to(sk_path_t* cpath, float x, float y) {
    as_path(cpath)->lineTo(x, y);
}

void sk_path_quad_to(sk_path_t* cpath, float x0, float y0, float x1, float y1) {
    as_path(cpath)->quadTo(x0, y0, x1, y1);
}

void sk_path_conic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float w) {
    as_path(cpath)->conicTo(x0, y0, x1, y1, w);
}

void sk_path_cubic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float x2, float y2) {
    as_path(cpath)->cubicTo(x0, y0, x1, y1, x2, y2);
}

void sk_path_close(sk_path_t* cpath) {
    as_path(cpath)->close();
}

void sk_path_add_rect(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!find_sk(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addRect(AsRect(*crect), dir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!find_sk(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addOval(AsRect(*crect), dir);
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

///////////////////////////////////////////////////////////////////////////////////////////

void sk_canvas_save(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->save();
}
void sk_canvas_discard(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->discard();
}
void sk_canvas_save_layer(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->saveLayer(AsRect(crect), AsPaint(cpaint));
}

void sk_canvas_restore(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->restore();
}

int sk_canvas_get_save_count(sk_canvas_t* ccanvas) {
    return AsCanvas(ccanvas)->getSaveCount();
}

void sk_canvas_restore_to_count(sk_canvas_t* ccanvas, int saveCount) {
    AsCanvas(ccanvas)->restoreToCount(saveCount);
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
    SkASSERT(cmatrix);
    SkMatrix matrix;
    from_c_matrix(cmatrix, &matrix);
    AsCanvas(ccanvas)->concat(matrix);
}

void sk_canvas_clip_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect) {
    AsCanvas(ccanvas)->clipRect(AsRect(*crect));
}

void sk_canvas_clip_path(sk_canvas_t* ccanvas, const sk_path_t* cpath) {
    AsCanvas(ccanvas)->clipPath(AsPath(*cpath));
}

void sk_canvas_draw_paint(sk_canvas_t* ccanvas, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPaint(AsPaint(*cpaint));
}

void sk_canvas_draw_color(sk_canvas_t* ccanvas, sk_color_t color, sk_xfermode_mode_t mode) {
    AsCanvas(ccanvas)->drawColor(color, MapXferMode (mode));
}

void sk_canvas_draw_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_oval(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawOval(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_path(sk_canvas_t* ccanvas, const sk_path_t* cpath, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPath(AsPath(*cpath), AsPaint(*cpaint));
}

void sk_canvas_draw_image(sk_canvas_t* ccanvas, const sk_image_t* cimage, float x, float y,
                          const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawImage(AsImage(cimage), x, y, AsPaint(cpaint));
}

void sk_canvas_draw_image_rect(sk_canvas_t* ccanvas, const sk_image_t* cimage,
                               const sk_rect_t* csrcR, const sk_rect_t* cdstR,
                               const sk_paint_t* cpaint) {
    SkCanvas* canvas = AsCanvas(ccanvas);
    const SkImage* image = AsImage(cimage);
    const SkRect& dst = AsRect(*cdstR);
    const SkPaint* paint = AsPaint(cpaint);

    if (csrcR) {
        canvas->drawImageRect(image, AsRect(*csrcR), dst, paint);
    } else {
        canvas->drawImageRect(image, dst, paint);
    }
}

void sk_canvas_draw_points(sk_canvas_t* ccanvas, sk_point_mode_t pointMode, size_t count, const sk_point_t points [], const sk_paint_t* cpaint)
{
    SkCanvas* canvas = AsCanvas(ccanvas);
    SkCanvas::PointMode mode = MapPointMode(pointMode);

    canvas->drawPoints (mode, count, reinterpret_cast<const SkPoint*>(points), *AsPaint(cpaint));
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

void sk_canvas_draw_picture(sk_canvas_t* ccanvas, const sk_picture_t* cpicture,
                            const sk_matrix_t* cmatrix, const sk_paint_t* cpaint) {
    const SkMatrix* matrixPtr = NULL;
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
        matrixPtr = &matrix;
    }
    AsCanvas(ccanvas)->drawPicture(AsPicture(cpicture), matrixPtr, AsPaint(cpaint));
}

void sk_canvas_draw_text (sk_canvas_t* ccanvas, const char *text, size_t byteLength, float x, float y, const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawText(text, byteLength, x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_pos_text (sk_canvas_t* ccanvas, const char *text, size_t byteLength, const sk_point_t pos[], const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawPosText(text, byteLength, reinterpret_cast<const SkPoint*>(pos), *AsPaint(cpaint));
}

void sk_canvas_draw_text_on_path (sk_canvas_t* ccanvas, const char *text, size_t byteLength, const sk_path_t* path,
				  float hOffset, float vOffset, const sk_paint_t* cpaint)
{
    AsCanvas(ccanvas)->drawTextOnPathHV(text, byteLength, AsPath(*path), hOffset, vOffset, *AsPaint(cpaint));
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t* cinfo,
                                    const sk_surfaceprops_t* props) {
    SkImageInfo info;
    if (!find_sk(*cinfo, &info)) {
        return NULL;
    }
    SkPixelGeometry geo = kUnknown_SkPixelGeometry;
    if (props && !find_sk(props->pixelGeometry, &geo)) {
        return NULL;
    }

    SkSurfaceProps surfProps(0, geo);
    return (sk_surface_t*)SkSurface::NewRaster(info, &surfProps);
}

sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels,
                                           size_t rowBytes,
                                           const sk_surfaceprops_t* props) {
    SkImageInfo info;
    if (!find_sk(*cinfo, &info)) {
        return NULL;
    }
    SkPixelGeometry geo = kUnknown_SkPixelGeometry;
    if (props && !find_sk(props->pixelGeometry, &geo)) {
        return NULL;
    }

    SkSurfaceProps surfProps(0, geo);
    return (sk_surface_t*)SkSurface::NewRasterDirect(info, pixels, rowBytes, &surfProps);
}

void sk_surface_unref(sk_surface_t* csurf) {
    SkSafeUnref((SkSurface*)csurf);
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_canvas_t*)surf->getCanvas();
}

sk_image_t* sk_surface_new_image_snapshot(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_image_t*)surf->newImageSnapshot();
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_picture_recorder_t* sk_picture_recorder_new() {
    return ToPictureRecorder(new SkPictureRecorder);
}

void sk_picture_recorder_delete(sk_picture_recorder_t* crec) {
    delete AsPictureRecorder(crec);
}

sk_canvas_t* sk_picture_recorder_begin_recording(sk_picture_recorder_t* crec,
                                                 const sk_rect_t* cbounds) {
    return ToCanvas(AsPictureRecorder(crec)->beginRecording(AsRect(*cbounds)));
}

sk_picture_t* sk_picture_recorder_end_recording(sk_picture_recorder_t* crec) {
    return ToPicture(AsPictureRecorder(crec)->endRecording());
}

void sk_picture_ref(sk_picture_t* cpic) {
    SkSafeRef(AsPicture(cpic));
}

void sk_picture_unref(sk_picture_t* cpic) {
    SkSafeUnref(AsPicture(cpic));
}

uint32_t sk_picture_get_unique_id(sk_picture_t* cpic) {
    return AsPicture(cpic)->uniqueID();
}

sk_rect_t sk_picture_get_bounds(sk_picture_t* cpic) {
    return ToRect(AsPicture(cpic)->cullRect());
}

///////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/effects/SkGradientShader.h"
#include "sk_shader.h"

void sk_shader_ref(sk_shader_t* cshader) {
    SkSafeRef(AsShader(cshader));
}

void sk_shader_unref(sk_shader_t* cshader) {
    SkSafeUnref(AsShader(cshader));
}

sk_shader_t* sk_shader_new_empty() {
    return (sk_shader_t*) SkShader::CreateEmptyShader();
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return (sk_shader_t*) SkShader::CreateColorShader(color);
}

sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t& src,
                                  sk_shader_tilemode_t tmx,
                                  sk_shader_tilemode_t tmy,
                                  const sk_matrix_t* localMatrix) {
    SkShader::TileMode modex;
    if (!find_sk(tmx, &modex)) {
        return NULL;
    }
    SkShader::TileMode modey;
    if (!find_sk(tmy, &modey)) {
        return NULL;
    }
    SkMatrix matrix;
    if (localMatrix) {
        from_c_matrix(localMatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = SkShader::CreateBitmapShader(AsBitmap(src), modex, modey, &matrix);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_local_matrix(sk_shader_t* proxy,
                                        const sk_matrix_t* localMatrix) {
    SkMatrix matrix;
    if (localMatrix) {
        from_c_matrix(localMatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = SkShader::CreateLocalMatrixShader(AsShader(proxy), matrix);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t pts[2],
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!find_sk(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = SkGradientShader::CreateLinear(reinterpret_cast<const SkPoint*>(pts),
                                                 reinterpret_cast<const SkColor*>(colors),
                                                 colorPos, colorCount, mode, 0, &matrix);
    return (sk_shader_t*)s;
}

static const SkPoint& to_skpoint(const sk_point_t& p) {
    return reinterpret_cast<const SkPoint&>(p);
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* ccenter,
                                           float radius,
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!find_sk(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkPoint center = to_skpoint(*ccenter);
    SkShader* s = SkGradientShader::CreateRadial(
            center, (SkScalar)radius,
            reinterpret_cast<const SkColor*>(colors),
            reinterpret_cast<const SkScalar*>(colorPos),
            colorCount, mode, 0, &matrix);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* ccenter,
                                          const sk_color_t colors[],
                                          const float colorPos[],
                                          int colorCount,
                                          const sk_matrix_t* cmatrix) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = SkGradientShader::CreateSweep(
            (SkScalar)(ccenter->x),
            (SkScalar)(ccenter->y),
            reinterpret_cast<const SkColor*>(colors),
            reinterpret_cast<const SkScalar*>(colorPos),
            colorCount, 0, &matrix);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start,
                                                      float startRadius,
                                                      const sk_point_t* end,
                                                      float endRadius,
                                                      const sk_color_t colors[],
                                                      const float colorPos[],
                                                      int colorCount,
                                                      sk_shader_tilemode_t cmode,
                                                      const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!find_sk(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkPoint skstart = to_skpoint(*start);
    SkPoint skend = to_skpoint(*end);
    SkShader* s = SkGradientShader::CreateTwoPointConical(
            skstart, (SkScalar)startRadius,
            skend, (SkScalar)endRadius,
            reinterpret_cast<const SkColor*>(colors),
            reinterpret_cast<const SkScalar*>(colorPos),
            colorCount, mode, 0, &matrix);
    return (sk_shader_t*)s;
}

///////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/effects/SkBlurMaskFilter.h"
#include "sk_maskfilter.h"

void sk_maskfilter_ref(sk_maskfilter_t* cfilter) {
    SkSafeRef(AsMaskFilter(cfilter));
}

void sk_maskfilter_unref(sk_maskfilter_t* cfilter) {
    SkSafeUnref(AsMaskFilter(cfilter));
}

sk_maskfilter_t* sk_maskfilter_new_blur(sk_blurstyle_t cstyle, float sigma) {
    SkBlurStyle style;
    if (!find_sk(cstyle, &style)) {
        return NULL;
    }
    return ToMaskFilter(SkBlurMaskFilter::Create(style, sigma));
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_data_t* sk_data_new_empty() {
    return ToData(SkData::NewEmpty());
}

sk_data_t* sk_data_new_with_copy(const void* src, size_t length) {
    return ToData(SkData::NewWithCopy(src, length));
}

sk_data_t* sk_data_new_from_malloc(const void* memory, size_t length) {
    return ToData(SkData::NewFromMalloc(memory, length));
}

sk_data_t* sk_data_new_subset(const sk_data_t* csrc, size_t offset, size_t length) {
    return ToData(SkData::NewSubset(AsData(csrc), offset, length));
}

void sk_data_ref(const sk_data_t* cdata) {
    SkSafeRef(AsData(cdata));
}

void sk_data_unref(const sk_data_t* cdata) {
    SkSafeUnref(AsData(cdata));
}

size_t sk_data_get_size(const sk_data_t* cdata) {
    return AsData(cdata)->size();
}

const void* sk_data_get_data(const sk_data_t* cdata) {
    return AsData(cdata)->data();
}

///////////////////////////////////////////////////////////////////////////////////////////

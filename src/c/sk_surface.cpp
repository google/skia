/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
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

sk_colortype_t sk_colortype_get_default_8888() {
    return (sk_colortype_t)SkColorType::kN32_SkColorType;
}

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t* cinfo, const sk_surfaceprops_t* props) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    SkSurfaceProps* surfProps = nullptr;
    if (props) {
        from_c(props, surfProps);
    }
    return ToSurface(SkSurface::MakeRaster(info, surfProps).release());
}

sk_image_t* sk_image_new_from_encoded(const sk_data_t* cdata, const sk_irect_t* subset) {
    return ToImage(SkImage::MakeFromEncoded(sk_ref_sp(AsData(cdata)),
                                           reinterpret_cast<const SkIRect*>(subset)).release());
}

sk_data_t* sk_image_encode(const sk_image_t* cimage) {
    return ToData(AsImage(cimage)->encodeToData().release());
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
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addRect(AsRect(*crect), dir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
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

void sk_canvas_save_layer(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
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

void sk_canvas_rotate_degress(sk_canvas_t* ccanvas, float degrees) {
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

void sk_canvas_draw_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_circle(sk_canvas_t* ccanvas, float cx, float cy, float rad,
                           const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawCircle(cx, cy, rad, AsPaint(*cpaint));
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

///////////////////////////////////////////////////////////////////////////////////////////

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t* cinfo,
                                    const sk_surfaceprops_t* props) {
    SkImageInfo info;
    if (!from_c_info(*cinfo, &info)) {
        return NULL;
    }
    SkPixelGeometry geo = kUnknown_SkPixelGeometry;
    if (props && !from_c_pixelgeometry(props->pixelGeometry, &geo)) {
        return NULL;
    }

    SkSurfaceProps surfProps(0, geo);
    return (sk_surface_t*)SkSurface::MakeRaster(info, &surfProps).release();
}

sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels,
                                           size_t rowBytes,
                                           const sk_surfaceprops_t* props) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    SkSurfaceProps* surfProps = nullptr;
    if (props) {
        from_c(props, surfProps);
    }
    return ToSurface(SkSurface::MakeRasterDirect(info, pixels, rowBytes, surfProps).release());
}

void sk_surface_unref(sk_surface_t* csurf) {
    SkSafeUnref(AsSurface(csurf));
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* csurf) {
    return ToCanvas(AsSurface(csurf)->getCanvas());
}

sk_image_t* sk_surface_new_image_snapshot(sk_surface_t* csurf) {
    return ToImage(AsSurface(csurf)->makeImageSnapshot().release());
}

sk_surface_t* sk_surface_new_backend_render_target(gr_context_t* context, const gr_backend_rendertarget_desc_t* desc, const sk_surfaceprops_t* props) {
    SkSurfaceProps* surfProps = nullptr;
    if (props) {
        from_c(props, surfProps);
    }
    return ToSurface(SkSurface::MakeFromBackendRenderTarget(AsGrContext(context), AsGrBackendRenderTargetDesc(*desc), surfProps).release());
}

sk_surface_t* sk_surface_new_backend_texture(gr_context_t* context, const gr_backend_texture_desc_t* desc, const sk_surfaceprops_t* props) {
    SkSurfaceProps* surfProps = nullptr;
    if (props) {
        from_c(props, surfProps);
    }
    return ToSurface(SkSurface::MakeFromBackendTexture(AsGrContext(context), AsGrBackendTextureDesc(*desc), surfProps).release());
}

sk_surface_t* sk_surface_new_backend_texture_as_render_target(gr_context_t* context, const gr_backend_texture_desc_t* desc, const sk_surfaceprops_t* props) {
    SkSurfaceProps* surfProps = nullptr;
    if (props) {
        from_c(props, surfProps);
    }
    return ToSurface(SkSurface::MakeFromBackendTextureAsRenderTarget(AsGrContext(context), AsGrBackendTextureDesc(*desc), surfProps).release());
}

sk_surface_t* sk_surface_new_render_target(gr_context_t* context, bool budgeted, const sk_imageinfo_t* cinfo, int sampleCount, const sk_surfaceprops_t* props) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    SkSurfaceProps* surfProps = nullptr;
    if (props) {
        from_c(props, surfProps);
    }
    return ToSurface(SkSurface::MakeRenderTarget(AsGrContext(context), (SkBudgeted)budgeted, info, sampleCount, surfProps).release());
}

void sk_surface_draw(sk_surface_t* surface, sk_canvas_t* canvas, float x, float y, const sk_paint_t* paint) {
    AsSurface(surface)->draw(AsCanvas(canvas), x, y, AsPaint(paint));
}

bool sk_surface_peek_pixels(sk_surface_t* surface, sk_pixmap_t* pixmap) {
    return AsSurface(surface)->peekPixels(AsPixmap(pixmap));
}

bool sk_surface_read_pixels(sk_surface_t* surface, sk_imageinfo_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY) {
    SkImageInfo info;
    from_c(*dstInfo, &info);
    return AsSurface(surface)->readPixels(info, dstPixels, dstRowBytes, srcX, srcY);
}

void sk_surface_get_props(sk_surface_t* surface, sk_surfaceprops_t* props) {
    SkSurfaceProps skProps = AsSurface(surface)->props();
    from_sk(&skProps, props);
}

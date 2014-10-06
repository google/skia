#include "sk_surface.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkSurface.h"

void sk_matrix_set_identity(sk_matrix_t* cmatrix) {
    sk_bzero(cmatrix->mat, sizeof(9 * sizeof(float)));
    cmatrix->mat[0] = cmatrix->mat[4] = cmatrix->mat[8] = 1;
}

static SkImageInfo make(const sk_image_info& cinfo) {
    return SkImageInfo::Make(cinfo.width, cinfo.height,
                             (SkColorType)cinfo.colorType, (SkAlphaType)cinfo.alphaType);
}

static const SkRect& AsRect(const sk_rect_t& crect) { return static_cast<const SkRect&>(crect); }

static const SkPaint& AsPaint(const sk_paint_t& cpaint) {
    return static_cast<const SkPaint&>(cpaint);
}

static const SkPaint* AsPaint(const sk_paint_t* cpaint) {
    return static_cast<const SkPaint*>(cpaint);
}

static SkCanvas* AsCanvas(sk_canvas_t* ccanvas) { return static_cast<SkCanvas*>(ccanvas); }

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
    AsCanvas(ccanvas)->scale(dx, dy);
}

void sk_canvas_concat(sk_canvas_t* ccanvas, const sk_matrix_t* cmatrix) {
    AsCanvas(ccanvas)->concat(AsMatrix(*cmatrix));
}

void sk_canvas_draw_paint(sk_canvas_t* ccanvas, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPaint(AsPaint(*cpaint));
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

///////////////////////////////////////////////////////////////////////////////////////////

sk_image_t* sk_image_new_raster_copy(const sk_image_info_t* cinfo, const void* pixels,
                                     size_t rowBytes) {
    return (sk_image_t*)SkImage::NewRasterCopy(make(*cinfo), pixels, rowBytes);
}

int sk_image_get_width(const sk_image_t* cimage) {
    return ((const SkImage*)cimage)->width();
}

int sk_image_get_height(const sk_image_t* cimage) {
    return ((const SkImage*)cimage)->height();
}

uint32_t sk_image_get_unique_id(const sk_image_t* cimage) {
    return ((const SkImage*)cimage)->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_surface_t* sk_surface_new_raster(const sk_image_info_t* cinfo) {
    return (sk_surface_t*)SkSurface::NewRaster(make(*cinfo));
}

sk_surface_t* sk_surface_new_raster_direct(const sk_image_info_t* cinfo, void* pixels,
                                           size_t rowBytes) {
    return (sk_surface_t*)SkSurface::NewRasterDirect(make(*cinfo), pixels, rowBytes);
}

void sk_surface_delete(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    SkSafeUnref(surf);
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return surf->getCanvas();
}

sk_image_t* sk_surface_new_image_snapshot(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return surf->newImageSnapshot();
}



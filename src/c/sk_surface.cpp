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
#include "sk_region.h"

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

sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_surfaceprops_t* props) {
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

/* REGIONS */
sk_region_t* sk_region_new() { return (sk_region_t*)new SkRegion(); }

sk_region_t* sk_region_new2(const sk_region_t* region) { return (sk_region_t*)new SkRegion(*AsRegion(region)); }

void sk_region_delete(sk_region_t* cpath) { delete AsRegion(cpath); }

void sk_region_contains(sk_region_t* r, const sk_region_t* region) {
	AsRegion(r)->contains(*AsRegion(region));
}

void sk_region_contains2(sk_region_t* r, int x, int y) {
	AsRegion(r)->contains(x, y);
}

bool sk_region_intersects(sk_region_t* r, const sk_region_t* src) {
	return AsRegion(r)->intersects(*AsRegion(src));
}

bool sk_region_set_path(sk_region_t* dst, const sk_path_t* t)
{
	SkRegion region = *AsRegion(dst);
	return region.setPath(AsPath(*t), region);
}

bool sk_region_set_rect(sk_region_t* dst, const sk_irect_t* rect)
{
	SkRegion region = *AsRegion(dst);
	return region.setRect(AsIRect(*rect));
}

bool sk_region_op(sk_region_t* dst, int left, int top, int right, int bottom, SkRegion::Op op)
{
	SkRegion region = *AsRegion(dst);
	return region.op(left, top, right, bottom, op);
}

bool sk_region_op2(sk_region_t* dst, sk_region_t* src, SkRegion::Op op)
{
	SkRegion region = *AsRegion(dst);
	return region.op(*AsRegion(src), op);
}

sk_irect_t sk_region_get_bounds(sk_region_t* r)
{
	SkRegion region = *AsRegion(r);
	return ToIRect(AsRegion(r)->getBounds());
}

/*END REGIONS*/

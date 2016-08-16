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

sk_colortype_t sk_colortype_get_default_8888() {
    return (sk_colortype_t)SkColorType::kN32_SkColorType;
}

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t* cinfo, const sk_surfaceprops_t* props) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    SkSurfaceProps surfProps = AsSurfaceProps(props);
    return ToSurface(SkSurface::MakeRaster(info, &surfProps).release());
}

sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_surfaceprops_t* props) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    SkSurfaceProps surfProps = AsSurfaceProps(props);
    return ToSurface(SkSurface::MakeRasterDirect(info, pixels, rowBytes, &surfProps).release());
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

sk_surface_t* sk_surface_new_backend_render_target(gr_context_t* context, const gr_backendrendertargetdesc_t* desc, const sk_surfaceprops_t* props) {
    const SkSurfaceProps surfProps = (const SkSurfaceProps)AsSurfaceProps(props);
    return ToSurface(SkSurface::MakeFromBackendRenderTarget(AsGrContext(context), AsGrBackendRenderTargetDesc(*desc), &surfProps).release());
}

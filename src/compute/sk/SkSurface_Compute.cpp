/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//
//
//

#include "SkSurface_Compute.h"
#include "SkDevice_Compute.h"
#include "SkImage_Compute.h"

//
//
//

#if SK_SUPPORT_GPU_COMPUTE

//
// C++
//

#include "gl/GrGLGpu.h"
#include "SkSurface_Gpu.h"

//
//
//

SkSurface_Compute::SkSurface_Compute(sk_sp<SkContext_Compute> compute,
                                     int const width, int const height)
  : INHERITED(width,height,nullptr),
    compute(compute)
{
  //
  // resize interop
  //
  // skc_interop_size_set(compute->interop,width,height,NULL); TODO skc.h
}

SkSurface_Compute::~SkSurface_Compute()
{
  ;
}

//
//
//

SkCanvas*
SkSurface_Compute::onNewCanvas()
{
  uint32_t w = 0,h = 0;

  // skc_interop_size_get(compute->interop,&w,&h); TODO skc.h

  SkDevice_Compute * const device_compute = new SkDevice_Compute(compute,w,h);
  SkCanvas         * const canvas         = new SkCanvas(device_compute,SkCanvas::kConservativeRasterClip_InitFlag);

  //
  // destroy device upon surface destruction
  //
  device = sk_sp<SkBaseDevice>(device_compute);

  //
  // move origin from upper left to lower left
  //
  SkMatrix matrix;

  matrix.setScaleTranslate(1.0f,-1.0f,0.0f,(SkScalar)h);

  canvas->setMatrix(matrix);

  return canvas;
}

//
//
//

sk_sp<SkSurface>
SkSurface_Compute::onNewSurface(const SkImageInfo& info)
{
  return sk_make_sp<SkSurface_Compute>(compute,info.width(),info.height());
}

//
//
//

sk_sp<SkImage>
SkSurface_Compute::onNewImageSnapshot()
{
  uint32_t w,h;

  // skc_interop_size_get(compute->interop,&w,&h); TODO skc.h

  GrGLuint snap;

  // skc_interop_snap_create(compute->interop, TODO skc.h
		// 	  skc_surface_interop_surface_get(compute->surface),
		// 	  &snap);

  return sk_make_sp<SkImage_Compute>(compute,snap,w,h);
}

//
//
//

void
SkSurface_Compute::onCopyOnWrite(ContentChangeMode mode)
{
  ;
}

//
//
//

#if 0

sk_sp<SkSurface>
SkSurface_Compute::MakeComputeBackedSurface(SkWindow                       * const window,
                                            const SkWindow::AttachmentInfo &       attachmentInfo,
                                            GrGLInterface const            * const grInterface,
                                            GrContext                      * const grContext,
                                            sk_sp<SkContext_Compute>               compute)
{
  GrBackendRenderTargetDesc desc;

  desc.fWidth  = SkScalarRoundToInt(window->width());
  desc.fHeight = SkScalarRoundToInt(window->height());

  if (0 == desc.fWidth || 0 == desc.fHeight) {
    return nullptr;
  }

  // TODO: Query the actual framebuffer for sRGB capable. However, to
  // preserve old (fake-linear) behavior, we don't do this. Instead, rely
  // on the flag (currently driven via 'C' mode in SampleApp).
  //
  // Also, we may not have real sRGB support (ANGLE, in particular), so check for
  // that, and fall back to L32:
  //
  // ... and, if we're using a 10-bit/channel FB0, it doesn't do sRGB conversion on write,
  // so pretend that it's non-sRGB 8888:
  desc.fConfig =
    grContext->caps()->srgbSupport() &&
    SkImageInfoIsGammaCorrect(window->info()) &&
    (attachmentInfo.fColorBits != 30)
    ? kSkiaGamma8888_GrPixelConfig : kSkia8888_GrPixelConfig;

  desc.fOrigin      = kBottomLeft_GrSurfaceOrigin;
  desc.fSampleCnt   = 0; //  attachmentInfo.fSampleCount;
  desc.fStencilBits = 0; //  attachmentInfo.fStencilBits;

  GrGLint buffer;

  GR_GL_GetIntegerv(grInterface,GR_GL_FRAMEBUFFER_BINDING,&buffer);
  desc.fRenderTargetHandle = buffer;

  sk_sp<SkColorSpace> colorSpace =
    grContext->caps()->srgbSupport() && SkImageInfoIsGammaCorrect(window->info())
    ? SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named) : nullptr;

  //
  //
  //

  if (!grContext) {
    return nullptr;
  }

  if (!SkSurface_Gpu::Valid(grContext,desc.fConfig,colorSpace.get())) {
    return nullptr;
  }

  return sk_make_sp<SkSurface_Compute>(compute,desc.fWidth,desc.fHeight);
}

#endif

//
//
//

#endif

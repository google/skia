/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrCanvas_DEFINED
#define GrCanvas_DEFINED

class GrRenderTargetProxy;
class SkCanvas;

namespace skgpu::ganesh {
class SurfaceDrawContext;
class SurfaceFillContext;

SurfaceDrawContext* TopDeviceSurfaceDrawContext(const SkCanvas*);
SurfaceFillContext* TopDeviceSurfaceFillContext(const SkCanvas*);
GrRenderTargetProxy* TopDeviceTargetProxy(const SkCanvas*);
}
#endif

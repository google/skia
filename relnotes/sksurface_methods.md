The following methods have been removed from SkSurface and relocated to other methods/functions:
  - `SkSurface::asImage` -> `SkSurfaces::AsImage` (include/gpu/graphite/Surface.h)
  - `SkSurface::flushAndSubmit` -> `GrDirectContext::flushAndSubmit`
  - `SkSurface::flush` -> `GrDirectContext::flush`
  - `SkSurface::makeImageCopy` -> `SkSurfaces::AsImageCopy` (include/gpu/graphite/Surface.h)
  - `SkSurface::resolveMSAA` -> `SkSurfaces::ResolveMSAA()` (include/gpu/ganesh/SkSurfaceGanesh.h)

Additionally, `SkSurface::BackendSurfaceAccess` is now in the `SkSurfaces` namespace.
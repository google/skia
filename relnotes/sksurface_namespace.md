SkSurface factory methods have been moved to the SkSurfaces namespace. Many have been renamed to
be more succinct or self-consistent. Factory methods specific to the Ganesh GPU backend are
defined publicly in include/gpu/ganesh/SkSurfaceGanesh.h. The Metal Ganesh backend has some
specific factories in include/gpu/ganesh/mtl/SkSurfaceMetal.h.
  * SkSurface::MakeFromAHardwareBuffer -> SkSurfaces::WrapAndroidHardwareBuffer
  * SkSurface::MakeFromBackendRenderTarget -> SkSurfaces::WrapBackendRenderTarget
  * SkSurface::MakeFromBackendTexture -> SkSurfaces::WrapBackendTexture
  * SkSurface::MakeFromCAMetalLayer -> SkSurfaces::WrapCAMetalLayer
  * SkSurface::MakeFromMTKView -> SkSurfaces::WrapMTKView
  * SkSurface::MakeGraphite -> SkSurfaces::RenderTarget
  * SkSurface::MakeGraphiteFromBackendTexture -> SkSurfaces::WrapBackendTexture
  * SkSurface::MakeNull -> SkSurfaces::Null
  * SkSurface::MakeRaster -> SkSurfaces::Raster
  * SkSurface::MakeRasterDirect -> SkSurfaces::WrapPixels
  * SkSurface::MakeRasterDirectReleaseProc -> SkSurfaces::WrapPixels
  * SkSurface::MakeRasterN32Premul -> SkSurfaces::Raster (clients should make SkImageInfo)
  * SkSurface::MakeRenderTarget -> SkSurfaces::RenderTarget


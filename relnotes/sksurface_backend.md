`SkSurface::getBackendTexture` and `SkSurface::getBackendRenderTarget` have been deprecated and
replaced with `SkSurfaces::GetBackendTexture` and `SkSurfaces::GetBackendRenderTarget` respectively.
These are found in `include/gpu/ganesh/SkSurfaceGanesh.h`. The supporting enum `BackendHandleAccess`
has also been moved to `SkSurfaces::BackendHandleAccess` as an enum class, with shorter member
names.
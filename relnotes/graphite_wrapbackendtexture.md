* `SkSurfaces::WrapBackendTexture` no longer requires providing an `SkColorType`. A closest
   compatible SkColorType will be chosen, so long as the backend texture's format is supported as
   renderable.

SkMesh now allows shaders, color filters, and blenders to be used in the mesh-fragment program.
Pass in effects using the `children` parameter of `SkMesh::Make` or `SkMesh::MakeIndexed`.
For a working example, see `gm/mesh.cpp`.

`sksl-minify` can now minify SkMesh programs. Pass `--meshvert` or `--meshfrag` to indicate
that the input program is an SkMesh vertex or fragment program. When minifying a mesh program,
you must supply `struct Varyings` and `struct Attributes` which correspond to the
SkMeshSpecification; these will be eliminated from the minified output.

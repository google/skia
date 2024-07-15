SkSL now properly reports an error if user code includes various GLSL reserved keywords.
Previously, Skia would correctly reject keywords that were included in "The OpenGL ES
Shading Language, Version 1.00," but did not detect reserved keywords added in more modern
GLSL versions. Instead, Skia would allow such code to compile during the construction of a
runtime effect, but actually rendering the effect using a modern version of OpenGL would
silently fail (or assert) due to the presence of the reserved name in the the code.

Examples of reserved names which SkSL will now reject include `dmat3x3`, `atomic_uint`,
`isampler2D`, or `imageCubeArray`.

For a more thorough list of reserved keywords, see the "3.6 Keywords" section of the
OpenGL Shading Language documentation.

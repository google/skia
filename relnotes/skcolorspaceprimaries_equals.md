To the `SkColorSpacePrimaries` class, add `operator==` and `operator!=`.

Because some software has defined this outside of Skia, add the preprocessor
define `SKIA_COLOR_SPACE_PRIMARIES_OPERATOR_EQUAL`, to help with integration.
This preprocessor define will be removed in the future.


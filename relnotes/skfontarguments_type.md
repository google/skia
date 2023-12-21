`SkFontArguments::Palette::Override`'s index member is changing from an `int`
type to `uint16_t` to make the size exact and remove an unneeded
signedness. This avoids platform/compiler-specific size ambiguiity and more
closely matches the OpenType CPAL table.

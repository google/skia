The Fontations SkTypeface backend has a new factory method to create a typeface from `SkData`,
not only from `SkStreamAsset`. The new signature is
`sk_sp<SkTypeface> SkTypeface_Make_Fontations(sk_sp<SkData> fontData, const SkFontArguments& args)`.
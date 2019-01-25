If the GN variable `use_icu` is set and `skia_use_system_icu` is unset, then
the build system will build ICU from the source.  The repository in
`../externals/icu` is a mirror of the Unicode Consortium's repository.

ICU requires a platform-independent data file, which it does not provide in its
repository.  The data file can be built by first building the ICU tools to
process the provided source data, but this is a giant hassle for building ICU
in a cross-platform way (which might necessitate building ICU twice).

When we will update the ICU branch, we will use the `update_icu.sh` script up
update the `icu.gni` file and produce a new `.dat` file for upload.




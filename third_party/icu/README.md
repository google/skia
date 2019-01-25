# ICU

If the GN variable `use_icu` is set and `skia_use_system_icu` is unset, then
the build system will build ICU from the source.  The repository in
`../externals/icu` is a mirror of the Unicode Consortium's repository.

ICU requires a platform-independent data file, which it does not provide in its
repository.  The data file can be built by first building the ICU tools to
process the provided source data, but this is a giant hassle for building ICU
in a cross-platform way (which might necessitate building ICU twice).

icu/BUILD.gn has two action build rules: `get_data` and `assemble_data`.
`get_data` downloads the `icu.dat` file if it is not already downloaded.  The
Google Cloud Storage location of the file is content-addressed with the
`icu_dat_md5` GN variable found in `icu.gni`.

`assemble_data` will produce either a C++ source file (for our WASM build), an
object file (for our Windows builds), or a assembly file (for everything else).
These files will contain a single exported symbol, `icudtNN_dat` which points
at a block of data containing the contents of the `icu.dat` file.

This build also provides the `SkLoadICU` function, which must be called on
Windows for some reason.

## How To Update ICU Dependency

When we update the ICU branch, we will use the `update_icu.sh` script to
update the `icu.gni` file and produce a new `.dat` file for upload.

1)  Edit `DEPS` file.

2)  Run update script:

        third_party/icu/update_icu.sh

3)  Upload new data file.  `DATA_FILE` and `GS_URL` will be provided by the
    output of the previous step.

        gsutil cp DATA_FILE GS_URL

4)  Commit:

        git add DEPS third_party/icu/icu.gni
        git commit -m 'DEPS: update ICU'
        git push origin @:refs/for/master
        bin/sysopen https://review.skia.org/$(bin/gerrit-number @)


If the GN variable `use_icu` is set and `skia_use_system_icu` is unset, then
the build system will build ICU from the source.  The repository in
`../externals/icu` is a mirror of the Unicode Consortium's repository.

ICU requires a platform-independent data file, which it does not provide in its
repository.  The data file can be built by first building the ICU tools to
process the provided source data, but this is a giant hassle for building ICU
in a cross-platform way (which might necessitate building ICU twice).

##How To Update ICU Dependency

When we update the ICU branch, we will use the `update_icu.sh` script up
update the `icu.gni` file and produce a new `.dat` file for upload.

1)  Edit `DEPS` file.

2)  Run update script:

        third_party/icu/update_icu.sh

3)  Upload now data file.  `DATA_FILE` and `GS_URL` will be provided by the
    output of the previous step.

         gsutil cp DATA_FILE GS_URL

4)  Commit:

        git add DEPS third_party/icu/icu.gni
        git commit -m 'DEPS: update ICU'
        git push origin @:refs/for/master
        bin/sysopen https://review.skia.org/$(bin/gerrit-number @)


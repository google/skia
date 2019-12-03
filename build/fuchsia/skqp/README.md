# Building SKQP for Fuchsia
Using an arm64 device as an example, to build skqp for Fuchsia:
```
gn gen out/fuchsia-arm64 --args="is_official_build=false is_debug=false skia_update_fuchsia_sdk=true target_cpu=\"arm64\" target_os=\"fuchsia\" using_fuchsia_sdk=true skia_tools_require_resources=true skia_skqp_global_error_tolerance=8"
```

The effect of `skia_update_fuchsia_sdk=true` is that it will download both the Fuchsia SDK and a compatible clang for building SKQP as part of the `gn gen ...` step above.

Next step, compile skqp for Fuchsia:
```
autoninja -C out/fuchsia-arm64 ":skqp_repo"
```

The effect of this build will be to produce a Fuchsia package repository named `skqp_repo` in the `out` directory of the build.  `skqp_repo` can then be served to a Fuchsia device using `//fuchsia/sdk/tools/pm serve -repo skqp_repo` (where // is the skia build root).

See [install fuchsia packages](https://fuchsia.dev/fuchsia-src/development/sdk/documentation/packages#install-package) for more on serving packages to Fuchsia devices.

# Fuchsia CIPD Package Creation and Upload Procedure
These steps assume the creation of the arm64 CIPD package as an example.  Because the package requires a path from the output directory of the build, the `gn gen` arguments must match the prescribed path declared in `cipd_arm64.yaml` in order for this CIPD package creation and upload to succeed.

## Create CIPD Package
```
cipd create -pkg-def=cipd_arm64.yaml
```

## Set CIPD Ref of `latest`
If applicable, set `latest` ref to new CIPD package.
```
cipd set-ref skia/fuchsia/skqp/arch/arm64 -ref latest -version mdhS7sryb2zxQuXT803Dv_XZ0r7B5j8jSbZmIi0JvOcC
```

## Set the git-commit Tag
```
cipd set-tag skia/fuchsia/skqp/arch/arm64 -tag=git-commit:9c2b7cfe9080c6c4692234667a671db216a2e229 -version mdhS7sryb2zxQuXT803Dv_XZ0r7B5j8jSbZmIi0JvOcC
```

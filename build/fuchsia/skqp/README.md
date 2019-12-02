# CIPD Package Creation and Upload Procedure
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

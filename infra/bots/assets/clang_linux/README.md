This has the clang compiler and other tools (e.g. clang-tidy and IWYU) built from source for
Linux hosts.

This is used both by our GN build and Bazel builds. The GN build will be updated automatically
after updating this asset with the `sk` tool.

To manually update the Bazel build:
  1) Download the latest version from CIPD as a .zip file.
     <https://chrome-infra-packages.appspot.com/p/skia/bots/clang_linux>
  2) Change the file name to be clang_linux_amd64_vNN.zip where NN is the new version in the
     `VERSION` file.
  3) Upload it to the GCS mirror bucket
     `go run ./bazel/gcs_mirror/gcs_mirror.go --sha256 <hash> --file /path/to/clang_linux_amd64_vNN.zip`
  4) Update the sha256 in `//toolchain/download_linux_amd64_toolchain.bzl`.
  5) Test it locally with `make -C bazel known_good_builds`.
     You may need to update the generated BUILD.bazel file to accommodate relocated/new files
     as well as `//toolchain/linux_amd64_toolchain_config.bzl` to use them correctly.

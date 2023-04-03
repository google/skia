// This file is copied from the SkCMS repository. Original file:
// https://skia.googlesource.com/skcms/+/ba39d81f9797aa973bdf01aa6b0363b280352fba/toolchain/android_trampolines/gen_trampolines/gen_trampolines.go
//
// Helper program to generate trampoline scripts for NDK tools.
//
// This program is meant to be run by hand when making changes to the hermetic Android NDK
// toolchain, e.g. when upgrading to a new Android NDK version.
//
// Trampoline scripts are necessary because the `cc_common.create_cc_toolchain_config_info`[1]
// built-in Bazel function expects tool paths to point to files under the directory in which it is
// invoked, thus we cannot directly reference tools under `external/ndk_linux_amd64`. The solution is
// to use trampoline scripts that pass through any command-line arguments to the NDK binaries under
// `external/android_sdk`.
//
// [1] https://bazel.build/rules/lib/cc_common#create_cc_toolchain_config_info
package main

import (
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

const bazelNdkPath = "external/ndk_linux_amd64"

// Paths relative to the Android NDK root directory. These paths can be determined by inspecting
// the Android NDK ZIP file downloaded by the `download_toolchains` macro defined in
// //toolchains/download_toolchains.bzl.
var tools = []string{
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar",
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-dwp",
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ld",
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-nm",
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-objcopy",
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-objdump",
	"toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-strip",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-dwp",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-ld",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-nm",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-objcopy",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-objdump",
	"toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip",
	"toolchains/llvm/prebuilt/linux-x86_64/bin/clang",
}

const trampolineScriptTemplate = `#!/bin/sh
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

%s $@
`

func main() {
	ndkDirFlag := flag.String("ndk-dir", "", "Path to a local copy of the NDK. Used only to verify that the tool paths assumed by this program are valid. Required.")
	outDirFlag := flag.String("out-dir", "", "Directory where to save the trampoline scripts. Required.")
	flag.Parse()

	if *ndkDirFlag == "" || *outDirFlag == "" {
		flag.Usage()
		os.Exit(1)
	}

	for _, tool := range tools {
		// Verify that the tool exists in the NDK.
		ndkPath := filepath.Join(*ndkDirFlag, tool)
		if _, err := os.Stat(ndkPath); errors.Is(err, os.ErrNotExist) {
			fmt.Fprintf(os.Stderr, "File %s not found.", ndkPath)
			os.Exit(1)
		}

		// Generate trampoline script.
		trampolineScript := fmt.Sprintf(trampolineScriptTemplate, filepath.Join(bazelNdkPath, tool))
		trampolineScriptPath := filepath.Join(*outDirFlag, filepath.Base(tool)+".sh")
		if err := os.WriteFile(trampolineScriptPath, []byte(trampolineScript), 0750); err != nil {
			fmt.Fprintf(os.Stderr, "Error writing file %s: %s", trampolineScriptPath, err)
		}
	}
}

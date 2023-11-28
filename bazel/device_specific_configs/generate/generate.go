// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This program generates file //bazel/devicesrc.

//go:generate bazelisk run //bazel/device_specific_configs/generate -- --output-file ${PWD}/../../devicesrc

package main

import (
	"flag"
	"fmt"
	"os"
	"sort"
	"strings"

	"go.skia.org/skia/bazel/device_specific_configs"
)

const header = "# GENERATED FILE - Please do not edit.\n\n"

func writeFlag(sb *strings.Builder, configName, flagName, flagValue string) {
	_, _ = sb.WriteString(fmt.Sprintf("test:%s %s=%s\n", configName, flagName, flagValue))
}

func writeTestArgFlag(sb *strings.Builder, configName, testArgFlag string) {
	_, _ = sb.WriteString(fmt.Sprintf("test:%s --test_arg=%s\n", configName, testArgFlag))
}

func writeDeviceFlagsFile(outputFile string) error {
	// Sort for determinism.
	var configNames []string
	for configName := range device_specific_configs.Configs {
		configNames = append(configNames, configName)
	}
	sort.Strings(configNames)

	var sb strings.Builder
	_, _ = sb.WriteString(header)

	for i, configName := range configNames {
		if i > 0 {
			sb.WriteString("\n")
		}

		// Force device-specific tests to run locally (as opposed to on RBE). For such tests, we assume
		// Bazel is running on the device under test, or on a machine that controls the device under
		// test (e.g. an Android device attached via USB). Compilation still happens on RBE.
		//
		// We force local execution via the --strategy flag[1]. In order to understand the --strategy
		// flag, we must first understand the --spawn_strategy flag[2], which controls where and how
		// commands are executed. For example:
		//
		//  - Flag --spawn_strategy=sandboxed executes commands inside a sandbox on the local system.
		//    This is the default Bazel behavior on systems that support sandboxing.
		//
		//  - Flag --spawn_strategy=local executes commands as regular, local subprocesses without any
		//    sandboxing.
		//
		//  - Flag --spawn_strategy=remote executes commands remotely, provided a remote executor has
		//    been configured. We use this strategy when running Bazel with --config=remote. See the
		//    //.bazelrc file[3].
		//
		// The --strategy flag allows us to override --spawn_strategy on a per-mnemonic basis. In our
		// case, we set --strategy=TestRunner=local to force test actions to run as a local subprocess.
		// In combination with --config=remote (or any configuration that implies it, such as
		// --config=linux_rbe) this has the effect of running test actions locally, while build actions
		// (and any other actions) run on RBE.
		//
		// The "TestRunner" mnemonic for test actions is determined here[4].
		//
		// [1] https://bazel.build/docs/user-manual#strategy
		// [2] https://bazel.build/docs/user-manual#spawn-strategy
		// [3] https://skia.googlesource.com/skia/+/e5c37860c792de6bba0c9465c3f5280cb13dbbb9/.bazelrc#128
		// [4] https://github.com/bazelbuild/bazel/blob/f79ca0275e14d7c8fb478bd910ad7fb127440fd8/src/main/java/com/google/devtools/build/lib/analysis/test/TestRunnerAction.java#L107
		writeFlag(&sb, configName, "--strategy", "TestRunner=local")

		config := device_specific_configs.Configs[configName]
		for _, arg := range config.TestRunnerArgs() {
			writeTestArgFlag(&sb, configName, arg)
		}
	}

	return os.WriteFile(outputFile, []byte(sb.String()), 0644)
}

func main() {
	outputFileFlag := flag.String("output-file", "", "Path to the output file.")
	flag.Parse()

	if *outputFileFlag == "" {
		fmt.Println("Flag --output-file is required.")
		os.Exit(1)
	}

	if err := writeDeviceFlagsFile(*outputFileFlag); err != nil {
		fmt.Printf("error: %s\n", err)
		os.Exit(1)
	}
}

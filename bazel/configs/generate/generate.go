// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This program generates file //bazel/configs/devicesrc.

//go:generate bazelisk run //bazel/configs/generate -- --output-file ${PWD}/../devicesrc

package main

import (
	"flag"
	"fmt"
	"os"
	"sort"
	"strings"

	"go.skia.org/skia/bazel/configs"
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
	for configName := range configs.DeviceSpecificBazelConfigs {
		configNames = append(configNames, configName)
	}
	sort.Strings(configNames)

	var sb strings.Builder
	_, _ = sb.WriteString(header)

	for i, configName := range configNames {
		if i > 0 {
			sb.WriteString("\n")
		}

		// Force device-specific tests to run locally. We assume Bazel is running on the device under
		// test, or that the device under test is attached to the machine where Bazel is running (e.g.
		// an Android device attached via USB).
		//
		// See https://bazel.build/docs/user-manual#strategy.
		writeFlag(&sb, configName, "--strategy", "TestRunner=local")

		config := configs.DeviceSpecificBazelConfigs[configName]

		// Sort keys for determinism.
		var keys []string
		for key := range config.Keys {
			keys = append(keys, key)
		}
		sort.Strings(keys)

		// Add key/value pairs.
		writeTestArgFlag(&sb, configName, "--key")
		for _, key := range keys {
			writeTestArgFlag(&sb, configName, key)
			writeTestArgFlag(&sb, configName, config.Keys[key])
		}

		if config.CPU != "" {
			writeTestArgFlag(&sb, configName, "--cpuName")
			writeTestArgFlag(&sb, configName, config.CPU)
		}

		if config.GPU != "" {
			writeTestArgFlag(&sb, configName, "--gpuName")
			writeTestArgFlag(&sb, configName, config.GPU)
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

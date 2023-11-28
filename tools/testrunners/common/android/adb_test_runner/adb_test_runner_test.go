// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestParseTestRunnerExtraArgsFlag(t *testing.T) {
	test := func(name, rawFlag, expectedTestRunnerExtraArgs, expectedDeviceSpecificBazelConfig string) {
		t.Run(name, func(t *testing.T) {
			testRunnerExtraArgs, deviceSpecificBazelConfig := parseTestRunnerExtraArgsFlag(rawFlag)
			assert.Equal(t, expectedTestRunnerExtraArgs, testRunnerExtraArgs)
			assert.Equal(t, expectedDeviceSpecificBazelConfig, deviceSpecificBazelConfig)
		})
	}

	test(
		"no device-specific config",
		"--foo aaa bbb --bar ccc",
		"--foo aaa bbb --bar ccc",
		"")

	test(
		"device-specific config as single flag",
		"--foo aaa bbb --device-specific-bazel-config=SomeDevice --bar ccc",
		"--foo aaa bbb --bar ccc",
		"SomeDevice")

	test(
		"device-specific config as single flag with extra spaces",
		"--foo aaa bbb     --device-specific-bazel-config=SomeDevice     --bar ccc",
		"--foo aaa bbb --bar ccc",
		"SomeDevice")

	test("device-specific config as two flags",
		"--foo aaa bbb --device-specific-bazel-config SomeDevice --bar ccc",
		"--foo aaa bbb --bar ccc",
		"SomeDevice")

	test("device-specific config as two flags with extra spaces",
		"--foo aaa bbb     --device-specific-bazel-config     SomeDevice     --bar ccc",
		"--foo aaa bbb --bar ccc",
		"SomeDevice")
}

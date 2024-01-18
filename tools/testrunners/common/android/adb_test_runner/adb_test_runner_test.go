// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
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

func TestRunTest_GMOrUnitTest_Success(t *testing.T) {
	outputDir := t.TempDir()

	// The below adb commands prepare the device to run a GM or unit test, which the test runner
	// refers to as "performance tests".
	//
	// Unlike benchmark tests, performance tests are not sensitive to variations in the device's
	// performance. For example, it is OK for the device to run as fast as possible and potentially
	// throttle the CPU if it runs too hot. Thus, performance tests use a CPU governor that optimizes
	// for speed. They also ensure that all CPU cores are enabled, which is important because some of
	// them could have been disabled by a previous benchmark test.
	commandCollector, assertCollectedAllExpectedCommands := makeCommandCollector(t,
		// Clean up device from previous test run.
		cmdAndArgs("adb", "shell", "su", "root", "rm", "-rf", "/sdcard/bazel-adb-test.tar.gz", "/data/bazel-adb-test", "/sdcard/bazel-adb-test-output-dir"),
		// Enable CPU cores that might have been disabled by an earlier benchmark test.
		cmdAndArgs("adb", "root"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu4/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", `echo "1" > /sys/devices/system/cpu/cpu4/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu4/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu5/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", `echo "1" > /sys/devices/system/cpu/cpu5/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu5/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu6/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", `echo "1" > /sys/devices/system/cpu/cpu6/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu6/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu7/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", `echo "1" > /sys/devices/system/cpu/cpu7/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu7/online").withStdout("1\n"),
		// Set CPU governor.
		cmdAndArgs("adb", "shell", `echo "performance" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor").withStdout("performance\n"),
		// Push and extract test.
		cmdAndArgs("adb", "push", "path/to/archive.tar.gz", "/sdcard/bazel-adb-test.tar.gz"),
		cmdAndArgs("adb", "shell", "su", "root", "mkdir", "-p", "/data/bazel-adb-test"),
		cmdAndArgs("adb", "shell", "su", "root", "tar", "xzvf", "/sdcard/bazel-adb-test.tar.gz", "-C", "/data/bazel-adb-test"),
		// Create output directory.
		cmdAndArgs("adb", "shell", "su", "root", "mkdir", "-p", "/sdcard/bazel-adb-test-output-dir"),
		// Run test.
		cmdAndArgs("adb", "shell", "su", "root").withStdin(
			"cd /data/bazel-adb-test && "+
				"ADB_TEST_OUTPUT_DIR=/sdcard/bazel-adb-test-output-dir "+
				"path/to/testrunner --test-runner-fake-arg value",
		),
		// Pull test outputs.
		cmdAndArgs("adb", "pull", "/sdcard/bazel-adb-test-output-dir", outputDir),
		// Reboot device.
		cmdAndArgs("adb", "reboot"),
		cmdAndArgs("adb", "wait-for-device"),
		// Clean up device.
		cmdAndArgs("adb", "shell", "su", "root", "rm", "-rf", "/sdcard/bazel-adb-test.tar.gz", "/data/bazel-adb-test", "/sdcard/bazel-adb-test-output-dir"),
	)

	// Populate the output dir with the results of pulling output files from the device.
	require.NoError(t, os.Mkdir(filepath.Join(outputDir, "bazel-adb-test-output-dir"), 0755))
	require.NoError(t, os.WriteFile(filepath.Join(outputDir, "bazel-adb-test-output-dir", "fake-output-1.txt"), []byte("Contents do not matter"), 0644))
	require.NoError(t, os.WriteFile(filepath.Join(outputDir, "bazel-adb-test-output-dir", "fake-output-2.txt"), []byte("Contents do not matter"), 0644))

	ctx := exec.NewContext(context.Background(), commandCollector.Run)
	// Pixel 6 was chosen because it exercises the behavior where certain CPU cores are re-enabled
	// after potentially being disabled by an earlier benchmark test.
	err := runTest(ctx, "Pixel6", false /* =isBenchmarkTest */, "path/to/archive.tar.gz", "path/to/testrunner", "--test-runner-fake-arg value", outputDir)
	require.NoError(t, err)

	assertCollectedAllExpectedCommands()

	// Assert that the expected files were produced.
	entries, err := os.ReadDir(outputDir)
	require.NoError(t, err)
	require.Len(t, entries, 2)
	assert.Equal(t, "fake-output-1.txt", entries[0].Name())
	assert.False(t, entries[0].IsDir())
	assert.Equal(t, "fake-output-2.txt", entries[1].Name())
	assert.False(t, entries[1].IsDir())
}

func TestRunTest_BenchmarkTest_Success(t *testing.T) {
	outputDir := t.TempDir()

	// The below adb commands prepare the device to run a benchmark test.
	//
	// For devices with multiple kinds of CPU cores (e.g. big.LITTLE), it's important to only enable
	// one kind of CPU, or the test could alternate between CPU core kinds across subsequent test
	// runs and produce noisy benchmarks. It's also important to use a CPU governor that will prevent
	// the cores from throttling, which can also cause noisy results.
	commandCollector, assertCollectedAllExpectedCommands := makeCommandCollector(t,
		// Clean up device from previous test run.
		cmdAndArgs("adb", "shell", "su", "root", "rm", "-rf", "/sdcard/bazel-adb-test.tar.gz", "/data/bazel-adb-test", "/sdcard/bazel-adb-test-output-dir"),
		// Set CPU governor.
		cmdAndArgs("adb", "root"),
		cmdAndArgs("adb", "shell", `echo "powersave" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor").withStdout("powersave\n"),
		// Disable CPUs.
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu4/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", `echo "0" > /sys/devices/system/cpu/cpu4/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu4/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu5/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", `echo "0" > /sys/devices/system/cpu/cpu5/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu5/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu6/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", `echo "0" > /sys/devices/system/cpu/cpu6/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu6/online").withStdout("0\n"),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu7/online").withStdout("1\n"),
		cmdAndArgs("adb", "shell", `echo "0" > /sys/devices/system/cpu/cpu7/online`),
		cmdAndArgs("adb", "shell", "cat /sys/devices/system/cpu/cpu7/online").withStdout("0\n"),
		// Push and extract test.
		cmdAndArgs("adb", "push", "path/to/archive.tar.gz", "/sdcard/bazel-adb-test.tar.gz"),
		cmdAndArgs("adb", "shell", "su", "root", "mkdir", "-p", "/data/bazel-adb-test"),
		cmdAndArgs("adb", "shell", "su", "root", "tar", "xzvf", "/sdcard/bazel-adb-test.tar.gz", "-C", "/data/bazel-adb-test"),
		// Create output directory.
		cmdAndArgs("adb", "shell", "su", "root", "mkdir", "-p", "/sdcard/bazel-adb-test-output-dir"),
		// Run test.
		cmdAndArgs("adb", "shell", "su", "root").withStdin(
			"cd /data/bazel-adb-test && "+
				"ADB_TEST_OUTPUT_DIR=/sdcard/bazel-adb-test-output-dir "+
				"path/to/testrunner --test-runner-fake-arg value",
		),
		// Pull test outputs.
		cmdAndArgs("adb", "pull", "/sdcard/bazel-adb-test-output-dir", outputDir),
		// Reboot device.
		cmdAndArgs("adb", "reboot"),
		cmdAndArgs("adb", "wait-for-device"),
		// Clean up device.
		cmdAndArgs("adb", "shell", "su", "root", "rm", "-rf", "/sdcard/bazel-adb-test.tar.gz", "/data/bazel-adb-test", "/sdcard/bazel-adb-test-output-dir"),
	)

	// Populate the output dir with the results of pulling output files from the device.
	require.NoError(t, os.Mkdir(filepath.Join(outputDir, "bazel-adb-test-output-dir"), 0755))
	require.NoError(t, os.WriteFile(filepath.Join(outputDir, "bazel-adb-test-output-dir", "fake-output-1.txt"), []byte("Contents do not matter"), 0644))
	require.NoError(t, os.WriteFile(filepath.Join(outputDir, "bazel-adb-test-output-dir", "fake-output-2.txt"), []byte("Contents do not matter"), 0644))

	ctx := exec.NewContext(context.Background(), commandCollector.Run)
	// Pixel 6 was chosen because it exercises the behavior where certain CPU cores are disabled for
	// benchmark tests.
	err := runTest(ctx, "Pixel6", true /* =isBenchmarkTest */, "path/to/archive.tar.gz", "path/to/testrunner", "--test-runner-fake-arg value", outputDir)
	require.NoError(t, err)

	assertCollectedAllExpectedCommands()

	// Assert that the expected files were produced.
	entries, err := os.ReadDir(outputDir)
	require.NoError(t, err)
	require.Len(t, entries, 2)
	assert.Equal(t, "fake-output-1.txt", entries[0].Name())
	assert.False(t, entries[0].IsDir())
	assert.Equal(t, "fake-output-2.txt", entries[1].Name())
	assert.False(t, entries[1].IsDir())
}

// commandAndResult represents and expected command and its mocked output.
type commandAndResult struct {
	// cmdAndArgs is the expected command and args
	cmdAndArgs []string

	// stdin is the expected standard input, if any.
	stdin string

	// fakeStdout is the command's simulated fakeStdout.
	fakeStdout string
}

// withStdin sets a commandAndResult's stdin and returns the resulting struct.
func (c commandAndResult) withStdin(stdin string) commandAndResult {
	c.stdin = stdin
	return c
}

// withStdout sets a commandAndResult's stdout and returns the resulting struct.
func (c commandAndResult) withStdout(stdout string) commandAndResult {
	c.fakeStdout = stdout
	return c
}

// cmdAndArgs returns a commandAndResult with the given command and arguments.
func cmdAndArgs(cmd string, args ...string) commandAndResult {
	return commandAndResult{
		cmdAndArgs: append([]string{cmd}, args...),
	}
}

// makeCommandCollector takes a list of expected commands and returns a CommandCollector that
// emulates the commands in order, and produces a test failure if any of the commands it collects
// do not match the expected commands. It also returns a function that asserst that all expected
// commands were collected.
func makeCommandCollector(t *testing.T, expectedCommands ...commandAndResult) (*exec.CommandCollector, func()) {
	collector := &exec.CommandCollector{}
	curCmdIdx := 0

	collector.SetDelegateRun(func(ctx context.Context, cmd *exec.Command) error {
		defer func() { curCmdIdx++ }()

		// We assert, rather than require, because the latter would mask any subsequent errors in the
		// test.
		assert.Less(t, curCmdIdx, len(expectedCommands), "Command and args: %q", strings.Join(append([]string{cmd.Name}, cmd.Args...), " "))
		if curCmdIdx >= len(expectedCommands) {
			return nil
		}

		assert.Equal(t, expectedCommands[curCmdIdx].cmdAndArgs[0], cmd.Name, "Command with index %d", curCmdIdx)
		assert.EqualValues(t, expectedCommands[curCmdIdx].cmdAndArgs[1:], cmd.Args, "Command with index %d", curCmdIdx)
		if expectedCommands[curCmdIdx].stdin != "" {
			bytes, err := io.ReadAll(cmd.Stdin)
			require.NoError(t, err)
			assert.Equal(t, expectedCommands[curCmdIdx].stdin, string(bytes))
		}

		_, err := fmt.Fprintf(cmd.CombinedOutput, expectedCommands[curCmdIdx].fakeStdout)
		require.NoError(t, err)
		return nil
	})

	assertCollectedAllExpectedCommands := func() {
		assert.Equal(t, curCmdIdx, len(expectedCommands), "Number of collected commands does not match the number of expected commands")
	}

	return collector, assertCollectedAllExpectedCommands
}

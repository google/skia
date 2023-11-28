// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This program is the brains behind the adb_test Bazel rule, which allows running a test on an
// Android device via adb.

package main

import (
	"bytes"
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
	"time"

	"go.skia.org/skia/bazel/device_specific_configs"
)

const (
	// timeout for this program.
	timeout = time.Hour

	// adbTestOutputDirEnvVar is the environment variable that tells the test running on device where
	// to write output files, if any.
	adbTestOutputDirEnvVar = "ADB_TEST_OUTPUT_DIR"
)

func main() {
	deviceSpecificBazelConfigFlag := flag.String("device-specific-bazel-config", "", "The Bazel config corresponding to this Android device (see //bazel/devicesrc).")
	benchmarkFlag := flag.Bool("benchmark", false, "Whether this is a benchmark terst or not. Might affect CPU/GPU settings, etc.")
	archiveFlag := flag.String("archive", "", "Tarball with the payload to upload to the device under test.")
	testRunnerFlag := flag.String("test-runner", "", "Path to the test runner inside the tarball.")
	// Some context regarding the parsing step mentioned in this flag's help text:
	//
	//  - The adb_test Bazel rule produces a Bash script that invokes this Go program with various
	//    flags. These flags can be divided into two groups: those that are determined when the
	//    adb_test target is built, which are hardcoded in the script; and those that are determined
	//    at runtime, which the script should set based on its own command-line arguments.
	//
	//  - The only two flags determined at runtime are --device-specific-bazel-config and
	//    --test-runner-extra-args. The first should be set with the value of the
	//    --device-specific-bazel-config flag passed to the script, while the second should be set to
	//    a sepace-separated string with any other command-line arguments passed to the script.
	//
	//  - Ideally, we would want the script to parse its own command-line arguments and set
	//    --device-specific-bazel-config and --test-runner-extra-args as described in the previous
	//    paragraph. However, parsing flags in Bash is awkward, and the resulting logic is hard to
	//    test.
	//
	//  - Instead, the script simply sets flag --test-runner-extra-args to a space-separated string
	//    with all command-line arguments it receives, and this Go program parses out flag
	//    --device-specific-bazel-config from said space-separated string.
	testRunnerExtraArgsFlag := flag.String("test-runner-extra-args", "", "Any extra command-line arguments to pass to the test runner inside the tarball. Note that if this string contains a --device-specific-bazel-config=<config name> flag, it will be omitted from the test runner's arguments, and <config name> will override this program's --device-specific-bazel-config flag.")
	outputDirFlag := flag.String("output-dir", "", "Path on the host machine where to write any outputs produced by the test.")
	flag.Parse()

	var quotedArgs []string
	for _, arg := range os.Args[1:] {
		quotedArgs = append(quotedArgs, fmt.Sprintf("%q", arg))
	}
	log("adb_test_runner invoked with arguments: %s", strings.Join(quotedArgs, " "))

	testRunnerExtraArgs, deviceSpecificBazelConfigName := parseTestRunnerExtraArgsFlag(*testRunnerExtraArgsFlag)
	if deviceSpecificBazelConfigName != "" {
		deviceSpecificBazelConfigFlag = &deviceSpecificBazelConfigName
	}

	if *deviceSpecificBazelConfigFlag == "" {
		die("Flag --device-specific-bazel-config is required.\n")
	}
	if *archiveFlag == "" {
		die("Flag --archive is required.\n")
	}
	if *testRunnerFlag == "" {
		die("Flag --test-runner is required.\n")
	}

	// Fail early if the output directory on the host machine is not empty or if it's non-writable.
	if *outputDirFlag != "" {
		// Check whether the directory exists.
		fileInfo, err := os.Stat(*outputDirFlag)
		if err != nil {
			die("while stating output dir %q: %s\n", *outputDirFlag, err)
		}
		if !fileInfo.IsDir() {
			die("output dir %q is not a directory.\n", *outputDirFlag)
		}

		// Check whether the directory is empty.
		entries, err := os.ReadDir(*outputDirFlag)
		if err != nil {
			die("while listing the contents of output dir %q: %s\n", *outputDirFlag, err)
		}
		if len(entries) != 0 {
			die("output dir %q is not empty.\n", *outputDirFlag)
		}

		// Check whether the directory is writable by creating and then removing an empty file.
		testFile := filepath.Join(*outputDirFlag, "test")
		if err := os.WriteFile(testFile, []byte{}, 0644); err != nil {
			die("while writing test file %q in output dir: %s\n", testFile, err)
		}
		if err := os.Remove(testFile); err != nil {
			die("while deleting test file %q in output dir: %s\n", testFile, err)
		}
	}

	deviceSpecificBazelConfig, ok := device_specific_configs.Configs[*deviceSpecificBazelConfigFlag]
	if !ok {
		die("Unknown device-specific Bazel config: %q\n", *deviceSpecificBazelConfigFlag)
	}

	ctx, cancelFn := context.WithTimeout(context.Background(), timeout)
	defer cancelFn()
	if err := runTest(ctx, deviceSpecificBazelConfig.Model(), *benchmarkFlag, *archiveFlag, *testRunnerFlag, testRunnerExtraArgs, *outputDirFlag); err != nil {
		die("%s\n", err)
	}
}

var deviceSpecificBazelConfigFlagRegexp = regexp.MustCompile(`\s*--device-specific-bazel-config(?:=|\s+)(?P<configName>[a-zA-Z0-9_-]+)\s*`)

// parseTestRunnerExtraArgsFlag takes the raw --test-runner-extra-args flag, which might contain a
// --device-specific-bazel-config=<config name> argument, and returns the former without the latter
// and the <config name>.
func parseTestRunnerExtraArgsFlag(rawTestRunnerExtraArgsFlag string) (testRunnerExtraArgs string, deviceSpecificBazelConfig string) {
	match := deviceSpecificBazelConfigFlagRegexp.FindStringSubmatch(rawTestRunnerExtraArgsFlag)
	if len(match) > 0 {
		deviceSpecificBazelConfig = match[deviceSpecificBazelConfigFlagRegexp.SubexpIndex("configName")]
		testRunnerExtraArgs = strings.ReplaceAll(rawTestRunnerExtraArgsFlag, match[0], " ")
	} else {
		testRunnerExtraArgs = rawTestRunnerExtraArgsFlag
	}
	return
}

// runTest runs the test on device via adb.
func runTest(ctx context.Context, model string, benchmark bool, archive, testRunner, testRunnerExtraArgs, outputDir string) error {
	// TODO(lovisolo): Add any necessary device-specific setup steps such as turning cores on/off and
	//                 setting the CPU/GPU frequencies, taking into account whether or not the test
	//                 is a benchmark.

	// TODO(lovisolo): Should we check that the machine is attached to the expected device type?
	//                 E.g. run "adb devices -l" and check that the output contains
	//                 "model:Pixel_5". What happens if there are more than one device?

	// Clean up the device before running the test. Previous tests might have left the device in a
	// dirty state.
	cleanUpDevice := func(model string) error {
		return adb(ctx, "shell", "su", "root", "rm", "-rf", getArchivePathOnDevice(model), getArchiveExtractionDirOnDevice(model), getOutputDirOnDevice(model))
	}
	if err := cleanUpDevice(model); err != nil {
		return fmt.Errorf("while cleaning up the device before running the test: %s", err)
	}

	// Also clean up device after running the test.
	defer func() {
		if err := cleanUpDevice(model); err != nil {
			die(fmt.Sprintf("while cleaning up the device after running the test: %s\n", err))
		}
	}()

	// Upload archive to device.
	if err := adb(ctx, "push", archive, getArchivePathOnDevice(model)); err != nil {
		return fmt.Errorf("while pushing archive to device: %s", err)
	}

	// Extract archive.
	if err := adb(ctx, "shell", "su", "root", "mkdir", "-p", getArchiveExtractionDirOnDevice(model)); err != nil {
		return fmt.Errorf("while creating archive extraction directory on device: %s", err)
	}
	if err := adb(ctx, "shell", "su", "root", "tar", "xzvf", getArchivePathOnDevice(model), "-C", getArchiveExtractionDirOnDevice(model)); err != nil {
		return fmt.Errorf("while extracting archive on device: %s", err)
	}

	// Create on-device output dir if necessary.
	if outputDir != "" {
		if err := adb(ctx, "shell", "su", "root", "mkdir", "-p", getOutputDirOnDevice(model)); err != nil {
			return fmt.Errorf("while creating output dir on device: %s", err)
		}
	}

	// If necessary, we will tell the test runner where to store output files via an environment
	// variable.
	outputDirEnvVar := ""
	if outputDir != "" {
		outputDirEnvVar = fmt.Sprintf("%s=%s", adbTestOutputDirEnvVar, getOutputDirOnDevice(model))
	}

	// Run test.
	stdin := fmt.Sprintf("cd %s && %s %s %s", getArchiveExtractionDirOnDevice(model), outputDirEnvVar, testRunner, testRunnerExtraArgs)
	if err := adbWithStdin(ctx, stdin, "shell", "su", "root"); err != nil {
		return fmt.Errorf("while running the test: %s", err)
	}

	// Pull output files from the device if necessary.
	if outputDir != "" {
		// This will save the output files to <output dir>/<output dir on device>.
		if err := adb(ctx, "pull", getOutputDirOnDevice(model), outputDir); err != nil {
			return fmt.Errorf("while pulling on-device output dir %q into host output dir %q: %s", getOutputDirOnDevice(model), outputDir, err)
		}

		// But we want the output files to be placed in <output dir>, so we'll move them one by one.
		srcDir := filepath.Join(outputDir, filepath.Base(getOutputDirOnDevice(model)))
		dstDir := outputDir
		entries, err := os.ReadDir(srcDir)
		if err != nil {
			return fmt.Errorf("while reading the contents of output dir %q: %s", outputDir, err)
		}
		for _, entry := range entries {
			oldPath := filepath.Join(srcDir, entry.Name())
			newPath := filepath.Join(dstDir, entry.Name())
			if err := os.Rename(oldPath, newPath); err != nil {
				return fmt.Errorf("while renaming %q to %q: %s", oldPath, newPath, err)
			}
		}

		// Finally, delete the spurious <output dir>/<output dir on device> directory created by
		// "adb pull".
		if err := os.Remove(srcDir); err != nil {
			return fmt.Errorf("while removing directory %q: %s", srcDir, err)
		}
	}

	return nil
}

// getArchivePathOnDevice returns the path in the device's file system where the archive should be
// uploaded.
func getArchivePathOnDevice(model string) string {
	// The /sdcard directory is writable by non-root users, but files in this directory cannot be
	// executed. For this reason, we extract the archive in a directory under /data, which allows
	// executing files but requires root privileges.
	//
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/sdcard/bazel-adb-test.tar.gz"
}

// getArchiveExtractionDirOnDevice returns the directory in the device's file system where the
// archive should be extracted.
func getArchiveExtractionDirOnDevice(model string) string {
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/data/bazel-adb-test"
}

// getOutputDirOnDevice returns the directory in the device's file system where the test should
// write any output files. These files will then be copied from the device to the machine where adb
// is running.
func getOutputDirOnDevice(model string) string {
	// We have tests write output files to a directory under /sdcard, rather than /data, because the
	// /data directory permissions make it impossible to "adb pull" from it.
	//
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/sdcard/bazel-adb-test-output-dir"
}

// adb runs adb with the given arguments.
func adb(ctx context.Context, args ...string) error {
	return adbWithStdin(ctx, "", args...)
}

// adbWithStdin runs adb with the given arguments, and pipes the given input via standard input.
func adbWithStdin(ctx context.Context, stdin string, args ...string) error {
	commandAndArgs := strings.Join(append([]string{"adb"}, args...), " ")
	withStdin := ""
	if stdin != "" {
		withStdin = fmt.Sprintf(" with standard input %q", stdin)
	}
	log("Executing: %q%s...", commandAndArgs, withStdin)

	cmd := exec.CommandContext(ctx, "adb", args...)
	if stdin != "" {
		cmd.Stdin = bytes.NewBufferString(stdin)
	}
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

func log(msg string, a ...interface{}) {
	timestamp := time.Now().Format(time.RFC3339)
	fmtString := "[%s] " + msg + "\n"
	args := append([]interface{}{timestamp}, a...)
	fmt.Printf(fmtString, args...)
}

func die(msg string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, msg, a...)
	os.Exit(1)
}

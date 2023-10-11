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
	"strings"
	"time"
)

const (
	// timeout for this program.
	timeout = time.Hour

	// adbTestOutputDirEnvVar is the environment variable that tells the test running on device where
	// to write output files, if any.
	adbTestOutputDirEnvVar = "ADB_TEST_OUTPUT_DIR"
)

type Device string

const (
	Pixel5 = Device("pixel_5")
	Pixel7 = Device("pixel_7")
)

var AllDevices = []Device{
	// TODO(lovisolo): Add more devices.
	Pixel5,
	Pixel7,
}

func main() {
	deviceFlag := flag.String("device", "", `Device under test, e.g. "pixel_5".`)
	archiveFlag := flag.String("archive", "", "Tarball with the payload to upload to the device under test.")
	testRunnerFlag := flag.String("test-runner", "", "Path to the test runner inside the tarball.")
	outputDirFlag := flag.String("output-dir", "", "Path on the host machine where to write any outputs produced by the test.")
	flag.Parse()

	if *deviceFlag == "" {
		die("Flag --device is required.\n")
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

	var device Device
	for _, d := range AllDevices {
		if *deviceFlag == string(d) {
			device = d
		}
	}
	if device == "" {
		die("Unknown device: %q\n", *deviceFlag)
	}

	ctx, cancelFn := context.WithTimeout(context.Background(), timeout)
	defer cancelFn()
	if err := runTest(ctx, device, *archiveFlag, *testRunnerFlag, *outputDirFlag); err != nil {
		die("%s\n", err)
	}
}

// runTest runs the test on device via adb.
func runTest(ctx context.Context, device Device, archive, testRunner, outputDir string) error {
	// TODO(lovisolo): Add any necessary device-specific setup steps such as turning cores on/off and
	//                 setting the CPU/GPU frequencies.

	// TODO(lovisolo): Should we check that the machine is attached to the expected device type?
	//                 E.g. run "adb devices -l" and check that the output contains
	//                 "model:Pixel_5".

	// Clean up the device before running the test. Previous tests might have left the device in a
	// dirty state.
	cleanUpDevice := func(device Device) error {
		return adb(ctx, "shell", "su", "root", "rm", "-rf", getArchivePathOnDevice(device), getArchiveExtractionDirOnDevice(device), getOutputDirOnDevice(device))
	}
	if err := cleanUpDevice(device); err != nil {
		return fmt.Errorf("while cleaning up the device before running the test: %s", err)
	}

	// Also clean up device after running the test.
	defer func() {
		if err := cleanUpDevice(device); err != nil {
			die(fmt.Sprintf("while cleaning up the device after running the test: %s\n", err))
		}
	}()

	// Upload archive to device.
	if err := adb(ctx, "push", archive, getArchivePathOnDevice(device)); err != nil {
		return fmt.Errorf("while pushing archive to device: %s", err)
	}

	// Extract archive.
	if err := adb(ctx, "shell", "su", "root", "mkdir", "-p", getArchiveExtractionDirOnDevice(device)); err != nil {
		return fmt.Errorf("while creating archive extraction directory on device: %s", err)
	}
	if err := adb(ctx, "shell", "su", "root", "tar", "xzvf", getArchivePathOnDevice(device), "-C", getArchiveExtractionDirOnDevice(device)); err != nil {
		return fmt.Errorf("while extracting archive on device: %s", err)
	}

	// Create on-device output dir if necessary.
	if outputDir != "" {
		if err := adb(ctx, "shell", "su", "root", "mkdir", "-p", getOutputDirOnDevice(device)); err != nil {
			return fmt.Errorf("while creating output dir on device: %s", err)
		}
	}

	// If necessary, we will tell the test runner where to store output files via an environment
	// variable.
	outputDirEnvVar := ""
	if outputDir != "" {
		outputDirEnvVar = fmt.Sprintf("%s=%s", adbTestOutputDirEnvVar, getOutputDirOnDevice(device))
	}

	// Run test.
	stdin := fmt.Sprintf("cd %s && %s %s", getArchiveExtractionDirOnDevice(device), outputDirEnvVar, testRunner)
	if err := adbWithStdin(ctx, stdin, "shell", "su", "root"); err != nil {
		return fmt.Errorf("while running the test: %s", err)
	}

	// Pull output files from the device if necessary.
	if outputDir != "" {
		// This will save the output files to <output dir>/<output dir on device>.
		if err := adb(ctx, "pull", getOutputDirOnDevice(device), outputDir); err != nil {
			return fmt.Errorf("while pulling on-device output dir %q into host output dir %q: %s", getOutputDirOnDevice(device), outputDir, err)
		}

		// But we want the output files to be placed in <output dir>, so we'll move them one by one.
		srcDir := filepath.Join(outputDir, filepath.Base(getOutputDirOnDevice(device)))
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
func getArchivePathOnDevice(device Device) string {
	// The /sdcard directory is writable by non-root users, but files in this directory cannot be
	// executed. For this reason, we extract the archive in a directory under /data, which allows
	// executing files but requires root privileges.
	//
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/sdcard/bazel-adb-test.tar.gz"
}

// getArchiveExtractionDirOnDevice returns the directory in the device's file system where the
// archive should be extracted.
func getArchiveExtractionDirOnDevice(device Device) string {
	// This might change in the future based on the device type, whether or not it's rooted, etc.
	return "/data/bazel-adb-test"
}

// getOutputDirOnDevice returns the directory in the device's file system where the test should
// write any output files. These files will then be copied from the device to the machine where adb
// is running.
func getOutputDirOnDevice(device Device) string {
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
	timestamp := time.Now().Format(time.RFC3339)
	commandAndArgs := strings.Join(append([]string{"adb"}, args...), " ")
	withStdin := ""
	if stdin != "" {
		withStdin = fmt.Sprintf(" with standard input %q", stdin)
	}
	fmt.Printf("[%s] Executing: %q%s...\n", timestamp, commandAndArgs, withStdin)

	cmd := exec.CommandContext(ctx, "adb", args...)
	if stdin != "" {
		cmd.Stdin = bytes.NewBufferString(stdin)
	}
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

func die(msg string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, msg, a...)
	os.Exit(1)
}

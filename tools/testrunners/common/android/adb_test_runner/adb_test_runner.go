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
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"

	"go.skia.org/infra/go/exec"

	"go.skia.org/skia/bazel/device_specific_configs"
	"golang.org/x/exp/slices"
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
	benchmarkFlag := flag.Bool("benchmark", false, "Whether this is a benchmark test or not. The Android device will be tuned to reduce variations in performance for single-threaded tests.")
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

	die := func(msg string, a ...interface{}) {
		printToStdErr(msg, a...)
		os.Exit(1)
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
func runTest(ctx context.Context, model string, isBenchmarkTest bool, archive, testRunner, testRunnerExtraArgs, outputDir string) error {
	// TODO(lovisolo): Should we check that the machine is attached to the expected device type?
	//                 E.g. run "adb devices -l" and check that the output contains
	//                 "model:Pixel_5". What happens if there are more than one device?

	// Clean up the device before running the test. Previous tests might have left the device in a
	// dirty state.
	cleanUpDevice := func(model string) error {
		_, err := adb(ctx, "shell", "su", "root", "rm", "-rf", getArchivePathOnDevice(model), getArchiveExtractionDirOnDevice(model), getOutputDirOnDevice(model))
		return err
	}
	if err := cleanUpDevice(model); err != nil {
		return fmt.Errorf("while cleaning up the device before running the test: %s", err)
	}

	// Also clean up device after running the test.
	defer func() {
		if err := cleanUpDevice(model); err != nil {
			printToStdErr("while cleaning up the device after running the test: %s\n", err)
		}
	}()

	// Reset the device after running the test.
	//
	// Based on
	// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#512.
	//
	// Note that android.py (see link above) used to set to quarantine the Raspberry Pi in case of
	// "infra failures", but we do not do that here because that is incompatible with the use case
	// where a developer runs an Android test on their local workstation.
	defer func() {
		if _, err := adb(ctx, "reboot"); err != nil {
			printToStdErr("while executing \"adb reboot\": %s", err)
			return
		}

		if _, err := adb(ctx, "wait-for-device"); err != nil {
			printToStdErr("while executing \"adb wait-for-device\": %s", err)
		}
	}()

	// Turn CPU cores on/off, set CPU core frequencies, etc.
	if err := scaleDevice(ctx, model, isBenchmarkTest); err != nil {
		return fmt.Errorf("while scaling device: %s", err)
	}

	// Upload archive to device.
	if _, err := adb(ctx, "push", archive, getArchivePathOnDevice(model)); err != nil {
		return fmt.Errorf("while pushing archive to device: %s", err)
	}

	// Extract archive.
	if _, err := adb(ctx, "shell", "su", "root", "mkdir", "-p", getArchiveExtractionDirOnDevice(model)); err != nil {
		return fmt.Errorf("while creating archive extraction directory on device: %s", err)
	}
	if _, err := adb(ctx, "shell", "su", "root", "tar", "xzvf", getArchivePathOnDevice(model), "-C", getArchiveExtractionDirOnDevice(model)); err != nil {
		return fmt.Errorf("while extracting archive on device: %s", err)
	}

	// Create on-device output dir if necessary.
	if outputDir != "" {
		if _, err := adb(ctx, "shell", "su", "root", "mkdir", "-p", getOutputDirOnDevice(model)); err != nil {
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
	if _, err := adbWithStdin(ctx, stdin, "shell", "su", "root"); err != nil {
		return fmt.Errorf("while running the test: %s", err)
	}

	// Pull output files from the device if necessary.
	if outputDir != "" {
		// This will save the output files to <output dir>/<output dir on device>.
		if _, err := adb(ctx, "pull", getOutputDirOnDevice(model), outputDir); err != nil {
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

// scaleDevice scales the CPU of the device as required based on the type of test.
func scaleDevice(ctx context.Context, model string, isBenchmarkTest bool) error {
	// Based on
	// https://skia.googlesource.com/skia/+/5a635f2211ceb7639ceca4200e0094a6ca17111b/infra/bots/recipe_modules/flavor/android.py#151
	// and
	// https://skia.googlesource.com/skia/+/5a635f2211ceb7639ceca4200e0094a6ca17111b/infra/bots/recipe_modules/flavor/android.py#179.
	if doesNotAllowADBRoot(model) {
		return nil
	}

	if err := adbRoot(ctx); err != nil {
		return fmt.Errorf("while executing \"adb root\": %s", err)
	}

	if isBenchmarkTest {
		return scaleDeviceForBenchmark(ctx, model)
	}
	return scaleDeviceForPerformance(ctx, model)
}

// scaleDeviceForPerformance tunes the device's CPUs for performance in order to make tests run as
// fast as possible. Do not use this function for benchmark tests, as it does not guarantee stable
// performance over successive runs.
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#149.
func scaleDeviceForPerformance(ctx context.Context, model string) error {
	// This is paranoia... any CPUs we disabled while running benchmark tests ought to be back online
	// now that we've restarted the device.
	for _, cpu := range getCPUsToDisableForBenchmarkTests(model) {
		if err := enableOrDisableCPU(ctx, model, cpu, true /* =enable */); err != nil {
			return err
		}
	}

	// CPU cores are grouped together by kind. Scaling a single core of the biggest kind has the
	// effect of scaling all cores of that kind.
	cpusToScale := []int{getBiggestKindCPU(model)}
	// For big.LITTLE devices, make sure we also scale the little cores up; there is a chance they
	// are still in powersave mode from when Swarming slows things down for cooling down and
	// charging.
	if !slices.Contains(cpusToScale, 0) {
		cpusToScale = append(cpusToScale, 0)
	}

	for _, cpu := range cpusToScale {
		if err := setCPUGovernor(ctx, model, cpu, getCPUGovernorForPerformanceTests(model)); err != nil {
			return err
		}
	}

	return nil
}

// scaleDeviceForBenchmark tunes the device's CPUs for single-threaded tests, such as benchmark
// tests. It tries to minimize variations in the performance over successive test runs.
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#177.
func scaleDeviceForBenchmark(ctx context.Context, model string) error {
	if err := setCPUGovernor(ctx, model, getBiggestKindCPU(model), getCPUGovernorForBenchmarkTests(model)); err != nil {
		return err
	}

	if model != "Pixel6" && model != "Pixel7" {
		// CPU cores are grouped together by kind. Scaling a single core of the biggest kind has the
		// effect of scaling all cores of that kind.
		if err := scaleCPU(ctx, model, getBiggestKindCPU(model), 0.6); err != nil {
			return err
		}
	}

	for _, cpu := range getCPUsToDisableForBenchmarkTests(model) {
		if err := enableOrDisableCPU(ctx, model, cpu, false /* =enable */); err != nil {
			return err
		}
	}

	return nil
}

// doesNotAllowADBRoot returns true if the ADB daemon (adbd) cannot be restarted as the root user
// on the device (e.g. via "adb root").
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#45.
func doesNotAllowADBRoot(model string) bool {
	return map[string]bool{
		"GalaxyS7_G930FD": true,
		"GalaxyS9":        true,
		"GalaxyS20":       true,
		"P30":             true,
		"Pixel4":          true,
		"Pixel4XL":        true,
		"JioNext":         true,

		// The below device is listed in the "cant_root" list in
		// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#45.
		// However, lovisolo@ did not encounter any issues running "adb root", so we are excluding it
		// from the list to see if anything breaks.
		//
		// "Pixel5":          true,
	}[model]
}

// getBiggestKindCPU returns the ID of a CPU core of the biggest kind.
//
// The reason why this function only returns one CPU ID is that CPUs are grouped together, so it
// suffices to scale one CPU in a group in order to scale them all. As an example, the Nexus 5x has
// a big.LITTLE CPU; it groups the little cores as cpu0-3 and the big cores as cpu4-5. Thus, for
// single-threaded tests such as benchmark tests it makes sense to disable cpu0-3 (i.e. the little
// cores) and scale up just one of the big cores, e.g. cpu4, which has the effect of scaling up all
// big cores (cpu4-5) at the same frequency.
//
// Based on
// https://skia.googlesource.com/skia/+/5a635f2211ceb7639ceca4200e0094a6ca17111b/infra/bots/recipe_modules/flavor/android.py#58.
func getBiggestKindCPU(model string) int {
	// We return 0 if the model is not in the below map, meaning that we assume 0 is the ID of a core
	// of the biggest kind.
	return map[string]int{
		"Nexus5x":  4,
		"Pixel":    2,
		"Pixel2XL": 4,
	}[model]
}

// Returns the Android device's CPU IDs that should be disabled when running single-threaded tests
// such as benchmark tests.
//
// For devices with multiple kinds of cores, such as big.LITTLE cores, we noticed a lot of noise
// that seems to be caused by alternating between the slow and fast cores. We reduce this noise by
// only enabling cores of a given kind.
//
// Based on
// https://skia.googlesource.com/skia/+/5a635f2211ceb7639ceca4200e0094a6ca17111b/infra/bots/recipe_modules/flavor/android.py#70.
func getCPUsToDisableForBenchmarkTests(model string) []int {
	// We return an empty slice if the model is not in the below map.
	return map[string][]int{
		"Nexus5x":  {0, 1, 2, 3},
		"Pixel":    {0, 1},
		"Pixel2XL": {0, 1, 2, 3},
		"Pixel6":   {4, 5, 6, 7}, // Only use the 4 small cores.
		"Pixel7":   {4, 5, 6, 7},
	}[model]
}

// getCPUGovernorForPerformanceTests returns the CPU governor that should be used for performance
// tests.
func getCPUGovernorForPerformanceTests(model string) string {
	switch model {
	// AndroidOne doesn't support the "ondemand" governor, but "hotplug" is similar.
	case "AndroidOne":
		return "hotplug"

	// Pixel3a/4/4a support the "userspace", "powersave", "performance" and "schedutil"
	// governors. The "performance" governor seems like a reasonable choice.
	case "Pixel3a":
		fallthrough
	case "Pixel4":
		fallthrough
	case "Pixel4a":
		fallthrough
	case "Pixel5":
		fallthrough
	case "Wembley":
		fallthrough
	case "Pixel6":
		fallthrough
	case "Pixel7":
		return "performance"

	default:
		return "ondemand"
	}
}

// getCPUGovernorForBenchmarkTests returns the CPU governor that should be used for benchmark
// tests.
func getCPUGovernorForBenchmarkTests(model string) string {
	switch model {
	// Pixel 6 and 7 use the "powersave" CPU governor. This decision was originally made for Pixel 6
	// in
	// https://skia-review.googlesource.com/c/skia/+/500439/14/infra/bots/recipe_modules/flavor/android.py#157,
	// and jcgregorio@ seems to remember that the "userspace" governor did not work for Pixel 6 for
	// some unknown reason.
	case "Pixel6":
		fallthrough
	case "Pixel7":
		return "powersave"

	default:
		return "userspace"
	}
}

// maxAttempts is the number of maximum attempts performed by withRetry.
const maxAttempts = 3

// withRetry runs the given function up to maxAttempts times until it succeeds. It performs device
// recovery steps between failures. If the function fails maxAttempts times, it returns the error
// returned by the last function invocation.
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/run/api.py#81.
func withRetry(ctx context.Context, model, description string, fn func() error) error {
	var err error

	for attempt := 1; attempt <= maxAttempts; attempt++ {
		log("Attempt %d/%d: %s", attempt, maxAttempts, description)

		err = fn()
		if err != nil {
			log("Attempt %d/%d failed with: %s", attempt, maxAttempts, err)

			if attempt == maxAttempts {
				break
			}

			log("Rebooting device and waiting for it to be ready...")
			if err := rebootAndWaitForDevice(ctx, model); err != nil {
				return fmt.Errorf("while rebooting and waiting for device: %s", err)
			}
		} else {
			log("Attempt %d/%d was successful.", attempt, maxAttempts)
			return nil
		}
	}

	return fmt.Errorf("last attempt at %q failed with: %s", description, err)
}

// rebootAndWaitForDevice reboots the device after a failed attempt at performing an action, and
// waits for it to become ready.
//
// Based on https://skia-review.googlesource.com/c/skia/+/631997.
func rebootAndWaitForDevice(ctx context.Context, model string) error {
	// We used to run "adb kill-server" in android.py:
	// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#87.
	// However, this breaks the use case when we are talking to an ADB server port-forwarded from
	// another machine (e.g. a Skolo Raspberry Pi). Specifically, any subsequent "adb" commands fail
	// with "Connection reset by peer" because there is no ADB server listening on the other side of
	// the port-forward.
	//
	// Thus, we use "adb reconnect" instead, which does not kill the ADB server and causes it to
	// reconnect with the device without killing the server. It is unclear to lovisolo@ why the
	// "adb kill-server" step was necessary in the first place, so it might be a good idea to remove
	// this step in the future and see if things still work without it.
	if _, err := adb(ctx, "reconnect"); err != nil {
		return fmt.Errorf("while executing \"adb reconnect\": %s", err)
	}

	if _, err := adb(ctx, "wait-for-device"); err != nil {
		return fmt.Errorf("while executing \"adb wait-for-device\": %s", err)
	}

	if _, err := adb(ctx, "reboot"); err != nil {
		return fmt.Errorf("while executing \"adb reboot\": %s", err)
	}

	// Wait until the boot is actually complete. See https://android.stackexchange.com/a/164050.
	waitForBootComplete := "while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done"
	if _, err := adb(ctx, "wait-for-device", "shell", waitForBootComplete); err != nil {
		return fmt.Errorf("while executing \"adb wait-for-device shell %q\": %s", waitForBootComplete, err)
	}

	if doesNotAllowADBRoot(model) {
		return nil
	}

	if err := adbRoot(ctx); err != nil {
		return fmt.Errorf("while executing \"adb root\": %s", err)
	}

	return nil
}

// adbRoot runs the "adb root" command and checks for errors.
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#212.
func adbRoot(ctx context.Context) error {
	if output, err := adb(ctx, "root"); err != nil {
		return err
	} else if strings.Contains(output, "cannot") {
		// Check for message like "adbd cannot run as root in production builds".
		return fmt.Errorf("output of \"adb root\" contains the word \"cannot\"; full output: %q", output)
	}
	return nil
}

// setCPUGovernor sets the CPU governor of the given CPU.
//
// Based on
// https://skia.googlesource.com/skia/+/5a635f2211ceb7639ceca4200e0094a6ca17111b/infra/bots/recipe_modules/flavor/android.py#251.
func setCPUGovernor(ctx context.Context, model string, cpu int, governor string) error {
	return withRetry(ctx, model, fmt.Sprintf("Setting CPU %d governor to %q", cpu, governor), func() error {
		return writeFileOnDeviceAndAssertContents(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu), governor)
	})
}

// scaleCPU sets the frequency of the given CPU.
//
// Based on
// https://skia.googlesource.com/skia/+/5a635f2211ceb7639ceca4200e0094a6ca17111b/infra/bots/recipe_modules/flavor/android.py#337.
func scaleCPU(ctx context.Context, model string, cpu int, maxFreqFactor float64) error {
	availableFreqs, err := getAvailableCPUFrequencies(ctx, cpu)
	if err != nil {
		return fmt.Errorf("while querying available CPU frequencies: %s", err)
	}

	// Find an available frequency that is close enough to the target frequency.
	//
	// Based on
	// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#374.
	maxFreq := availableFreqs[len(availableFreqs)-1]
	targetFreq := int64(float64(maxFreq) * maxFreqFactor)
	chosenFreq := maxFreq
	for i := len(availableFreqs) - 1; i >= 0; i-- {
		candidateFreq := availableFreqs[i]
		if candidateFreq <= targetFreq {
			chosenFreq = candidateFreq
			break
		}
	}

	// We will try scaling the CPU multiple times. Some devices, especially Nexus 7s, seem to
	// occassionally fail when setting the CPU frequency.
	//
	// See https://skia-review.googlesource.com/c/skia/+/78140.
	return withRetry(ctx, model, fmt.Sprintf("Setting CPU ID %d frequency to %d", cpu, chosenFreq), func() error {
		chosenFreqAsStr := fmt.Sprintf("%d", chosenFreq)

		// Scale the CPU to the chosen frequency.
		//
		// If scaling_max_freq is lower than our chosen frequency, it won't take. We must set
		// scaling_min_freq first, because if we try to set scaling_max_freq to be less than
		// scaling_min_freq (which sometimes happens after certain devices reboot) it returns a
		// perplexing permissions error.
		if err := writeFileOnDevice(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cpu), "0"); err != nil {
			return err
		}
		if err := writeFileOnDevice(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cpu), chosenFreqAsStr); err != nil {
			return err
		}
		if err := writeFileOnDevice(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", cpu), chosenFreqAsStr); err != nil {
			return err
		}

		// Wait for settings to take effect. See https://skia-review.googlesource.com/c/skia/+/78140.
		log("Sleeping for 5 seconds before checking whether CPU frequency change took effect...")
		time.Sleep(5 * time.Second)

		// Check that the frequency change took effect.
		return assertContentsOfFileOnDevice(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu), chosenFreqAsStr)
	})
}

// getAvailableCPUFrequencies returns the list of available frequencies for a given CPU ID.
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#362
// to
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#372.
func getAvailableCPUFrequencies(ctx context.Context, cpu int) ([]int64, error) {
	// All devices we test on give a list of their available frequencies.
	scalingAvailableFrequenciesFile := fmt.Sprintf("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cpu)

	output, err := readFileOnDevice(ctx, scalingAvailableFrequenciesFile)
	if err != nil {
		return nil, err
	}

	if len(output) == 0 {
		return nil, fmt.Errorf("file %s is empty", scalingAvailableFrequenciesFile)
	}

	// Check for errors like "/system/bin/sh: file not found".
	if strings.Contains(output, "/system/bin/sh") {
		return nil, fmt.Errorf("unrecognized file %s contents: %q", scalingAvailableFrequenciesFile, output)
	}

	// Parse available frequencies.
	var availableFreqs []int64
	for _, freqAsString := range strings.Split(strings.TrimSpace(output), " ") {
		freq, err := strconv.ParseInt(freqAsString, 10, 64)
		if err != nil {
			return nil, fmt.Errorf("while parsing frequency %q: %s", freqAsString, err)
		}
		availableFreqs = append(availableFreqs, freq)
	}
	sort.Slice(availableFreqs, func(i, j int) bool { return availableFreqs[i] < availableFreqs[j] })

	return availableFreqs, nil
}

// enableOrDisableCPU enables or disables the given CPU ID.
//
// Based on
// https://skia.googlesource.com/skia/+/0e8023dc0a1a5655703b39454c090b5a004415d6/infra/bots/recipe_modules/flavor/android.py#287.
func enableOrDisableCPU(ctx context.Context, model string, cpu int, enable bool) error {
	targetState := "0"
	msg := fmt.Sprintf("Disabling CPU %d", cpu)
	if enable {
		targetState = "1"
		msg = fmt.Sprintf("Enabling CPU %d", cpu)
	}

	return withRetry(ctx, model, msg, func() error {
		// Is the CPU already at the target state?
		//
		// ADB returns exit code 1 if we try to echo "1" to a CPU that's already online.
		if output, err := readFileOnDevice(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/online", cpu)); err != nil {
			return fmt.Errorf("while checking whether CPU %d is online: %s", cpu, err)
		} else if output == targetState {
			log("CPU %d is already at the desired state.", cpu)
			return nil
		}

		// Change the CPU state.
		return writeFileOnDeviceAndAssertContents(ctx, fmt.Sprintf("/sys/devices/system/cpu/cpu%d/online", cpu), targetState)
	})
}

// writeFileOnDevice writes a file on the device using an "echo contents > path" command executed
// via "adb shell".
func writeFileOnDevice(ctx context.Context, path, contents string) error {
	if _, err := adb(ctx, "shell", fmt.Sprintf("echo %q > %s", contents, path)); err != nil {
		return fmt.Errorf("while writing %s: %s", path, err)
	}
	return nil
}

// readFileOnDevice reads the contents of a file on the device.
func readFileOnDevice(ctx context.Context, path string) (string, error) {
	contents, err := adb(ctx, "shell", "cat "+path)
	if err != nil {
		return "", fmt.Errorf("while reading file %s: %s", path, err)
	}
	return contents, nil
}

// assertContentsOfFileOnDevice asserts that a file on the device has the expected contents. Both
// the expected and actual contents are trimmed of leading and trailing spaces prior to comparing
// them.
func assertContentsOfFileOnDevice(ctx context.Context, path, expectedContents string) error {
	actualContents, err := readFileOnDevice(ctx, path)
	if err != nil {
		return err
	}
	actualContents = strings.TrimSpace(actualContents)
	expectedContents = strings.TrimSpace(expectedContents)
	if actualContents != expectedContents {
		return fmt.Errorf("file %s does not have the expected contents; expected %q, got %q (leading/trailing spaces trimmed)", path, expectedContents, actualContents)
	}
	return nil
}

// writeFileOnDeviceAndAssertContents combines writeFileOnDevice and assertContentsOfFileOnDevice
// in a single function.
func writeFileOnDeviceAndAssertContents(ctx context.Context, path, contents string) error {
	if err := writeFileOnDevice(ctx, path, contents); err != nil {
		return err
	}
	return assertContentsOfFileOnDevice(ctx, path, contents)
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

// adb runs adb with the given arguments. It returns the combined standard output and standard
// error.
func adb(ctx context.Context, args ...string) (string, error) {
	return adbWithStdin(ctx, "", args...)
}

// adbWithStdin runs adb with the given arguments, and pipes the given input via standard input. It
// returns the combined standard output and standard error.
func adbWithStdin(ctx context.Context, stdin string, args ...string) (string, error) {
	commandAndArgs := strings.Join(append([]string{"adb"}, args...), " ")
	withStdin := ""
	if stdin != "" {
		withStdin = fmt.Sprintf(" with standard input %q", stdin)
	}
	log("Executing: %q%s...", commandAndArgs, withStdin)

	cmd := &exec.Command{
		Name:   "adb",
		Args:   args,
		Stdout: os.Stdout,
		Stderr: os.Stderr,
	}
	if stdin != "" {
		cmd.Stdin = bytes.NewBufferString(stdin)
	}
	return exec.RunCommand(ctx, cmd)
}

func log(msg string, a ...interface{}) {
	timestamp := time.Now().Format(time.RFC3339)
	fmtString := "[%s] " + msg + "\n"
	args := append([]interface{}{timestamp}, a...)
	if _, err := fmt.Printf(fmtString, args...); err != nil {
		panic(err)
	}
}

func printToStdErr(msg string, a ...interface{}) {
	if _, err := fmt.Fprintf(os.Stderr, msg, a...); err != nil {
		panic(err)
	}
}

// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This task driver runs a single Bazel test target representing one or more GMs, or a Bazel test
// suite consisting of multiple such targets, using the provided config (which is assumed to be in
// //bazel/buildrc). GM results are uploaded to Gold via goldctl. This task driver handles any
// setup steps needed to run Bazel on our CI machines before running the task, such as setting up
// logs and the Bazel cache.

package main

import (
	"archive/zip"
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strings"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// goldctlBazelLabelAllowList is the list of Bazel targets that are allowed to upload results to
// Gold via goldctl. This is to prevent polluting Gold with spurious digests, or digests with the
// wrong keys while we experiment with running GMs with Bazel.
//
// TODO(lovisolo): Delete once migration is complete.
var goldctlBazelLabelAllowList = map[string]bool{
	"//gm:hello_bazel_world_test": true,
}

var (
	// Required properties for this task.
	//
	// We want the cache to be on a bigger disk than default. The root disk, where the home directory
	// (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
	cachePath = flag.String("cache_path", "/mnt/pd0/bazel_cache", "The path where the Bazel cache should live. This should be able to hold tens of GB at least.")
	cross     = flag.String("cross", "", "An identifier specifying the target platform that Bazel should build for. If empty, Bazel builds for the host platform (the machine on which this executable is run).")
	label     = flag.String("test_label", "", "The label of the Bazel target to test.")
	config    = flag.String("test_config", "", "A custom configuration specified in //bazel/buildrc. This configuration potentially encapsulates many features and options.")
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory.")

	// goldctl data.
	goldctlPath      = flag.String("goldctl_path", "", "The path to the golctl binary on disk.")
	gitCommit        = flag.String("git_commit", "", "The git hash to which the data should be associated. This will be used when changelist_id and patchset_order are not set to report data to Gold that belongs on the primary branch.")
	changelistID     = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	patchsetOrderStr = flag.String("patchset_order", "", "Should be non-zero only when run on the CQ.")
	tryjobID         = flag.String("tryjob_id", "", "Should be non-zero only when run on the CQ.")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// StartRun calls flag.Parse().
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *label == "" {
		td.Fatal(ctx, fmt.Errorf("--test_label is required"))
	}

	if *config == "" {
		td.Fatal(ctx, fmt.Errorf("--test_config is required"))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	opts := bazel.BazelOptions{
		CachePath: *cachePath,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if *cross != "" {
		// See https://bazel.build/concepts/platforms-intro and https://bazel.build/docs/platforms when
		// ready to support this.
		td.Fatal(ctx, fmt.Errorf("cross compilation not yet supported"))
	}

	if err := run(ctx, taskDriverArgs{
		checkoutDir:   filepath.Join(wd, "skia"),
		bazelLabel:    *label,
		bazelConfig:   *config,
		goldctlPath:   filepath.Join(wd, *goldctlPath),
		gitCommit:     *gitCommit,
		changelistID:  *changelistID,
		patchsetOrder: *patchsetOrderStr,
		tryjobID:      *tryjobID,
	}); err != nil {
		td.Fatal(ctx, err)
	}
}

// taskDriverArgs gathers the inputs to this task driver, and decouples the task driver's
// entry-point function from the command line flags, which facilitates writing unit tests.
type taskDriverArgs struct {
	checkoutDir   string
	bazelLabel    string
	bazelConfig   string
	goldctlPath   string
	gitCommit     string
	changelistID  string
	patchsetOrder string // 1, 2, 3, etc.
	tryjobID      string

	// testOnlyAllowAnyBazelLabel should only be used from tests. If true, the
	// goldctlBazelLabelAllowList will be ignored.
	//
	// TODO(lovisolo): Delete once migration is complete.
	testOnlyAllowAnyBazelLabel bool
}

// run is the entrypoint of this task driver.
func run(ctx context.Context, tdArgs taskDriverArgs) error {
	outputsZipPath, err := validateLabelAndReturnOutputsZipPath(tdArgs.checkoutDir, tdArgs.bazelLabel)
	if err != nil {
		return skerr.Wrap(err)
	}

	if err := bazelTest(ctx, tdArgs.checkoutDir, tdArgs.bazelLabel, tdArgs.bazelConfig); err != nil {
		return skerr.Wrap(err)
	}

	// TODO(lovisolo): Delete once migration is complete.
	if !tdArgs.testOnlyAllowAnyBazelLabel {
		if _, ok := goldctlBazelLabelAllowList[tdArgs.bazelLabel]; !ok {
			return nil
		}
	}

	if err := maybeUploadToGold(ctx, tdArgs, outputsZipPath); err != nil {
		return skerr.Wrap(err)
	}

	return nil
}

// validLabelRegexps represent valid, fully-qualified Bazel labels.
var validLabelRegexps = []*regexp.Regexp{
	regexp.MustCompile(`^//:[a-zA-Z0-9_-]+$`),                  // Matches "//:foo".
	regexp.MustCompile(`^/(/[a-zA-Z0-9_-]+)+:[a-zA-Z0-9_-]+$`), // Matches "//foo:bar", "//foo/bar:baz", etc.
}

// validateLabelAndReturnOutputsZipPath validates the given Bazel label and returns the path within
// the checkout directory where the ZIP archive with undeclared test outputs will be found, if
// applicable.
func validateLabelAndReturnOutputsZipPath(checkoutDir, label string) (string, error) {
	valid := false
	for _, re := range validLabelRegexps {
		if re.MatchString(label) {
			valid = true
			break
		}
	}
	if !valid {
		return "", skerr.Fmt("invalid label: %q", label)
	}

	return filepath.Join(
		checkoutDir,
		"bazel-testlogs",
		strings.ReplaceAll(strings.TrimPrefix(label, "//"), ":", "/"),
		"test.outputs",
		"outputs.zip"), nil
}

// bazelTest runs the test referenced by the given fully qualified Bazel label under the given
// config.
func bazelTest(ctx context.Context, checkoutDir, label, config string) error {
	return td.Do(ctx, td.Props(fmt.Sprintf("Test %s with config %s", label, config)), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: []string{"test",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc.
				"--test_output=errors",
				"--jobs=100",
			},
			InheritEnv: true, // Makes sure bazelisk is on PATH.
			Dir:        checkoutDir,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
}

// maybeUploadToGold uploads any GM results to Gold via goldctl.
func maybeUploadToGold(ctx context.Context, tdArgs taskDriverArgs, outputsZipPath string) error {
	// Were there any undeclared test outputs?
	if _, err := os.Stat(outputsZipPath); err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return td.Do(ctx, td.Props("Test did not produce an undeclared test outputs ZIP file; nothing to upload to Gold"), func(ctx context.Context) error {
				return nil
			})
		} else {
			return skerr.Wrap(err)
		}
	}

	// Extract undeclared outputs ZIP archive.
	outputsDir, err := extractOutputsZip(ctx, outputsZipPath)
	if err != nil {
		return skerr.Wrap(err)
	}
	defer util.RemoveAll(outputsDir)

	// Gather GM outputs.
	gmOutputs, err := gatherGMOutputs(ctx, outputsDir)
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(gmOutputs) == 0 {
		return td.Do(ctx, td.Props("Undeclared test outputs ZIP file contains no GM outputs; nothing to upload to Gold"), func(ctx context.Context) error {
			return nil
		})
	}

	return td.Do(ctx, td.Props("Upload GM outputs to Gold"), func(ctx context.Context) error {
		// Create working directory for goldctl.
		goldctlWorkDir, err := os_steps.TempDir(ctx, "", "goldctl-workdir-*")
		if err != nil {
			return skerr.Wrap(err)
		}
		defer util.RemoveAll(goldctlWorkDir)

		// Authorize goldctl.
		if err := goldctl(ctx, tdArgs.checkoutDir, tdArgs.goldctlPath, "auth", "--work-dir", goldctlWorkDir, "--luci"); err != nil {
			return skerr.Wrap(err)
		}

		// Prepare task-specific key:value pairs.
		var taskSpecificKeyValuePairs []string
		for k, v := range computeTaskSpecificGoldctlKeyValuePairs() {
			taskSpecificKeyValuePairs = append(taskSpecificKeyValuePairs, k+":"+v)
		}
		sort.Strings(taskSpecificKeyValuePairs) // Sort for determinism.

		// Initialize goldctl.
		args := []string{
			"imgtest", "init",
			"--work-dir", goldctlWorkDir,
			"--instance", "skia",
			// If we use flag --instance alone, goldctl will incorrectly infer the Gold instance URL as
			// https://skia-gold.skia.org.
			"--url", "https://gold.skia.org",
			// Similarly, unless we specify a GCE bucket explicitly, goldctl will incorrectly infer
			// "skia-gold-skia" as the instance's bucket.
			"--bucket", "skia-infra-gm",
			"--git_hash", tdArgs.gitCommit,
		}
		if tdArgs.changelistID != "" && tdArgs.patchsetOrder != "" {
			args = append(args,
				"--crs", "gerrit",
				"--cis", "buildbucket",
				"--changelist", tdArgs.changelistID,
				"--patchset", tdArgs.patchsetOrder,
				"--jobid", tdArgs.tryjobID)
		}
		for _, kv := range taskSpecificKeyValuePairs {
			args = append(args, "--key", kv)
		}
		if err := goldctl(ctx, tdArgs.checkoutDir, tdArgs.goldctlPath, args...); err != nil {
			return skerr.Wrap(err)
		}

		// Add PNGs.
		for _, gmOutput := range gmOutputs {
			args := []string{
				"imgtest", "add",
				"--work-dir", goldctlWorkDir,
				"--test-name", gmOutput.TestName,
				"--png-file", gmOutput.PNGPath,
				"--png-digest", gmOutput.MD5,
			}
			var testSpecificKeyValuePairs []string
			for k, v := range gmOutput.Keys {
				testSpecificKeyValuePairs = append(testSpecificKeyValuePairs, k+":"+v)
			}
			sort.Strings(testSpecificKeyValuePairs) // Sort for determinism.
			for _, kv := range testSpecificKeyValuePairs {
				// We assume that all keys are non-optional. That is, all keys are part of the trace. It is
				// possible to add support for optional keys in the future, which can be specified via the
				// --add-test-optional-key flag.
				args = append(args, "--add-test-key", kv)
			}

			if err := goldctl(ctx, tdArgs.checkoutDir, tdArgs.goldctlPath, args...); err != nil {
				return skerr.Wrap(err)
			}
		}

		// Finalize and upload screenshots to Gold.
		return goldctl(ctx, tdArgs.checkoutDir, tdArgs.goldctlPath, "imgtest", "finalize", "--work-dir", goldctlWorkDir)
	})
}

// extractOutputsZip extracts the undeclared outputs ZIP archive into a temporary directory, and
// returns the path to said directory.
func extractOutputsZip(ctx context.Context, outputsZipPath string) (string, error) {
	// Create extraction directory.
	extractionDir, err := os.MkdirTemp("", "bazel-test-output-dir-*")
	if err != nil {
		return "", skerr.Wrap(err)
	}

	// Extract ZIP archive.
	if err := td.Do(ctx, td.Props(fmt.Sprintf("Extract undeclared outputs archive %s into %s", outputsZipPath, extractionDir)), func(ctx context.Context) error {
		outputsZip, err := zip.OpenReader(outputsZipPath)
		if err != nil {
			return skerr.Wrap(err)
		}
		defer util.Close(outputsZip)

		for _, file := range outputsZip.File {
			// Skip directories. We assume all output files are at the root directory of the archive.
			if file.FileInfo().IsDir() {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Ignoring directory: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Ignore anything that is not a PNG or JSON file.
			if !strings.HasSuffix(strings.ToLower(file.Name), ".png") && !strings.HasSuffix(strings.ToLower(file.Name), ".json") {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Ignoring non-PNG / non-JSON file: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Extract file.
			if err := td.Do(ctx, td.Props(fmt.Sprintf("Extracting file: %s", file.Name)), func(ctx context.Context) error {
				reader, err := file.Open()
				if err != nil {
					return skerr.Wrap(err)
				}
				defer util.Close(reader)

				buf := &bytes.Buffer{}
				if _, err := io.Copy(buf, reader); err != nil {
					return skerr.Wrap(err)
				}

				return skerr.Wrap(os.WriteFile(filepath.Join(extractionDir, file.Name), buf.Bytes(), 0644))
			}); err != nil {
				return skerr.Wrap(err)
			}
		}

		return nil
	}); err != nil {
		return "", skerr.Wrap(err)
	}

	return extractionDir, nil
}

// gmJSONOutput represents a JSON file produced by //gm/BazelGMRunner.cpp, plus bookkeeping
// information required by this task driver.
type gmJSONOutput struct {
	MD5  string            `json:"md5"`
	Keys map[string]string `json:"keys"`

	TestName string `json:"-"` // Convenience alias, should be the same as the "name" key.
	PNGPath  string `json:"-"`
}

// gatherGMOutputs inspects a directory with the contents of the undeclared test outputs ZIP
// archive and gathers any GM outputs found therein.
func gatherGMOutputs(ctx context.Context, outputsDir string) ([]gmJSONOutput, error) {
	var outputs []gmJSONOutput

	if err := td.Do(ctx, td.Props("Gather JSON and PNG files produced by GMs"), func(ctx context.Context) error {
		files, err := os.ReadDir(outputsDir)
		if err != nil {
			return skerr.Wrap(err)
		}

		for _, file := range files {
			if !strings.HasSuffix(file.Name(), ".json") {
				continue
			}

			jsonPath := file.Name()
			pngPath := strings.TrimSuffix(jsonPath, ".json") + ".png"
			testName := strings.TrimSuffix(jsonPath, ".json")

			// Skip JSON file if there is no associated PNG file.
			if _, err := os.Stat(filepath.Join(outputsDir, pngPath)); err != nil {
				if errors.Is(err, os.ErrNotExist) {
					if err := td.Do(ctx, td.Props(fmt.Sprintf("Ignoring %q: file %q not found", jsonPath, pngPath)), func(ctx context.Context) error {
						return nil
					}); err != nil {
						return skerr.Wrap(err)
					}
					continue
				} else {
					return skerr.Wrap(err)
				}
			}

			// Parse JSON file. Skip it if parsing fails (rather than failing the entire task in the off
			// chance that the test has other kinds of undeclared outputs).
			bytes, err := os.ReadFile(filepath.Join(outputsDir, jsonPath))
			if err != nil {
				return skerr.Wrap(err)
			}
			output := gmJSONOutput{
				TestName: testName,
				PNGPath:  filepath.Join(outputsDir, pngPath),
			}
			if err := json.Unmarshal(bytes, &output); err != nil {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Ignoring %q; JSON parsing error: %s", jsonPath, err)), func(ctx context.Context) error {
					return nil
				}); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}
			if output.MD5 == "" {
				if err := td.Do(ctx, td.Props(fmt.Sprintf(`Ignoring %q: field "md5" not found`, jsonPath)), func(ctx context.Context) error {
					return nil
				}); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Save GM output.
			if err := td.Do(ctx, td.Props(fmt.Sprintf("Gather %q", pngPath)), func(ctx context.Context) error {
				outputs = append(outputs, output)
				return nil
			}); err != nil {
				return skerr.Wrap(err)
			}
		}

		return nil
	}); err != nil {
		return nil, skerr.Wrap(err)
	}

	// Sort outputs for determinism.
	sort.Slice(outputs, func(i, j int) bool {
		return outputs[i].TestName < outputs[j].TestName
	})

	return outputs, nil
}

// goldctl runs the goldctl command.
func goldctl(ctx context.Context, checkoutDir, goldctlPath string, args ...string) error {
	cmd := &sk_exec.Command{
		Name:      goldctlPath,
		Args:      args,
		Dir:       checkoutDir,
		LogStdout: true,
		LogStderr: true,
	}
	_, err := sk_exec.RunCommand(ctx, cmd)
	return skerr.Wrap(err)
}

// computeTaskSpecificGoldctlKeyValuePairs returns the set of task-specific key-value pairs.
func computeTaskSpecificGoldctlKeyValuePairs() map[string]string {
	// The "os" key produced by DM can have values like these:
	//
	// - Android
	// - ChromeOS
	// - Debian10
	// - Mac10.15.7
	// - Mac11
	// - Ubuntu18
	// - Win10
	// - Win2019
	// - iOS
	//
	// TODO(lovisolo): Determine the "os" key in a fashion similar to DM.
	if runtime.GOOS != "linux" {
		panic("only linux is supported at this time")
	}
	os := "linux"

	// TODO(lovisolo): "arch" key ("arm", "arm64", "x86", "x86_64", etc.).
	// TODO(lovisolo): "configuration" key ("Debug", "Release", "OptimizeForSize", etc.).
	// TODO(lovisolo): "model" key ("MacBook10.1", "Pixel5", "iPadPro", "iPhone11", etc.).

	return map[string]string{
		"os": os,
	}
}

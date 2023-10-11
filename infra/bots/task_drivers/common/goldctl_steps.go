// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"archive/zip"
	"bytes"
	"context"
	"encoding/json"
	"errors"
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
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// goldctlBazelLabelAllowList is the list of Bazel targets that are allowed to upload results to
// Gold via goldctl. This is to prevent polluting Gold with spurious digests, or digests with the
// wrong keys while we experiment with running GMs with Bazel.
//
// TODO(lovisolo): Delete once migration is complete.
var goldctlBazelLabelAllowList = map[string]bool{
	"//gm:hello_bazel_world_test":         true,
	"//gm:hello_bazel_world_android_test": true,
}

// validLabelRegexps represent valid, fully-qualified Bazel labels.
var validLabelRegexps = []*regexp.Regexp{
	regexp.MustCompile(`^//:[a-zA-Z0-9_-]+$`),                  // Matches "//:foo".
	regexp.MustCompile(`^/(/[a-zA-Z0-9_-]+)+:[a-zA-Z0-9_-]+$`), // Matches "//foo:bar", "//foo/bar:baz", etc.
}

// ValidateLabelAndReturnOutputsZipPath validates the given Bazel label and returns the path within
// the checkout directory where the ZIP archive with undeclared test outputs will be found, if
// applicable.
func ValidateLabelAndReturnOutputsZipPath(checkoutDir, label string) (string, error) {
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

// UploadToGoldArgs gathers the inputs to the UploadToGold function.
type UploadToGoldArgs struct {
	BazelLabel    string
	GoldctlPath   string
	GitCommit     string
	ChangelistID  string
	PatchsetOrder string // 1, 2, 3, etc.
	TryjobID      string

	// TestOnlyAllowAnyBazelLabel should only be used from tests. If true, the
	// goldctlBazelLabelAllowList will be ignored.
	//
	// TODO(lovisolo): Delete once migration is complete.
	TestOnlyAllowAnyBazelLabel bool
}

// UploadToGold uploads any GM results to Gold via goldctl.
func UploadToGold(ctx context.Context, utgArgs UploadToGoldArgs, outputsZIPOrDir string) error {
	// TODO(lovisolo): Delete once migration is complete.
	if !utgArgs.TestOnlyAllowAnyBazelLabel {
		if _, ok := goldctlBazelLabelAllowList[utgArgs.BazelLabel]; !ok {
			return skerr.Wrap(td.Do(ctx, td.Props(fmt.Sprintf("Bazel label %q is not allowlisted to upload to Gold; skipping goldctl steps", utgArgs.BazelLabel)), func(ctx context.Context) error {
				return nil
			}))
		}
	}

	// Were there any undeclared test outputs?
	fileInfo, err := os.Stat(outputsZIPOrDir)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return td.Do(ctx, td.Props("Test did not produce an undeclared test outputs ZIP file or directory; nothing to upload to Gold"), func(ctx context.Context) error {
				return nil
			})
		} else {
			return skerr.Wrap(err)
		}
	}

	// If the undeclared outputs ZIP file or directory is a ZIP file, extract it.
	outputsDir := ""
	if fileInfo.IsDir() {
		outputsDir = outputsZIPOrDir
	} else {
		var err error
		outputsDir, err = extractOutputsZip(ctx, outputsZIPOrDir)
		if err != nil {
			return skerr.Wrap(err)
		}
		defer util.RemoveAll(outputsDir)
	}

	// Gather GM outputs.
	gmOutputs, err := gatherGMOutputs(ctx, outputsDir)
	if err != nil {
		return skerr.Wrap(err)
	}
	if len(gmOutputs) == 0 {
		return td.Do(ctx, td.Props("Undeclared test outputs ZIP file or directory contains no GM outputs; nothing to upload to Gold"), func(ctx context.Context) error {
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
		if err := goldctl(ctx, utgArgs.GoldctlPath, "auth", "--work-dir", goldctlWorkDir, "--luci"); err != nil {
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
			"--git_hash", utgArgs.GitCommit,
		}
		if utgArgs.ChangelistID != "" && utgArgs.PatchsetOrder != "" {
			args = append(args,
				"--crs", "gerrit",
				"--cis", "buildbucket",
				"--changelist", utgArgs.ChangelistID,
				"--patchset", utgArgs.PatchsetOrder,
				"--jobid", utgArgs.TryjobID)
		}
		for _, kv := range taskSpecificKeyValuePairs {
			args = append(args, "--key", kv)
		}
		if err := goldctl(ctx, utgArgs.GoldctlPath, args...); err != nil {
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

			if err := goldctl(ctx, utgArgs.GoldctlPath, args...); err != nil {
				return skerr.Wrap(err)
			}
		}

		// Finalize and upload screenshots to Gold.
		return goldctl(ctx, utgArgs.GoldctlPath, "imgtest", "finalize", "--work-dir", goldctlWorkDir)
	})
}

// extractOutputsZip extracts the undeclared outputs ZIP archive into a temporary directory, and
// returns the path to said directory.
func extractOutputsZip(ctx context.Context, outputsZipPath string) (string, error) {
	// Create extraction directory.
	extractionDir, err := os_steps.TempDir(ctx, "", "bazel-test-output-dir-*")
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
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Not extracting subdirectory: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Ignore anything that is not a PNG or JSON file.
			if !strings.HasSuffix(strings.ToLower(file.Name), ".png") && !strings.HasSuffix(strings.ToLower(file.Name), ".json") {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Not extracting non-PNG / non-JSON file: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
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

// gmJSONOutput represents a JSON file produced by //tools/testrunners/gm/BazelGMTestRunner.cpp,
// plus bookkeeping information required by this task driver.
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
func goldctl(ctx context.Context, goldctlPath string, args ...string) error {
	cmd := &sk_exec.Command{
		Name:      goldctlPath,
		Args:      args,
		LogStdout: true,
		LogStderr: true,
	}
	_, err := sk_exec.RunCommand(ctx, cmd)
	return skerr.Wrap(err)
}

// computeTaskSpecificGoldctlKeyValuePairs returns the set of task-specific key-value pairs.
//
// TODO(lovisolo): Infer these key-value pairs from the Bazel config, host, etc.
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

	// TODO(lovisolo): Delete this temporary hack.
	if runtime.GOARCH == "arm" || runtime.GOARCH == "arm64" {
		// As a temporary hack to be able to generate diferent traces for the same GM on Linux vs.
		// Android, we assume that if the task driver is running on an ARM machine, then it's a
		// Raspberry Pi connected to an Android phone. This is only for use while we experiment with
		// Bazel-built GMs.
		//
		// Moving forward, we should try to derive the "os", "model" and "arch" keys from the
		// BazelTest-* task's "host" component. A potential approach could be to use hosts such as
		// "NUC9i7QN_Debian11". In this example, we can derive the "model" and "arch" keys from the
		// "NUC9i7QN" part, and the "os" key would match the "Debian11" part.
		os = "android"
	}

	// TODO(lovisolo): "arch" key ("arm", "arm64", "x86", "x86_64", etc.).
	// TODO(lovisolo): "configuration" key ("Debug", "Release", "OptimizeForSize", etc.).
	// TODO(lovisolo): "model" key ("MacBook10.1", "Pixel5", "iPadPro", "iPhone11", etc.).

	return map[string]string{
		"os": os,
	}
}

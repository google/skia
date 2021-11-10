// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		builtPath       = flag.String("built_path", "", "The directory where the built wasm/js code will be.")
		gitCommit       = flag.String("git_commit", "", "The commit at which we are testing.")
		goldCtlPath     = flag.String("gold_ctl_path", "", "Path to the goldctl binary")
		goldHashesURL   = flag.String("gold_hashes_url", "", "URL from which to download pre-existing hashes")
		goldKeys        = common.NewMultiStringFlag("gold_key", nil, "The keys that will tag this data")
		nodeBinPath     = flag.String("node_bin_path", "", "Path to the node bin directory (should have npm also). This directory *must* be on the PATH when this executable is called, otherwise, the wrong node or npm version may be found (e.g. the one on the system), even if we are explicitly calling npm with the absolute path.")
		projectID       = flag.String("project_id", "", "ID of the Google Cloud project.")
		resourcePath    = flag.String("resource_path", "", "The directory housing the images, fonts, and other assets used by tests.")
		taskID          = flag.String("task_id", "", "task id this data was generated on")
		taskName        = flag.String("task_name", "", "Name of the task.")
		testHarnessPath = flag.String("test_harness_path", "", "Path to test harness folder (tools/run-wasm-gm-tests)")
		webGLVersion    = flag.Int("webgl_version", 2, "The version of web gl to use. 0 means CPU")
		workPath        = flag.String("work_path", "", "The directory to use to store temporary files (e.g. pngs and JSON)")

		// Provided for tryjobs
		changelistID = flag.String("changelist_id", "", "The id the Gerrit CL. Omit for primary branch.")
		tryjobID     = flag.String("tryjob_id", "", "The id of the Buildbucket job for tryjobs. Omit for primary branch.")
		// Because we pass in patchset_order via a placeholder, it can be empty string. As such, we
		// cannot use flag.Int, because that errors on "" being passed in.
		patchsetOrder = flag.String("patchset_order", "0", "Represents if this is the nth patchset")

		// Debugging flags.
		local              = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps        = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		serviceAccountPath = flag.String("service_account_path", "", "Used in local mode for authentication. Non-local mode uses Luci config.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	builtAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *builtPath, "built_path")
	goldctlAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *goldCtlPath, "gold_ctl_path")
	nodeBinAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *nodeBinPath, "node_bin_path")
	resourceAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *resourcePath, "resource_path")
	testHarnessAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *testHarnessPath, "test_harness_path")
	workAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *workPath, "work_path")

	goldctlWorkPath := filepath.Join(workAbsPath, "goldctl")
	if err := os_steps.MkdirAll(ctx, goldctlWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
	testsWorkPath := filepath.Join(workAbsPath, "tests")
	if err := os_steps.MkdirAll(ctx, testsWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
	if *goldHashesURL == "" {
		td.Fatalf(ctx, "Must supply --gold_hashes_url")
	}

	patchset := 0
	if *patchsetOrder != "" {
		p, err := strconv.Atoi(*patchsetOrder)
		if err != nil {
			td.Fatalf(ctx, "Invalid patchset_order %q", *patchsetOrder)
		}
		patchset = p
	}

	keys := *goldKeys
	switch *webGLVersion {
	case 0:
		keys = append(keys, "cpu_or_gpu:CPU")
	case 1:
		keys = append(keys, "cpu_or_gpu:GPU", "extra_config:WebGL1")
	case 2:
		keys = append(keys, "cpu_or_gpu:GPU", "extra_config:WebGL2")
	default:
		td.Fatalf(ctx, "Invalid value for webgl_version, must be 0, 1, 2 got %d", *webGLVersion)
	}

	// initialize goldctl
	if err := setupGoldctl(ctx, *local, *gitCommit, *changelistID, *tryjobID, goldctlAbsPath, goldctlWorkPath,
		*serviceAccountPath, keys, patchset); err != nil {
		td.Fatal(ctx, err)
	}

	if err := downloadKnownHashes(ctx, testsWorkPath, *goldHashesURL); err != nil {
		td.Fatal(ctx, err)
	}
	if err := setupTests(ctx, nodeBinAbsPath, testHarnessAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	// Run puppeteer tests. The input is a list of known hashes. The output will be a JSON array and
	// any new images to be written to disk in the testsWorkPath. See WriteToDisk in DM for how that
	// is done on the C++ side.
	if err := runTests(ctx, builtAbsPath, nodeBinAbsPath, resourceAbsPath, testHarnessAbsPath, testsWorkPath, *webGLVersion); err != nil {
		td.Fatal(ctx, err)
	}

	// Parse JSON and call goldctl imgtest add them.
	if err := processTestData(ctx, testsWorkPath, goldctlAbsPath, goldctlWorkPath); err != nil {
		td.Fatal(ctx, err)
	}

	// call goldctl finalize to upload stuff.
	if err := finalizeGoldctl(ctx, goldctlAbsPath, goldctlWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
}

func setupGoldctl(ctx context.Context, local bool, gitCommit, gerritCLID, tryjobID, goldctlPath, workPath, serviceAccountPath string, keys []string, psOrder int) error {
	ctx = td.StartStep(ctx, td.Props("setup goldctl").Infra())
	defer td.EndStep(ctx)

	args := []string{goldctlPath, "auth", "--work-dir", workPath}
	if !local {
		args = append(args, "--luci")
	} else {
		// When testing locally, it can also be handy to add in --dry-run here.
		args = append(args, "--service-account", serviceAccountPath)
	}

	if _, err := exec.RunCwd(ctx, workPath, args...); err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "running %s", args))
	}

	args = []string{
		goldctlPath, "imgtest", "init", "--work-dir", workPath, "--instance", "skia", "--corpus", "gm",
		"--commit", gitCommit, "--url", "https://gold.skia.org", "--bucket", "skia-infra-gm",
	}
	if gerritCLID != "" {
		ps := strconv.Itoa(psOrder)
		args = append(args, "--crs", "gerrit", "--changelist", gerritCLID, "--patchset", ps,
			"--cis", "buildbucket", "--jobid", tryjobID)
	}

	for _, key := range keys {
		args = append(args, "--key", key)
	}

	if _, err := exec.RunCwd(ctx, workPath, args...); err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "running %s", args))
	}
	return nil
}

// downloadKnownHashes downloads the known hashes from Gold and stores it as a text file in
// workPath/hashes.txt
func downloadKnownHashes(ctx context.Context, workPath, knownHashesURL string) error {
	ctx = td.StartStep(ctx, td.Props("download known hashes").Infra())
	defer td.EndStep(ctx)

	client := httputils.DefaultClientConfig().With2xxOnly().Client()
	resp, err := client.Get(knownHashesURL)
	if err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "downloading known hashes"))
	}
	defer resp.Body.Close()
	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "reading known hashes"))
	}
	return os_steps.WriteFile(ctx, filepath.Join(workPath, "hashes.txt"), data, 0666)
}

func setupTests(ctx context.Context, nodeBinPath string, testHarnessPath string) error {
	ctx = td.StartStep(ctx, td.Props("setup npm").Infra())
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, testHarnessPath, filepath.Join(nodeBinPath, "npm"), "ci"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

func runTests(ctx context.Context, builtPath, nodeBinPath, resourcePath, testHarnessPath, workPath string, webglVersion int) error {
	ctx = td.StartStep(ctx, td.Props("run GMs and unit tests"))
	defer td.EndStep(ctx)

	err := td.Do(ctx, td.Props("Run GMs and Unit Tests"), func(ctx context.Context) error {
		args := []string{filepath.Join(nodeBinPath, "node"),
			"run-wasm-gm-tests",
			"--js_file", filepath.Join(builtPath, "wasm_gm_tests.js"),
			"--wasm_file", filepath.Join(builtPath, "wasm_gm_tests.wasm"),
			"--known_hashes", filepath.Join(workPath, "hashes.txt"),
			"--use_gpu", // TODO(kjlubick) use webglVersion and account for CPU
			"--output", workPath,
			"--resources", resourcePath,
			"--timeout", "180", // seconds per batch of 50 tests.
		}

		_, err := exec.RunCwd(ctx, testHarnessPath, args...)
		if err != nil {
			return skerr.Wrap(err)
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

type goldResult struct {
	TestName string `json:"name"`
	MD5Hash  string `json:"digest"`
}

func processTestData(ctx context.Context, testOutputPath, goldctlPath, goldctlWorkPath string) error {
	ctx = td.StartStep(ctx, td.Props("process test data").Infra())
	defer td.EndStep(ctx)

	// Read in the file, process it as []goldResult
	var results []goldResult
	resultFile := filepath.Join(testOutputPath, "gold_results.json")

	err := td.Do(ctx, td.Props("Load results from "+resultFile), func(ctx context.Context) error {
		b, err := os_steps.ReadFile(ctx, resultFile)
		if err != nil {
			return skerr.Wrap(err)
		}
		if err := json.Unmarshal(b, &results); err != nil {
			return skerr.Wrap(err)
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	err = td.Do(ctx, td.Props(fmt.Sprintf("Call goldtl on %d results", len(results))), func(ctx context.Context) error {
		for _, result := range results {
			// These args are the same regardless of if we need to upload the png file or not.
			args := []string{goldctlPath, "imgtest", "add", "--work-dir", goldctlWorkPath,
				"--test-name", result.TestName, "--png-digest", result.MD5Hash}
			// check to see if there's an image we need to upload
			potentialPNGFile := filepath.Join(testOutputPath, result.MD5Hash+".png")
			_, err := os_steps.Stat(ctx, potentialPNGFile)
			if os.IsNotExist(err) {
				// PNG was not produced, we assume it is already uploaded to Gold and just say the digest
				// we produced.
				_, err = exec.RunCwd(ctx, goldctlWorkPath, args...)
				if err != nil {
					return skerr.Wrapf(err, "reporting result %#v to goldctl", result)
				}
				continue
			} else if err != nil {
				return skerr.Wrapf(err, "reading %s", potentialPNGFile)
			}
			// call goldctl with the png file
			args = append(args, "--png-file", potentialPNGFile)
			_, err = exec.RunCwd(ctx, goldctlWorkPath, args...)
			if err != nil {
				return skerr.Wrapf(err, "reporting result %#v to goldctl", result)
			}
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

func finalizeGoldctl(ctx context.Context, goldctlPath, workPath string) error {
	ctx = td.StartStep(ctx, td.Props("finalize goldctl data").Infra())
	defer td.EndStep(ctx)

	_, err := exec.RunCwd(ctx, workPath, goldctlPath, "imgtest", "finalize", "--work-dir", workPath)
	if err != nil {
		return skerr.Wrapf(err, "Finalizing goldctl")
	}
	return nil
}

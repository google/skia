// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"io/ioutil"
	"path/filepath"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// Test with go run ./infra/bots/task_drivers/run_wasm_gm_tests/ --logtostderr --local --service_account_path $HOME/service_accounts/gold-external-uploader.json --work_path /tmp/gold2 --gold_ctl_path=`which goldctl` --built_path $SKIA_ROOT/out/wasm_gm_tests --skia_path $SKIA_ROOT --git_commit "154e656f6cdadb1eba23a0b24a8142630cfe9f54" --gold_key "browser:Chrome" --gold_key "alpha_type:Premul" --gold_key "arch:wasm" --gold_key "color_depth:8888"

func main() {
	var (
		// Required properties for this task.
		builtPath   = flag.String("built_path", "", "The directory where the built wasm/js code will be.")
		gitCommit   = flag.String("git_commit", "", "The commit at which we are testing.")
		goldCtlPath = flag.String("gold_ctl_path", "", "Path to the goldctl binary")
		goldKeys    = common.NewMultiStringFlag("gold_key", nil, "The keys that will tag this data")
		nodeBinPath = flag.String("node_bin_path", "", "Path to the node bin directory (should have npm also). This directory *must* be on the PATH when this executable is called, otherwise, the wrong node or npm version may be found (e.g. the one on the system), even if we are explicitly calling npm with the absolute path.")
		projectID   = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath    = flag.String("skia_path", "", "Path to skia repo root.")
		taskID      = flag.String("task_id", "", "task id this data was generated on")
		taskName    = flag.String("task_name", "", "Name of the task.")
		workPath    = flag.String("work_path", "", "The directory to use to store temporary files (e.g. docker build)")

		// Debugging flags.
		local              = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps        = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		serviceAccountPath = flag.String("service_account_path", "", "Used in local mode for authentication. Non-local mode uses Luci config.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	builtAbsPath := getAbsoluteOfRequiredFlag(ctx, *builtPath, "built_path")
	goldctlAbsPath := getAbsoluteOfRequiredFlag(ctx, *goldCtlPath, "gold_ctl_path")
	nodeBinAbsPath := getAbsoluteOfRequiredFlag(ctx, *nodeBinPath, "node_bin_path")
	skiaAbsPath := getAbsoluteOfRequiredFlag(ctx, *skiaPath, "skia_path")
	workAbsPath := getAbsoluteOfRequiredFlag(ctx, *workPath, "work_path")

	goldctlWorkPath := filepath.Join(workAbsPath, "goldctl")
	if err := os_steps.MkdirAll(ctx, goldctlWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
	testsWorkPath := filepath.Join(workAbsPath, "tests")
	if err := os_steps.MkdirAll(ctx, testsWorkPath); err != nil {
		td.Fatal(ctx, err)
	}

	// initialize goldctl
	if err := setupGoldctl(ctx, *local, *gitCommit, goldctlAbsPath, goldctlWorkPath, *serviceAccountPath, *goldKeys); err != nil {
		td.Fatal(ctx, err)
	}

	if err := downloadKnownHashes(ctx, testsWorkPath); err != nil {
		td.Fatal(ctx, err)
	}

	if err := setupTests(ctx, nodeBinAbsPath, skiaAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	// Run puppeteer tests. The input is a list of known hashes. The output will be a JSON array and
	// any new images to be written to disk in the testsWorkPath. See WriteToDisk in DM for how that
	// is done on the C++ side.
	if err := runTests(ctx, builtAbsPath, nodeBinAbsPath, skiaAbsPath, testsWorkPath); err != nil {
		td.Fatal(ctx, err)
	}

	// Parse JSON and call goldctl imgtest add them.

	// call goldctl finalize to upload stuff.
	if err := finalizeGoldctl(ctx, goldctlAbsPath, goldctlWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
}

func setupGoldctl(ctx context.Context, local bool, gitCommit, goldctlPath, workPath, serviceAccountPath string, keys []string) error {
	ctx = td.StartStep(ctx, td.Props("setup goldctl").Infra())
	defer td.EndStep(ctx)

	args := []string{goldctlPath, "auth", "--work-dir", workPath}
	if !local {
		args = append(args, "--luci")
	} else {
		args = append(args, "--dryrun", "--service-account", serviceAccountPath)
	}

	if _, err := exec.RunCwd(ctx, workPath, args...); err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "running %s", args))
	}

	args = []string{
		goldctlPath, "imgtest", "init", "--work-dir", workPath, "--instance", "skia",
		"--commit", gitCommit,
	}
	for _, key := range keys {
		args = append(args, "--key", key)
	}

	if _, err := exec.RunCwd(ctx, workPath, args...); err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "running %s", args))
	}
	return nil
}

const knownHashesURL = "https://storage.googleapis.com/skia-infra-gm/hash_files/gold-prod-hashes.txt"

// downloadKnownHashes downloads the known hashes from Gold and stores it as a text file in
// workPath/hashes.txt
func downloadKnownHashes(ctx context.Context, workPath string) error {
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

func setupTests(ctx context.Context, nodeBinPath string, skiaPath string) error {
	ctx = td.StartStep(ctx, td.Props("setup npm").Infra())
	defer td.EndStep(ctx)

	testHarnessPath := filepath.Join(skiaPath, "tools", "run-wasm-gm-tests")
	if _, err := exec.RunCwd(ctx, testHarnessPath, filepath.Join(nodeBinPath, "npm"), "ci"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

func runTests(ctx context.Context, builtPath, nodeBinPath, skiaPath, workPath string) error {
	ctx = td.StartStep(ctx, td.Props("download known hashes").Infra())
	defer td.EndStep(ctx)

	err := td.Do(ctx, td.Props("Run GMs and Unit Tests"), func(ctx context.Context) error {
		args := []string{filepath.Join(nodeBinPath, "node"),
			"run-wasm-gm-tests",
			"--js-file", filepath.Join(builtPath, "wasm_gm_tests.js"),
			"--wasm-file", filepath.Join(builtPath, "wasm_gm_tests.wasm"),
			"--known-hashes", filepath.Join(workPath, "hashes.txt"),
			"--output", filepath.Join(workPath, "gold_results.json"),
			"--timeout_per_test", "60",
		}

		testHarnessPath := filepath.Join(skiaPath, "tools", "run-wasm-gm-tests")
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

func finalizeGoldctl(ctx context.Context, goldctlPath, workPath string) error {
	return nil
}

func getAbsoluteOfRequiredFlag(ctx context.Context, nonEmptyPath, flag string) string {
	if nonEmptyPath == "" {
		td.Fatalf(ctx, "--%s must be specified", flag)
	}
	absPath, err := filepath.Abs(nonEmptyPath)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	return absPath
}

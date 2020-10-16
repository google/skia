// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"path/filepath"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// Test with go run ./infra/bots/task_drivers/run_wasm_gm_tests/ --logtostderr --local --service_account_path $HOME/service_accounts/gold-external-uploader.json --work_path /tmp/gold2 --gold_ctl_path=`which goldctl` --built_path $SKIA_ROOT/out/wasm_gm_tests --skia_path $SKIA_ROOT --git_commit "154e656f6cdadb1eba23a0b24a8142630cfe9f54" --gold_key "browser:Chrome" --gold_key "alpha_type:Premul" --gold_key "arch:wasm" --gold_key "color_depth:8888"

func main() {
	var (
		// Required properties for this task.
		builtPath   = flag.String("built_path", "", "The directory where the built wasm/js code will be.")
		goldCtlPath = flag.String("gold_ctl_path", "", "Path to the goldctl binary")
		projectID   = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath    = flag.String("skia_path", "", "Path to skia repo root.")
		taskID      = flag.String("task_id", "", "task id this data was generated on")
		taskName    = flag.String("task_name", "", "Name of the task.")
		workPath    = flag.String("work_path", "", "The directory to use to store temporary files (e.g. docker build)")
		goldKeys    = common.NewMultiStringFlag("gold_key", nil, "The keys that will tag this data")
		gitCommit   = flag.String("git_commit", "", "The commit at which we are testing.")

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
	skiaAbsPath := getAbsoluteOfRequiredFlag(ctx, *skiaPath, "skia_path")
	workAbsPath := getAbsoluteOfRequiredFlag(ctx, *workPath, "work_path")

	workAbsPath = filepath.Join(workAbsPath, "goldctl")
	if err := os_steps.MkdirAll(ctx, workAbsPath); err != nil {
		td.Fatal(ctx, err)
	}

	// initialize goldctl
	err := setupGoldctl(ctx, *local, *gitCommit, goldctlAbsPath, workAbsPath, *serviceAccountPath, *goldKeys)
	if err != nil {
		td.Fatal(ctx, err)
	}

	sklog.Infof("%s %s", builtAbsPath, skiaAbsPath)

	// Start webserver to receive gold images and data

	// Eventually read in https://gold.skia.org/json/v1/hashes See skbug.com/10824

	// Run puppeteer tests. HTML will post to the server we indicate. They will only finish
	// when the last post succeeds, so we are sure the server has handled everything.

	// stop webserver.

	// call goldctl finalize to upload stuff.
	err = finalizeGoldctl(ctx, goldctlAbsPath, workAbsPath)
	if err != nil {
		td.Fatal(ctx, err)
	}

}

func setupGoldctl(ctx context.Context, local bool, gitCommit, goldctlPath, workPath, serviceAccountPath string, keys []string) error {
	ctx = td.StartStep(ctx, td.Props("setup cifuzz").Infra())
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

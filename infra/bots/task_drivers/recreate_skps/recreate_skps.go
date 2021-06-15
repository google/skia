// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"net/http"
	"os"
	"path/filepath"

	"go.skia.org/infra/go/depot_tools"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gerrit"
	"go.skia.org/infra/go/recipe_cfg"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/gerrit_steps"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		dryRun    = flag.Bool("dry_run", false, "If set, generate SKPs but do not upload or commit them.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	// Setup.
	var client *http.Client
	var g gerrit.GerritInterface
	if !*dryRun {
		var err error
		client, err = auth_steps.InitHttpClient(ctx, *local, gerrit.AuthScope)
		if err != nil {
			td.Fatal(ctx, err)
		}
		g, err = gerrit.NewGerrit("https://skia-review.googlesource.com", client)
		if err != nil {
			td.Fatal(ctx, err)
		}
	}
	cwd, err := os.Getwd()
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(cwd, "skia")

	// Check out depot_tools at the exact revision expected by tests (defined in recipes.cfg), and
	// make it available to tests by by adding it to the PATH.
	var depotToolsDir string
	if err := td.Do(ctx, td.Props("Check out depot_tools"), func(ctx context.Context) error {
		var err error
		depotToolsDir, err = depot_tools.Sync(ctx, ".", filepath.Join(skiaDir, recipe_cfg.RECIPE_CFG_PATH))
		return err
	}); err != nil {
		td.Fatal(ctx, err)
	}
	ctx = td.WithEnv(ctx, []string{"PATH=%(PATH)s:" + depotToolsDir})

	// Sync Chrome.
	if _, err := exec.RunCwd(ctx, ".", "python", filepath.Join(depotToolsDir, "fetch"), "chromium"); err != nil {
		td.Fatal(ctx, err)
	}

	chromiumDir := filepath.Join(cwd, "src")
	outDir := filepath.Join(chromiumDir, "out", "Release")

	// Fetch `sk`
	if _, err := exec.RunCwd(ctx, skiaDir, "python", filepath.Join("tools", "fetch-sk")); err != nil {
		td.Fatal(ctx, err)
	}

	// Build Chrome.
	if err := td.Do(ctx, td.Props("Build Chrome"), func(ctx context.Context) error {
		// Run "gn gen".  This task driver only runs on Linux.
		gn := filepath.Join(chromiumDir, "buildtools", "linux64", "gn")
		if _, err := exec.RunCommand(ctx, &exec.Command{
			Name: gn,
			Args: []string{"gen", outDir},
			Dir:  chromiumDir,
			Env: []string{
				"CPPFLAGS=-DSK_ALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS=1",
				"GYP_GENERATORS=ninja",
			},
			InheritEnv:  true,
			InheritPath: true,
		}); err != nil {
			return err
		}

		// Perform the build.
		if _, err := exec.RunCwd(ctx, chromiumDir, "ninja", "-C", outDir, "chrome"); err != nil {
			return err
		}
		return nil
	}); err != nil {
		td.Fatal(ctx, err)
	}

	// Capture and upload the SKPs.
	script := filepath.Join(skiaDir, "infra", "bots", "assets", "skp", "create_and_upload.py")
	cmd := []string{
		"python", script,
		"--chrome_src_path", chromiumDir,
		"--browser_executable", filepath.Join(outDir, "chrome"),
	}
	if *dryRun {
		cmd = append(cmd, "--dry_run")
	}
	if _, err := exec.RunCwd(ctx, skiaDir, cmd...); err != nil {
		td.Fatal(ctx, err)
	}
	if *dryRun {
		return
	}

	// Retrieve the new SKP version.
	versionFileSubPath := filepath.Join("infra", "bots", "assets", "skp", "VERSION")
	skpVersion, err := os_steps.ReadFile(ctx, filepath.Join(skiaDir, versionFileSubPath))
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Sync a new checkout of Skia to create the CL.
	tmp, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		td.Fatal(ctx, err)
	}
	tmpSkia := filepath.Join(tmp, "skia")
	co, err := checkout.EnsureGitCheckout(ctx, tmpSkia, types.RepoState{
		Repo:     "https://skia.googlesource.com/skia.git",
		Revision: "origin/main",
	})
	if err != nil {
		td.Fatal(ctx, err)
	}
	baseRev, err := co.FullHash(ctx, "HEAD")
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Write the new SKP version.
	if err := os_steps.WriteFile(ctx, filepath.Join(co.Dir(), versionFileSubPath), skpVersion, os.ModePerm); err != nil {
		td.Fatal(ctx, err)
	}

	// Regenerate tasks.json.
	if _, err := exec.RunCwd(ctx, tmpSkia, "go", "run", "./infra/bots/gen_tasks.go"); err != nil {
		td.Fatal(ctx, err)
	}

	// Upload a CL.
	commitMsg := `Update SKP version

Automatic commit by the RecreateSKPs bot.
`
	gerrit_steps.UploadCL(ctx, g, co, "skia", "main", baseRev, commitMsg, []string{"rmistry@google.com"}, false)
}

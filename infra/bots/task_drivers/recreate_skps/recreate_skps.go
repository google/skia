// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/depot_tools"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gerrit"
	"go.skia.org/infra/go/git/git_common"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/promk/go/pushgateway"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/gerrit_steps"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

const (
	// Metric constants for pushgateway.
	jobName                       = "recreate-skps"
	buildFailureMetricName        = "recreate_skps_build_failure"
	creatingSKPsFailureMetricName = "recreate_skps_creation_failure"
	metricValue_NoFailure         = "0"
	metricValue_Failure           = "1"
)

func botUpdate(ctx context.Context, checkoutRoot, gitCacheDir, skiaRev, patchRef, depotToolsDir string, local bool) error {
	return td.Do(ctx, td.Props("bot_update").Infra(), func(ctx context.Context) error {
		tmp, err := os_steps.TempDir(ctx, "", "")
		if err != nil {
			return err
		}
		defer func() {
			_ = os_steps.RemoveAll(ctx, tmp)
		}()
		ctx = td.WithEnv(ctx, []string{
			"DEPOT_TOOLS_COLLECT_METRICS",
			"DEPOT_TOOLS_UPDATE=0",
			"GIT_CONFIG_NOSYSTEM: 1",
			"GIT_HTTP_LOW_SPEED_LIMIT: 102400",
			"GIT_HTTP_LOW_SPEED_TIME: 1800",
			"GIT_TERMINAL_PROMPT: 0",
			"GIT_USER_AGENT=",
		})
		if _, err := exec.RunCwd(ctx, ".", "which", "git"); err != nil {
			return err
		}
		if _, err := exec.RunCwd(ctx, ".", "git", "--version"); err != nil {
			return err
		}
		if !local {
			if err := git_common.EnsureGitIsFromCIPD(ctx); err != nil {
				return err
			}
		}

		spec := `
solutions = [
  { "name"        : 'src',
    "url"         : 'https://chromium.googlesource.com/chromium/src.git',
    "deps_file"   : 'DEPS',
    "managed"     : False,
    "custom_deps" : {
    },
    "custom_vars": {},
  },
]
`
		specPath := filepath.Join(checkoutRoot, ".gclient")
		if err := os_steps.WriteFile(ctx, specPath, []byte(spec), os.ModePerm); err != nil {
			return err
		}
		skiaRepoURL := "https://skia.googlesource.com/skia.git"
		botUpdateScript := filepath.Join(depotToolsDir, "recipes", "recipe_modules", "bot_update", "resources", "bot_update.py")
		mainRepoName := "src"
		patchRoot := "src/third_party/skia"
		revision := "origin/main"
		// These are required for some reason, despite our not using them.
		outputJson := filepath.Join(tmp, "bot_update.json")
		revMapFile := filepath.Join(tmp, "revmap")
		revMap := `{"got_revision": "src"}`
		if err := os_steps.WriteFile(ctx, revMapFile, []byte(revMap), os.ModePerm); err != nil {
			return err
		}
		cleanupDir := filepath.Join(tmp, "cleanup")
		if err := os_steps.MkdirAll(ctx, cleanupDir); err != nil {
			return err
		}

		cmd := []string{
			"vpython", "-u", botUpdateScript,
			"--spec-path", specPath,
			"--patch_root", patchRoot,
			"--revision_mapping_file", revMapFile,
			"--cleanup-dir", cleanupDir,
			"--output_json", outputJson,
			"--revision", fmt.Sprintf("%s@%s", mainRepoName, revision),
			"--revision", fmt.Sprintf("%s@%s", "https://skia.googlesource.com/skia.git", skiaRev),
		}
		if gitCacheDir != "" {
			cmd = append(cmd, "--git-cache-dir", gitCacheDir)
		}
		if patchRef != "" {
			patchRepoURL := skiaRepoURL
			patchBaseRev := skiaRev
			cmd = append(cmd, "--patch_ref", fmt.Sprintf("%s@%s:%s", patchRepoURL, patchBaseRev, patchRef))
		}
		if _, err := exec.RunCwd(ctx, checkoutRoot, cmd...); err != nil {
			return err
		}

		if _, err := exec.RunCwd(ctx, checkoutRoot, "vpython", "-u", filepath.Join(depotToolsDir, "gclient.py"), "runhooks"); err != nil {
			return err
		}

		return nil
	})
}

func main() {
	var (
		projectId        = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId           = flag.String("task_id", "", "ID of this task.")
		taskName         = flag.String("task_name", "", "Name of the task.")
		output           = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local            = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		dryRun           = flag.Bool("dry_run", false, "If set, generate SKPs but do not upload or commit them.")
		skiaRev          = flag.String("skia_revision", "origin/main", "Revision of Skia at which this task is running.")
		patchRef         = flag.String("patch_ref", "", "Patch ref, if any, associated with this task.")
		skipSync         = flag.Bool("skip-sync", false, "Skip sync. Helpful for running locally.")
		skipBuild        = flag.Bool("skip-build", false, "skip build. Helpful for running locally.")
		gitCacheDirFlag  = flag.String("git_cache", "", "Git cache directory.")
		checkoutRootFlag = flag.String("checkout_root", "", "Directory to use for checkouts.")
		dmPathFlag       = flag.String("dm_path", "", "Path to the DM binary.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	// Setup.
	client, err := auth_steps.InitHttpClient(ctx, *local, gerrit.AuthScope)
	if err != nil {
		td.Fatal(ctx, err)
	}
	var g gerrit.GerritInterface
	if !*dryRun {
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
	gitCacheDir := ""
	if *gitCacheDirFlag != "" {
		gitCacheDir = filepath.Join(cwd, *gitCacheDirFlag)
	}
	checkoutRoot := cwd
	if *checkoutRootFlag != "" {
		checkoutRoot = filepath.Join(cwd, *checkoutRootFlag)
	}
	if *dmPathFlag == "" {
		td.Fatal(ctx, fmt.Errorf("Must specify --dm_path"))
	}
	dmPath := filepath.Join(cwd, *dmPathFlag)

	// Fetch `sk`
	if _, err := exec.RunCwd(ctx, skiaDir, "python3", filepath.Join("bin", "fetch-sk")); err != nil {
		td.Fatal(ctx, err)
	}

	// Create a temporary directory.
	tmp, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		td.Fatal(ctx, err)
	}
	defer func() {
		_ = os_steps.RemoveAll(ctx, tmp)
	}()
	recipesCfgFile := filepath.Join(skiaDir, "infra", "config", "recipes.cfg")

	// Check out depot_tools at the exact revision expected by tests (defined in recipes.cfg), and
	// make it available to tests by by adding it to the PATH.
	var depotToolsDir string
	if err := td.Do(ctx, td.Props("Check out depot_tools"), func(ctx context.Context) error {
		var err error
		depotToolsDir, err = depot_tools.Sync(ctx, tmp, recipesCfgFile)
		return err
	}); err != nil {
		td.Fatal(ctx, err)
	}
	ctx = td.WithEnv(ctx, []string{"PATH=%(PATH)s:" + depotToolsDir})

	// Sync Chrome.
	if !*skipSync {
		if err := botUpdate(ctx, checkoutRoot, gitCacheDir, *skiaRev, *patchRef, depotToolsDir, *local); err != nil {
			td.Fatal(ctx, err)
		}
	}

	// For updating metrics.
	pg := pushgateway.New(client, jobName, pushgateway.DefaultPushgatewayURL)

	chromiumDir := filepath.Join(checkoutRoot, "src")
	outDir := filepath.Join(chromiumDir, "out", "Release")

	// Build Chrome.
	if !*skipBuild {
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
			ninja := filepath.Join(depotToolsDir, "ninja")
			if _, err := exec.RunCwd(ctx, chromiumDir, ninja, "-C", outDir, "chrome"); err != nil {
				return err
			}
			return nil
		}); err != nil {
			// Report that the build failed.
			pg.Push(ctx, buildFailureMetricName, metricValue_Failure)
			td.Fatal(ctx, err)
		}
		// Report that the build was successful.
		pg.Push(ctx, buildFailureMetricName, metricValue_NoFailure)
	}

	// Capture and upload the SKPs.
	script := filepath.Join(skiaDir, "infra", "bots", "assets", "skp", "create_and_upload.py")
	cmd := []string{
		"vpython3", "-u", script,
		"--chrome_src_path", chromiumDir,
		"--browser_executable", filepath.Join(outDir, "chrome"),
		"--dm_path", dmPath,
	}
	if *dryRun {
		cmd = append(cmd, "--dry_run")
	}
	if *local {
		cmd = append(cmd, "--local")
	}
	command := &exec.Command{
		Name: filepath.Join(depotToolsDir, cmd[0]),
		Args: cmd[1:],
		Dir:  skiaDir,
		Env: []string{
			fmt.Sprintf("PATH=%s:%s", os.Getenv("PATH"), depotToolsDir),
		},
	}
	sklog.Infof("Running command: %s %s", command.Name, strings.Join(command.Args, " "))
	if err := exec.Run(ctx, command); err != nil {
		// Creating SKP asset in RecreateSKPs failed.
		pg.Push(ctx, creatingSKPsFailureMetricName, metricValue_Failure)
		td.Fatal(ctx, err)
	}
	// Report that the asset creation was successful.
	pg.Push(ctx, creatingSKPsFailureMetricName, metricValue_NoFailure)
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

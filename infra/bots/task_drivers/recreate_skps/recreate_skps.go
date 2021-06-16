// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"net/http"
	"os"
	"path/filepath"

	"go.skia.org/infra/go/depot_tools"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/gerrit"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/gerrit_steps"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

func botUpdate(ctx context.Context, checkoutRoot, gitCacheDir, skiaRev, patchRef, recipesCfgFile string) error {
	// /opt/s/w/ir/cipd_bin_packages/cpython/bin/python -u /opt/s/w/ir/recipe_bundle/depot_tools/recipes/recipe_modules/bot_update/resources/bot_update.py
	// --spec-path /opt/s/w/ir/tmp/t/tmp5h6PX7
	// --patch_root skia
	// --revision_mapping_file /opt/s/w/ir/tmp/t/tmpHh88SC.json
	// --git-cache-dir /opt/s/w/ir/cache/git
	// --cleanup-dir /opt/s/w/ir/k/recipe_cleanup/bot_update
	// --output_json /opt/s/w/ir/tmp/t/tmpBz57dm.json
	// --patch_ref https://skia.googlesource.com/skia.git@416477a8afccc27e3e1268ae7c5263bfc0b2daf4:refs/changes/38/414438/19
	// --revision skia@416477a8afccc27e3e1268ae7c5263bfc0b2daf4

	return td.Do(ctx, td.Props("bot_update").Infra(), func(ctx context.Context) error {
		tmp, err := os_steps.TempDir(ctx, "", "")
		if err != nil {
			return err
		}
		defer func() {
			_ = os_steps.RemoveAll(ctx, tmp)
		}()

		// Check out depot_tools at the exact revision expected by tests (defined in recipes.cfg), and
		// make it available to tests by by adding it to the PATH.
		var depotToolsDir string
		if err := td.Do(ctx, td.Props("Check out depot_tools"), func(ctx context.Context) error {
			var err error
			depotToolsDir, err = depot_tools.Sync(ctx, ".", recipesCfgFile)
			return err
		}); err != nil {
			return err
		}
		ctx = td.WithEnv(ctx, []string{"PATH=%(PATH)s:" + depotToolsDir})

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
		specPath := filepath.Join(tmp, ".gclient")
		if err := os_steps.WriteFile(ctx, specPath, []byte(spec), os.ModePerm); err != nil {
			return err
		}
		skiaRepoURL := "https://skia.googlesource.com/skia.git"
		botUpdateScript := filepath.Join(depotToolsDir, "recipes", "recipe_modules", "bot_update", "resources", "bot_update.py")
		mainRepoName := "src"
		patchRoot := "src/third_party/skia"
		revision := "origin/main"
		cmd := []string{
			"python", "-u", botUpdateScript,
			"--spec-path", specPath,
			"--patch_root", patchRoot,
			//"--revision_mapping_file", revMapFile,
			//"--output_json", outputJson,
			"--revision", fmt.Sprintf("%s@%s", mainRepoName, revision),
			"--revision", fmt.Sprintf("%s@%s", "https://skia.googlesource.com/skia.git", skiaRev),
		}
		if gitCacheDir != "" {
			cmd = append(cmd, "--git-cache-dir", gitCacheDir)
		}
		if patchRef != "" {
			patchRepoURL := skiaRepoURL
			patchBaseRev := skiaRev
			cmd = append(cmd, "--patch_rev", fmt.Sprintf("%s@%s:%s", patchRepoURL, patchBaseRev, patchRef))
		}
		if _, err := exec.RunCwd(ctx, checkoutRoot, cmd...); err != nil {
			return err
		}

		return nil
	})
}

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		dryRun    = flag.Bool("dry_run", false, "If set, generate SKPs but do not upload or commit them.")
		skiaRev   = flag.String("skia_revision", "origin/main", "Revision of Skia at which this task is running.")
		patchRef  = flag.String("patch_ref", "", "Patch ref, if any, associated with this task.")
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

	// Fetch `sk`
	if _, err := exec.RunCwd(ctx, skiaDir, filepath.Join("bin", "fetch-sk")); err != nil {
		td.Fatal(ctx, err)
	}

	// Sync Chrome.
	checkoutRoot := cwd
	gitCacheDir := ""
	recipesCfgFile := filepath.Join(skiaDir, "infra", "config", "recipes.cfg")
	if !*local {
		// TODO(borenet): Don't hard-code these!
		checkoutRoot = filepath.Join(cwd, "cache", "work")
		gitCacheDir = filepath.Join(cwd, "cache", "git")
	}
	if err := botUpdate(ctx, checkoutRoot, gitCacheDir, *skiaRev, *patchRef, recipesCfgFile); err != nil {
		td.Fatal(ctx, err)
	}

	chromiumDir := filepath.Join(cwd, "src")
	outDir := filepath.Join(chromiumDir, "out", "Release")

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

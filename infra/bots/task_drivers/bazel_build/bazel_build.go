// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable runs a Bazel(isk) build command for a single label using the provided
// config (which is assumed to be in //bazel/buildrc) and any provided Bazel args.
// This handles any setup needed to run Bazel on our CI machines before running the task, like
// setting up logs and the Bazel cache.
package main

import (
	"context"
	"flag"
	"fmt"
	"io/fs"
	"path/filepath"

	infra_common "go.skia.org/infra/go/common"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

var (
	// Required properties for this task.
	projectId      = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId         = flag.String("task_id", "", "ID of this task.")
	taskName       = flag.String("task_name", "", "Name of the task.")
	workdir        = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	outPath        = flag.String("out_path", "", "Directory into which to copy the //bazel-bin subdirectories provided via --saved_output_dir. If unset, nothing will be copied.")
	savedOutputDir = infra_common.NewMultiStringFlag("saved_output_dir", nil, `//bazel-bin subdirectories to copy into the path provided via --out_path (e.g. "tests" will copy the contents of //bazel-bin/tests).`)

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		Label:          true,
		Config:         true,
		AdditionalArgs: true,
	})

	// StartRun calls flag.Parse()
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	if *outPath != "" && len(*savedOutputDir) == 0 {
		td.Fatal(ctx, fmt.Errorf("at least one --saved_output_dir is required if --out_path is set"))
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(wd, "skia")

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: *bazelFlags.CacheDir,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelBuild(ctx, skiaDir, *bazelFlags.Label, *bazelFlags.Config, *bazelFlags.AdditionalArgs...); err != nil {
		td.Fatal(ctx, err)
	}

	if *outPath != "" {
		if err := copyBazelBinSubdirs(ctx, skiaDir, *savedOutputDir, filepath.Join(wd, *outPath)); err != nil {
			td.Fatal(ctx, err)
		}
	}

	if !*local {
		if err := common.BazelCleanIfLowDiskSpace(ctx, *bazelFlags.CacheDir, skiaDir, "bazelisk"); err != nil {
			td.Fatal(ctx, err)
		}
	}
}

// bazelBuild builds the target referenced by the given absolute label passing the provided
// config and any additional args to the build command. Instead of calling Bazel directly, we use
// Bazelisk to make sure we use the right version of Bazel, as defined in the .bazelversion file
// at the Skia root.
func bazelBuild(ctx context.Context, checkoutDir, label, config string, args ...string) error {
	step := fmt.Sprintf("Build %s with config %s and %d extra flags", label, config, len(args))
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"build",
				label,
				"--config=" + config, // Should be defined in //bazel/buildrc
			}, args...),
			InheritEnv: true, // Makes sure bazelisk is on PATH
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

// copyBazelBinSubdirs copies the contents of the bazel-bin directory into the given path.
func copyBazelBinSubdirs(ctx context.Context, checkoutDir string, bazelBinSubdirs []string, destinationDir string) error {
	for _, subdir := range bazelBinSubdirs {
		if err := td.Do(ctx, td.Props(fmt.Sprintf("Copying bazel-bin subdirectory %q into %q", subdir, destinationDir)), func(ctx context.Context) error {
			srcDir := filepath.Join(checkoutDir, "bazel-bin", subdir)
			dstDir := filepath.Join(destinationDir, subdir)

			return filepath.WalkDir(srcDir, func(path string, d fs.DirEntry, err error) error {
				if err != nil {
					// A non-nil err argument tells us the reason why filepath.WalkDir will not walk into
					// that directory (see https://pkg.go.dev/io/fs#WalkDirFunc). We choose to fail loudly
					// as this might reveal permission issues, problems with symlinks, etc.
					return skerr.Wrap(err)
				}

				relPath, err := filepath.Rel(srcDir, path)
				if err != nil {
					return skerr.Wrap(err)
				}
				dstPath := filepath.Join(dstDir, relPath)

				if d.IsDir() {
					return skerr.Wrap(os_steps.MkdirAll(ctx, dstPath))
				}
				return skerr.Wrap(os_steps.CopyFile(ctx, path, dstPath))
			})
		}); err != nil {
			return skerr.Wrap(err)
		}
	}
	return nil
}

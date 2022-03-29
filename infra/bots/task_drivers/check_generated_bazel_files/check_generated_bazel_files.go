// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable checks that there are no diffs to the BUILD.bazel files after we run
// gazelle to autogenerate the CPP rules (e.g. generated_cc_atom)
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strconv"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// This value is arbitrarily selected.
const rbeJobs = 100

var (
	// Required properties for this task.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaDir := filepath.Join(wd, "skia")

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := ensureNoGeneratedDiffs(ctx, skiaDir); err != nil {
		td.Fatal(ctx, err)
	}

	if err := smokeTestBuild(ctx, skiaDir); err != nil {
		td.Fatal(ctx, err)
	}
}

func ensureNoGeneratedDiffs(ctx context.Context, checkoutDir string) error {
	// CAS doesn't keep the .git folder around, so we cannot use git diff.
	// However, we can copy just the BUILD.bazel files into a temporary directory
	// regenerate the build.bazel files, copy the BUILD.bazel files to a different
	// directory and then use the standard diff tool to compare them.
	beforeDir, err := os.MkdirTemp("", "before")
	if err != nil {
		return skerr.Wrap(err)
	}
	defer os.RemoveAll(beforeDir)
	afterDir, err := os.MkdirTemp("", "after")
	if err != nil {
		return skerr.Wrap(err)
	}
	defer os.RemoveAll(afterDir)

	return td.Do(ctx, td.Props("Regenerate Bazel files"), func(ctx context.Context) error {
		if err := copyAllBUILDFilesTo(ctx, checkoutDir, beforeDir); err != nil {
			return err
		}

		genCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"generate"},
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        filepath.Join(checkoutDir, "bazel"),
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err = sk_exec.RunCommand(ctx, genCmd)
		if err != nil {
			return err
		}

		if err := copyAllBUILDFilesTo(ctx, checkoutDir, afterDir); err != nil {
			return err
		}

		// diff returns a non-zero exit code if it finds a diff. If it does, the diffs will
		// be written to stderr, which shows up in the error returned by RunCommand.
		diffCmd := &sk_exec.Command{
			Name:      "diff",
			Args:      []string{"--unified", "--recursive", beforeDir, afterDir},
			Dir:       checkoutDir,
			LogStdout: true,
			LogStderr: true,
		}
		_, err := sk_exec.RunCommand(ctx, diffCmd)
		if err != nil {
			// By returning the diff in an error, it shows up in a prominent place in the
			// task driver logs, so it is more clear how to fix this.
			return errors.New("Unexpected diffs after generating Bazel files\n" + err.Error() + `


You need to run make generate from //bazel.
You may also need to manually add your new files to srcs
in the BUILD.bazel of the parent of your added files.
`)
		}
		return nil
	})
}

func copyAllBUILDFilesTo(ctx context.Context, srcDir, targetDir string) error {
	copyCmd := &sk_exec.Command{
		Name: "bash",
		// We want ** to expand with the shell, so we need to use bash -c
		// Moreover, we want to match all BUILD.bazel files, no matter how deep, so we need to
		// set the globstar option, which is off by default https://askubuntu.com/a/1010708
		Args:      []string{"-c", `shopt -s globstar && cp --recursive --parents ./**/BUILD.bazel ` + targetDir},
		Dir:       srcDir,
		LogStdout: true,
		LogStderr: true,
	}
	_, err := sk_exec.RunCommand(ctx, copyCmd)
	return err
}

// smokeTestBuild builds and links something with Bazelisk to make sure the generated BUILD.bazel
// files and the manually curated lists of files continue to work.
func smokeTestBuild(ctx context.Context, checkoutDir string) error {
	return td.Do(ctx, td.Props("Smoke test compile+link"), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: []string{"build",
				"--config=linux-rbe", // Compile using RBE
				"--jobs=" + strconv.Itoa(rbeJobs),
				"//example:hello_world_gl", // This compiles and links, so is a good smoke test
			},
			InheritEnv: true, // Makes sure bazelisk is on PATH
			Dir:        checkoutDir,
			LogStdout:  true,
			LogStderr:  true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			fmt.Println("===============================================")
			fmt.Println("Bazel smoke test failed!")
			fmt.Println("You may need to manually update the :srcs target in the appropriate")
			fmt.Println("BUILD.bazel file to include files/targets you added or delete ones")
			fmt.Println("corresponding to deleted files.")
			return err
		}
		return nil
	})
}

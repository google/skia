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
	"os"
	"path/filepath"
	"runtime"

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
	workdir        = flag.String("workdir", ".", "Working directory in which the build will be performed.")
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

	checkoutPath, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// For Mac builds, ensure Xcode is downloaded/installed via mac_toolchain before we build.
	if runtime.GOOS == "darwin" {
		parentDir := filepath.Dir(checkoutPath)
		macToolchainBin := filepath.Join(parentDir, "mac_toolchain", "mac_toolchain")
		xcodeAppPath := filepath.Join(parentDir, "cache", "Xcode.app")

		if _, err := os.Stat(macToolchainBin); err == nil {
			err = td.Do(ctx, td.Props("Ensure Xcode via mac_toolchain"), func(ctx context.Context) error {
				runCmd := &sk_exec.Command{
					Name: macToolchainBin,
					Args: []string{
						"install",
						"-kind", "ios",
						"-xcode-version", "16a242d", // Xcode 16.0
						"-output-dir", xcodeAppPath,
					},
					InheritEnv: true,
					LogStdout:  true,
					LogStderr:  true,
				}
				_, runErr := sk_exec.RunCommand(ctx, runCmd)
				return runErr
			})
			if err != nil {
				td.Fatal(ctx, err)
			}
		}

		cachedXcodeDevDir := filepath.Join(xcodeAppPath, "Contents", "Developer")
		if _, err := os.Stat(cachedXcodeDevDir); err == nil {
			// We set DEVELOPER_DIR to the absolute path of the cached Xcode to bypass
			// the global system-wide 'xcode-select' pointer. This is similar to
			// 'sudo xcode-select -switch' but doesn't affect the global state.
			os.Setenv("DEVELOPER_DIR", cachedXcodeDevDir)
		}
	}

	var outputPath string
	if *outPath != "" {
		outputPath, err = os_steps.Abs(ctx, *outPath)
		if err != nil {
			td.Fatal(ctx, err)
		}
	}

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: *bazelFlags.CacheDir,
	}
	bzl, err := bazel.New(ctx, checkoutPath, "", opts)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Schedule the cleanup steps.
	defer func() {
		if !*local {
			// Ignore any error here until after we've run "shutdown".
			cleanErr := common.BazelCleanIfLowDiskSpace(ctx, *bazelFlags.CacheDir, checkoutPath, "bazelisk")
			if _, err := bzl.Do(ctx, "shutdown"); err != nil {
				td.Fatal(ctx, err)
			}
			if cleanErr != nil {
				td.Fatal(ctx, cleanErr)
			}
		}
	}()

	// Perform the build.
	args := append([]string{*bazelFlags.Label, fmt.Sprintf("--config=%s", *bazelFlags.Config)}, *bazelFlags.AdditionalArgs...)
	if os.Getenv("DEVELOPER_DIR") != "" {
		args = append(args, "--repo_env=DEVELOPER_DIR")
	}
	if runtime.GOOS == "windows" {
		// Dynamically locate and add the MSVC compiler and runtime folders to the system PATH.
		var newPATH, bazelVC string
		ctx, newPATH, bazelVC = locateAndAddMSVCToPATH(ctx, checkoutPath)

		// Bazel's strict action environment strips the PATH variable from actions (e.g. the
		// win_toolchain we add to PATH in locateAndAddMSVCToPATH). While we compile our own
		//generated tools and targets statically (/NODEFAULTLIB in the toolchain_config.bzl) to
		// make them self-contained, prebuilt binaries (like rustc.exe and compiler drivers like
		// clang-cl.exe) are dynamically linked and must resolve their MSVC runtime DLLs (and
		// link.exe) from the PATH during action execution.
		//
		// We would like to say "Bazel, just pass in our PATH when running commands" (which is
		// --action_env=PATH and --host_action_env=PATH). However, this does not work because
		// Windows has PATH spelled "Path" (mixed case). If you don't believe me, run the following
		// in a PowerShell environment:
		// [System.Environment]::GetEnvironmentVariables().Keys | Where-Object { $_ -like "*path*" })
		// So, since Bazel rules use "PATH", it doesn't get copied correctly. Thus we make things
		// explicit by listing the full PATH. For good measure, we do this with BAZEL_VC
		// https://bazel.build/configure/windows
		args = append(args, fmt.Sprintf("--action_env=PATH=%s", newPATH))
		args = append(args, fmt.Sprintf("--host_action_env=PATH=%s", newPATH))

		args = append(args, fmt.Sprintf("--action_env=BAZEL_VC=%s", bazelVC))
		args = append(args, fmt.Sprintf("--host_action_env=BAZEL_VC=%s", bazelVC))
	}

	if _, err := bzl.Do(ctx, "build", args...); err != nil {
		td.Fatal(ctx, err)
	}

	if outputPath != "" {
		if err := copyBazelBinSubdirs(ctx, checkoutPath, *savedOutputDir, outputPath); err != nil {
			td.Fatal(ctx, err)
		}
	}
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

// locateAndAddMSVCToPATH resolves the MSVC C++ runtime directory (sys64), the MSVC compiler tools
// directory, and the VC directory, and appends them to PATH so that bazel.exe/bazelisk.exe and
// compiler drivers (clang-cl) can run on Windows. It sets BAZEL_VC as well to make sure
// our CIPD win_toolchain is used by Bazel. This also returns the updated PATH and BAZEL_VC as
// strings so they can be added as arguments to the command (e.g. with --action_env)
func locateAndAddMSVCToPATH(ctx context.Context, checkoutPath string) (context.Context, string, string) {
	var newPATH, bazelVC string
	err := td.Do(ctx, td.Props("MSVC Runtime Path Resolver"), func(ctx context.Context) error {
		winToolchainDir := "win_toolchain"
		if _, err := os.Stat(winToolchainDir); err != nil {
			winToolchainDir = filepath.Join(filepath.Dir(checkoutPath), "win_toolchain")
		}

		sys64Dir, _ := filepath.Abs(filepath.Join(winToolchainDir, "sys64"))
		vcDir, _ := filepath.Abs(filepath.Join(winToolchainDir, "VC"))

		// Dynamically resolve the MSVC version directory under VC/Tools/MSVC/ (e.g. 14.51.36231)
		// without hardcoding to avoid maintenance overhead when updating the Windows toolchain.
		msvcRoot := filepath.Join(winToolchainDir, "VC", "Tools", "MSVC")
		var msvcVersion string
		if entries, err := os.ReadDir(msvcRoot); err == nil {
			for _, entry := range entries {
				if entry.IsDir() {
					msvcVersion = entry.Name()
					break
				}
			}
		}

		var compilerBinDir string
		if msvcVersion != "" {
			compilerBinDir, _ = filepath.Abs(filepath.Join(msvcRoot, msvcVersion, "bin", "Hostx64", "x64"))
		} else {
			return fmt.Errorf("MSVC compiler root directory empty or not found: %s\n", msvcRoot)
		}

		if _, err := os.Stat(sys64Dir); err != nil {
			return fmt.Errorf("MSVC sys64 folder not found at %s: %s\n", sys64Dir, err)
		}
		currentPATH := os.Getenv("PATH")
		// Add sys64 for runtime DLLs, and compilerBinDir so clang-cl can find link.exe
		newPATH = fmt.Sprintf("%s;%s;%s", sys64Dir, compilerBinDir, currentPATH)
		os.Setenv("PATH", newPATH)
		os.Setenv("BAZEL_VC", vcDir)
		// Makes debugging easier to list these out
		td.StepText(ctx, "PATH env var", newPATH)
		td.StepText(ctx, "BAZEL_VC env var", vcDir)
		bazelVC = vcDir
		return nil
	})
	if err != nil {
		td.Fatal(ctx, err)
	}
	return td.WithEnv(ctx, []string{
		"PATH=" + newPATH,
		"BAZEL_VC=" + bazelVC,
	}), newPATH, bazelVC
}

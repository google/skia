// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds and tests CanvasKit. The tests produce images (aka gms) and these
// are uploaded to Gold.
// It requires unzip to be installed (which Bazel already requires).
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
)

// This value is arbitrarily selected. It is smaller than our maximum RBE pool size.
const rbeJobs = 100

var (
	// Required properties for this task.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory, the root directory of a full Skia checkout")
	// goldctl data
	goldctlPath      = flag.String("goldctl_path", "", "The path to the golctl binary on disk.")
	gitCommit        = flag.String("git_commit", "", "The git hash to which the data should be associated. This will be used when changelist_id and patchset_order are not set to report data to Gold that belongs on the primary branch.")
	changelistID     = flag.String("changelist_id", "", "Should be non-empty only when run on the CQ.")
	patchsetOrderStr = flag.String("patchset_order", "", "Should be non-zero only when run on the CQ.")
	tryjobID         = flag.String("tryjob_id", "", "Should be non-zero only when run on the CQ.")
	// goldctl keys
	browser         = flag.String("browser", "Chrome", "The browser running the tests")
	compilationMode = flag.String("compilation_mode", "Release", "How the binary was compiled")
	cpuOrGPU        = flag.String("cpu_or_gpu", "GPU", "The render backend")
	cpuOrGPUValue   = flag.String("cpu_or_gpu_value", "WebGL2", "What variant of the render backend")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the CI/CQ)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	bazelFlags := common.MakeBazelFlags(common.MakeBazelFlagsOpts{
		Label:  true,
		Config: true,
	})

	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	bazelFlags.Validate(ctx)

	goldctlAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *goldctlPath, "gold_ctl_path")
	wd := td.MustGetAbsolutePathOfFlag(ctx, *workdir, "workdir")
	skiaDir := filepath.Join(wd, "skia")
	patchsetOrder := 0
	if *patchsetOrderStr != "" {
		var err error
		patchsetOrder, err = strconv.Atoi(*patchsetOrderStr)
		if err != nil {
			fmt.Println("Non-integer value passed in to --patchset_order")
			td.Fatal(ctx, err)
		}
	}

	opts := bazel.BazelOptions{
		// We want the cache to be on a bigger disk than default. The root disk, where the home
		// directory (and default Bazel cache) lives, is only 15 GB on our GCE VMs.
		CachePath: *bazelFlags.CacheDir,
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := bazelTest(ctx, skiaDir, *bazelFlags.Label, *bazelFlags.Config,
		"--config=linux_rbe", "--test_output=streamed", "--jobs="+strconv.Itoa(rbeJobs)); err != nil {
		td.Fatal(ctx, err)
	}

	conf := goldctlConfig{
		goldctlPath:   goldctlAbsPath,
		gitCommit:     *gitCommit,
		changelistID:  *changelistID,
		patchsetOrder: patchsetOrder,
		tryjobID:      *tryjobID,
		corpus:        "canvaskit",
		keys: map[string]string{
			"arch":             "wasm32", // https://github.com/bazelbuild/platforms/blob/da5541f26b7de1dc8e04c075c99df5351742a4a2/cpu/BUILD#L109
			"configuration":    *bazelFlags.Config,
			"browser":          *browser,
			"compilation_mode": *compilationMode,
			"cpu_or_gpu":       *cpuOrGPU,
			"cpu_or_gpu_value": *cpuOrGPUValue,
		},
	}
	if err := uploadDataToGold(ctx, *bazelFlags.Label, skiaDir, conf); err != nil {
		td.Fatal(ctx, err)
	}

	if !*local {
		if err := common.BazelCleanIfLowDiskSpace(ctx, *bazelFlags.CacheDir, skiaDir, "bazelisk"); err != nil {
			td.Fatal(ctx, err)
		}
	}
}

func bazelTest(ctx context.Context, checkoutDir, label, config string, args ...string) error {
	step := fmt.Sprintf("Running Test %s with config %s and %d extra flags", label, config, len(args))
	return td.Do(ctx, td.Props(step), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name: "bazelisk",
			Args: append([]string{"test",
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

type goldctlConfig struct {
	goldctlPath   string
	gitCommit     string
	changelistID  string
	patchsetOrder int
	tryjobID      string
	corpus        string
	keys          map[string]string
}

func uploadDataToGold(ctx context.Context, label, checkoutDir string, cfg goldctlConfig) error {
	return td.Do(ctx, td.Props("Upload to Gold"), func(ctx context.Context) error {
		zipExtractDir, err := os_steps.TempDir(ctx, "", "gold_outputs")
		if err != nil {
			return err
		}

		// Turn "//path/to:target" into "path/to/target".
		labelBazelTestlogsPath := strings.ReplaceAll(label, "//", "")
		labelBazelTestlogsPath = strings.ReplaceAll(label, ":", "/")

		if err := extractZip(ctx, filepath.Join(checkoutDir, "bazel-testlogs", labelBazelTestlogsPath, "test.outputs", "outputs.zip"), zipExtractDir); err != nil {
			return err
		}

		goldWorkDir, err := os_steps.TempDir(ctx, "", "gold_workdir")
		if err != nil {
			return err
		}

		if err := setupGoldctl(ctx, cfg, goldWorkDir); err != nil {
			return err
		}

		if err := addAllGoldImages(ctx, cfg.goldctlPath, zipExtractDir, goldWorkDir); err != nil {
			return err
		}

		if err := finalizeGoldctl(ctx, cfg.goldctlPath, goldWorkDir); err != nil {
			return err
		}
		return nil
	})
}

func extractZip(ctx context.Context, zipPath, targetDir string) error {
	runCmd := &sk_exec.Command{
		Name:      "unzip",
		Args:      []string{zipPath, "-d", targetDir},
		LogStdout: true,
		LogStderr: true,
	}
	_, err := sk_exec.RunCommand(ctx, runCmd)
	if err != nil {
		return err
	}
	return nil
}

func setupGoldctl(ctx context.Context, cfg goldctlConfig, workDir string) error {
	authCmd := &sk_exec.Command{
		Name:      cfg.goldctlPath,
		Args:      []string{"auth", "--work-dir=" + workDir, "--luci"},
		LogStdout: true,
		LogStderr: true,
	}
	if _, err := sk_exec.RunCommand(ctx, authCmd); err != nil {
		return err
	}

	initArgs := []string{"imgtest", "init", "--work-dir", workDir,
		"--instance", "skia", "--corpus", cfg.corpus,
		"--commit", cfg.gitCommit, "--url", "https://gold.skia.org", "--bucket", "skia-infra-gm"}

	if cfg.changelistID != "" {
		ps := strconv.Itoa(cfg.patchsetOrder)
		initArgs = append(initArgs, "--crs", "gerrit", "--changelist", cfg.changelistID,
			"--patchset", ps, "--cis", "buildbucket", "--jobid", cfg.tryjobID)
	}

	for key, value := range cfg.keys {
		initArgs = append(initArgs, "--key="+key+":"+value)
	}

	initCmd := &sk_exec.Command{
		Name:      cfg.goldctlPath,
		Args:      initArgs,
		LogStdout: true,
		LogStderr: true,
	}
	if _, err := sk_exec.RunCommand(ctx, initCmd); err != nil {
		return err
	}
	return nil
}

func addAllGoldImages(ctx context.Context, goldctlPath, pngsDir, workDir string) error {
	pngFiles, err := os.ReadDir(pngsDir)
	if err != nil {
		return err
	}
	return td.Do(ctx, td.Props(fmt.Sprintf("Upload %d images to Gold", len(pngFiles))), func(ctx context.Context) error {
		for _, entry := range pngFiles {
			// We expect the filename to be testname.optional_config.png
			baseName := filepath.Base(entry.Name())
			parts := strings.Split(baseName, ".")
			testName := parts[0]
			addArgs := []string{
				"imgtest", "add",
				"--work-dir", workDir,
				"--png-file", filepath.Join(pngsDir, filepath.Base(entry.Name())),
				"--test-name", testName,
			}
			if len(parts) == 3 {
				// There was a config specified.
				addArgs = append(addArgs, "--add-test-key=config:"+parts[1])
			}

			addCmd := &sk_exec.Command{
				Name:      goldctlPath,
				Args:      addArgs,
				LogStdout: true,
				LogStderr: true,
			}
			if _, err := sk_exec.RunCommand(ctx, addCmd); err != nil {
				return err
			}
		}
		return nil
	})
}

// finalizeGoldctl uploads the JSON file created from adding all the test PNGs. Then, Gold begins
// ingesting the data.
func finalizeGoldctl(ctx context.Context, goldctlPath, workDir string) error {
	finalizeCmd := &sk_exec.Command{
		Name:      goldctlPath,
		Args:      []string{"imgtest", "finalize", "--work-dir=" + workDir},
		LogStdout: true,
		LogStderr: true,
	}
	if _, err := sk_exec.RunCommand(ctx, finalizeCmd); err != nil {
		return err
	}
	return nil
}

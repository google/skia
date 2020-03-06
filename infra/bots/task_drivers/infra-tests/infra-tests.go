package main

import (
	"flag"
	"fmt"
	"path/filepath"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/git"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/dirs"
	"go.skia.org/infra/task_driver/go/lib/golang"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory")

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	// Setup Go.
	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	ctx = golang.WithEnv(ctx, wd)
	skiaDir := filepath.Join(wd, "skia")

	// We get depot_tools via isolate. It's required for some tests.
	ctx = td.WithEnv(ctx, []string{fmt.Sprintf("SKIABOT_TEST_DEPOT_TOOLS=%s", dirs.DepotTools(*workdir))})

	// Initialize the Git repo. We receive the code via Isolate, but it
	// doesn't include the .git dir.
	gd := git.GitDir(skiaDir)
	if gitVer, err := gd.Git(ctx, "version"); err != nil {
		td.Fatal(ctx, err)
	} else {
		sklog.Infof("Git version %s", gitVer)
	}
	if _, err := gd.Git(ctx, "init"); err != nil {
		td.Fatal(ctx, err)
	}
	if _, err := gd.Git(ctx, "config", "--local", "user.name", "Skia bots"); err != nil {
		td.Fatal(ctx, err)
	}
	if _, err := gd.Git(ctx, "config", "--local", "user.email", "fake@skia.bots"); err != nil {
		td.Fatal(ctx, err)
	}
	if _, err := gd.Git(ctx, "add", "."); err != nil {
		td.Fatal(ctx, err)
	}
	if _, err := gd.Git(ctx, "commit", "--no-verify", "-m", "Fake commit to satisfy recipe tests"); err != nil {
		td.Fatal(ctx, err)
	}

	// Run the infra tests.
	if _, err := exec.RunCwd(ctx, filepath.Join(gd.Dir(), "infra", "bots"), "python", "-u", "infra_tests.py"); err != nil {
		td.Fatal(ctx, err)
	}

	// Sanity check; none of the above should have modified the go.mod file.
	if _, err := gd.Git(ctx, "diff", "--no-ext-diff", "--exit-code", "go.mod"); err != nil {
		td.Fatal(ctx, err)
	}
}

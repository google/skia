// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds the Docker images based off the Skia executables in the
// gcr.io/skia-public/skia-release image. It then issues a PubSub notification to have those apps
// tagged and deployed by docker_pushes_watcher.
// See //docker_pushes_watcher/README.md in the infra repo for more.
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path"
	"path/filepath"

	"cloud.google.com/go/pubsub"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/common"
	docker_pubsub "go.skia.org/infra/go/docker/build/pubsub"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/docker"
	"go.skia.org/infra/task_driver/go/lib/golang"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/infra/task_scheduler/go/types"
)

var (
	// Required properties for this task.
	projectId     = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId        = flag.String("task_id", "", "ID of this task.")
	taskName      = flag.String("task_name", "", "Name of the task.")
	workdir       = flag.String("workdir", ".", "Working directory")
	infraRevision = flag.String("infra_revision", "origin/main", "Specifies which revision of the infra repo the images should be built off")

	checkoutFlags = checkout.SetupFlags(nil)

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

const (
	fiddlerImageName = "fiddler"
	apiImageName     = "api"
)

func buildPushFiddlerImage(ctx context.Context, dkr *docker.Docker, tag, infraCheckoutDir string, topic *pubsub.Topic) error {
	// Run skia-release image and extract products out of /tmp/skia/skia. See
	// https://skia.googlesource.com/skia/+/0e845dc8b05cb2d40d1c880184e33dd76081283a/docker/skia-release/Dockerfile#33
	productsDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", productsDir),
	}
	skiaCopyCmd := []string{"/bin/sh", "-c", "cd /tmp; tar cvzf skia.tar.gz --directory=/tmp/skia skia; cp /tmp/skia.tar.gz /OUT/"}
	releaseImg := fmt.Sprintf("gcr.io/skia-public/skia-release:%s", tag)
	if err := dkr.Run(ctx, releaseImg, skiaCopyCmd, volumes, nil); err != nil {
		return err
	}

	err = td.Do(ctx, td.Props("Build "+fiddlerImageName+" image").Infra(), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"release-fiddler-ci"},
			InheritEnv: true,
			Env: []string{
				"COPY_FROM_DIR=" + productsDir,
				"STABLE_DOCKER_TAG=" + tag,
			},
			Dir:       filepath.Join(infraCheckoutDir, "fiddlek"),
			LogStdout: true,
			LogStderr: true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		return err
	}
	if err := docker.PublishToTopic(ctx, "gcr.io/skia-public/"+fiddlerImageName, tag, common.REPO_SKIA, topic); err != nil {
		return err
	}

	return cleanupTempFiles(ctx, dkr, releaseImg, volumes)
}

func cleanupTempFiles(ctx context.Context, dkr *docker.Docker, image string, volumes []string) error {
	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	return dkr.Run(ctx, image, cleanupCmd, volumes, nil)
}

func buildPushApiImage(ctx context.Context, dkr *docker.Docker, tag, checkoutDir, infraCheckoutDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	// Change perms of the directory for doxygen to be able to write to it.
	if err := os.Chmod(tempDir, 0777); err != nil {
		return err
	}
	// Run Doxygen pointing to the location of the checkout and the out dir.
	volumes := []string{
		fmt.Sprintf("%s:/OUT", tempDir),
		fmt.Sprintf("%s:/CHECKOUT", checkoutDir),
	}
	env := []string{
		"OUTPUT_DIRECTORY=/OUT",
	}
	doxygenCmd := []string{"/bin/sh", "-c", "cd /CHECKOUT/tools/doxygen && doxygen ProdDoxyfile"}
	doxygenImg := "gcr.io/skia-public/doxygen:testing-slim"
	if err := dkr.Run(ctx, doxygenImg, doxygenCmd, volumes, env); err != nil {
		return err
	}

	err = td.Do(ctx, td.Props("Build "+apiImageName+" image").Infra(), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"release-api-ci"},
			InheritEnv: true,
			Env: []string{
				"COPY_FROM_DIR=" + filepath.Join(tempDir, "html"),
				"STABLE_DOCKER_TAG=" + tag,
			},
			Dir:       filepath.Join(infraCheckoutDir, "api"),
			LogStdout: true,
			LogStderr: true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		return err
	}
	if err := docker.PublishToTopic(ctx, "gcr.io/skia-public/"+apiImageName, tag, common.REPO_SKIA, topic); err != nil {
		return err
	}

	return cleanupTempFiles(ctx, dkr, doxygenImg, volumes)
}

func main() {
	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if *infraRevision == "" {
		td.Fatalf(ctx, "Must specify --infra_revision")
	}

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, err)
	}
	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	// Check out the Skia repo code.
	co, err := checkout.EnsureGitCheckout(ctx, path.Join(wd, "repo"), rs)
	if err != nil {
		td.Fatal(ctx, err)
	}
	skiaCheckoutDir := co.Dir()

	// Checkout out the Skia infra repo at the specified commit.
	infraRS := types.RepoState{
		Repo:     common.REPO_SKIA_INFRA,
		Revision: *infraRevision,
	}
	infraCheckoutDir := filepath.Join("infra_repo")
	if _, err := checkout.EnsureGitCheckout(ctx, infraCheckoutDir, infraRS); err != nil {
		td.Fatal(ctx, err)
	}

	// Setup go.
	ctx = golang.WithEnv(ctx, wd)

	// Ensure that the bazel cache is setup.
	opts := bazel.BazelOptions{
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	// Create token source with scope for cloud registry (storage) and pubsub.
	ts, err := auth_steps.Init(ctx, *local, auth.ScopeUserinfoEmail, auth.ScopeFullControl, pubsub.ScopePubSub)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Create pubsub client.
	client, err := pubsub.NewClient(ctx, docker_pubsub.TOPIC_PROJECT_ID, option.WithTokenSource(ts))
	if err != nil {
		td.Fatal(ctx, err)
	}
	topic := client.Topic(docker_pubsub.TOPIC)

	// Figure out which tag to use for docker build and push.
	tag := rs.Revision
	if rs.Issue != "" && rs.Patchset != "" {
		tag = fmt.Sprintf("%s_%s", rs.Issue, rs.Patchset)
	}

	// Instantiate docker.
	dkr, err := docker.New(ctx, ts)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Build and push all apps of interest below.
	if err := buildPushApiImage(ctx, dkr, tag, skiaCheckoutDir, infraCheckoutDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
	if err := buildPushFiddlerImage(ctx, dkr, tag, infraCheckoutDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
}

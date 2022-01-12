// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds the Docker images based off the WASM executables in the
// gcr.io/skia-public/skia-wasm-release image. It then issues a PubSub notification to have those apps
// tagged and deployed by docker_pushes_watcher.
// See //docker_pushes_watcher/README.md in the skia infra repo.
package main

import (
	"context"
	"flag"
	"fmt"
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
	skiaRevision  = flag.String("skia_revision", "", "Specifies which revision of Skia should be used to find the docker image containing the WASM products.")
	infraRevision = flag.String("infra_revision", "origin/main", "Specifies which revision of the infra repo the images should be built off")
	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

func main() {
	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	if *skiaRevision == "" {
		td.Fatalf(ctx, "Must specify --skia_revision")
	}

	checkoutDir := wd
	if !*local {
		if *infraRevision == "" {
			td.Fatalf(ctx, "Must specify --infra_revision")
		}
		// Check out the Skia infra repo at the specified commit.
		rs := types.RepoState{
			Repo:     common.REPO_SKIA_INFRA,
			Revision: *infraRevision,
		}
		checkoutDir = filepath.Join("repo")
		_, err = checkout.EnsureGitCheckout(ctx, checkoutDir, rs)
		if err != nil {
			td.Fatal(ctx, err)
		}
	}

	// Setup go.
	ctx = golang.WithEnv(ctx, wd)

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

	dkr, err := docker.New(ctx, ts)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Run skia-wasm-release image and extract wasm products out of it.
	wasmProductsDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		td.Fatal(ctx, err)
	}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", wasmProductsDir),
	}
	wasmCopyCmd := []string{"/bin/sh", "-c", "cp -r /tmp/* /OUT"}
	releaseImg := fmt.Sprintf("gcr.io/skia-public/skia-wasm-release:%s", *skiaRevision)
	if err := dkr.Run(ctx, releaseImg, wasmCopyCmd, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}

	opts := bazel.BazelOptions{
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	if err := buildPush(ctx, "debugger-app", wasmProductsDir, checkoutDir, *skiaRevision, topic); err != nil {
		td.Fatal(ctx, err)
	}

	if err := buildPush(ctx, "jsfiddle", wasmProductsDir, checkoutDir, *skiaRevision, topic); err != nil {
		td.Fatal(ctx, err)
	}

	if err := buildPush(ctx, "particles", wasmProductsDir, checkoutDir, *skiaRevision, topic); err != nil {
		td.Fatal(ctx, err)
	}

	if err := buildPush(ctx, "shaders", wasmProductsDir, checkoutDir, *skiaRevision, topic); err != nil {
		td.Fatal(ctx, err)
	}

	if err := buildPush(ctx, "skottie", wasmProductsDir, checkoutDir, *skiaRevision, topic); err != nil {
		td.Fatal(ctx, err)
	}

	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	if err := dkr.Run(ctx, releaseImg, cleanupCmd, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}
}

// buildPush pushes the app in the given skia-infra folder using `make bazel_release_ci`.
// By convention, all of these apps should have the k8s deployment name be the same as the app
// name and be built using the same Make target.
func buildPush(ctx context.Context, appName, wasmProductsDir, checkoutDir, skiaRevision string, topic *pubsub.Topic) error {
	err := td.Do(ctx, td.Props("Build "+appName+" image").Infra(), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"bazel_release_ci"},
			InheritEnv: true,
			Env: []string{
				"COPY_FROM_DIR=" + wasmProductsDir,
				"STABLE_DOCKER_TAG=" + skiaRevision,
			},
			Dir:       filepath.Join(checkoutDir, appName),
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
	return docker.PublishToTopic(ctx, "gcr.io/skia-public/"+appName, skiaRevision, common.REPO_SKIA, topic)
}

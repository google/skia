// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds the Docker images based off the WASM executables in the
// gcr.io/skia-public/skia-wasm-release image. It then issues a PubSub notification to have those apps
// tagged and deployed by docker_pushes_watcher.
// See //docker_pushes_watcher/README.md in the infra repo for more.
package main

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"

	"cloud.google.com/go/pubsub"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	docker_pubsub "go.skia.org/infra/go/docker/build/pubsub"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/docker"
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

	checkoutFlags = checkout.SetupFlags(nil)

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

const (
	debuggerImageName  = "debugger-app"
	particlesImageName = "particles"
	shaderImageName    = "shaders"
	skottieImageName   = "skottie"
)

var (
	infraCommonEnv = []string{
		"SKIP_BUILD=1",
		"ROOT=/WORKSPACE",
	}
)

func buildPushSkottieImage(ctx context.Context, tag, repo, wasmProductsDir, configDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	image := fmt.Sprintf("gcr.io/skia-public/%s", skottieImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/skottie && make release_ci"}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", wasmProductsDir),
		fmt.Sprintf("%s:/WORKSPACE", tempDir),
	}
	return docker.BuildPushImageFromInfraImage(ctx, "Skottie", image, tag, repo, configDir, tempDir, "prod", topic, cmd, volumes, infraCommonEnv, nil)
}

func buildPushParticlesImage(ctx context.Context, tag, repo, wasmProductsDir, configDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	image := fmt.Sprintf("gcr.io/skia-public/%s", particlesImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/particles && make release_ci"}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", wasmProductsDir),
		fmt.Sprintf("%s:/WORKSPACE", tempDir),
	}
	return docker.BuildPushImageFromInfraImage(ctx, "Particles", image, tag, repo, configDir, tempDir, "prod", topic, cmd, volumes, infraCommonEnv, nil)
}

func buildPushDebuggerImage(ctx context.Context, tag, repo, wasmProductsDir, configDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	image := fmt.Sprintf("gcr.io/skia-public/%s", debuggerImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/debugger-app && make release_ci"}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", wasmProductsDir),
		fmt.Sprintf("%s:/WORKSPACE", tempDir),
	}
	return docker.BuildPushImageFromInfraImage(ctx, "Debugger-App", image, tag, repo, configDir, tempDir, "prod", topic, cmd, volumes, infraCommonEnv, nil)
}

func buildPushShadersImage(ctx context.Context, tag, repo, wasmProductsDir, configDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	image := fmt.Sprintf("gcr.io/skia-public/%s", shaderImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/shaders && make release_ci"}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", wasmProductsDir),
		fmt.Sprintf("%s:/WORKSPACE", tempDir),
	}
	return docker.BuildPushImageFromInfraImage(ctx, "Shaders", image, tag, repo, configDir, tempDir, "prod", topic, cmd, volumes, infraCommonEnv, nil)
}

func main() {
	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, err)
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
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

	// Figure out which tag to use for docker build and push.
	tag := rs.Revision
	if rs.Issue != "" && rs.Patchset != "" {
		tag = fmt.Sprintf("%s_%s", rs.Issue, rs.Patchset)
	}

	// Create a temporary config dir for Docker.
	configDir, err := ioutil.TempDir("", "")
	if err != nil {
		td.Fatal(ctx, err)
	}
	defer util.RemoveAll(configDir)

	// Login to docker (required to push to docker).
	token, err := ts.Token()
	if err != nil {
		td.Fatal(ctx, err)
	}
	if err := docker.Login(ctx, token.AccessToken, "gcr.io/skia-public/", configDir); err != nil {
		td.Fatal(ctx, err)
	}

	// Run skia-wasm-release image and extract wasm products out of it.
	wasmProductsDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		td.Fatal(ctx, err)
	}
	// Run Doxygen pointing to the location of the checkout and the out dir.
	volumes := []string{
		fmt.Sprintf("%s:/OUT", wasmProductsDir),
	}
	wasmCopyCmd := []string{"/bin/sh", "-c", "cp -r /tmp/* /OUT"}
	releaseImg := fmt.Sprintf("gcr.io/skia-public/skia-wasm-release:%s", tag)
	if err := docker.Run(ctx, releaseImg, configDir, wasmCopyCmd, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}

	// Build and push all apps of interest below.
	if err := buildPushSkottieImage(ctx, tag, rs.Repo, wasmProductsDir, configDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
	if err := buildPushParticlesImage(ctx, tag, rs.Repo, wasmProductsDir, configDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
	if err := buildPushDebuggerImage(ctx, tag, rs.Repo, wasmProductsDir, configDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
	if err := buildPushShadersImage(ctx, tag, rs.Repo, wasmProductsDir, configDir, topic); err != nil {
		td.Fatal(ctx, err)
	}

	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	if err := docker.Run(ctx, releaseImg, configDir, cleanupCmd, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}
}

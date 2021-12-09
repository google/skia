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
	"io/ioutil"
	"os"
	"path"

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
	fiddlerImageName = "fiddler"
	apiImageName     = "api"
)

var (
	infraCommonEnv = []string{
		"SKIP_BUILD=1",
		"ROOT=/OUT",
	}
	infraCommonBuildArgs = map[string]string{
		"SKIA_IMAGE_NAME": "skia-release",
	}
)

func buildPushFiddlerImage(ctx context.Context, tag, repo, configDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	image := fmt.Sprintf("gcr.io/skia-public/%s", fiddlerImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/fiddlek && ./build_fiddler_release"}
	volumes := []string{fmt.Sprintf("%s:/OUT", tempDir)}
	err = docker.BuildPushImageFromInfraImage(ctx, "Fiddler", image, tag, repo, configDir, tempDir, "prod", topic, cmd, volumes, infraCommonEnv, infraCommonBuildArgs)
	if err != nil {
		return err
	}
	return cleanupTempFiles(ctx, configDir, volumes)
}

func cleanupTempFiles(ctx context.Context, configDir string, volumes []string) error {
	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	const infraImageWithTag = "gcr.io/skia-public/infra:prod"
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	return docker.Run(ctx, infraImageWithTag, configDir, cleanupCmd, volumes, nil)
}

func buildPushApiImage(ctx context.Context, tag, repo, configDir, checkoutDir string, topic *pubsub.Topic) error {
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
	if err := docker.Run(ctx, "gcr.io/skia-public/doxygen:testing-slim", configDir, doxygenCmd, volumes, env); err != nil {
		return err
	}

	image := fmt.Sprintf("gcr.io/skia-public/%s", apiImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/api && make release_ci"}
	infraEnv := util.CopyStringSlice(infraCommonEnv)
	infraEnv = append(infraEnv, "DOXYGEN_HTML=/OUT/html")
	infraVolumes := []string{fmt.Sprintf("%s:/OUT", tempDir)}
	err = docker.BuildPushImageFromInfraImage(ctx, "Api", image, tag, repo, configDir, tempDir, "prod", topic, cmd, infraVolumes, infraEnv, infraCommonBuildArgs)
	if err != nil {
		return err
	}
	return cleanupTempFiles(ctx, configDir, volumes)
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

	// Check out the code.
	co, err := checkout.EnsureGitCheckout(ctx, path.Join(wd, "repo"), rs)
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
	// Add the tag to infraCommonBuildArgs.
	infraCommonBuildArgs["SKIA_IMAGE_TAG"] = tag

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

	// Build and push all apps of interest below.
	if err := buildPushFiddlerImage(ctx, tag, rs.Repo, configDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
	if err := buildPushApiImage(ctx, tag, rs.Repo, configDir, co.Dir(), topic); err != nil {
		td.Fatal(ctx, err)
	}
}

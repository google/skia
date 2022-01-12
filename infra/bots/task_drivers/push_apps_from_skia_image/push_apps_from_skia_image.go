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
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"

	"cloud.google.com/go/pubsub"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/common"
	docker_pubsub "go.skia.org/infra/go/docker/build/pubsub"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/util"
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
	// Checkout out the Skia infra repo at the specified commit.
	if *infraRevision == "" {
		return errors.New("Must specify --infra_revision")
	}
	// Check out the Skia infra repo at the specified commit.
	rs := types.RepoState{
		Repo:     common.REPO_SKIA_INFRA,
		Revision: *infraRevision,
	}
	infraCheckoutDir := filepath.Join("infra_repo")
	if _, err := checkout.EnsureGitCheckout(ctx, infraCheckoutDir, rs); err != nil {
		return err
	}

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
	if err := docker.Run(ctx, releaseImg, configDir, skiaCopyCmd, volumes, nil); err != nil {
		return err
	}

	// Ensure that the bazel cache is setup.
	opts := bazel.BazelOptions{
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
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

	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	// TODO(rmistry): Move to cleanupTempFiles after api is migrated to bazel.
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	if err := docker.Run(ctx, releaseImg, configDir, cleanupCmd, volumes, nil); err != nil {
		return err
	}

	return nil
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
	// TODO(rmistry): After api is migrated to bazel use this way to instantiate
	// docker instead:
	// dkr, err := docker.New(ctx, ts)
	// if err != nil {
	// 	td.Fatal(ctx, err)
	// }

	// Build and push all apps of interest below.
	if err := buildPushApiImage(ctx, tag, rs.Repo, configDir, co.Dir(), topic); err != nil {
		td.Fatal(ctx, err)
	}
	if err := buildPushFiddlerImage(ctx, tag, rs.Repo, configDir, topic); err != nil {
		td.Fatal(ctx, err)
	}
}

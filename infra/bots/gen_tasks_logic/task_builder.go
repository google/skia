// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

import (
	"fmt"
	"log"
	"reflect"
	"strings"
	"time"

	"go.skia.org/infra/go/cipd"
	"go.skia.org/infra/task_scheduler/go/specs"
)

// taskBuilder is a helper for creating a task.
type taskBuilder struct {
	*jobBuilder
	parts
	Name             string
	Spec             *specs.TaskSpec
	recipeProperties map[string]string
}

// newTaskBuilder returns a taskBuilder instance.
func newTaskBuilder(b *jobBuilder, name string) *taskBuilder {
	parts, err := b.jobNameSchema.ParseJobName(name)
	if err != nil {
		log.Fatal(err)
	}
	return &taskBuilder{
		jobBuilder:       b,
		parts:            parts,
		Name:             name,
		Spec:             &specs.TaskSpec{},
		recipeProperties: map[string]string{},
	}
}

// attempts sets the desired MaxAttempts for this task.
func (b *taskBuilder) attempts(a int) {
	b.Spec.MaxAttempts = a
}

// cache adds the given caches to the task.
func (b *taskBuilder) cache(caches ...*specs.Cache) {
	for _, c := range caches {
		alreadyHave := false
		for _, exist := range b.Spec.Caches {
			if c.Name == exist.Name {
				if !reflect.DeepEqual(c, exist) {
					log.Fatalf("Already have cache %s with a different definition!", c.Name)
				}
				alreadyHave = true
				break
			}
		}
		if !alreadyHave {
			b.Spec.Caches = append(b.Spec.Caches, c)
		}
	}
}

// cmd sets the command for the task.
func (b *taskBuilder) cmd(c ...string) {
	b.Spec.Command = c
}

// dimension adds the given dimensions to the task.
func (b *taskBuilder) dimension(dims ...string) {
	for _, dim := range dims {
		if !In(dim, b.Spec.Dimensions) {
			b.Spec.Dimensions = append(b.Spec.Dimensions, dim)
		}
	}
}

// expiration sets the expiration of the task.
func (b *taskBuilder) expiration(e time.Duration) {
	b.Spec.Expiration = e
}

// idempotent marks the task as idempotent.
func (b *taskBuilder) idempotent() {
	b.Spec.Idempotent = true
}

// cas sets the CasSpec used by the task.
func (b *taskBuilder) cas(casSpec string) {
	b.Spec.CasSpec = casSpec
}

// env sets the value for the given environment variable for the task.
func (b *taskBuilder) env(key, value string) {
	if b.Spec.Environment == nil {
		b.Spec.Environment = map[string]string{}
	}
	b.Spec.Environment[key] = value
}

// envPrefixes appends the given values to the given environment variable for
// the task.
func (b *taskBuilder) envPrefixes(key string, values ...string) {
	if b.Spec.EnvPrefixes == nil {
		b.Spec.EnvPrefixes = map[string][]string{}
	}
	for _, value := range values {
		if !In(value, b.Spec.EnvPrefixes[key]) {
			b.Spec.EnvPrefixes[key] = append(b.Spec.EnvPrefixes[key], value)
		}
	}
}

// addToPATH adds the given locations to PATH for the task.
func (b *taskBuilder) addToPATH(loc ...string) {
	b.envPrefixes("PATH", loc...)
}

// output adds the given paths as outputs to the task, which results in their
// contents being uploaded to the isolate server.
func (b *taskBuilder) output(paths ...string) {
	for _, path := range paths {
		if !In(path, b.Spec.Outputs) {
			b.Spec.Outputs = append(b.Spec.Outputs, path)
		}
	}
}

// serviceAccount sets the service account for this task.
func (b *taskBuilder) serviceAccount(sa string) {
	b.Spec.ServiceAccount = sa
}

// timeout sets the timeout(s) for this task.
func (b *taskBuilder) timeout(timeout time.Duration) {
	b.Spec.ExecutionTimeout = timeout
	b.Spec.IoTimeout = timeout // With kitchen, step logs don't count toward IoTimeout.
}

// dep adds the given tasks as dependencies of this task.
func (b *taskBuilder) dep(tasks ...string) {
	for _, task := range tasks {
		if !In(task, b.Spec.Dependencies) {
			b.Spec.Dependencies = append(b.Spec.Dependencies, task)
		}
	}
}

// cipd adds the given CIPD packages to the task.
func (b *taskBuilder) cipd(pkgs ...*specs.CipdPackage) {
	for _, pkg := range pkgs {
		alreadyHave := false
		for _, exist := range b.Spec.CipdPackages {
			if pkg.Name == exist.Name {
				if !reflect.DeepEqual(pkg, exist) {
					log.Fatalf("Already have package %s with a different definition!", pkg.Name)
				}
				alreadyHave = true
				break
			}
		}
		if !alreadyHave {
			b.Spec.CipdPackages = append(b.Spec.CipdPackages, pkg)
		}
	}
}

// useIsolatedAssets returns true if this task should use assets which are
// isolated rather than downloading directly from CIPD.
func (b *taskBuilder) useIsolatedAssets() bool {
	// Only do this on the RPIs for now. Other, faster machines shouldn't
	// see much benefit and we don't need the extra complexity, for now.
	if b.os("ChromeOS", "iOS") || b.matchOs("Android") {
		return true
	}
	return false
}

// uploadAssetCASCfg represents a task which copies a CIPD package into
// isolate.
type uploadAssetCASCfg struct {
	alwaysIsolate  bool
	uploadTaskName string
	path           string
}

// assetWithVersion adds the given asset with the given version number to the
// task as a CIPD package.
func (b *taskBuilder) assetWithVersion(assetName string, version int) {
	pkg := &specs.CipdPackage{
		Name:    fmt.Sprintf("skia/bots/%s", assetName),
		Path:    assetName,
		Version: fmt.Sprintf("version:%d", version),
	}
	b.cipd(pkg)
}

// asset adds the given assets to the task as CIPD packages.
func (b *taskBuilder) asset(assets ...string) {
	shouldIsolate := b.useIsolatedAssets()
	pkgs := make([]*specs.CipdPackage, 0, len(assets))
	for _, asset := range assets {
		if cfg, ok := ISOLATE_ASSET_MAPPING[asset]; ok && (cfg.alwaysIsolate || shouldIsolate) {
			b.dep(b.uploadCIPDAssetToCAS(asset))
		} else {
			pkgs = append(pkgs, b.MustGetCipdPackageFromAsset(asset))
		}
	}
	b.cipd(pkgs...)
}

// usesCCache adds attributes to tasks which need bazel (via bazelisk).
func (b *taskBuilder) usesBazel(hostOSArch string) {
	archToPkg := map[string]string{
		"linux_x64": "bazelisk_linux_amd64",
		"mac_x64":   "bazelisk_mac_amd64",
	}
	pkg, ok := archToPkg[hostOSArch]
	if !ok {
		panic("Unsupported osAndArch for bazelisk: " + hostOSArch)
	}
	b.cipd(b.MustGetCipdPackageFromAsset(pkg))
	b.addToPATH(pkg)
}

// usesCCache adds attributes to tasks which use ccache.
func (b *taskBuilder) usesCCache() {
	b.cache(CACHES_CCACHE...)
}

// usesGit adds attributes to tasks which use git.
func (b *taskBuilder) usesGit() {
	b.cache(CACHES_GIT...)
	if b.matchOs("Win") || b.matchExtraConfig("Win") {
		b.cipd(specs.CIPD_PKGS_GIT_WINDOWS_AMD64...)
	} else if b.matchOs("Mac") || b.matchExtraConfig("Mac") {
		b.cipd(specs.CIPD_PKGS_GIT_MAC_AMD64...)
	} else {
		b.cipd(specs.CIPD_PKGS_GIT_LINUX_AMD64...)
	}
}

// usesGo adds attributes to tasks which use go. Recipes should use
// "with api.context(env=api.infra.go_env)".
func (b *taskBuilder) usesGo() {
	b.usesGit() // Go requires Git.
	b.cache(CACHES_GO...)
	pkg := b.MustGetCipdPackageFromAsset("go")
	if b.matchOs("Win") || b.matchExtraConfig("Win") {
		pkg = b.MustGetCipdPackageFromAsset("go_win")
		pkg.Path = "go"
	}
	b.cipd(pkg)
	b.addToPATH(pkg.Path + "/go/bin")
	b.envPrefixes("GOROOT", pkg.Path+"/go")
}

// usesDocker adds attributes to tasks which use docker.
func (b *taskBuilder) usesDocker() {
	b.dimension("docker_installed:true")

	// The "docker" binary reads its config from $HOME/.docker/config.json which, after running
	// "gcloud auth configure-docker", typically looks like this:
	//
	//     {
	//       "credHelpers": {
	//         "gcr.io": "gcloud",
	//         "us.gcr.io": "gcloud",
	//         "eu.gcr.io": "gcloud",
	//         "asia.gcr.io": "gcloud",
	//         "staging-k8s.gcr.io": "gcloud",
	//         "marketplace.gcr.io": "gcloud"
	//       }
	//     }
	//
	// This instructs "docker" to get its GCR credentials from a credential helper [1] program
	// named "docker-credential-gcloud" [2], which is part of the Google Cloud SDK. This program is
	// a shell script that invokes the "gcloud" command, which is itself a shell script that probes
	// the environment to find a viable Python interpreter, and then invokes
	// /usr/lib/google-cloud-sdk/lib/gcloud.py. For some unknown reason, sometimes "gcloud" decides
	// to use "/b/s/w/ir/cache/vpython/875f1a/bin/python" as the Python interpreter (exact path may
	// vary), which causes gcloud.py to fail with the following error:
	//
	//     ModuleNotFoundError: No module named 'contextlib'
	//
	// Fortunately, "gcloud" supports specifying a Python interpreter via the GCLOUDSDK_PYTHON
	// environment variable.
	//
	// [1] https://docs.docker.com/engine/reference/commandline/login/#credential-helpers
	// [2] See /usr/bin/docker-credential-gcloud on your gLinux system, which is provided by the
	//     google-cloud-sdk package.
	b.envPrefixes("CLOUDSDK_PYTHON", "cipd_bin_packages/cpython3/bin/python3")

	// As mentioned, Docker uses gcloud for authentication against GCR, and gcloud requires Python.
	b.usesPython()
}

// usesGSUtil adds the gsutil dependency from CIPD and puts it on PATH.
func (b *taskBuilder) usesGSUtil() {
	b.asset("gsutil")
	b.addToPATH("gsutil/gsutil")
}

// needsFontsForParagraphTests downloads the skparagraph CIPD package to
// a subdirectory of the Skia checkout: resources/extra_fonts
func (b *taskBuilder) needsFontsForParagraphTests() {
	pkg := b.MustGetCipdPackageFromAsset("skparagraph")
	pkg.Path = "skia/resources/extra_fonts"
	b.cipd(pkg)
}

// recipeProp adds the given recipe property key/value pair. Panics if
// getRecipeProps() was already called.
func (b *taskBuilder) recipeProp(key, value string) {
	if b.recipeProperties == nil {
		log.Fatal("taskBuilder.recipeProp() cannot be called after taskBuilder.getRecipeProps()!")
	}
	b.recipeProperties[key] = value
}

// recipeProps calls recipeProp for every key/value pair in the given map.
// Panics if getRecipeProps() was already called.
func (b *taskBuilder) recipeProps(props map[string]string) {
	for k, v := range props {
		b.recipeProp(k, v)
	}
}

// getRecipeProps returns JSON-encoded recipe properties. Subsequent calls to
// recipeProp[s] will panic, to prevent accidentally adding recipe properties
// after they have been added to the task.
func (b *taskBuilder) getRecipeProps() string {
	props := make(map[string]interface{}, len(b.recipeProperties)+2)
	// TODO(borenet): I'm not sure why we supply the original task name
	// and not the upload task name.  We should investigate whether this is
	// needed.
	buildername := b.Name
	if b.role("Upload") {
		buildername = strings.TrimPrefix(buildername, "Upload-")
	}
	props["buildername"] = buildername
	props["$kitchen"] = struct {
		DevShell bool `json:"devshell"`
		GitAuth  bool `json:"git_auth"`
	}{
		DevShell: true,
		GitAuth:  true,
	}
	for k, v := range b.recipeProperties {
		props[k] = v
	}
	b.recipeProperties = nil
	return marshalJson(props)
}

// cipdPlatform returns the CIPD platform for this task.
func (b *taskBuilder) cipdPlatform() string {
	if b.role("Upload") {
		return cipd.PlatformLinuxAmd64
	} else if b.matchOs("Win") || b.matchExtraConfig("Win") {
		return cipd.PlatformWindowsAmd64
	} else if b.matchOs("Mac") {
		return cipd.PlatformMacAmd64
	} else if b.matchArch("Arm64") {
		return cipd.PlatformLinuxArm64
	} else if b.matchOs("Android", "ChromeOS") {
		return cipd.PlatformLinuxArm64
	} else if b.matchOs("iOS") {
		return cipd.PlatformLinuxArm64
	} else {
		return cipd.PlatformLinuxAmd64
	}
}

// usesPython adds attributes to tasks which use python.
func (b *taskBuilder) usesPython() {
	pythonPkgs := cipd.PkgsPython[b.cipdPlatform()]
	b.cipd(pythonPkgs...)
	b.addToPATH(
		"cipd_bin_packages/cpython3",
		"cipd_bin_packages/cpython3/bin",
	)
	b.cache(&specs.Cache{
		Name: "vpython3",
		Path: "cache/vpython3",
	})
	b.envPrefixes("VPYTHON_VIRTUALENV_ROOT", "cache/vpython3")
	b.env("VPYTHON_LOG_TRACE", "1")
}

func (b *taskBuilder) usesNode() {
	// It is very important when including node via CIPD to also add it to the PATH of the
	// taskdriver or mysterious things can happen when subprocesses try to resolve node/npm.
	b.asset("node")
	b.addToPATH("node/node/bin")
}

func (b *taskBuilder) needsLottiesWithAssets() {
	// This CIPD package was made by hand with the following invocation:
	//   cipd create -name skia/internal/lotties_with_assets -in ./lotties/ -tag version:2
	//   cipd acl-edit skia/internal/lotties_with_assets -reader group:project-skia-external-task-accounts
	//   cipd acl-edit skia/internal/lotties_with_assets -reader user:pool-skia@chromium-swarm.iam.gserviceaccount.com
	// Where lotties is a hand-selected set of lottie animations and (optionally) assets used in
	// them (e.g. fonts, images).
	// Each test case is in its own folder, with a data.json file and an optional images/ subfolder
	// with any images/fonts/etc loaded by the animation.
	// Note: If you are downloading the existing package to update them, remove the CIPD-generated
	// .cipdpkg subfolder before trying to re-upload it.
	// Note: It is important that the folder names do not special characters like . (), &, as
	// the Android filesystem does not support folders with those names well.
	b.cipd(&specs.CipdPackage{
		Name:    "skia/internal/lotties_with_assets",
		Path:    "lotties_with_assets",
		Version: "version:4",
	})
}

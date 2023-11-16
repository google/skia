// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

import (
	"log"
	"strings"

	"go.skia.org/infra/task_scheduler/go/specs"
)

// jobBuilder provides helpers for creating a job.
type jobBuilder struct {
	*builder
	parts
	Name string
	Spec *specs.JobSpec
}

// newJobBuilder returns a jobBuilder for the given job name.
func newJobBuilder(b *builder, name string) *jobBuilder {
	p, err := b.jobNameSchema.ParseJobName(name)
	if err != nil {
		log.Fatal(err)
	}
	return &jobBuilder{
		builder: b,
		parts:   p,
		Name:    name,
		Spec:    &specs.JobSpec{},
	}
}

// priority sets the priority of the job.
func (b *jobBuilder) priority(p float64) {
	b.Spec.Priority = p
}

// trigger dictates when the job should be triggered.
func (b *jobBuilder) trigger(trigger string) {
	b.Spec.Trigger = trigger
}

// Create a taskBuilder and run the given function for it.
func (b *jobBuilder) addTask(name string, fn func(*taskBuilder)) {
	tb := newTaskBuilder(b, name)
	fn(tb)
	b.MustAddTask(tb.Name, tb.Spec)
	// Add the task to the Job's dependency set, removing any which are
	// accounted for by the new task's dependencies.
	b.Spec.TaskSpecs = append(b.Spec.TaskSpecs, tb.Name)
	newSpecs := make([]string, 0, len(b.Spec.TaskSpecs))
	for _, t := range b.Spec.TaskSpecs {
		if !In(t, tb.Spec.Dependencies) {
			newSpecs = append(newSpecs, t)
		}
	}
	b.Spec.TaskSpecs = newSpecs
}

// uploadCIPDAssetToCAS generates a task to isolate the given CIPD asset. Returns
// the name of the task.
func (b *jobBuilder) uploadCIPDAssetToCAS(asset string) string {
	cfg, ok := ISOLATE_ASSET_MAPPING[asset]
	if !ok {
		log.Fatalf("No isolate task for asset %q", asset)
	}
	b.addTask(cfg.uploadTaskName, func(b *taskBuilder) {
		b.cipd(b.MustGetCipdPackageFromAsset(asset))
		b.cmd("/bin/cp", "-rL", cfg.path, "${ISOLATED_OUTDIR}")
		b.linuxGceDimensions(MACHINE_TYPE_SMALL)
		b.idempotent()
		b.cas(CAS_EMPTY)
	})
	return cfg.uploadTaskName
}

// genTasksForJob generates the tasks needed by this job.
func (b *jobBuilder) genTasksForJob() {
	// Bundle Recipes.
	if b.Name == BUNDLE_RECIPES_NAME {
		b.bundleRecipes()
		return
	}
	if strings.HasPrefix(b.Name, BUILD_TASK_DRIVERS_PREFIX) {
		parts := strings.Split(b.Name, "_")
		b.buildTaskDrivers(parts[1], parts[2])
		return
	}

	// Isolate CIPD assets.
	if b.matchExtraConfig("Isolate") {
		for asset, cfg := range ISOLATE_ASSET_MAPPING {
			if cfg.uploadTaskName == b.Name {
				b.uploadCIPDAssetToCAS(asset)
				return
			}
		}
	}

	// RecreateSKPs.
	if b.extraConfig("RecreateSKPs") {
		b.recreateSKPs()
		return
	}

	// Create docker image.
	if b.extraConfig("CreateDockerImage") {
		b.createDockerImage(b.extraConfig("WASM"))
		return
	}

	// Push apps from docker image.
	if b.extraConfig("PushAppsFromSkiaDockerImage") {
		b.createPushAppsFromSkiaDockerImage()
		return
	} else if b.extraConfig("PushBazelAppsFromWASMDockerImage") {
		b.createPushBazelAppsFromWASMDockerImage()
		return
	}

	// Infra tests.
	if b.extraConfig("InfraTests") {
		b.infra()
		return
	}

	// Housekeepers.
	if b.Name == "Housekeeper-PerCommit" {
		b.housekeeper()
		return
	}
	if b.Name == "Housekeeper-PerCommit-CheckGeneratedFiles" {
		b.checkGeneratedFiles()
		return
	}
	if b.Name == "Housekeeper-PerCommit-GoLinters" {
		b.goLinters()
		return
	}
	if b.Name == "Housekeeper-PerCommit-RunGnToBp" {
		b.checkGnToBp()
		return
	}
	if b.Name == "Housekeeper-OnDemand-Presubmit" {
		b.priority(1)
		b.presubmit()
		return
	}

	// Compile bots.
	if b.role("Build") {
		b.compile()
		return
	}

	// BuildStats bots. This computes things like binary size.
	if b.role("BuildStats") {
		b.buildstats()
		return
	}

	if b.role("CodeSize") {
		b.codesize()
		return
	}

	// Valgrind runs at a low priority so that it doesn't occupy all the bots.
	if b.extraConfig("Valgrind") {
		// Priority of 0.085 should result in Valgrind tasks with a blamelist of ~10 commits having the
		// same score as other tasks with a blamelist of 1 commit, when we have insufficient bot
		// capacity to run more frequently.
		b.priority(0.085)
	}

	// Test bots.
	if b.role("Test") {
		if b.extraConfig("WasmGMTests") {
			b.runWasmGMTests()
			return
		}
		b.dm()
		return
	}

	// Canary bots.
	if b.role("Canary") {
		if b.project("G3") {
			b.g3FrameworkCanary()
			return
		} else if b.project("Android") {
			b.canary("android-master-autoroll", "Canary-Android-Topic", "https://googleplex-android-review.googlesource.com/q/topic:")
			return
		} else if b.project("Chromium") {
			b.canary("skia-autoroll", "Canary-Chromium-CL", "https://chromium-review.googlesource.com/c/")
			return
		} else if b.project("Flutter") {
			b.canary("skia-flutter-autoroll", "Canary-Flutter-PR", "https://github.com/flutter/engine/pull/")
			return
		}
	}

	if b.extraConfig("Puppeteer") {
		// TODO(kjlubick) make this a new role
		b.puppeteer()
		return
	}

	// Perf bots.
	if b.role("Perf") {
		b.perf()
		return
	}

	if b.role("BazelBuild") {
		b.bazelBuild()
		return
	}

	if b.role("BazelTest") {
		b.bazelTest()
		return
	}

	log.Fatalf("Don't know how to handle job %q", b.Name)
}

func (b *jobBuilder) finish() {
	// Add the Job spec.
	if b.frequency("Nightly") {
		b.trigger(specs.TRIGGER_NIGHTLY)
	} else if b.frequency("Weekly") {
		b.trigger(specs.TRIGGER_WEEKLY)
	} else if b.extraConfig("Flutter", "CreateDockerImage", "PushAppsFromSkiaDockerImage", "PushBazelAppsFromWASMDockerImage") {
		b.trigger(specs.TRIGGER_MAIN_ONLY)
	} else if b.frequency("OnDemand") || b.role("Canary") {
		b.trigger(specs.TRIGGER_ON_DEMAND)
	} else {
		b.trigger(specs.TRIGGER_ANY_BRANCH)
	}
	b.MustAddJob(b.Name, b.Spec)
}

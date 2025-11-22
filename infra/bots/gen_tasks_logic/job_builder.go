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
	Parts
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
		Parts:   p,
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
func (b *jobBuilder) addTask(name string, fn func(*TaskBuilder)) {
	tb := newTaskBuilder(b, name)
	fn(tb)
	if b.cfg.AddTaskCallback != nil {
		b.cfg.AddTaskCallback(tb)
	}
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
	b.addTask(cfg.uploadTaskName, func(b *TaskBuilder) {
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
	if b.MatchExtraConfig("Isolate") {
		for asset, cfg := range ISOLATE_ASSET_MAPPING {
			if cfg.uploadTaskName == b.Name {
				b.uploadCIPDAssetToCAS(asset)
				return
			}
		}
	}

	// RecreateSKPs.
	if b.ExtraConfig("RecreateSKPs") {
		b.recreateSKPs()
		return
	}

	// Infra tests.
	if b.ExtraConfig("InfraTests") {
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
	if b.Role("Build") {
		b.compile()
		return
	}

	// BuildStats bots. This computes things like binary size.
	if b.Role("BuildStats") {
		b.buildstats()
		return
	}

	if b.Role("CodeSize") {
		b.codesize()
		return
	}

	// Test bots.
	if b.Role("Test") {
		if b.ExtraConfig("WasmGMTests") {
			b.runWasmGMTests()
			return
		}
		b.dm()
		return
	}

	// Canary bots.
	if b.Role("Canary") {
		if b.Project("G3") {
			b.g3FrameworkCanary()
			return
		} else if b.Project("Android") {
			b.canary("android-master-autoroll", "Canary-Android-Topic", "https://googleplex-android-review.googlesource.com/q/topic:")
			return
		} else if b.Project("Chromium") {
			b.canary("skia-autoroll", "Canary-Chromium-CL", "https://chromium-review.googlesource.com/c/")
			return
		} else if b.Project("Flutter") {
			b.canary("skia-flutter-autoroll", "Canary-Flutter-PR", "https://github.com/flutter/engine/pull/")
			return
		}
	}

	if b.ExtraConfig("Puppeteer") {
		// TODO(kjlubick) make this a new role
		b.puppeteer()
		return
	}

	// Perf bots.
	if b.Role("Perf") {
		b.perf()
		return
	}

	if b.Role("BazelBuild") {
		b.bazelBuild()
		return
	}

	if b.Role("BazelTest") {
		b.bazelTest()
		return
	}

	log.Fatalf("Don't know how to handle job %q", b.Name)
}

func (b *jobBuilder) finish() {
	// Add the Job spec.
	if b.Frequency("Nightly") {
		b.trigger(specs.TRIGGER_NIGHTLY)
	} else if b.Frequency("Weekly") {
		b.trigger(specs.TRIGGER_WEEKLY)
	} else if b.ExtraConfig("Flutter", "PushBazelAppsFromWASMDockerImage") {
		b.trigger(specs.TRIGGER_MAIN_ONLY)
	} else if b.Frequency("OnDemand") || b.Role("Canary") {
		b.trigger(specs.TRIGGER_ON_DEMAND)
	} else {
		b.trigger(specs.TRIGGER_ANY_BRANCH)
	}
	b.MustAddJob(b.Name, b.Spec)
}

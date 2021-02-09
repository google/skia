// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

import (
	"fmt"
	"sort"

	"go.skia.org/infra/task_scheduler/go/specs"
)

// nanobenchFlags generates flags to Nanobench based on the given task properties.
func (b *taskBuilder) nanobenchFlags(doUpload bool) {
	args := []string{
		"nanobench",
		"--pre_log",
	}

	if b.gpu() {
		args = append(args, "--gpuStatsDump", "true")
	}

	args = append(args, "--scales", "1.0", "1.1")

	configs := []string{}
	if b.cpu() {
		args = append(args, "--nogpu")
		configs = append(configs, "8888", "nonrendering")

		if b.extraConfig("BonusConfigs") {
			configs = []string{
				"f16",
				"srgb",
				"esrgb",
				"narrow",
				"enarrow",
			}
		}

		if b.model("Nexus7") {
			args = append(args, "--purgeBetweenBenches") // Debugging skia:8929
		}

	} else if b.gpu() {
		args = append(args, "--nocpu")

		glPrefix := "gl"
		sampleCount := 8
		if b.os("Android", "iOS") {
			sampleCount = 4
			// The NVIDIA_Shield has a regular OpenGL implementation. We bench that
			// instead of ES.
			if !b.model("NVIDIA_Shield") {
				glPrefix = "gles"
			}
			// iOS crashes with MSAA (skia:6399)
			// Nexus7 (Tegra3) does not support MSAA.
			// MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
			if b.os("iOS") || b.model("Nexus7", "Pixel3a") {
				sampleCount = 0
			}
		} else if b.matchGpu("Intel") {
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			sampleCount = 0
		} else if b.os("ChromeOS") {
			glPrefix = "gles"
		}

		configs = append(configs, glPrefix, glPrefix+"srgb")

		// glnarrow/glesnarrow tests the case of color converting *all* content
		// It hangs on the AndroidOne (Mali400)  skia:10669
		if (!b.gpu("Mali400MP2")) {
			configs = append(configs, glPrefix+"narrow")
		}

		// skia:10644 The fake ES2 config is used to compare highest available ES version to
		// when we're limited to ES2. We could consider adding a MSAA fake config as well.
		if b.os("Android") && glPrefix == "gles" {
			// These only support ES2. No point in running twice.
			if (!b.gpu("Mali400MP2", "Tegra3")) {
				configs = append(configs, "glesfakev2")
			}
		}

		if sampleCount > 0 {
			configs = append(configs, fmt.Sprintf("%smsaa%d", glPrefix, sampleCount))
		}

		// We want to test both the OpenGL config and the GLES config on Linux Intel:
		// GL is used by Chrome, GLES is used by ChromeOS.
		if b.matchGpu("Intel") && b.isLinux() {
			configs = append(configs, "gles", "glessrgb")
		}

		if b.extraConfig("CommandBuffer") {
			configs = []string{"commandbuffer"}
		}

		if b.extraConfig("Vulkan") {
			configs = []string{"vk"}
			if b.os("Android") {
				// skbug.com/9274
				if !b.model("Pixel2XL") {
					configs = append(configs, "vkmsaa4")
				}
			} else {
				// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926, skia:9023
				if !b.matchGpu("Intel") {
					configs = append(configs, "vkmsaa8")
				}
			}
		}
		if b.extraConfig("Metal") {
			configs = []string{"mtl"}
			if b.os("iOS") {
				configs = append(configs, "mtlmsaa4")
			} else {
				configs = append(configs, "mtlmsaa8")
			}
		}

		if b.extraConfig("ANGLE") {
			// Test only ANGLE configs.
			configs = []string{"angle_d3d11_es2", "angle_d3d11_es3"}
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("angle_d3d11_es2_msaa%d", sampleCount))
				configs = append(configs, fmt.Sprintf("angle_d3d11_es3_msaa%d", sampleCount))
			}
			if b.gpu("QuadroP400") {
				// See skia:7823 and chromium:693090.
				configs = append(configs, "angle_gl_es2")
				configs = append(configs, "angle_gl_es3")
				if sampleCount > 0 {
					configs = append(configs, fmt.Sprintf("angle_gl_es2_msaa%d", sampleCount))
					configs = append(configs, fmt.Sprintf("angle_gl_es3_msaa%d", sampleCount))
				}
			}
		}
		if b.os("ChromeOS") {
			// Just run GLES for now - maybe add gles_msaa4 in the future
			configs = []string{"gles"}
		}
	}

	args = append(args, "--config")
	args = append(args, configs...)

	// By default, we test with GPU threading enabled, unless specifically
	// disabled.
	if b.extraConfig("NoGPUThreads") {
		args = append(args, "--gpuThreads", "0")
	}

	if b.debug() || b.extraConfig("ASAN") || b.extraConfig("Valgrind") {
		args = append(args, "--loops", "1")
		args = append(args, "--samples", "1")
		// Ensure that the bot framework does not think we have timed out.
		args = append(args, "--keepAlive", "true")
	}

	// skia:9036
	if b.model("NVIDIA_Shield") {
		args = append(args, "--dontReduceOpsTaskSplitting")
	}

	// Some people don't like verbose output.
	verbose := false

	match := []string{}
	match = append(match, "TextBlobFirstTimeBench")

	if len(match) > 0 {
		args = append(args, "--match")
		args = append(args, match...)
	}

	if verbose {
		args = append(args, "--verbose")
	}

	// Add properties indicating which assets the task should use.
	b.recipeProp("do_upload", fmt.Sprintf("%t", doUpload))
	if !b.gpu() {
		b.asset("skimage")
		b.recipeProp("images", "true")
	}
	b.recipeProp("resources", "true")
	if !b.os("iOS") {
		b.asset("skp")
		b.recipeProp("skps", "true")
	}
	if !b.extraConfig("Valgrind") {
		b.asset("svg")
		b.recipeProp("svgs", "true")
	}
	if b.cpu() && b.os("Android") {
		// TODO(borenet): Where do these come from?
		b.recipeProp("textTraces", "true")
	}

	// These properties are plumbed through nanobench and into Perf results.
	nanoProps := map[string]string{
		"gitHash":          specs.PLACEHOLDER_REVISION,
		"issue":            specs.PLACEHOLDER_ISSUE,
		"patchset":         specs.PLACEHOLDER_PATCHSET,
		"patch_storage":    specs.PLACEHOLDER_PATCH_STORAGE,
		"swarming_bot_id":  "${SWARMING_BOT_ID}",
		"swarming_task_id": "${SWARMING_TASK_ID}",
	}

	if doUpload {
		keysExclude := map[string]bool{
			"configuration": true,
			"role":          true,
			"test_filter":   true,
		}
		keys := make([]string, 0, len(b.parts))
		for k := range b.parts {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		args = append(args, "--key")
		for _, k := range keys {
			if !keysExclude[k] {
				args = append(args, k, b.parts[k])
			}
		}
	}

	// Finalize the nanobench flags and properties.
	b.recipeProp("nanobench_flags", marshalJson(args))
	b.recipeProp("nanobench_properties", marshalJson(nanoProps))
}

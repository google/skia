// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

import (
	"fmt"
	"sort"
	"strings"

	"go.skia.org/infra/task_scheduler/go/specs"
)

// nanobenchFlags generates flags to Nanobench based on the given task properties.
func nanobenchFlags(bot string, parts map[string]string, doUpload bool) ([]string, map[string]string) {
	has := func(keyword string) bool {
		return strings.Contains(bot, keyword)
	}

	// TODO(borenet): This duplicates code in recipes_modules/vars/api.py and will
	// be removed soon.
	isLinux := has("Ubuntu") || has("Debian") || has("Housekeeper")

	args := []string{
		"nanobench",
		"--pre_log",
	}

	if has("GPU") {
		args = append(args, "--gpuStatsDump", "true")
	}

	args = append(args, "--scales", "1.0", "1.1")

	configs := []string{}
	if parts["cpu_or_gpu"] == "CPU" {
		args = append(args, "--nogpu")
		configs = append(configs, "8888", "nonrendering")

		if has("BonusConfigs") {
			configs = []string{
				"f16",
				"srgb",
				"esrgb",
				"narrow",
				"enarrow",
			}
		}

		if has("Nexus7") {
			args = append(args, "--purgeBetweenBenches") // Debugging skia:8929
		}

	} else if parts["cpu_or_gpu"] == "GPU" {
		args = append(args, "--nocpu")

		glPrefix := "gl"
		sampleCount := 8
		if has("Android") || has("iOS") {
			sampleCount = 4
			// The NVIDIA_Shield has a regular OpenGL implementation. We bench that
			// instead of ES.
			if !has("NVIDIA_Shield") {
				glPrefix = "gles"
			}
			// iOS crashes with MSAA (skia:6399)
			// Nexus7 (Tegra3) does not support MSAA.
			// MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
			if has("iOS") || has("Nexus7") || has("Pixel3a") {
				sampleCount = 0
			}
		} else if has("Intel") {
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			sampleCount = 0
		} else if has("ChromeOS") {
			glPrefix = "gles"
		}

		configs = append(configs, glPrefix, glPrefix+"srgb")
		if sampleCount > 0 {
			configs = append(configs, fmt.Sprintf("%smsaa%d", glPrefix, sampleCount))
		}

		// We want to test both the OpenGL config and the GLES config on Linux Intel:
		// GL is used by Chrome, GLES is used by ChromeOS.
		if has("Intel") && isLinux {
			configs = append(configs, "gles", "glessrgb")
		}

		if has("CommandBuffer") {
			configs = []string{"commandbuffer"}
		}

		if has("Vulkan") {
			configs = []string{"vk"}
			if has("Android") {
				// skbug.com/9274
				if !has("Pixel2XL") {
					configs = append(configs, "vkmsaa4")
				}
			} else {
				// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926, skia:9023
				if !has("Intel") {
					configs = append(configs, "vkmsaa8")
				}
			}
		}
		if has("Metal") {
			configs = []string{"mtl"}
			if has("iOS") {
				configs = append(configs, "mtlmsaa4")
			} else {
				configs = append(configs, "mtlmsaa8")
			}
		}

		if has("ANGLE") {
			// Test only ANGLE configs.
			configs = []string{"angle_d3d11_es2"}
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("angle_d3d11_es2_msaa%d", sampleCount))
			}
			if has("QuadroP400") {
				// See skia:7823 and chromium:693090.
				configs = append(configs, "angle_gl_es2")
				if sampleCount > 0 {
					configs = append(configs, fmt.Sprintf("angle_gl_es2_msaa%d", sampleCount))
				}
			}
		}
		if has("ChromeOS") {
			// Just run GLES for now - maybe add gles_msaa4 in the future
			configs = []string{"gles"}
		}
	}

	args = append(args, "--config")
	args = append(args, configs...)

	// By default, we test with GPU threading enabled, unless specifically
	// disabled.
	if has("NoGPUThreads") {
		args = append(args, "--gpuThreads", "0")
	}

	if has("Debug") || has("ASAN") || has("Valgrind") {
		args = append(args, "--loops", "1")
		args = append(args, "--samples", "1")
		// Ensure that the bot framework does not think we have timed out.
		args = append(args, "--keepAlive", "true")
	}

	// skia:9036
	if has("NVIDIA_Shield") {
		args = append(args, "--dontReduceOpsTaskSplitting")
	}

	// Some people don't like verbose output.
	verbose := false

	match := []string{}
	if has("Android") {
		// Segfaults when run as GPU bench. Very large texture?
		match = append(match, "~blurroundrect")
		match = append(match, "~patch_grid") // skia:2847
		match = append(match, "~desk_carsvg")
	}
	if has("Nexus5") {
		match = append(match, "~keymobi_shop_mobileweb_ebay_com.skp") // skia:5178
	}
	if has("iOS") {
		match = append(match, "~blurroundrect")
		match = append(match, "~patch_grid") // skia:2847
		match = append(match, "~desk_carsvg")
		match = append(match, "~keymobi")
		match = append(match, "~path_hairline")
		match = append(match, "~GLInstancedArraysBench") // skia:4714
	}
	if has("iOS") && has("Metal") {
		// skia:9799
		match = append(match, "~compositing_images_tile_size")
	}
	if has("Intel") && isLinux && !has("Vulkan") {
		// TODO(dogben): Track down what's causing bots to die.
		verbose = true
	}
	if has("IntelHD405") && isLinux && has("Vulkan") {
		// skia:7322
		match = append(match, "~desk_carsvg.skp_1")
		match = append(match, "~desk_googlehome.skp")
		match = append(match, "~desk_tiger8svg.skp_1")
		match = append(match, "~desk_wowwiki.skp")
		match = append(match, "~desk_ynevsvg.skp_1.1")
		match = append(match, "~desk_nostroke_tiger8svg.skp")
		match = append(match, "~keymobi_booking_com.skp_1")
		match = append(match, "~keymobi_booking_com.skp_1_mpd")
		match = append(match, "~keymobi_cnn_article.skp_1")
		match = append(match, "~keymobi_cnn_article.skp_1_mpd")
		match = append(match, "~keymobi_forecast_io.skp_1")
		match = append(match, "~keymobi_forecast_io.skp_1_mpd")
		match = append(match, "~keymobi_sfgate.skp_1")
		match = append(match, "~keymobi_techcrunch_com.skp_1.1")
		match = append(match, "~keymobi_techcrunch.skp_1.1")
		match = append(match, "~keymobi_techcrunch.skp_1.1_mpd")
		match = append(match, "~svgparse_Seal_of_California.svg_1.1")
		match = append(match, "~svgparse_NewYork-StateSeal.svg_1.1")
		match = append(match, "~svgparse_Vermont_state_seal.svg_1")
		match = append(match, "~tabl_gamedeksiam.skp_1.1")
		match = append(match, "~tabl_pravda.skp_1")
		match = append(match, "~top25desk_ebay_com.skp_1.1")
		match = append(match, "~top25desk_ebay.skp_1.1")
		match = append(match, "~top25desk_ebay.skp_1.1_mpd")
	}
	if has("Vulkan") && has("GTX660") {
		// skia:8523 skia:9271
		match = append(match, "~compositing_images")
	}
	if has("MacBook10.1") && has("CommandBuffer") {
		match = append(match, "~^desk_micrographygirlsvg.skp_1.1$")
	}
	if has("ASAN") && has("CPU") {
		// floor2int_undef benches undefined behavior, so ASAN correctly complains.
		match = append(match, "~^floor2int_undef$")
	}
	if has("AcerChromebook13_CB5_311-GPU-TegraK1") {
		// skia:7551
		match = append(match, "~^shapes_rrect_inner_rrect_50_500x500$")
	}
	if has("Perf-Android-Clang-Pixel3a-GPU-Adreno615-arm64-Release-All-Android") {
		// skia:9413
		match = append(match, "~^path_text$")
		match = append(match, "~^path_text_clipped_uncached$")
	}
	if has("Perf-Android-Clang-Pixel3-GPU-Adreno630-arm64-Release-All-Android_Vulkan") {
		// skia:9972
		match = append(match, "~^path_text_clipped_uncached$")
	}

	// We do not need or want to benchmark the decodes of incomplete images.
	// In fact, in nanobench we assert that the full image decode succeeds.
	match = append(match, "~inc0.gif")
	match = append(match, "~inc1.gif")
	match = append(match, "~incInterlaced.gif")
	match = append(match, "~inc0.jpg")
	match = append(match, "~incGray.jpg")
	match = append(match, "~inc0.wbmp")
	match = append(match, "~inc1.wbmp")
	match = append(match, "~inc0.webp")
	match = append(match, "~inc1.webp")
	match = append(match, "~inc0.ico")
	match = append(match, "~inc1.ico")
	match = append(match, "~inc0.png")
	match = append(match, "~inc1.png")
	match = append(match, "~inc2.png")
	match = append(match, "~inc12.png")
	match = append(match, "~inc13.png")
	match = append(match, "~inc14.png")
	match = append(match, "~inc0.webp")
	match = append(match, "~inc1.webp")

	if len(match) > 0 {
		args = append(args, "--match")
		args = append(args, match...)
	}

	if verbose {
		args = append(args, "--verbose")
	}

	props := map[string]string{
		"gitHash":          specs.PLACEHOLDER_REVISION,
		"issue":            specs.PLACEHOLDER_ISSUE,
		"patchset":         specs.PLACEHOLDER_PATCHSET,
		"patch_storage":    specs.PLACEHOLDER_PATCH_STORAGE,
		"swarming_bot_id":  "${SWARMING_BOT_ID}",
		"swarming_task_id": "${SWARMING_TASK_ID}",
	}

	if doUpload {
		keysBlacklist := map[string]bool{
			"configuration": true,
			"role":          true,
			"test_filter":   true,
		}
		keys := make([]string, 0, len(parts))
		for k := range parts {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		args = append(args, "--key")
		for _, k := range keys {
			if !keysBlacklist[k] {
				args = append(args, k, parts[k])
			}
		}
	}

	return args, props
}

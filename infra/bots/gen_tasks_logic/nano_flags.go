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

	configs := []string{}
	if b.cpu() {
		args = append(args, "--nogpu")
		configs = append(configs, "8888", "nonrendering")

		if b.extraConfig("BonusConfigs") {
			configs = []string{
				"f16",
				"srgb-rgba",
				"srgb-f16",
				"narrow-rgba",
				"narrow-f16",
			}
		}

		if b.model("Nexus7") {
			args = append(args, "--purgeBetweenBenches") // Debugging skia:8929
		}

	} else if b.gpu() {
		args = append(args, "--nocpu")

		glPrefix := "gl"
		sampleCount := 8
		if b.matchOs("Android") || b.os("iOS") {
			sampleCount = 4
			glPrefix = "gles"
			// iOS crashes with MSAA (skia:6399)
			// Nexus7 (Tegra3) does not support MSAA.
			// MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
			// MSAA is disabled on Pixel5 (https://skbug.com/11152).
			if b.os("iOS") || b.model("Nexus7", "Pixel3a", "Pixel5") {
				sampleCount = 0
			}
		} else if b.matchGpu("AppleM1") {
			sampleCount = 4
		} else if b.matchGpu("Intel") {
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			if b.gpu("IntelIrisXe") && b.matchOs("Win") && b.extraConfig("ANGLE") {
				// Make an exception for newer GPUs + D3D
				args = append(args, "--allowMSAAOnNewIntel", "true")
			} else {
				sampleCount = 0
			}
		} else if b.os("ChromeOS") {
			glPrefix = "gles"
		}

		configs = append(configs, glPrefix, "srgb-"+glPrefix)

		if b.os("Ubuntu18") && b.noExtraConfig() {
			configs = append(configs, glPrefix+"reducedshaders")
		}
		// narrow-gl/gles tests the case of color converting *all* content
		// It hangs on the AndroidOne (Mali400)  skia:10669
		if !b.gpu("Mali400MP2") {
			configs = append(configs, "narrow-"+glPrefix)
		}

		// skia:10644 The fake ES2 config is used to compare highest available ES version to
		// when we're limited to ES2. We could consider adding a MSAA fake config as well.
		if b.matchOs("Android") && glPrefix == "gles" {
			// These only support ES2. No point in running twice.
			if !b.gpu("Mali400MP2", "Tegra3") {
				configs = append(configs, "glesfakev2")
			}
		}

		if sampleCount > 0 {
			configs = append(configs, fmt.Sprintf("%smsaa%d", glPrefix, sampleCount))
			if b.gpu("QuadroP400", "MaliG77", "AppleM1") {
				configs = append(configs, fmt.Sprintf("%sdmsaa", glPrefix))
			}
		}

		// We want to test both the OpenGL config and the GLES config on Linux Intel:
		// GL is used by Chrome, GLES is used by ChromeOS.
		if b.matchGpu("Intel") && b.isLinux() {
			configs = append(configs, "gles", "srgb-gles")
		}

		if b.extraConfig("Vulkan") {
			configs = []string{"vk"}
			if b.matchOs("Android") {
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
			if b.gpu("QuadroP400", "MaliG77") {
				configs = append(configs, "vkdmsaa")
			}
		}
		if b.extraConfig("Metal") && !b.extraConfig("Graphite") {
			configs = []string{"mtl"}
			if b.os("iOS") {
				configs = append(configs, "mtlmsaa4")
			} else {
				configs = append(configs, "mtlmsaa8")
			}
			if b.model("iPhone11") {
				configs = append(configs, "mtlreducedshaders")
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

		if b.extraConfig("Graphite") {
			if b.extraConfig("Dawn") {
				if b.extraConfig("D3D11") {
					configs = []string{"grdawn_d3d11"}
				}
				if b.extraConfig("D3D12") {
					configs = []string{"grdawn_d3d12"}
				}
				if b.extraConfig("Metal") {
					configs = []string{"grdawn_mtl"}
				}
				if b.extraConfig("Vulkan") {
					configs = []string{"grdawn_vk"}
				}
				if b.extraConfig("GL") {
					configs = []string{"grdawn_gl"}
				}
				if b.extraConfig("GLES") {
					configs = []string{"grdawn_gles"}
				}

			}
			if b.extraConfig("Native") {
				if b.extraConfig("Metal") {
					configs = []string{"grmtl"}
				}
				if b.extraConfig("Vulkan") {
					configs = []string{"grvk"}
				}
			}
		}

		if b.os("ChromeOS") {
			// Just run GLES for now - maybe add gles_msaa4 in the future
			configs = []string{"gles"}
		}
		if b.extraConfig("SwiftShader") {
			configs = []string{"vk", "vkdmsaa"}
		}
	}

	args = append(args, "--config")
	args = append(args, configs...)

	// Use 4 internal msaa samples on mobile and AppleM1, otherwise 8.
	args = append(args, "--internalSamples")
	if b.matchOs("Android") || b.os("iOS") || b.matchGpu("AppleM1") {
		args = append(args, "4")
	} else {
		args = append(args, "8")
	}

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

	// Some people don't like verbose output.
	verbose := false

	match := []string{}
	if b.matchOs("Android") {
		// Segfaults when run as GPU bench. Very large texture?
		match = append(match, "~blurroundrect")
		match = append(match, "~patch_grid") // skia:2847
		match = append(match, "~desk_carsvg")
	}
	if b.os("iOS") {
		match = append(match, "~blurroundrect")
		match = append(match, "~patch_grid") // skia:2847
		match = append(match, "~desk_carsvg")
		match = append(match, "~keymobi")
		match = append(match, "~path_hairline")
		match = append(match, "~GLInstancedArraysBench") // skia:4714
	}
	if b.os("iOS") && b.extraConfig("Metal") && !b.extraConfig("Graphite") {
		// skia:9799
		match = append(match, "~compositing_images_tile_size")
	}
	if b.matchGpu("Intel") && b.isLinux() && !b.extraConfig("Vulkan") {
		// TODO(dogben): Track down what's causing bots to die.
		verbose = true
	}
	if b.gpu("IntelHD405") && b.isLinux() && b.extraConfig("Vulkan") {
		// skia:7322
		match = append(match, "~desk_carsvg.skp_1")
		match = append(match, "~desk_googlehome.skp")
		match = append(match, "~desk_tiger8svg.skp_1")
		match = append(match, "~desk_wowwiki.skp")
		match = append(match, "~desk_ynevsvg.skp_1.1")
		match = append(match, "~desk_nostroke_tiger8svg.skp")
		match = append(match, "~keymobi_booking_com.skp_1")
		match = append(match, "~keymobi_cnn_article.skp_1")
		match = append(match, "~keymobi_forecast_io.skp_1")
		match = append(match, "~keymobi_sfgate.skp_1")
		match = append(match, "~keymobi_techcrunch_com.skp_1.1")
		match = append(match, "~keymobi_techcrunch.skp_1.1")
		match = append(match, "~svgparse_Seal_of_California.svg_1.1")
		match = append(match, "~svgparse_NewYork-StateSeal.svg_1.1")
		match = append(match, "~svgparse_Vermont_state_seal.svg_1")
		match = append(match, "~tabl_gamedeksiam.skp_1.1")
		match = append(match, "~tabl_pravda.skp_1")
		match = append(match, "~top25desk_ebay_com.skp_1.1")
		match = append(match, "~top25desk_ebay.skp_1.1")
	}
	if b.gpu("Tegra3") {
		// skbug.com/338376730
		match = append(match, "~GM_matrixconvolution_bigger")
		match = append(match, "~GM_matrixconvolution_biggest")
	}
	if b.extraConfig("Vulkan") && b.gpu("GTX660") {
		// skia:8523 skia:9271
		match = append(match, "~compositing_images")
	}
	if b.extraConfig("ASAN") && b.cpu() {
		// floor2int_undef benches undefined behavior, so ASAN correctly complains.
		match = append(match, "~^floor2int_undef$")
	}
	if b.model("Pixel3a") {
		// skia:9413
		match = append(match, "~^path_text$")
		match = append(match, "~^path_text_clipped_uncached$")
	}

	if b.model("Wembley") {
		// These tests spin forever on the Wembley.
		match = append(match, "~^create_backend_texture")
		match = append(match, "~^draw_coverage")
		match = append(match, "~^compositing_images")
	}
	if b.extraConfig("Graphite") && b.extraConfig("Dawn") {
		if b.matchOs("Win10") && b.matchGpu("RadeonR9M470X") {
			// The Dawn Win10 Radeon allocates too many Vulkan resources in bulk rect tests (b/318725123)
			match = append(match, "~bulkrect_1000_grid_uniqueimages")
			match = append(match, "~bulkrect_1000_random_uniqueimages")
		}
	}

	if b.model(DONT_REDUCE_OPS_TASK_SPLITTING_MODELS...) {
		args = append(args, "--dontReduceOpsTaskSplitting", "true")
	}
	if !b.isLinux() && b.extraConfig("Vulkan") && b.gpu("QuadroP400") {
		// skia:14302 (desk_carsvg.skp hangs indefinitely on Windows QuadroP400 vkdmsaa configs)
		match = append(match, "~desk_carsvg.skp")
	}

	if b.extraConfig("DMSAAStats") {
		// Render tiled, single-frame skps with an extremely tall canvas that hopefully allows for
		// us to tile most or all of the content.
		args = append(args,
			"--sourceType", "skp", "--clip", "0,0,1600,16384", "--GPUbenchTileW", "1600",
			"--GPUbenchTileH", "512", "--samples", "1", "--loops", "1", "--config", "gldmsaa",
			"--dmsaaStatsDump")
		// Don't collect stats on the skps generated from vector content. We want these to actually
		// trigger dmsaa.
		match = append(match, "~svg", "~chalkboard", "~motionmark")
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
	if b.cpu() && b.matchOs("Android") {
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
			"role":        true,
			"test_filter": true,
		}
		keys := make([]string, 0, len(b.parts))
		for k := range b.parts {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		args = append(args, "--key")
		for _, k := range keys {
			// We had not been adding this to our traces for a long time. We then started doing
			// performance data on an "OptimizeForSize" build. We didn't want to disrupt the
			// existing traces, so we skip the configuration for Release builds.
			if k == "configuration" && b.parts[k] == "Release" {
				continue
			}
			if !keysExclude[k] {
				args = append(args, k, b.parts[k])
			}
		}
	}

	// Finalize the nanobench flags and properties.
	b.recipeProp("nanobench_flags", marshalJson(args))
	b.recipeProp("nanobench_properties", marshalJson(nanoProps))
}

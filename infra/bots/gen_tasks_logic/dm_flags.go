// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package gen_tasks_logic

import (
	"fmt"
	"sort"
	"strconv"
	"strings"

	"github.com/golang/glog"
	"go.skia.org/infra/task_scheduler/go/specs"
)

// keyParams generates the key used by DM for Gold results.
func keyParams(parts map[string]string) []string {
	// Don't bother to include role, which is always Test.
	ignored := []string{"role", "test_filter"}
	keys := make([]string, 0, len(parts))
	for key := range parts {
		found := false
		for _, b := range ignored {
			if key == b {
				found = true
				break
			}
		}
		if !found {
			keys = append(keys, key)
		}
	}
	sort.Strings(keys)
	rv := make([]string, 0, 2*len(keys))
	for _, key := range keys {
		rv = append(rv, key, parts[key])
	}
	return rv
}

// dmFlags generates flags to DM based on the given task properties.
func (b *taskBuilder) dmFlags(internalHardwareLabel string) {
	properties := map[string]string{
		"gitHash":              specs.PLACEHOLDER_REVISION,
		"builder":              b.Name,
		"buildbucket_build_id": specs.PLACEHOLDER_BUILDBUCKET_BUILD_ID,
		"task_id":              specs.PLACEHOLDER_TASK_ID,
		"issue":                specs.PLACEHOLDER_ISSUE,
		"patchset":             specs.PLACEHOLDER_PATCHSET,
		"patch_storage":        specs.PLACEHOLDER_PATCH_STORAGE,
		"swarming_bot_id":      "${SWARMING_BOT_ID}",
		"swarming_task_id":     "${SWARMING_TASK_ID}",
	}

	args := []string{
		"dm",
		"--nameByHash",
	}

	configs := []string{}
	skipped := []string{}

	hasConfig := func(cfg string) bool {
		for _, c := range configs {
			if c == cfg {
				return true
			}
		}
		return false
	}
	filter := func(slice []string, elems ...string) []string {
		m := make(map[string]bool, len(elems))
		for _, e := range elems {
			m[e] = true
		}
		rv := make([]string, 0, len(slice))
		for _, e := range slice {
			if m[e] {
				rv = append(rv, e)
			}
		}
		return rv
	}
	remove := func(slice []string, elem string) []string {
		rv := make([]string, 0, len(slice))
		for _, e := range slice {
			if e != elem {
				rv = append(rv, e)
			}
		}
		return rv
	}
	removeContains := func(slice []string, elem string) []string {
		rv := make([]string, 0, len(slice))
		for _, e := range slice {
			if !strings.Contains(e, elem) {
				rv = append(rv, e)
			}
		}
		return rv
	}
	suffix := func(slice []string, sfx string) []string {
		rv := make([]string, 0, len(slice))
		for _, e := range slice {
			rv = append(rv, e+sfx)
		}
		return rv
	}

	// When matching skip logic, _ is a wildcard that matches all parts for the field.
	// For example, "8888 _ _ _" means everything that is part of an 8888 config and
	// "_ skp _ SomeBigDraw" means the skp named SomeBigDraw on all config and options.
	const ALL = "_"
	// The inputs here are turned into a --skip flag which represents a
	// "Space-separated config/src/srcOptions/name quadruples to skip."
	// See DM.cpp for more, especially should_skip(). ~ negates the match.
	skip := func(config, src, srcOptions, name string) {
		// config is also called "sink" in DM
		if config == "_" ||
			hasConfig(config) ||
			(config[0] == '~' && hasConfig(config[1:])) {
			skipped = append(skipped, config, src, srcOptions, name)
		}
	}

	// Keys.
	keys := keyParams(b.parts)
	if b.extraConfig("Lottie") {
		keys = append(keys, "renderer", "skottie")
	}
	if b.matchExtraConfig("DDL") {
		// 'DDL' style means "--skpViewportSize 2048"
		keys = append(keys, "style", "DDL")
	} else {
		keys = append(keys, "style", "default")
	}
	args = append(args, "--key")
	args = append(args, keys...)

	// This enables non-deterministic random seeding of the GPU FP optimization
	// test.
	// Not Android due to:
	//  - https://skia.googlesource.com/skia/+/5910ed347a638ded8cd4c06dbfda086695df1112/BUILD.gn#160
	//  - https://skia.googlesource.com/skia/+/ce06e261e68848ae21cac1052abc16bc07b961bf/tests/ProcessorTest.cpp#307
	// Not MSAN due to:
	//  - https://skia.googlesource.com/skia/+/0ac06e47269a40c177747310a613d213c95d1d6d/infra/bots/recipe_modules/flavor/gn_flavor.py#80
	if !b.os("Android") && !b.extraConfig("MSAN") {
		args = append(args, "--randomProcessorTest")
	}

	threadLimit := -1
	const MAIN_THREAD_ONLY = 0

	// 32-bit desktop bots tend to run out of memory, because they have relatively
	// far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
	if b.arch("x86") {
		threadLimit = 4
	}

	// These bots run out of memory easily.
	if b.model("MotoG4", "Nexus7") {
		threadLimit = MAIN_THREAD_ONLY
	}

	// Avoid issues with dynamically exceeding resource cache limits.
	if b.matchExtraConfig("DISCARDABLE") {
		threadLimit = MAIN_THREAD_ONLY
	}

	if threadLimit >= 0 {
		args = append(args, "--threads", strconv.Itoa(threadLimit))
	}

	sampleCount := 0
	glPrefix := ""
	if b.extraConfig("SwiftShader") {
		configs = append(configs, "vk", "vkdmsaa")
		// skbug.com/12820
		skip(ALL, "gm", ALL, "ycbcrimage")
		// skbug.com/12820
		skip(ALL, "test", ALL, "VkYCbcrSampler_DrawImageWithYcbcrSampler")
		// skbug.com/12826
		skip(ALL, "test", ALL, "GrThreadSafeCache16Verts")
		// skbug.com/12829
		skip(ALL, "test", ALL, "image_subset")
	} else if b.cpu() {
		args = append(args, "--nogpu")

		configs = append(configs, "8888")

		if b.extraConfig("SkVM") {
			args = append(args, "--skvm")
		}

		if b.extraConfig("BonusConfigs") {
			configs = []string{
				"g8", "565",
				"pic-8888", "serialize-8888",
				"linear-f16", "srgb-rgba", "srgb-f16", "narrow-rgba", "narrow-f16",
				"p3-rgba", "p3-f16", "rec2020-rgba", "rec2020-f16"}
		}

		if b.extraConfig("PDF") {
			configs = []string{"pdf"}
			args = append(args, "--rasterize_pdf") // Works only on Mac.
			// Take ~forever to rasterize:
			skip("pdf", "gm", ALL, "lattice2")
			skip("pdf", "gm", ALL, "hairmodes")
			skip("pdf", "gm", ALL, "longpathdash")
		}

	} else if b.gpu() {
		args = append(args, "--nocpu")

		// Add in either gles or gl configs to the canonical set based on OS
		glPrefix = "gl"
		// Use 4x MSAA for all our testing. It's more consistent and 8x MSAA is nondeterministic (by
		// design) on NVIDIA hardware. The problem is especially bad on ANGLE.  skia:6813 skia:6545
		sampleCount = 4
		if b.os("Android", "iOS") {
			glPrefix = "gles"
			// MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
			// MSAA is disabled on Pixel5 (https://skbug.com/11152).
			if b.model("Pixel3a", "Pixel5") {
				sampleCount = 0
			}
		} else if b.matchGpu("Intel") {
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			sampleCount = 0
		} else if b.os("ChromeOS") {
			glPrefix = "gles"
		}

		if b.extraConfig("NativeFonts") {
			configs = append(configs, glPrefix)
		} else {
			configs = append(configs, glPrefix, glPrefix+"dft", "srgb-"+glPrefix)
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("%smsaa%d", glPrefix, sampleCount))
				// Temporarily limit the bots we test dynamic MSAA on.
				if b.gpu("QuadroP400", "MaliG77") || b.matchOs("Mac") {
					configs = append(configs, fmt.Sprintf("%sdmsaa", glPrefix))
				}
			}
		}

		// The Tegra3 doesn't support MSAA
		if b.gpu("Tegra3") ||
			// We aren't interested in fixing msaa bugs on current iOS devices.
			b.model("iPad4", "iPadPro", "iPhone7") ||
			// skia:5792
			b.gpu("IntelHD530", "IntelIris540") {
			configs = removeContains(configs, "msaa")
		}

		// We want to test both the OpenGL config and the GLES config on Linux Intel:
		// GL is used by Chrome, GLES is used by ChromeOS.
		// Also do the Ganesh threading verification test (render with and without
		// worker threads, using only the SW path renderer, and compare the results).
		if b.matchGpu("Intel") && b.isLinux() {
			configs = append(configs, "gles", "glesdft", "srgb-gles", "gltestthreading")
			// skbug.com/6333, skbug.com/6419, skbug.com/6702
			skip("gltestthreading", "gm", ALL, "lcdblendmodes")
			skip("gltestthreading", "gm", ALL, "lcdoverlap")
			skip("gltestthreading", "gm", ALL, "textbloblooper")
			// All of these GMs are flaky, too:
			skip("gltestthreading", "gm", ALL, "savelayer_with_backdrop")
			skip("gltestthreading", "gm", ALL, "persp_shaders_bw")
			skip("gltestthreading", "gm", ALL, "dftext_blob_persp")
			skip("gltestthreading", "gm", ALL, "dftext")
			skip("gltestthreading", "gm", ALL, "gpu_blur_utils")
			skip("gltestthreading", "gm", ALL, "gpu_blur_utils_ref")
			skip("gltestthreading", "gm", ALL, "gpu_blur_utils_subset_rect")
			skip("gltestthreading", "gm", ALL, "gpu_blur_utils_subset_rect_ref")
			// skbug.com/7523 - Flaky on various GPUs
			skip("gltestthreading", "gm", ALL, "orientation")
			// These GMs only differ in the low bits
			skip("gltestthreading", "gm", ALL, "stroketext")
			skip("gltestthreading", "gm", ALL, "draw_image_set")
		}

		// CommandBuffer bot *only* runs the cmdbuffer_es2 configs.
		if b.extraConfig("CommandBuffer") {
			configs = []string{"cmdbuffer_es2"}
			if sampleCount > 0 {
				configs = append(configs, "cmdbuffer_es2_dmsaa")
			}
		}

		// Dawn bot *only* runs the dawn config
		if b.extraConfig("Dawn") {
			// tint:1045: Tint doesn't implement MatrixInverse yet.
			skip(ALL, "gm", ALL, "runtime_intrinsics_matrix")
			configs = []string{"dawn"}
		}

		// Graphite bot *only* runs the grmtl config
		if b.extraConfig("Graphite") {
			args = append(args, "--nogpu") // disable non-Graphite tests

			// TODO: re-enable - currently fails with "Failed to make lazy image"
			skip(ALL, "gm", ALL, "image_subset")

			if b.extraConfig("ASAN") {
				// skbug.com/12507 (Neon UB during JPEG compression on M1 ASAN Graphite bot)
				skip(ALL, "gm", ALL, "yuv420_odd_dim") // Oddly enough yuv420_odd_dim_repeat doesn't crash
				skip(ALL, "gm", ALL, "encode-alpha-jpeg")
				skip(ALL, "gm", ALL, "encode")
				skip(ALL, "gm", ALL, "jpg-color-cube")
			}
			configs = []string{"grmtl"}
		}

		// ANGLE bot *only* runs the angle configs
		if b.extraConfig("ANGLE") {
			configs = []string{"angle_d3d11_es2",
				"angle_gl_es2",
				"angle_d3d11_es3"}
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("angle_d3d11_es2_msaa%d", sampleCount))
				configs = append(configs, fmt.Sprintf("angle_d3d11_es2_dmsaa"))
				configs = append(configs, fmt.Sprintf("angle_gl_es2_dmsaa"))
				configs = append(configs, fmt.Sprintf("angle_d3d11_es3_msaa%d", sampleCount))
				configs = append(configs, fmt.Sprintf("angle_d3d11_es3_dmsaa"))
				configs = append(configs, fmt.Sprintf("angle_gl_es3_dmsaa"))
			}
			if b.matchGpu("GTX", "Quadro") {
				// See skia:7823 and chromium:693090.
				configs = append(configs, "angle_gl_es3")
				if sampleCount > 0 {
					configs = append(configs, fmt.Sprintf("angle_gl_es2_msaa%d", sampleCount))
					configs = append(configs, fmt.Sprintf("angle_gl_es2_dmsaa"))
					configs = append(configs, fmt.Sprintf("angle_gl_es3_msaa%d", sampleCount))
					configs = append(configs, fmt.Sprintf("angle_gl_es3_dmsaa"))
				}
			}
			if !b.matchGpu("GTX", "Quadro", "GT610") {
				// See skia:10149
				configs = append(configs, "angle_d3d9_es2")
			}
			if b.model("NUC5i7RYH") {
				// skbug.com/7376
				skip(ALL, "test", ALL, "ProcessorCloneTest")
			}
		}

		if b.model("AndroidOne", "Nexus5", "Nexus7", "JioNext") {
			// skbug.com/9019
			skip(ALL, "test", ALL, "ProcessorCloneTest")
			skip(ALL, "test", ALL, "Programs")
			skip(ALL, "test", ALL, "ProcessorOptimizationValidationTest")
		}

		if b.model("GalaxyS20") {
			// skbug.com/10595
			skip(ALL, "test", ALL, "ProcessorCloneTest")
		}

		if b.extraConfig("CommandBuffer") && b.model("MacBook10.1") {
			// skbug.com/9235
			skip(ALL, "test", ALL, "Programs")
		}

		if b.model("Spin513") {
			// skbug.com/11876
			skip(ALL, "test", ALL, "Programs")
			// skbug.com/12486
			skip(ALL, "test", ALL, "TestMockContext")
			skip(ALL, "test", ALL, "TestGpuRenderingContexts")
			skip(ALL, "test", ALL, "TestGpuAllContexts")
			skip(ALL, "test", ALL, "OverdrawSurface_Gpu")
			skip(ALL, "test", ALL, "ReplaceSurfaceBackendTexture")
			skip(ALL, "test", ALL, "SurfaceAttachStencil_Gpu")
			skip(ALL, "test", ALL, "SurfaceWrappedWithRelease_Gpu")
		}

		if b.extraConfig("CommandBuffer") {
			// skbug.com/10412
			skip(ALL, "test", ALL, "GLBackendAllocationTest")
			skip(ALL, "test", ALL, "InitialTextureClear")
			// skbug.com/12437
			skip(ALL, "test", ALL, "GrDDLImage_MakeSubset")
			skip(ALL, "test", ALL, "GrContext_oomed")
		}

		// skbug.com/9043 - these devices render this test incorrectly
		// when opList splitting reduction is enabled
		if b.gpu() && b.extraConfig("Vulkan") && (b.gpu("RadeonR9M470X", "RadeonHD7770")) {
			skip(ALL, "tests", ALL, "VkDrawableImportTest")
		}
		if b.extraConfig("Vulkan") {
			configs = []string{"vk"}
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926, skia:9023
			if !b.matchGpu("Intel") {
				configs = append(configs, "vkmsaa4")
			}
			// Temporarily limit the bots we test dynamic MSAA on.
			if b.gpu("QuadroP400", "MaliG77") && !b.extraConfig("TSAN") {
				configs = append(configs, "vkdmsaa")
			}
		}
		if b.extraConfig("Metal") {
			configs = []string{"mtl"}
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			if !b.matchGpu("Intel") {
				configs = append(configs, "mtlmsaa4")
			}
		}
		if b.extraConfig("Direct3D") {
			configs = []string{"d3d"}
		}

		// Test 1010102 on our Linux/NVIDIA bots and the persistent cache config
		// on the GL bots.
		if b.gpu("QuadroP400") && !b.extraConfig("PreAbandonGpuContext") && !b.extraConfig("TSAN") && b.isLinux() {
			if b.extraConfig("Vulkan") {
				configs = append(configs, "vk1010102")
				// Decoding transparent images to 1010102 just looks bad
				skip("vk1010102", "image", ALL, ALL)
			} else {
				configs = append(configs, "gl1010102", "gltestpersistentcache", "gltestglslcache", "gltestprecompile")
				// Decoding transparent images to 1010102 just looks bad
				skip("gl1010102", "image", ALL, ALL)
				// These tests produce slightly different pixels run to run on NV.
				skip("gltestpersistentcache", "gm", ALL, "atlastext")
				skip("gltestpersistentcache", "gm", ALL, "dftext")
				skip("gltestpersistentcache", "gm", ALL, "glyph_pos_h_b")
				skip("gltestpersistentcache", "gm", ALL, "glyph_pos_h_f")
				skip("gltestpersistentcache", "gm", ALL, "glyph_pos_n_f")
				skip("gltestglslcache", "gm", ALL, "atlastext")
				skip("gltestglslcache", "gm", ALL, "dftext")
				skip("gltestglslcache", "gm", ALL, "glyph_pos_h_b")
				skip("gltestglslcache", "gm", ALL, "glyph_pos_h_f")
				skip("gltestglslcache", "gm", ALL, "glyph_pos_n_f")
				skip("gltestprecompile", "gm", ALL, "atlastext")
				skip("gltestprecompile", "gm", ALL, "dftext")
				skip("gltestprecompile", "gm", ALL, "glyph_pos_h_b")
				skip("gltestprecompile", "gm", ALL, "glyph_pos_h_f")
				skip("gltestprecompile", "gm", ALL, "glyph_pos_n_f")
				// Tessellation shaders do not yet participate in the persistent cache.
				skip("gltestpersistentcache", "gm", ALL, "tessellation")
				skip("gltestglslcache", "gm", ALL, "tessellation")
				skip("gltestprecompile", "gm", ALL, "tessellation")
			}
		}

		// We also test the SkSL precompile config on Pixel2XL as a representative
		// Android device - this feature is primarily used by Flutter.
		if b.model("Pixel2XL") && !b.extraConfig("Vulkan") {
			configs = append(configs, "glestestprecompile")
		}

		// Test SkSL precompile on iPhone 8 as representative iOS device
		if b.model("iPhone8") && b.extraConfig("Metal") {
			configs = append(configs, "mtltestprecompile")
			// avoid tests that can generate slightly different pixels per run
			skip("mtltestprecompile", "gm", ALL, "atlastext")
			skip("mtltestprecompile", "gm", ALL, "circular_arcs_hairline")
			skip("mtltestprecompile", "gm", ALL, "dashcircle")
			skip("mtltestprecompile", "gm", ALL, "dftext")
			skip("mtltestprecompile", "gm", ALL, "fontmgr_bounds")
			skip("mtltestprecompile", "gm", ALL, "fontmgr_bounds_1_-0.25")
			skip("mtltestprecompile", "gm", ALL, "glyph_pos_h_b")
			skip("mtltestprecompile", "gm", ALL, "glyph_pos_h_f")
			skip("mtltestprecompile", "gm", ALL, "glyph_pos_n_f")
			skip("mtltestprecompile", "gm", ALL, "persp_images")
			skip("mtltestprecompile", "gm", ALL, "ovals")
			skip("mtltestprecompile", "gm", ALL, "roundrects")
			skip("mtltestprecompile", "gm", ALL, "shadow_utils_occl")
			skip("mtltestprecompile", "gm", ALL, "strokedlines")
			skip("mtltestprecompile", "gm", ALL, "strokerect")
			skip("mtltestprecompile", "gm", ALL, "strokes3")
			skip("mtltestprecompile", "gm", ALL, "texel_subset_linear_mipmap_nearest_down")
			skip("mtltestprecompile", "gm", ALL, "texel_subset_linear_mipmap_linear_down")
			skip("mtltestprecompile", "gm", ALL, "textblobmixedsizes_df")
			skip("mtltestprecompile", "gm", ALL, "yuv420_odd_dim_repeat")
			skip("mtltestprecompile", "svg", ALL, "A_large_blank_world_map_with_oceans_marked_in_blue.svg")
			skip("mtltestprecompile", "svg", ALL, "Chalkboard.svg")
			skip("mtltestprecompile", "svg", ALL, "Ghostscript_Tiger.svg")
			skip("mtltestprecompile", "svg", ALL, "Seal_of_American_Samoa.svg")
			skip("mtltestprecompile", "svg", ALL, "Seal_of_Illinois.svg")
			skip("mtltestprecompile", "svg", ALL, "desk_motionmark_paths.svg")
			skip("mtltestprecompile", "svg", ALL, "rg1024_green_grapes.svg")
			skip("mtltestprecompile", "svg", ALL, "shapes-intro-02-f.svg")
			skip("mtltestprecompile", "svg", ALL, "tiger-8.svg")
		}
		// Test reduced shader mode on iPhone 11 as representative iOS device
		if b.model("iPhone11") && b.extraConfig("Metal") {
			configs = append(configs, "mtlreducedshaders")
		}

		if b.gpu("AppleM1") && !b.extraConfig("Metal") {
			skip(ALL, "test", ALL, "TransferPixelsFromTextureTest") // skia:11814
		}

		if b.model(DONT_REDUCE_OPS_TASK_SPLITTING_MODELS...) {
			args = append(args, "--dontReduceOpsTaskSplitting", "true")
		}

		// Test reduceOpsTaskSplitting fallback when over budget.
		if b.model("NUC7i5BNK") && b.extraConfig("ASAN") {
			args = append(args, "--gpuResourceCacheLimit", "16777216")
		}

		// Test rendering to wrapped dsts on a few bots
		// Also test "narrow-glf16", which hits F16 surfaces and F16 vertex colors.
		if b.extraConfig("BonusConfigs") {
			configs = []string{"glbetex", "glbert", "narrow-glf16", "glreducedshaders"}
		}

		if b.os("ChromeOS") {
			// Just run GLES for now - maybe add gles_msaa4 in the future
			configs = []string{"gles"}
		}

		// Test GPU tessellation path renderer.
		if b.extraConfig("GpuTess") {
			configs = []string{glPrefix + "msaa4"}
			// Use hardware tessellation as much as possible for testing. Use 16 segments max to
			// verify the chopping logic.
			args = append(args,
				"--pr", "atlas", "tess", "--hwtess", "--alwaysHwTess",
				"--maxTessellationSegments", "16")
		}

		// DDL is a GPU-only feature
		if b.extraConfig("DDL1") {
			// This bot generates comparison images for the large skps and the gms
			configs = filter(configs, "gl", "vk", "mtl")
			args = append(args, "--skpViewportSize", "2048")
		}
		if b.extraConfig("DDL3") {
			// This bot generates the real ddl images for the large skps and the gms
			configs = suffix(filter(configs, "gl", "vk", "mtl"), "ddl")
			args = append(args, "--skpViewportSize", "2048")
			args = append(args, "--gpuThreads", "0")
		}
		if b.extraConfig("OOPRDDL") {
			// This bot generates the real oopr/DDL images for the large skps and the GMs
			configs = suffix(filter(configs, "gl", "vk", "mtl"), "ooprddl")
			args = append(args, "--skpViewportSize", "2048")
			args = append(args, "--gpuThreads", "0")
		}
	}

	// Sharding.
	tf := b.parts["test_filter"]
	if tf != "" && tf != "All" {
		// Expected format: shard_XX_YY
		split := strings.Split(tf, ALL)
		if len(split) == 3 {
			args = append(args, "--shard", split[1])
			args = append(args, "--shards", split[2])
		} else {
			glog.Fatalf("Invalid task name - bad shards: %s", tf)
		}
	}

	args = append(args, "--config")
	args = append(args, configs...)

	removeFromArgs := func(arg string) {
		args = remove(args, arg)
	}

	// Run tests, gms, and image decoding tests everywhere.
	args = append(args, "--src", "tests", "gm", "image", "lottie", "colorImage", "svg", "skp")
	if b.gpu() {
		// Don't run the "svgparse_*" svgs on GPU.
		skip(ALL, "svg", ALL, "svgparse_")
	} else if b.Name == "Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN" {
		// Only run the CPU SVGs on 8888.
		skip("~8888", "svg", ALL, ALL)
	} else {
		// On CPU SVGs we only care about parsing. Only run them on the above bot.
		removeFromArgs("svg")
	}

	// Eventually I'd like these to pass, but for now just skip 'em.
	if b.extraConfig("SK_FORCE_RASTER_PIPELINE_BLITTER") {
		removeFromArgs("tests")
	}

	if b.extraConfig("NativeFonts") { // images won't exercise native font integration :)
		removeFromArgs("image")
		removeFromArgs("colorImage")
	}

	if b.matchExtraConfig("Graphite") {
		// The Graphite bots run the skps, gms and tests
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	} else if b.matchExtraConfig("DDL", "PDF") {
		// The DDL and PDF bots just render the large skps and the gms
		removeFromArgs("tests")
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	} else {
		// No other bots render the .skps.
		removeFromArgs("skp")
	}

	if b.extraConfig("Lottie") {
		// Only run the lotties on Lottie bots.
		removeFromArgs("tests")
		removeFromArgs("gm")
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
		removeFromArgs("skp")
	} else {
		removeFromArgs("lottie")
	}

	if b.extraConfig("TSAN") {
		// skbug.com/10848
		removeFromArgs("svg")
		// skbug.com/12900 avoid OOM on
		// Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-TSAN_Vulkan
		if b.Name == "Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-TSAN_Vulkan" {
			skip("_", "test", "_", "_")
		}
	}

	// TODO: ???
	skip("f16", ALL, ALL, "dstreadshuffle")
	skip("srgb-gl", "image", ALL, ALL)
	skip("srgb-gles", "image", ALL, ALL)

	// --src image --config g8 means "decode into Gray8", which isn't supported.
	skip("g8", "image", ALL, ALL)
	skip("g8", "colorImage", ALL, ALL)

	if b.extraConfig("Valgrind") {
		// These take 18+ hours to run.
		skip("pdf", "gm", ALL, "fontmgr_iter")
		skip("pdf", ALL, ALL, "PANO_20121023_214540.jpg")
		skip("pdf", "skp", ALL, "worldjournal")
		skip("pdf", "skp", ALL, "desk_baidu.skp")
		skip("pdf", "skp", ALL, "desk_wikipedia.skp")
		skip(ALL, "svg", ALL, ALL)
		// skbug.com/9171 and 8847
		skip(ALL, "test", ALL, "InitialTextureClear")
	}

	if b.model("Pixel3") {
		// skbug.com/10546
		skip("vkddl", "gm", ALL, "compressed_textures_nmof")
		skip("vkddl", "gm", ALL, "compressed_textures_npot")
		skip("vkddl", "gm", ALL, "compressed_textures")
	}

	if b.model("TecnoSpark3Pro", "Wembley") {
		// skbug.com/9421
		skip(ALL, "test", ALL, "InitialTextureClear")
	}

	if b.model("Wembley", "JioNext") {
		// These tests run forever on the Wembley.
		skip(ALL, "gm", ALL, "async_rescale_and_read")
	}

	if b.os("iOS") {
		skip(glPrefix, "skp", ALL, ALL)
	}

	if b.matchOs("Mac", "iOS") {
		// CG fails on questionable bmps
		skip(ALL, "image", "gen_platf", "rgba32abf.bmp")
		skip(ALL, "image", "gen_platf", "rgb24prof.bmp")
		skip(ALL, "image", "gen_platf", "rgb24lprof.bmp")
		skip(ALL, "image", "gen_platf", "8bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "4bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "32bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "24bpp-pixeldata-cropped.bmp")

		// CG has unpredictable behavior on this questionable gif
		// It's probably using uninitialized memory
		skip(ALL, "image", "gen_platf", "frame_larger_than_image.gif")

		// CG has unpredictable behavior on incomplete pngs
		// skbug.com/5774
		skip(ALL, "image", "gen_platf", "inc0.png")
		skip(ALL, "image", "gen_platf", "inc1.png")
		skip(ALL, "image", "gen_platf", "inc2.png")
		skip(ALL, "image", "gen_platf", "inc3.png")
		skip(ALL, "image", "gen_platf", "inc4.png")
		skip(ALL, "image", "gen_platf", "inc5.png")
		skip(ALL, "image", "gen_platf", "inc6.png")
		skip(ALL, "image", "gen_platf", "inc7.png")
		skip(ALL, "image", "gen_platf", "inc8.png")
		skip(ALL, "image", "gen_platf", "inc9.png")
		skip(ALL, "image", "gen_platf", "inc10.png")
		skip(ALL, "image", "gen_platf", "inc11.png")
		skip(ALL, "image", "gen_platf", "inc12.png")
		skip(ALL, "image", "gen_platf", "inc13.png")
		skip(ALL, "image", "gen_platf", "inc14.png")
		skip(ALL, "image", "gen_platf", "incInterlaced.png")

		// These images fail after Mac 10.13.1 upgrade.
		skip(ALL, "image", "gen_platf", "incInterlaced.gif")
		skip(ALL, "image", "gen_platf", "inc1.gif")
		skip(ALL, "image", "gen_platf", "inc0.gif")
		skip(ALL, "image", "gen_platf", "butterfly.gif")
	}

	// WIC fails on questionable bmps
	if b.matchOs("Win") {
		skip(ALL, "image", "gen_platf", "pal8os2v2.bmp")
		skip(ALL, "image", "gen_platf", "pal8os2v2-16.bmp")
		skip(ALL, "image", "gen_platf", "rgba32abf.bmp")
		skip(ALL, "image", "gen_platf", "rgb24prof.bmp")
		skip(ALL, "image", "gen_platf", "rgb24lprof.bmp")
		skip(ALL, "image", "gen_platf", "8bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "4bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "32bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "24bpp-pixeldata-cropped.bmp")
		if b.arch("x86_64") && b.cpu() {
			// This GM triggers a SkSmallAllocator assert.
			skip(ALL, "gm", ALL, "composeshader_bitmap")
		}
	}

	if b.matchOs("Win", "Mac") {
		// WIC and CG fail on arithmetic jpegs
		skip(ALL, "image", "gen_platf", "testimgari.jpg")
		// More questionable bmps that fail on Mac, too. skbug.com/6984
		skip(ALL, "image", "gen_platf", "rle8-height-negative.bmp")
		skip(ALL, "image", "gen_platf", "rle4-height-negative.bmp")
	}

	// These PNGs have CRC errors. The platform generators seem to draw
	// uninitialized memory without reporting an error, so skip them to
	// avoid lots of images on Gold.
	skip(ALL, "image", "gen_platf", "error")

	if b.os("Android", "iOS") {
		// This test crashes the N9 (perhaps because of large malloc/frees). It also
		// is fairly slow and not platform-specific. So we just disable it on all of
		// Android and iOS. skia:5438
		skip(ALL, "test", ALL, "GrStyledShape")
	}

	if internalHardwareLabel == "5" {
		// http://b/118312149#comment9
		skip(ALL, "test", ALL, "SRGBReadWritePixels")
	}

	// skia:4095
	badSerializeGMs := []string{
		"strict_constraint_batch_no_red_allowed", // https://crbug.com/skia/10278
		"strict_constraint_no_red_allowed",       // https://crbug.com/skia/10278
		"fast_constraint_red_is_allowed",         // https://crbug.com/skia/10278
		"c_gms",
		"colortype",
		"colortype_xfermodes",
		"drawfilter",
		"fontmgr_bounds_0.75_0",
		"fontmgr_bounds_1_-0.25",
		"fontmgr_bounds",
		"fontmgr_match",
		"fontmgr_iter",
		"imagemasksubset",
		"wacky_yuv_formats_domain",
		"imagemakewithfilter",
		"imagemakewithfilter_crop",
		"imagemakewithfilter_crop_ref",
		"imagemakewithfilter_ref",
	}

	// skia:5589
	badSerializeGMs = append(badSerializeGMs,
		"bitmapfilters",
		"bitmapshaders",
		"convex_poly_clip",
		"extractalpha",
		"filterbitmap_checkerboard_32_32_g8",
		"filterbitmap_image_mandrill_64",
		"shadows",
		"simpleaaclip_aaclip",
	)

	// skia:5595
	badSerializeGMs = append(badSerializeGMs,
		"composeshader_bitmap",
		"scaled_tilemodes_npot",
		"scaled_tilemodes",
	)

	// skia:5778
	badSerializeGMs = append(badSerializeGMs, "typefacerendering_pfaMac")
	// skia:5942
	badSerializeGMs = append(badSerializeGMs, "parsedpaths")

	// these use a custom image generator which doesn't serialize
	badSerializeGMs = append(badSerializeGMs, "ImageGeneratorExternal_rect")
	badSerializeGMs = append(badSerializeGMs, "ImageGeneratorExternal_shader")

	// skia:6189
	badSerializeGMs = append(badSerializeGMs, "shadow_utils")

	// skia:7938
	badSerializeGMs = append(badSerializeGMs, "persp_images")

	// Not expected to round trip encoding/decoding.
	badSerializeGMs = append(badSerializeGMs, "all_bitmap_configs")
	badSerializeGMs = append(badSerializeGMs, "makecolorspace")
	badSerializeGMs = append(badSerializeGMs, "readpixels")
	badSerializeGMs = append(badSerializeGMs, "draw_image_set_rect_to_rect")
	badSerializeGMs = append(badSerializeGMs, "draw_image_set_alpha_only")
	badSerializeGMs = append(badSerializeGMs, "compositor_quads_shader")
	badSerializeGMs = append(badSerializeGMs, "wacky_yuv_formats_qtr")
	badSerializeGMs = append(badSerializeGMs, "runtime_effect_image")
	badSerializeGMs = append(badSerializeGMs, "ctmpatheffect")

	// This GM forces a path to be convex. That property doesn't survive
	// serialization.
	badSerializeGMs = append(badSerializeGMs, "analytic_antialias_convex")

	for _, test := range badSerializeGMs {
		skip("serialize-8888", "gm", ALL, test)
	}

	// We skip these to avoid out-of-memory failures.
	if b.matchOs("Win", "Android") {
		for _, test := range []string{"verylargebitmap", "verylarge_picture_image"} {
			skip("serialize-8888", "gm", ALL, test)
		}
	}
	if b.matchOs("Mac") && b.cpu() {
		// skia:6992
		skip("pic-8888", "gm", ALL, "encode-platform")
		skip("serialize-8888", "gm", ALL, "encode-platform")
	}

	// skia:4769
	skip("pic-8888", "gm", ALL, "drawfilter")

	// skia:4703
	for _, test := range []string{"image-cacherator-from-picture",
		"image-cacherator-from-raster",
		"image-cacherator-from-ctable"} {
		skip("pic-8888", "gm", ALL, test)
		skip("serialize-8888", "gm", ALL, test)
	}

	// GM that requires raster-backed canvas
	for _, test := range []string{"complexclip4_bw", "complexclip4_aa", "p3",
		"async_rescale_and_read_text_up_large",
		"async_rescale_and_read_text_up",
		"async_rescale_and_read_text_down",
		"async_rescale_and_read_dog_up",
		"async_rescale_and_read_dog_down",
		"async_rescale_and_read_rose",
		"async_rescale_and_read_no_bleed",
		"async_rescale_and_read_alpha_type"} {
		skip("pic-8888", "gm", ALL, test)
		skip("serialize-8888", "gm", ALL, test)

		// GM requires canvas->makeSurface() to return a valid surface.
		// TODO(borenet): These should be just outside of this block but are
		// left here to match the recipe which has an indentation bug.
		skip("pic-8888", "gm", ALL, "blurrect_compare")
		skip("serialize-8888", "gm", ALL, "blurrect_compare")
	}

	// Extensions for RAW images
	r := []string{
		"arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
		"ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
	}

	// skbug.com/4888
	// Skip RAW images (and a few large PNGs) on GPU bots
	// until we can resolve failures.
	if b.gpu() {
		skip(ALL, "image", ALL, "interlaced1.png")
		skip(ALL, "image", ALL, "interlaced2.png")
		skip(ALL, "image", ALL, "interlaced3.png")
		for _, rawExt := range r {
			skip(ALL, "image", ALL, "."+rawExt)
		}
	}

	// Skip memory intensive tests on 32-bit bots.
	if b.os("Win8") && b.arch("x86") {
		skip(ALL, "image", "f16", ALL)
		skip(ALL, "image", ALL, "abnormal.wbmp")
		skip(ALL, "image", ALL, "interlaced1.png")
		skip(ALL, "image", ALL, "interlaced2.png")
		skip(ALL, "image", ALL, "interlaced3.png")
		for _, rawExt := range r {
			skip(ALL, "image", ALL, "."+rawExt)
		}
	}

	if b.model("Nexus5", "Nexus5x", "JioNext") && b.gpu() {
		// skia:5876
		skip(ALL, "gm", ALL, "encode-platform")
	}

	if b.model("AndroidOne") && b.gpu() { // skia:4697, skia:4704, skia:4694, skia:4705, skia:11133
		skip(ALL, "gm", ALL, "bigblurs")
		skip(ALL, "gm", ALL, "strict_constraint_no_red_allowed")
		skip(ALL, "gm", ALL, "fast_constraint_red_is_allowed")
		skip(ALL, "gm", ALL, "dropshadowimagefilter")
		skip(ALL, "gm", ALL, "filterfastbounds")
		skip(glPrefix, "gm", ALL, "imageblurtiled")
		skip(ALL, "gm", ALL, "imagefiltersclipped")
		skip(ALL, "gm", ALL, "imagefiltersscaled")
		skip(ALL, "gm", ALL, "imageresizetiled")
		skip(ALL, "gm", ALL, "matrixconvolution")
		skip(ALL, "gm", ALL, "strokedlines")
		skip(ALL, "gm", ALL, "runtime_intrinsics_matrix")
		if sampleCount > 0 {
			glMsaaConfig := fmt.Sprintf("%smsaa%d", glPrefix, sampleCount)
			skip(glMsaaConfig, "gm", ALL, "imageblurtiled")
			skip(glMsaaConfig, "gm", ALL, "imagefiltersbase")
		}
	}

	if b.matchGpu("Adreno[3456]") { // disable broken tests on Adreno 3/4/5/6xx
		skip(ALL, "tests", ALL, "SkSLArrayCast_GPU")       // skia:12332
		skip(ALL, "tests", ALL, "SkSLArrayComparison_GPU") // skia:12332
		skip(ALL, "tests", ALL, "SkSLCommaSideEffects_GPU")
		skip(ALL, "tests", ALL, "SkSLIntrinsicMixFloat_GPU")
		skip(ALL, "tests", ALL, "SkSLIntrinsicClampFloat_GPU")
	}

	if b.matchGpu("Adreno[345]") && !b.extraConfig("Vulkan") { // disable broken tests on Adreno 3/4/5xx GLSL
		skip(ALL, "tests", ALL, "DSLFPTest_SwitchStatement")  // skia:11891
		skip(ALL, "tests", ALL, "SkSLMatrixToVectorCast_GPU") // skia:12192
		skip(ALL, "tests", ALL, "SkSLStructsInFunctions_GPU") // skia:11929
	}

	if b.matchGpu("Adreno6") || b.matchGpu("MaliG77") || b.matchGpu("QuadroP400") {
		skip(ALL, "tests", ALL, "SkSLRecursiveComparison_Arrays_GPU") // skia:12642
		skip(ALL, "tests", ALL, "SkSLRecursiveComparison_Structs_GPU")
		skip(ALL, "tests", ALL, "SkSLRecursiveComparison_Types_GPU")
		skip(ALL, "tests", ALL, "SkSLRecursiveComparison_Vectors_GPU")
	}

	if b.matchGpu("Adreno6") && !b.extraConfig("Vulkan") { // disable broken tests on Adreno 6xx GLSL
		skip(ALL, "tests", ALL, "SkSLIntrinsicIsInf_GPU") // skia:12377
	}

	if b.matchGpu("Adreno[56]") && b.extraConfig("Vulkan") { // disable broken tests on Adreno 5/6xx Vulkan
		skip(ALL, "tests", ALL, "SkSLInoutParameters_GPU")   // skia:12869
		skip(ALL, "tests", ALL, "SkSLOutParams_GPU")         // skia:11919
		skip(ALL, "tests", ALL, "SkSLOutParamsTricky_GPU")   // skia:11919
		skip(ALL, "tests", ALL, "SkSLOutParamsNoInline_GPU") // skia:11919
	}

	if (b.matchGpu("Adreno3") || b.matchGpu("Mali400")) && !b.extraConfig("Vulkan") {
		skip(ALL, "tests", ALL, "SkSLMatrices") // skia:12456
	}

	if b.gpu("QuadroP400") {
		skip(ALL, "tests", ALL, "SkSLCommaSideEffects")
	}

	if b.matchGpu("Mali400") || b.matchGpu("Tegra3") {
		skip(ALL, "tests", ALL, "SkSLMatrixScalarMath") // skia:12681
	}

	if b.gpu("IntelIris6100", "IntelHD4400") && b.matchOs("Win") && !b.extraConfig("Vulkan") {
		skip(ALL, "tests", ALL, "SkSLVectorToMatrixCast_GPU") // skia:12179, vec4(mat2) crash
		skip(ALL, "tests", ALL, "SkSLVectorScalarMath_GPU")   // skia:11919
		skip(ALL, "tests", ALL, "SkSLMatrixFoldingES2_GPU")   // skia:11919
	}

	if b.matchGpu("Intel") && b.matchOs("Win") && !b.extraConfig("Vulkan") {
		skip(ALL, "tests", ALL, "SkSLReturnsValueOnEveryPathES3_GPU") // skia:12465
	}

	if b.extraConfig("Vulkan") && b.isLinux() && b.matchGpu("Intel") {
		skip(ALL, "tests", ALL, "SkSLSwitchDefaultOnly_GPU") // skia:12465
	}

	if b.extraConfig("ANGLE") && b.matchOs("Win") && b.matchGpu("IntelIris(540|655)") {
		skip(ALL, "tests", ALL, "SkSLSwitchDefaultOnly_GPU") // skia:12465
		skip(ALL, "tests", ALL, "SkSLVectorScalarMath_GPU")  // skia:11919
	}

	if b.gpu("Tegra3") {
		// Tegra3 fails to compile break stmts inside a for loop (skia:12477)
		skip(ALL, "tests", ALL, "SkSLSwitch_GPU")
		skip(ALL, "tests", ALL, "SkSLSwitchDefaultOnly_GPU")
		skip(ALL, "tests", ALL, "SkSLSwitchWithFallthrough_GPU")
		skip(ALL, "tests", ALL, "SkSLSwitchWithLoops_GPU")
		skip(ALL, "tests", ALL, "SkSLSwitchCaseFolding_GPU")
		skip(ALL, "tests", ALL, "SkSLLoopFloat_GPU")
		skip(ALL, "tests", ALL, "SkSLLoopInt_GPU")
	}

	if b.gpu("QuadroP400") || b.gpu("GTX660") || b.gpu("GTX960") || b.gpu("Tegra3") {
		if !b.extraConfig("Vulkan") {
			// Various Nvidia GPUs crash or generate errors when assembling weird matrices
			skip(ALL, "tests", ALL, "SkSLMatrixConstructorsES2_GPU") // skia:12443
			skip(ALL, "tests", ALL, "SkSLMatrixConstructorsES3_GPU") // skia:12443
		}
		skip(ALL, "tests", ALL, "SkSLMatrixFoldingES2_GPU") // skia:11919
	}

	if b.gpu("PowerVRGE8320") || b.gpu("Tegra3") || b.gpu("Adreno308") {
		skip(ALL, "tests", ALL, "SkSLVectorScalarMath_GPU") // skia:11919
	}

	if b.gpu("PowerVRGE8320") {
		skip(ALL, "tests", ALL, "SkSLOutParamsAreDistinct_GPU")
	}

	if !b.extraConfig("Vulkan") && (b.gpu("RadeonR9M470X") || b.gpu("RadeonHD7770")) {
		// Some AMD GPUs can get the wrong result when assembling non-square matrices (skia:12443)
		skip(ALL, "tests", ALL, "SkSLMatrixConstructorsES3_GPU")
	}

	if b.matchGpu("Intel") { // some Intel GPUs don't return zero for the derivative of a uniform
		skip(ALL, "tests", ALL, "SkSLIntrinsicDFdy_GPU")
		skip(ALL, "tests", ALL, "SkSLIntrinsicDFdx_GPU")
		skip(ALL, "tests", ALL, "SkSLIntrinsicFwidth_GPU")
	}

	if b.matchOs("Mac") && b.matchGpu("Intel(Iris5100|HD6000)") {
		skip(ALL, "tests", ALL, "SkSLLoopFloat_GPU") // skia:12426
	}

	match := []string{}
	if b.extraConfig("Valgrind") { // skia:3021
		match = append(match, "~Threaded")
	}

	if b.extraConfig("Valgrind") && b.extraConfig("PreAbandonGpuContext") {
		// skia:6575
		match = append(match, "~multipicturedraw_")
	}

	if b.model("AndroidOne") {
		match = append(match, "~WritePixels")                             // skia:4711
		match = append(match, "~PremulAlphaRoundTrip_Gpu")                // skia:7501
		match = append(match, "~ReimportImageTextureWithMipLevels")       // skia:8090
		match = append(match, "~MorphologyFilterRadiusWithMirrorCTM_Gpu") // skia:10383
	}

	if b.extraConfig("MSAN") {
		match = append(match, "~Once", "~Shared") // Not sure what's up with these tests.
	}

	// By default, we test with GPU threading enabled, unless specifically
	// disabled.
	if b.extraConfig("NoGPUThreads") {
		args = append(args, "--gpuThreads", "0")
	}

	if b.extraConfig("Vulkan") && b.gpu("Adreno530") {
		// skia:5777
		match = append(match, "~CopySurface")
	}

	if b.extraConfig("Vulkan") && b.matchGpu("Adreno") {
		// skia:7663
		match = append(match, "~WritePixelsNonTextureMSAA_Gpu")
		match = append(match, "~WritePixelsMSAA_Gpu")
	}

	if b.extraConfig("Vulkan") && b.isLinux() && b.gpu("IntelIris640") {
		match = append(match, "~VkHeapTests") // skia:6245
	}

	if b.isLinux() && b.gpu("IntelIris640") {
		match = append(match, "~Programs") // skia:7849
	}

	if b.model("TecnoSpark3Pro", "Wembley") {
		// skia:9814
		match = append(match, "~Programs")
		match = append(match, "~ProcessorCloneTest")
		match = append(match, "~ProcessorOptimizationValidationTest")
	}

	if b.gpu("IntelIris640", "IntelHD615", "IntelHDGraphics615") {
		match = append(match, "~^SRGBReadWritePixels$") // skia:9225
	}

	if b.extraConfig("Vulkan") && b.isLinux() && b.gpu("IntelHD405") {
		// skia:7322
		skip("vk", "gm", ALL, "skbug_257")
		skip("vk", "gm", ALL, "filltypespersp")
		match = append(match, "~^ClearOp$")
		match = append(match, "~^CopySurface$")
		match = append(match, "~^ImageNewShader_GPU$")
		match = append(match, "~^InitialTextureClear$")
		match = append(match, "~^PinnedImageTest$")
		match = append(match, "~^ReadPixels_Gpu$")
		match = append(match, "~^ReadPixels_Texture$")
		match = append(match, "~^SRGBReadWritePixels$")
		match = append(match, "~^VkUploadPixelsTests$")
		match = append(match, "~^WritePixelsNonTexture_Gpu$")
		match = append(match, "~^WritePixelsNonTextureMSAA_Gpu$")
		match = append(match, "~^WritePixels_Gpu$")
		match = append(match, "~^WritePixelsMSAA_Gpu$")
	}

	if b.extraConfig("Vulkan") && b.gpu("GTX660") && b.matchOs("Win") {
		// skbug.com/8047
		match = append(match, "~FloatingPointTextureTest$")
	}

	if b.extraConfig("Metal") && b.gpu("RadeonHD8870M") && b.matchOs("Mac") {
		// skia:9255
		match = append(match, "~WritePixelsNonTextureMSAA_Gpu")
		// skbug.com/11366
		match = append(match, "~SurfacePartialDraw_Gpu")
	}

	if b.extraConfig("Metal") && b.gpu("PowerVRGX6450") && b.matchOs("iOS") {
		// skbug.com/11885
		match = append(match, "~flight_animated_image")
	}

	if b.extraConfig("ANGLE") {
		// skia:7835
		match = append(match, "~BlurMaskBiggerThanDest")
	}

	if b.gpu("IntelIris6100") && b.extraConfig("ANGLE") && !b.debug() {
		// skia:7376
		match = append(match, "~^ProcessorOptimizationValidationTest$")
	}

	if b.gpu("IntelIris6100", "IntelHD4400") && b.extraConfig("ANGLE") {
		// skia:6857
		skip("angle_d3d9_es2", "gm", ALL, "lighting")
	}

	if b.gpu("PowerVRGX6250") {
		match = append(match, "~gradients_view_perspective_nodither") //skia:6972
	}

	if b.arch("arm") && b.extraConfig("ASAN") {
		// TODO: can we run with env allocator_may_return_null=1 instead?
		match = append(match, "~BadImage")
	}

	if b.matchOs("Mac") && b.gpu("IntelHD6000") {
		// skia:7574
		match = append(match, "~^ProcessorCloneTest$")
		match = append(match, "~^GrMeshTest$")
	}

	if b.matchOs("Mac") && b.gpu("IntelHD615") {
		// skia:7603
		match = append(match, "~^GrMeshTest$")
	}

	if b.extraConfig("Vulkan") && b.model("GalaxyS20") {
		// skia:10247
		match = append(match, "~VkPrepareForExternalIOQueueTransitionTest")
	}
	if b.matchExtraConfig("Graphite") {
		// skia:12813
		match = append(match, "~async_rescale_and_read")
	}

	if len(skipped) > 0 {
		args = append(args, "--skip")
		args = append(args, skipped...)
	}

	if len(match) > 0 {
		args = append(args, "--match")
		args = append(args, match...)
	}

	// These bots run out of memory running RAW codec tests. Do not run them in
	// parallel
	// TODO(borenet): Previously this was `'Nexus5' in bot or 'Nexus9' in bot`
	// which also matched 'Nexus5x'. I added That here to maintain the
	// existing behavior, but we should verify that it's needed.
	if b.model("Nexus5", "Nexus5x", "Nexus9", "JioNext") {
		args = append(args, "--noRAW_threading")
	}

	if b.extraConfig("FSAA") {
		args = append(args, "--analyticAA", "false")
	}
	if b.extraConfig("FAAA") {
		args = append(args, "--forceAnalyticAA")
	}

	if !b.extraConfig("NativeFonts") {
		args = append(args, "--nonativeFonts")
	}

	if b.extraConfig("GDI") {
		args = append(args, "--gdi")
	}

	// Let's make all bots produce verbose output by default.
	args = append(args, "--verbose")

	// See skia:2789.
	if b.extraConfig("AbandonGpuContext") {
		args = append(args, "--abandonGpuContext")
	}
	if b.extraConfig("PreAbandonGpuContext") {
		args = append(args, "--preAbandonGpuContext")
	}
	if b.extraConfig("ReleaseAndAbandonGpuContext") {
		args = append(args, "--releaseAndAbandonGpuContext")
	}

	// Finalize the DM flags and properties.
	b.recipeProp("dm_flags", marshalJson(args))
	b.recipeProp("dm_properties", marshalJson(properties))

	// Add properties indicating which assets the task should use.
	if b.matchExtraConfig("Lottie") {
		b.asset("lottie-samples")
		b.recipeProp("lotties", "true")
	} else {
		b.asset("skimage")
		b.recipeProp("images", "true")
		b.asset("skp")
		b.recipeProp("skps", "true")
		b.asset("svg")
		b.recipeProp("svgs", "true")
	}
	b.recipeProp("do_upload", fmt.Sprintf("%t", b.doUpload()))
	b.recipeProp("resources", "true")
}

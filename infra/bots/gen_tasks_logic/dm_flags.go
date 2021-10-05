// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package gen_tasks_logic

import (
	"fmt"
	"log"
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

	skip := func(quad ...string) {
		if len(quad) == 1 {
			quad = strings.Fields(quad[0])
		}
		if len(quad) != 4 {
			log.Fatalf("Invalid value for --skip: %+v", quad)
		}
		config := quad[0]
		src := quad[1]
		options := quad[2]
		name := quad[3]
		if config == "_" ||
			hasConfig(config) ||
			(config[0] == '~' && hasConfig(config[1:])) {
			skipped = append(skipped, config, src, options, name)
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
		configs = append(configs, "gles", "glesdft", "glesdmsaa")
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
			skip("pdf gm _ lattice2")
			skip("pdf gm _ hairmodes")
			skip("pdf gm _ longpathdash")
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
			b.model("iPad4", "iPadPro", "iPhone6", "iPhone7") ||
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
			skip("gltestthreading gm _ lcdblendmodes")
			skip("gltestthreading gm _ lcdoverlap")
			skip("gltestthreading gm _ textbloblooper")
			// All of these GMs are flaky, too:
			skip("gltestthreading gm _ savelayer_with_backdrop")
			skip("gltestthreading gm _ persp_shaders_bw")
			skip("gltestthreading gm _ dftext_blob_persp")
			skip("gltestthreading gm _ dftext")
			skip("gltestthreading gm _ gpu_blur_utils")
			skip("gltestthreading gm _ gpu_blur_utils_ref")
			skip("gltestthreading gm _ gpu_blur_utils_subset_rect")
			skip("gltestthreading gm _ gpu_blur_utils_subset_rect_ref")
			// skbug.com/7523 - Flaky on various GPUs
			skip("gltestthreading gm _ orientation")
			// These GMs only differ in the low bits
			skip("gltestthreading gm _ stroketext")
			skip("gltestthreading gm _ draw_image_set")
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
			skip("_", "gm", "_", "runtime_intrinsics_matrix")
			configs = []string{"dawn"}
		}

		// Graphite bot *only* runs the grmtl config
		if b.extraConfig("Graphite") {
			if b.extraConfig("ASAN") {
			    // skbug.com/12507 (Neon UB during JPEG compression on M1 ASAN Graphite bot)
				skip("_", "gm", "_", "yuv420_odd_dim")  // Oddly enough yuv420_odd_dim_repeat doesn't crash
				skip("_", "gm", "_", "encode-alpha-jpeg")
				skip("_", "gm", "_", "encode")
				skip("_", "gm", "_", "jpg-color-cube")
				// TODO: re-enable - currently fails with "Failed to make lazy image"
				skip("_", "gm", "_", "image_subset")
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
				skip("_ test _ ProcessorCloneTest")
			}
		}

		if b.model("AndroidOne", "Nexus5", "Nexus7") {
			// skbug.com/9019
			skip("_ test _ ProcessorCloneTest")
			skip("_ test _ Programs")
			skip("_ test _ ProcessorOptimizationValidationTest")
		}

		if b.model("GalaxyS20") {
			// skbug.com/10595
			skip("_ test _ ProcessorCloneTest")
		}

		if b.extraConfig("CommandBuffer") && b.model("MacBook10.1") {
			// skbug.com/9235
			skip("_ test _ Programs")
		}

		if b.model("Spin513") {
			// skbug.com/11876
			skip("_ test _ Programs")
            // skbug.com/12486
			skip("_ test _ TestMockContext")
			skip("_ test _ TestGpuRenderingContexts")
			skip("_ test _ TestGpuAllContexts")
			skip("_ test _ OverdrawSurface_Gpu")
			skip("_ test _ ReplaceSurfaceBackendTexture")
			skip("_ test _ SurfaceAttachStencil_Gpu")
			skip("_ test _ SurfaceWrappedWithRelease_Gpu")
		}

		if b.extraConfig("CommandBuffer") {
			// skbug.com/10412
			skip("_ test _ GLBackendAllocationTest")
			skip("_ test _ InitialTextureClear")
			// skbug.com/12437
			skip("_ test _ GrDDLImage_MakeSubset")
			skip("_ test _ GrContext_oomed")
		}

		// skbug.com/9043 - these devices render this test incorrectly
		// when opList splitting reduction is enabled
		if b.gpu() && b.extraConfig("Vulkan") && (b.gpu("RadeonR9M470X", "RadeonHD7770")) {
			skip("_", "tests", "_", "VkDrawableImportTest")
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
				skip("vk1010102 image _ _")
			} else {
				configs = append(configs, "gl1010102", "gltestpersistentcache", "gltestglslcache", "gltestprecompile")
				// Decoding transparent images to 1010102 just looks bad
				skip("gl1010102 image _ _")
				// These tests produce slightly different pixels run to run on NV.
				skip("gltestpersistentcache gm _ atlastext")
				skip("gltestpersistentcache gm _ dftext")
				skip("gltestpersistentcache gm _ glyph_pos_h_b")
				skip("gltestpersistentcache gm _ glyph_pos_h_f")
				skip("gltestpersistentcache gm _ glyph_pos_n_f")
				skip("gltestglslcache gm _ atlastext")
				skip("gltestglslcache gm _ dftext")
				skip("gltestglslcache gm _ glyph_pos_h_b")
				skip("gltestglslcache gm _ glyph_pos_h_f")
				skip("gltestglslcache gm _ glyph_pos_n_f")
				skip("gltestprecompile gm _ atlastext")
				skip("gltestprecompile gm _ dftext")
				skip("gltestprecompile gm _ glyph_pos_h_b")
				skip("gltestprecompile gm _ glyph_pos_h_f")
				skip("gltestprecompile gm _ glyph_pos_n_f")
				// Tessellation shaders do not yet participate in the persistent cache.
				skip("gltestpersistentcache gm _ tessellation")
				skip("gltestglslcache gm _ tessellation")
				skip("gltestprecompile gm _ tessellation")
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
			skip("mtltestprecompile gm _ atlastext")
			skip("mtltestprecompile gm _ circular_arcs_hairline")
			skip("mtltestprecompile gm _ dftext")
			skip("mtltestprecompile gm _ fontmgr_bounds")
			skip("mtltestprecompile gm _ fontmgr_bounds_1_-0.25")
			skip("mtltestprecompile gm _ glyph_pos_h_b")
			skip("mtltestprecompile gm _ glyph_pos_h_f")
			skip("mtltestprecompile gm _ glyph_pos_n_f")
			skip("mtltestprecompile gm _ persp_images")
			skip("mtltestprecompile gm _ ovals")
			skip("mtltestprecompile gm _ roundrects")
			skip("mtltestprecompile gm _ shadow_utils_occl")
			skip("mtltestprecompile gm _ strokedlines")
			skip("mtltestprecompile gm _ strokerect")
			skip("mtltestprecompile gm _ strokes3")
			skip("mtltestprecompile gm _ texel_subset_linear_mipmap_nearest_down")
			skip("mtltestprecompile gm _ texel_subset_linear_mipmap_linear_down")
			skip("mtltestprecompile gm _ textblobmixedsizes_df")
			skip("mtltestprecompile gm _ yuv420_odd_dim_repeat")
			skip("mtltestprecompile svg _ A_large_blank_world_map_with_oceans_marked_in_blue.svg")
			skip("mtltestprecompile svg _ Chalkboard.svg")
			skip("mtltestprecompile svg _ Ghostscript_Tiger.svg")
			skip("mtltestprecompile svg _ Seal_of_American_Samoa.svg")
			skip("mtltestprecompile svg _ Seal_of_Illinois.svg")
			skip("mtltestprecompile svg _ desk_motionmark_paths.svg")
			skip("mtltestprecompile svg _ rg1024_green_grapes.svg")
			skip("mtltestprecompile svg _ shapes-intro-02-f.svg")
			skip("mtltestprecompile svg _ tiger-8.svg")
		}
		// Test reduced shader mode on iPhone 11 as representative iOS device
		if b.model("iPhone11") && b.extraConfig("Metal") {
			configs = append(configs, "mtlreducedshaders")
		}

		if b.gpu("AppleM1") && !b.extraConfig("Metal") {
			skip("_ test _ TransferPixelsFromTextureTest") // skia:11814
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
		split := strings.Split(tf, "_")
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
		skip("_ svg _ svgparse_")
	} else if b.Name == "Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN" {
		// Only run the CPU SVGs on 8888.
		skip("~8888 svg _ _")
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

	if b.matchExtraConfig("DDL", "PDF", "Graphite") {
		// The DDL, PDF and Graphite bots just render the large skps and the gms
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
	}

	// TODO: ???
	skip("f16 _ _ dstreadshuffle")
	skip("srgb-gl image _ _")
	skip("srgb-gles image _ _")

	// --src image --config g8 means "decode into Gray8", which isn't supported.
	skip("g8 image _ _")
	skip("g8 colorImage _ _")

	if b.extraConfig("Valgrind") {
		// These take 18+ hours to run.
		skip("pdf gm _ fontmgr_iter")
		skip("pdf _ _ PANO_20121023_214540.jpg")
		skip("pdf skp _ worldjournal")
		skip("pdf skp _ desk_baidu.skp")
		skip("pdf skp _ desk_wikipedia.skp")
		skip("_ svg _ _")
		// skbug.com/9171 and 8847
		skip("_ test _ InitialTextureClear")
	}

	if b.model("Pixel3") {
		// skbug.com/10546
		skip("vkddl gm _ compressed_textures_nmof")
		skip("vkddl gm _ compressed_textures_npot")
		skip("vkddl gm _ compressed_textures")
	}

	if b.model("TecnoSpark3Pro") {
		// skbug.com/9421
		skip("_ test _ InitialTextureClear")
	}

	if b.os("iOS") {
		skip(glPrefix + " skp _ _")
	}

	if b.matchOs("Mac", "iOS") {
		// CG fails on questionable bmps
		skip("_ image gen_platf rgba32abf.bmp")
		skip("_ image gen_platf rgb24prof.bmp")
		skip("_ image gen_platf rgb24lprof.bmp")
		skip("_ image gen_platf 8bpp-pixeldata-cropped.bmp")
		skip("_ image gen_platf 4bpp-pixeldata-cropped.bmp")
		skip("_ image gen_platf 32bpp-pixeldata-cropped.bmp")
		skip("_ image gen_platf 24bpp-pixeldata-cropped.bmp")

		// CG has unpredictable behavior on this questionable gif
		// It's probably using uninitialized memory
		skip("_ image gen_platf frame_larger_than_image.gif")

		// CG has unpredictable behavior on incomplete pngs
		// skbug.com/5774
		skip("_ image gen_platf inc0.png")
		skip("_ image gen_platf inc1.png")
		skip("_ image gen_platf inc2.png")
		skip("_ image gen_platf inc3.png")
		skip("_ image gen_platf inc4.png")
		skip("_ image gen_platf inc5.png")
		skip("_ image gen_platf inc6.png")
		skip("_ image gen_platf inc7.png")
		skip("_ image gen_platf inc8.png")
		skip("_ image gen_platf inc9.png")
		skip("_ image gen_platf inc10.png")
		skip("_ image gen_platf inc11.png")
		skip("_ image gen_platf inc12.png")
		skip("_ image gen_platf inc13.png")
		skip("_ image gen_platf inc14.png")
		skip("_ image gen_platf incInterlaced.png")

		// These images fail after Mac 10.13.1 upgrade.
		skip("_ image gen_platf incInterlaced.gif")
		skip("_ image gen_platf inc1.gif")
		skip("_ image gen_platf inc0.gif")
		skip("_ image gen_platf butterfly.gif")
	}

	// WIC fails on questionable bmps
	if b.matchOs("Win") {
		skip("_ image gen_platf pal8os2v2.bmp")
		skip("_ image gen_platf pal8os2v2-16.bmp")
		skip("_ image gen_platf rgba32abf.bmp")
		skip("_ image gen_platf rgb24prof.bmp")
		skip("_ image gen_platf rgb24lprof.bmp")
		skip("_ image gen_platf 8bpp-pixeldata-cropped.bmp")
		skip("_ image gen_platf 4bpp-pixeldata-cropped.bmp")
		skip("_ image gen_platf 32bpp-pixeldata-cropped.bmp")
		skip("_ image gen_platf 24bpp-pixeldata-cropped.bmp")
		if b.arch("x86_64") && b.cpu() {
			// This GM triggers a SkSmallAllocator assert.
			skip("_ gm _ composeshader_bitmap")
		}
	}

	if b.matchOs("Win", "Mac") {
		// WIC and CG fail on arithmetic jpegs
		skip("_ image gen_platf testimgari.jpg")
		// More questionable bmps that fail on Mac, too. skbug.com/6984
		skip("_ image gen_platf rle8-height-negative.bmp")
		skip("_ image gen_platf rle4-height-negative.bmp")
	}

	// These PNGs have CRC errors. The platform generators seem to draw
	// uninitialized memory without reporting an error, so skip them to
	// avoid lots of images on Gold.
	skip("_ image gen_platf error")

	if b.os("Android", "iOS") {
		// This test crashes the N9 (perhaps because of large malloc/frees). It also
		// is fairly slow and not platform-specific. So we just disable it on all of
		// Android and iOS. skia:5438
		skip("_ test _ GrStyledShape")
	}

	if internalHardwareLabel == "5" {
		// http://b/118312149#comment9
		skip("_ test _ SRGBReadWritePixels")
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
		skip("serialize-8888", "gm", "_", test)
	}

	// It looks like we skip these only for out-of-memory concerns.
	if b.matchOs("Win", "Android") {
		for _, test := range []string{"verylargebitmap", "verylarge_picture_image"} {
			skip("serialize-8888", "gm", "_", test)
		}
	}
	if b.matchOs("Mac") && b.cpu() {
		// skia:6992
		skip("pic-8888", "gm", "_", "encode-platform")
		skip("serialize-8888", "gm", "_", "encode-platform")
	}

	// skia:4769
	skip("pic-8888", "gm", "_", "drawfilter")

	// skia:4703
	for _, test := range []string{"image-cacherator-from-picture",
		"image-cacherator-from-raster",
		"image-cacherator-from-ctable"} {
		skip("pic-8888", "gm", "_", test)
		skip("serialize-8888", "gm", "_", test)
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
		skip("pic-8888", "gm", "_", test)
		skip("serialize-8888", "gm", "_", test)

		// GM requires canvas->makeSurface() to return a valid surface.
		// TODO(borenet): These should be just outside of this block but are
		// left here to match the recipe which has an indentation bug.
		skip("pic-8888", "gm", "_", "blurrect_compare")
		skip("serialize-8888", "gm", "_", "blurrect_compare")
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
		skip("_ image _ interlaced1.png")
		skip("_ image _ interlaced2.png")
		skip("_ image _ interlaced3.png")
		for _, rawExt := range r {
			skip(fmt.Sprintf("_ image _ .%s", rawExt))
		}
	}

	// Skip memory intensive tests on 32-bit bots.
	if b.os("Win8") && b.arch("x86") {
		skip("_ image f16 _")
		skip("_ image _ abnormal.wbmp")
		skip("_ image _ interlaced1.png")
		skip("_ image _ interlaced2.png")
		skip("_ image _ interlaced3.png")
		for _, rawExt := range r {
			skip(fmt.Sprintf("_ image _ .%s", rawExt))
		}
	}

	if b.model("Nexus5", "Nexus5x") && b.gpu() {
		// skia:5876
		skip("_", "gm", "_", "encode-platform")
	}

	if b.model("AndroidOne") && b.gpu() { // skia:4697, skia:4704, skia:4694, skia:4705, skia:11133
		skip("_", "gm", "_", "bigblurs")
		skip("_", "gm", "_", "strict_constraint_no_red_allowed")
		skip("_", "gm", "_", "fast_constraint_red_is_allowed")
		skip("_", "gm", "_", "dropshadowimagefilter")
		skip("_", "gm", "_", "filterfastbounds")
		skip(glPrefix, "gm", "_", "imageblurtiled")
		skip("_", "gm", "_", "imagefiltersclipped")
		skip("_", "gm", "_", "imagefiltersscaled")
		skip("_", "gm", "_", "imageresizetiled")
		skip("_", "gm", "_", "matrixconvolution")
		skip("_", "gm", "_", "strokedlines")
		skip("_", "gm", "_", "runtime_intrinsics_matrix")
		if sampleCount > 0 {
			glMsaaConfig := fmt.Sprintf("%smsaa%d", glPrefix, sampleCount)
			skip(glMsaaConfig, "gm", "_", "imageblurtiled")
			skip(glMsaaConfig, "gm", "_", "imagefiltersbase")
		}
	}

	if b.matchGpu("Adreno[3456]") { // disable broken tests on Adreno 3/4/5/6xx
		skip("_", "tests", "_", "SkSLArrayCast_GPU")       // skia:12332
		skip("_", "tests", "_", "SkSLArrayComparison_GPU") // skia:12332
	}

	if b.matchGpu("Adreno[345]") && !b.extraConfig("Vulkan") { // disable broken tests on Adreno 3/4/5xx GLSL
		skip("_", "tests", "_", "DSLFPTest_SwitchStatement")  // skia:11891
		skip("_", "tests", "_", "SkSLMatrixToVectorCast_GPU") // skia:12192
		skip("_", "tests", "_", "SkSLStructsInFunctions_GPU") // skia:11929
	}

	if b.matchGpu("Adreno6") && !b.extraConfig("Vulkan") { // disable broken tests on Adreno 6xx GLSL
		skip("_", "tests", "_", "SkSLIntrinsicIsInf_GPU") // skia:12377
	}

	if (b.matchGpu("Adreno3") || b.matchGpu("Mali400")) && !b.extraConfig("Vulkan") {
		skip("_", "tests", "_", "SkSLMatrices") // skia:12456
	}

	if b.gpu("IntelIris6100", "IntelHD4400") && b.matchOs("Win") && !b.extraConfig("Vulkan") {
		skip("_", "tests", "_", "SkSLVectorToMatrixCast_GPU") // skia:12179
	}

	if ((b.extraConfig("Vulkan") && b.isLinux() && b.matchGpu("Intel")) ||
        (b.extraConfig("ANGLE") && b.matchOs("Win") && b.matchGpu("IntelIris(540|655)"))) {
		skip("_", "tests", "_", "SkSLSwitchDefaultOnly_GPU") // skia:12465
	}

	if (b.gpu("Tegra3")) { // Tegra3 fails to compile break stmts inside a for loop (skia:12477)
		skip("_", "tests", "_", "SkSLSwitch_GPU")
		skip("_", "tests", "_", "SkSLSwitchDefaultOnly_GPU")
		skip("_", "tests", "_", "SkSLSwitchWithFallthrough_GPU")
		skip("_", "tests", "_", "SkSLSwitchWithLoops_GPU")
	}

	if b.matchGpu("Intel") { // some Intel GPUs don't return zero for the derivative of a uniform
		skip("_", "tests", "_", "SkSLIntrinsicDFdy_GPU")
		skip("_", "tests", "_", "SkSLIntrinsicDFdx_GPU")
		skip("_", "tests", "_", "SkSLIntrinsicFwidth_GPU")
	}

	if b.matchOs("Mac") && b.matchGpu("Intel(Iris5100|HD6000)") {
		skip("_", "tests", "_", "SkSLLoopFloat_GPU") // skia:12426
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

	if b.model("TecnoSpark3Pro") {
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
		skip("vk", "gm", "_", "skbug_257")
		skip("vk", "gm", "_", "filltypespersp")
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
		skip("angle_d3d9_es2", "gm", "_", "lighting")
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
	if b.model("Nexus5", "Nexus5x", "Nexus9") {
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

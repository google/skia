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
	blacklist := []string{"role", "test_filter"}
	keys := make([]string, 0, len(parts))
	for key := range parts {
		found := false
		for _, b := range blacklist {
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
func dmFlags(bot string, parts map[string]string, doUpload bool, internalHardwareLabel string) ([]string, map[string]string) {
	properties := map[string]string{
		"gitHash":              specs.PLACEHOLDER_REVISION,
		"builder":              bot, //specs.PLACEHOLDER_TASK_NAME,
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
	blacklisted := []string{}

	has := func(keyword string) bool {
		return strings.Contains(bot, keyword)
	}
	hasConfig := func(cfg string) bool {
		for _, c := range configs {
			if c == cfg {
				return true
			}
		}
		return false
	}
	hasExtraConfig := func(ec string) bool {
		for _, e := range strings.Split(parts["extra_config"], "_") {
			if e == ec {
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
	prefix := func(slice []string, pfx string) []string {
		rv := make([]string, 0, len(slice))
		for _, e := range slice {
			rv = append(rv, pfx+e)
		}
		return rv
	}

	blacklist := func(quad ...string) {
		if len(quad) == 1 {
			quad = strings.Fields(quad[0])
		}
		if len(quad) != 4 {
			log.Fatalf("Invalid value for blacklist: %+v", quad)
		}
		config := quad[0]
		src := quad[1]
		options := quad[2]
		name := quad[3]
		if config == "_" ||
			hasConfig(config) ||
			(config[0] == '~' && hasConfig(config[1:])) {
			blacklisted = append(blacklisted, config, src, options, name)
		}
	}

	isLinux := has("Debian") || has("Ubuntu") || has("Housekeeper")

	// Keys.
	keys := keyParams(parts)
	if has("Lottie") {
		keys = append(keys, "renderer", "skottie")
	}
	if strings.Contains(parts["extra_config"], "DDL") {
		// 'DDL' style means "--skpViewportSize 2048 --pr ~small"
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
	if !has("Android") && !has("MSAN") {
		args = append(args, "--randomProcessorTest")
	}

	if has("Pixel3") && has("Vulkan") {
		args = append(args, "--dontReduceOpsTaskSplitting")
	}

	threadLimit := -1
	const MAIN_THREAD_ONLY = 0

	// 32-bit desktop bots tend to run out of memory, because they have relatively
	// far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
	if has("-x86-") {
		threadLimit = 4
	}

	// These bots run out of memory easily.
	if has("MotoG4") || has("Nexus7") {
		threadLimit = MAIN_THREAD_ONLY
	}

	// Avoid issues with dynamically exceeding resource cache limits.
	if has("Test") && has("DISCARDABLE") {
		threadLimit = MAIN_THREAD_ONLY
	}

	if threadLimit >= 0 {
		args = append(args, "--threads", strconv.Itoa(threadLimit))
	}

	sampleCount := 0
	glPrefix := ""
	if has("SwiftShader") {
		configs = append(configs, "gles", "glesdft")
		args = append(args, "--disableDriverCorrectnessWorkarounds")
	} else if parts["cpu_or_gpu"] == "CPU" {
		args = append(args, "--nogpu")

		configs = append(configs, "8888")

		if has("BonusConfigs") {
			configs = []string{
				"g8", "565",
				"pic-8888", "serialize-8888",
				"f16", "srgb", "esrgb", "narrow", "enarrow",
				"p3", "ep3", "rec2020", "erec2020"}
		}

		if has("PDF") {
			configs = []string{"pdf"}
			args = append(args, "--rasterize_pdf") // Works only on Mac.
			// Take ~forever to rasterize:
			blacklist("pdf gm _ lattice2")
			blacklist("pdf gm _ hairmodes")
			blacklist("pdf gm _ longpathdash")
		}

	} else if parts["cpu_or_gpu"] == "GPU" {
		args = append(args, "--nocpu")

		// Add in either gles or gl configs to the canonical set based on OS
		sampleCount = 8
		glPrefix = "gl"
		if has("Android") || has("iOS") {
			sampleCount = 4
			// We want to test the OpenGL config not the GLES config on the Shield
			if !has("NVIDIA_Shield") {
				glPrefix = "gles"
			}
			// MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
			if has("Pixel3a") {
				sampleCount = 0
			}
		} else if has("Intel") {
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			sampleCount = 0
		} else if has("ChromeOS") {
			glPrefix = "gles"
		}

		if has("NativeFonts") {
			configs = append(configs, glPrefix)
		} else {
			configs = append(configs, glPrefix, glPrefix+"dft", glPrefix+"srgb")
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("%smsaa%d", glPrefix, sampleCount))
			}
		}

		// The Tegra3 doesn't support MSAA
		if has("Tegra3") ||
			// We aren't interested in fixing msaa bugs on current iOS devices.
			has("iPad4") ||
			has("iPadPro") ||
			has("iPhone6") ||
			has("iPhone7") ||
			// skia:5792
			has("IntelHD530") ||
			has("IntelIris540") {
			configs = removeContains(configs, "msaa")
		}

		// We want to test both the OpenGL config and the GLES config on Linux Intel:
		// GL is used by Chrome, GLES is used by ChromeOS.
		// Also do the Ganesh threading verification test (render with and without
		// worker threads, using only the SW path renderer, and compare the results).
		if has("Intel") && isLinux {
			configs = append(configs, "gles", "glesdft", "glessrgb", "gltestthreading")
			// skbug.com/6333, skbug.com/6419, skbug.com/6702
			blacklist("gltestthreading gm _ lcdblendmodes")
			blacklist("gltestthreading gm _ lcdoverlap")
			blacklist("gltestthreading gm _ textbloblooper")
			// All of these GMs are flaky, too:
			blacklist("gltestthreading gm _ bleed_alpha_bmp")
			blacklist("gltestthreading gm _ bleed_alpha_bmp_shader")
			blacklist("gltestthreading gm _ bleed_alpha_image")
			blacklist("gltestthreading gm _ bleed_alpha_image_shader")
			blacklist("gltestthreading gm _ savelayer_with_backdrop")
			blacklist("gltestthreading gm _ persp_shaders_bw")
			blacklist("gltestthreading gm _ dftext_blob_persp")
			blacklist("gltestthreading gm _ dftext")
			// skbug.com/7523 - Flaky on various GPUs
			blacklist("gltestthreading gm _ orientation")
			// These GMs only differ in the low bits
			blacklist("gltestthreading gm _ stroketext")
			blacklist("gltestthreading gm _ draw_image_set")
		}

		// CommandBuffer bot *only* runs the command_buffer config.
		if has("CommandBuffer") {
			configs = []string{"commandbuffer"}
		}

		// ANGLE bot *only* runs the angle configs
		if has("ANGLE") {
			configs = []string{"angle_d3d11_es2",
				"angle_d3d9_es2",
				"angle_gl_es2",
				"angle_d3d11_es3"}
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("angle_d3d11_es2_msaa%d", sampleCount))
				configs = append(configs, fmt.Sprintf("angle_d3d11_es3_msaa%d", sampleCount))
			}
			if has("LenovoYogaC630") {
				// LenovoYogaC630 only supports D3D11, and to save time, we only test ES3
				configs = []string{
					"angle_d3d11_es3",
					fmt.Sprintf("angle_d3d11_es3_msaa%d", sampleCount),
				}
			}
			if has("GTX") || has("Quadro") {
				// See skia:7823 and chromium:693090.
				configs = append(configs, "angle_gl_es3")
				if sampleCount > 0 {
					configs = append(configs, fmt.Sprintf("angle_gl_es2_msaa%d", sampleCount))
					configs = append(configs, fmt.Sprintf("angle_gl_es3_msaa%d", sampleCount))
				}
			}
			if has("NUC5i7RYH") {
				// skbug.com/7376
				blacklist("_ test _ ProcessorCloneTest")
			}
		}

		if has("AndroidOne") || (has("Nexus") && !has("Nexus5x")) || has("GalaxyS6") {
			// skbug.com/9019
			blacklist("_ test _ ProcessorCloneTest")
			blacklist("_ test _ Programs")
			blacklist("_ test _ ProcessorOptimizationValidationTest")
		}

		if has("CommandBuffer") && has("MacBook10.1-") {
			// skbug.com/9235
			blacklist("_ test _ Programs")
		}

		// skbug.com/9033 - these devices run out of memory on this test
		// when opList splitting reduction is enabled
		if has("GPU") && (has("Nexus7") ||
			has("NVIDIA_Shield") ||
			has("Nexus5x") ||
			(has("Win10") && has("GTX660") && has("Vulkan"))) {
			blacklist("_", "gm", "_", "savelayer_clipmask")
		}

		// skbug.com/9123
		if has("CommandBuffer") && has("IntelIris5100") {
			blacklist("_", "test", "_", "AsyncReadPixels")
		}

		// skbug.com/9043 - these devices render this test incorrectly
		// when opList splitting reduction is enabled
		if has("GPU") && has("Vulkan") && (has("RadeonR9M470X") || has("RadeonHD7770")) {
			blacklist("_", "tests", "_", "VkDrawableImportTest")
		}
		if has("Vulkan") {
			configs = []string{"vk"}
			if has("Android") {
				configs = append(configs, "vkmsaa4")
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
				// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
				if !has("Intel") {
					configs = append(configs, "mtlmsaa8")
				}
			}
		}

		// Test 1010102 on our Linux/NVIDIA bots and the persistent cache config
		// on the GL bots.
		if has("QuadroP400") && !has("PreAbandonGpuContext") && !has("TSAN") && isLinux {
			if has("Vulkan") {
				configs = append(configs, "vk1010102")
				// Decoding transparent images to 1010102 just looks bad
				blacklist("vk1010102 image _ _")
			} else {
				configs = append(configs, "gl1010102", "gltestpersistentcache", "gltestglslcache", "gltestprecompile")
				// Decoding transparent images to 1010102 just looks bad
				blacklist("gl1010102 image _ _")
				// These tests produce slightly different pixels run to run on NV.
				blacklist("gltestpersistentcache gm _ atlastext")
				blacklist("gltestpersistentcache gm _ dftext")
				blacklist("gltestpersistentcache gm _ glyph_pos_h_b")
				blacklist("gltestglslcache gm _ atlastext")
				blacklist("gltestglslcache gm _ dftext")
				blacklist("gltestglslcache gm _ glyph_pos_h_b")
				blacklist("gltestprecompile gm _ atlastext")
				blacklist("gltestprecompile gm _ dftext")
				blacklist("gltestprecompile gm _ glyph_pos_h_b")
				// Tessellation shaders do not yet participate in the persistent cache.
				blacklist("gltestpersistentcache gm _ tessellation")
				blacklist("gltestglslcache gm _ tessellation")
				blacklist("gltestprecompile gm _ tessellation")
			}
		}

		// We also test the SkSL precompile config on Pixel2XL as a representative
		// Android device - this feature is primarily used by Flutter.
		if has("Pixel2XL") && !has("Vulkan") {
			configs = append(configs, "glestestprecompile")
		}

		// Test rendering to wrapped dsts on a few bots
		// Also test "glenarrow", which hits F16 surfaces and F16 vertex colors.
		if has("BonusConfigs") {
			configs = []string{"glbetex", "glbert", "glenarrow"}
		}

		if has("ChromeOS") {
			// Just run GLES for now - maybe add gles_msaa4 in the future
			configs = []string{"gles"}
		}

		// Test coverage counting path renderer.
		if has("CCPR") {
			configs = filter(configs, "gl", "gles")
			args = append(args, "--pr", "ccpr", "--cc", "true", "--cachePathMasks", "false")
		}

		// Test GPU tessellation path renderer.
		if has("GpuTess") {
			configs = []string{glPrefix + "msaa4"}
			args = append(args, "--pr", "gtess")
		}

		// Test non-nvpr on NVIDIA.
		if has("NonNVPR") {
			configs = []string{"gl", "glmsaa4"}
			args = append(args, "--pr", "~nvpr")
		}

		// DDL is a GPU-only feature
		if has("DDL1") {
			// This bot generates gl and vk comparison images for the large skps
			configs = filter(configs, "gl", "vk", "mtl")
			args = append(args, "--skpViewportSize", "2048")
			args = append(args, "--pr", "~small")
		}
		if has("DDL3") {
			// This bot generates the ddl-gl and ddl-vk images for the
			// large skps and the gms
			ddlConfigs := prefix(filter(configs, "gl", "vk", "mtl"), "ddl-")
			ddl2Configs := prefix(filter(configs, "gl", "vk", "mtl"), "ddl2-")
			configs = append(ddlConfigs, ddl2Configs...)
			args = append(args, "--skpViewportSize", "2048")
			args = append(args, "--gpuThreads", "0")
		}
	}

	// Sharding.
	tf := parts["test_filter"]
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
	if has("GPU") {
		// Don't run the "svgparse_*" svgs on GPU.
		blacklist("_ svg _ svgparse_")
	} else if bot == "Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN" {
		// Only run the CPU SVGs on 8888.
		blacklist("~8888 svg _ _")
	} else {
		// On CPU SVGs we only care about parsing. Only run them on the above bot.
		removeFromArgs("svg")
	}

	// Eventually I'd like these to pass, but for now just skip 'em.
	if has("SK_FORCE_RASTER_PIPELINE_BLITTER") {
		removeFromArgs("tests")
	}

	if has("NativeFonts") { // images won't exercise native font integration :)
		removeFromArgs("image")
		removeFromArgs("colorImage")
	}

	if has("DDL") || has("PDF") {
		// The DDL and PDF bots just render the large skps and the gms
		removeFromArgs("tests")
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	} else {
		// No other bots render the .skps.
		removeFromArgs("skp")
	}

	if has("Lottie") {
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

	// TODO: ???
	blacklist("f16 _ _ dstreadshuffle")
	blacklist("glsrgb image _ _")
	blacklist("glessrgb image _ _")

	// --src image --config g8 means "decode into Gray8", which isn't supported.
	blacklist("g8 image _ _")
	blacklist("g8 colorImage _ _")

	if has("Valgrind") {
		// These take 18+ hours to run.
		blacklist("pdf gm _ fontmgr_iter")
		blacklist("pdf _ _ PANO_20121023_214540.jpg")
		blacklist("pdf skp _ worldjournal")
		blacklist("pdf skp _ desk_baidu.skp")
		blacklist("pdf skp _ desk_wikipedia.skp")
		blacklist("_ svg _ _")
		// skbug.com/9171 and 8847
		blacklist("_ test _ InitialTextureClear")
	}

	if has("TecnoSpark3Pro") {
		// skbug.com/9421
		blacklist("_ test _ InitialTextureClear")
	}

	if has("iOS") {
		blacklist(glPrefix + " skp _ _")
	}

	if has("Mac") || has("iOS") {
		// CG fails on questionable bmps
		blacklist("_ image gen_platf rgba32abf.bmp")
		blacklist("_ image gen_platf rgb24prof.bmp")
		blacklist("_ image gen_platf rgb24lprof.bmp")
		blacklist("_ image gen_platf 8bpp-pixeldata-cropped.bmp")
		blacklist("_ image gen_platf 4bpp-pixeldata-cropped.bmp")
		blacklist("_ image gen_platf 32bpp-pixeldata-cropped.bmp")
		blacklist("_ image gen_platf 24bpp-pixeldata-cropped.bmp")

		// CG has unpredictable behavior on this questionable gif
		// It's probably using uninitialized memory
		blacklist("_ image gen_platf frame_larger_than_image.gif")

		// CG has unpredictable behavior on incomplete pngs
		// skbug.com/5774
		blacklist("_ image gen_platf inc0.png")
		blacklist("_ image gen_platf inc1.png")
		blacklist("_ image gen_platf inc2.png")
		blacklist("_ image gen_platf inc3.png")
		blacklist("_ image gen_platf inc4.png")
		blacklist("_ image gen_platf inc5.png")
		blacklist("_ image gen_platf inc6.png")
		blacklist("_ image gen_platf inc7.png")
		blacklist("_ image gen_platf inc8.png")
		blacklist("_ image gen_platf inc9.png")
		blacklist("_ image gen_platf inc10.png")
		blacklist("_ image gen_platf inc11.png")
		blacklist("_ image gen_platf inc12.png")
		blacklist("_ image gen_platf inc13.png")
		blacklist("_ image gen_platf inc14.png")
		blacklist("_ image gen_platf incInterlaced.png")

		// These images fail after Mac 10.13.1 upgrade.
		blacklist("_ image gen_platf incInterlaced.gif")
		blacklist("_ image gen_platf inc1.gif")
		blacklist("_ image gen_platf inc0.gif")
		blacklist("_ image gen_platf butterfly.gif")
	}

	// WIC fails on questionable bmps
	if has("Win") {
		blacklist("_ image gen_platf pal8os2v2.bmp")
		blacklist("_ image gen_platf pal8os2v2-16.bmp")
		blacklist("_ image gen_platf rgba32abf.bmp")
		blacklist("_ image gen_platf rgb24prof.bmp")
		blacklist("_ image gen_platf rgb24lprof.bmp")
		blacklist("_ image gen_platf 8bpp-pixeldata-cropped.bmp")
		blacklist("_ image gen_platf 4bpp-pixeldata-cropped.bmp")
		blacklist("_ image gen_platf 32bpp-pixeldata-cropped.bmp")
		blacklist("_ image gen_platf 24bpp-pixeldata-cropped.bmp")
		if has("x86_64") && has("CPU") {
			// This GM triggers a SkSmallAllocator assert.
			blacklist("_ gm _ composeshader_bitmap")
		}
	}

	if has("Win") || has("Mac") {
		// WIC and CG fail on arithmetic jpegs
		blacklist("_ image gen_platf testimgari.jpg")
		// More questionable bmps that fail on Mac, too. skbug.com/6984
		blacklist("_ image gen_platf rle8-height-negative.bmp")
		blacklist("_ image gen_platf rle4-height-negative.bmp")
	}

	// These PNGs have CRC errors. The platform generators seem to draw
	// uninitialized memory without reporting an error, so skip them to
	// avoid lots of images on Gold.
	blacklist("_ image gen_platf error")

	if has("Android") || has("iOS") {
		// This test crashes the N9 (perhaps because of large malloc/frees). It also
		// is fairly slow and not platform-specific. So we just disable it on all of
		// Android and iOS. skia:5438
		blacklist("_ test _ GrShape")
	}

	if internalHardwareLabel == "5" {
		// http://b/118312149#comment9
		blacklist("_ test _ SRGBReadWritePixels")
	}

	// skia:4095
	badSerializeGMs := []string{
		"bleed_image",
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
		"bleed",
		"bleed_alpha_bmp",
		"bleed_alpha_bmp_shader",
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
	badSerializeGMs = append(badSerializeGMs, "compositor_quads_shader")
	badSerializeGMs = append(badSerializeGMs, "wacky_yuv_formats_qtr")

	// This GM forces a path to be convex. That property doesn't survive
	// serialization.
	badSerializeGMs = append(badSerializeGMs, "analytic_antialias_convex")

	for _, test := range badSerializeGMs {
		blacklist("serialize-8888", "gm", "_", test)
	}

	if !has("Mac") {
		for _, test := range []string{"bleed_alpha_image", "bleed_alpha_image_shader"} {
			blacklist("serialize-8888", "gm", "_", test)
		}
	}
	// It looks like we skip these only for out-of-memory concerns.
	if has("Win") || has("Android") {
		for _, test := range []string{"verylargebitmap", "verylarge_picture_image"} {
			blacklist("serialize-8888", "gm", "_", test)
		}
	}
	if has("Mac") && has("CPU") {
		// skia:6992
		blacklist("pic-8888", "gm", "_", "encode-platform")
		blacklist("serialize-8888", "gm", "_", "encode-platform")
	}

	// skia:4769
	blacklist("pic-8888", "gm", "_", "drawfilter")

	// skia:4703
	for _, test := range []string{"image-cacherator-from-picture",
		"image-cacherator-from-raster",
		"image-cacherator-from-ctable"} {
		blacklist("pic-8888", "gm", "_", test)
		blacklist("serialize-8888", "gm", "_", test)
	}

	// GM that requires raster-backed canvas
	for _, test := range []string{"complexclip4_bw", "complexclip4_aa", "p3",
		"async_rescale_and_read_text_up_large",
		"async_rescale_and_read_text_up",
		"async_rescale_and_read_text_down",
		"async_rescale_and_read_dog_up",
		"async_rescale_and_read_dog_down",
		"async_rescale_and_read_rose",
		"async_rescale_and_read_no_bleed"} {
		blacklist("pic-8888", "gm", "_", test)
		blacklist("serialize-8888", "gm", "_", test)

		// GM requires canvas->makeSurface() to return a valid surface.
		// TODO(borenet): These should be just outside of this block but are
		// left here to match the recipe which has an indentation bug.
		blacklist("pic-8888", "gm", "_", "blurrect_compare")
		blacklist("serialize-8888", "gm", "_", "blurrect_compare")
	}

	// Extensions for RAW images
	r := []string{
		"arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
		"ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
	}

	// skbug.com/4888
	// Blacklist RAW images (and a few large PNGs) on GPU bots
	// until we can resolve failures.
	if has("GPU") {
		blacklist("_ image _ interlaced1.png")
		blacklist("_ image _ interlaced2.png")
		blacklist("_ image _ interlaced3.png")
		for _, rawExt := range r {
			blacklist(fmt.Sprintf("_ image _ .%s", rawExt))
		}
	}

	// Blacklist memory intensive tests on 32-bit bots.
	if has("Win8") && has("x86-") {
		blacklist("_ image f16 _")
		blacklist("_ image _ abnormal.wbmp")
		blacklist("_ image _ interlaced1.png")
		blacklist("_ image _ interlaced2.png")
		blacklist("_ image _ interlaced3.png")
		for _, rawExt := range r {
			blacklist(fmt.Sprintf("_ image _ .%s", rawExt))
		}
	}

	if has("Nexus5") && has("GPU") {
		// skia:5876
		blacklist("_", "gm", "_", "encode-platform")
	}

	if has("AndroidOne-GPU") { // skia:4697, skia:4704, skia:4694, skia:4705
		blacklist("_", "gm", "_", "bigblurs")
		blacklist("_", "gm", "_", "bleed")
		blacklist("_", "gm", "_", "bleed_alpha_bmp")
		blacklist("_", "gm", "_", "bleed_alpha_bmp_shader")
		blacklist("_", "gm", "_", "bleed_alpha_image")
		blacklist("_", "gm", "_", "bleed_alpha_image_shader")
		blacklist("_", "gm", "_", "bleed_image")
		blacklist("_", "gm", "_", "dropshadowimagefilter")
		blacklist("_", "gm", "_", "filterfastbounds")
		blacklist(glPrefix, "gm", "_", "imageblurtiled")
		blacklist("_", "gm", "_", "imagefiltersclipped")
		blacklist("_", "gm", "_", "imagefiltersscaled")
		blacklist("_", "gm", "_", "imageresizetiled")
		blacklist("_", "gm", "_", "matrixconvolution")
		blacklist("_", "gm", "_", "strokedlines")
		if sampleCount > 0 {
			glMsaaConfig := fmt.Sprintf("%smsaa%d", glPrefix, sampleCount)
			blacklist(glMsaaConfig, "gm", "_", "imageblurtiled")
			blacklist(glMsaaConfig, "gm", "_", "imagefiltersbase")
		}
	}

	match := []string{}
	if has("Valgrind") { // skia:3021
		match = append(match, "~Threaded")
	}

	if has("Valgrind") && has("PreAbandonGpuContext") {
		// skia:6575
		match = append(match, "~multipicturedraw_")
	}

	if has("AndroidOne") {
		match = append(match, "~WritePixels")                       // skia:4711
		match = append(match, "~PremulAlphaRoundTrip_Gpu")          // skia:7501
		match = append(match, "~ReimportImageTextureWithMipLevels") // skia:8090
	}

	if has("GalaxyS6") {
		match = append(match, "~SpecialImage") // skia:6338
		match = append(match, "~skbug6653")    // skia:6653
	}

	if has("MSAN") {
		match = append(match, "~Once", "~Shared") // Not sure what's up with these tests.
	}

	if has("TSAN") {
		match = append(match, "~ReadWriteAlpha")      // Flaky on TSAN-covered on nvidia bots.
		match = append(match, "~RGBA4444TextureTest", // Flakier than they are important.
			"~RGB565TextureTest")
	}

	// By default, we test with GPU threading enabled, unless specifically
	// disabled.
	if has("NoGPUThreads") {
		args = append(args, "--gpuThreads", "0")
	}

	if has("Vulkan") && has("Adreno530") {
		// skia:5777
		match = append(match, "~CopySurface")
	}

	if has("Vulkan") && has("Adreno") {
		// skia:7663
		match = append(match, "~WritePixelsNonTextureMSAA_Gpu")
		match = append(match, "~WritePixelsMSAA_Gpu")
	}

	if has("Vulkan") && isLinux && has("IntelIris640") {
		match = append(match, "~VkHeapTests") // skia:6245
	}

	if isLinux && has("IntelIris640") {
		match = append(match, "~Programs") // skia:7849
	}

	if has("TecnoSpark3Pro") {
		match = append(match, "~Programs") // skia:9814
	}

	if has("IntelIris640") || has("IntelHD615") || has("IntelHDGraphics615") {
		match = append(match, "~^SRGBReadWritePixels$") // skia:9225
	}

	if has("Vulkan") && isLinux && has("IntelHD405") {
		// skia:7322
		blacklist("vk", "gm", "_", "skbug_257")
		blacklist("vk", "gm", "_", "filltypespersp")
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

	if has("Vulkan") && has("GTX660") && has("Win") {
		// skbug.com/8047
		match = append(match, "~FloatingPointTextureTest$")
	}

	if has("Metal") && has("HD8870M") && has("Mac") {
		// skia:9255
		match = append(match, "~WritePixelsNonTextureMSAA_Gpu")
	}

	if has("ANGLE") {
		// skia:7835
		match = append(match, "~BlurMaskBiggerThanDest")
	}

	if has("IntelIris6100") && has("ANGLE") && has("Release") {
		// skia:7376
		match = append(match, "~^ProcessorOptimizationValidationTest$")
	}

	if (has("IntelIris6100") || has("IntelHD4400")) && has("ANGLE") {
		// skia:6857
		blacklist("angle_d3d9_es2", "gm", "_", "lighting")
	}

	if has("PowerVRGX6250") {
		match = append(match, "~gradients_view_perspective_nodither") //skia:6972
	}

	if has("-arm-") && has("ASAN") {
		// TODO: can we run with env allocator_may_return_null=1 instead?
		match = append(match, "~BadImage")
	}

	if has("Mac") && has("IntelHD6000") {
		// skia:7574
		match = append(match, "~^ProcessorCloneTest$")
		match = append(match, "~^GrMeshTest$")
	}

	if has("Mac") && has("IntelHD615") {
		// skia:7603
		match = append(match, "~^GrMeshTest$")
	}

	if has("LenovoYogaC630") && has("ANGLE") {
		// skia:9275
		blacklist("_", "tests", "_", "Programs")
		// skia:8976
		blacklist("_", "tests", "_", "GrDefaultPathRendererTest")
		// https://bugs.chromium.org/p/angleproject/issues/detail?id=3414
		blacklist("_", "tests", "_", "PinnedImageTest")
	}

	if len(blacklisted) > 0 {
		args = append(args, "--blacklist")
		args = append(args, blacklisted...)
	}

	if len(match) > 0 {
		args = append(args, "--match")
		args = append(args, match...)
	}

	// These bots run out of memory running RAW codec tests. Do not run them in
	// parallel
	if has("Nexus5") || has("Nexus9") {
		args = append(args, "--noRAW_threading")
	}

	if has("FSAA") {
		args = append(args, "--analyticAA", "false")
	}
	if has("FAAA") {
		args = append(args, "--forceAnalyticAA")
	}

	if !has("NativeFonts") {
		args = append(args, "--nonativeFonts")
	}

	if has("GDI") {
		args = append(args, "--gdi")
	}

	// Let's make all bots produce verbose output by default.
	args = append(args, "--verbose")

	// See skia:2789.
	if hasExtraConfig("AbandonGpuContext") {
		args = append(args, "--abandonGpuContext")
	}
	if hasExtraConfig("PreAbandonGpuContext") {
		args = append(args, "--preAbandonGpuContext")
	}
	if hasExtraConfig("ReleaseAndAbandonGpuContext") {
		args = append(args, "--releaseAndAbandonGpuContext")
	}
	return args, properties
}

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
func (b *TaskBuilder) dmFlags(internalHardwareLabel string) {
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
	keys := keyParams(b.Parts)
	if b.ExtraConfig("Lottie") {
		keys = append(keys, "renderer", "skottie")
	}
	if b.MatchExtraConfig("DDL") {
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
	if !b.MatchOs("Android") && !b.ExtraConfig("MSAN") {
		args = append(args, "--randomProcessorTest")
	}

	threadLimit := -1
	const MAIN_THREAD_ONLY = 0

	// 32-bit desktop machines tend to run out of memory, because they have relatively
	// far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
	if b.Arch("x86") {
		threadLimit = 4
	}

	// These devices run out of memory easily.
	if b.Model("MotoG4", "Nexus7") {
		threadLimit = MAIN_THREAD_ONLY
	}

	// Avoid issues with dynamically exceeding resource cache limits.
	if b.MatchExtraConfig("DISCARDABLE") {
		threadLimit = MAIN_THREAD_ONLY
	}

	if threadLimit >= 0 {
		args = append(args, "--threads", strconv.Itoa(threadLimit))
	}

	sampleCount := 0
	glPrefix := ""
	if b.ExtraConfig("SwiftShader") {
		if b.ExtraConfig("Graphite") {
			configs = append(configs, "grvk")
			// b/441484662
			skip(ALL, "test", ALL, "BigImage")
			skip(ALL, "test", ALL, "PaintParamsKey")
			skip(ALL, "test", ALL, "AndroidPrecompile")
			skip(ALL, "test", ALL, "MultisampleRetain")
			skip(ALL, "gm", ALL, "lcd")
		} else {
			configs = append(configs, "vk", "vkdmsaa")
		}
		// skbug.com/40043920
		skip(ALL, "test", ALL, "GrThreadSafeCache16Verts")
		// b/296440036
		skip(ALL, "test", ALL, "ImageAsyncReadPixels")
		// skbug.com/40043921
		skip(ALL, "test", ALL, "image_subset")
	} else if b.CPU() {
		args = append(args, "--nogpu")

		configs = append(configs, "8888")

		if b.ExtraConfig("BonusConfigs") {
			configs = []string{
				"r8", "565",
				"pic-8888", "serialize-8888"}
		}

		if b.ExtraConfig("PDF") {
			configs = []string{"pdf"}
			args = append(args, "--rasterize_pdf") // Works only on Mac.
			// Take ~forever to rasterize:
			skip("pdf", "gm", ALL, "lattice2")
			skip("pdf", "gm", ALL, "hairmodes")
			skip("pdf", "gm", ALL, "longpathdash")
		}

		if b.ExtraConfig("OldestSupportedSkpVersion") {
			// For triaging convenience, make the old-skp job's output match the size of the DDL jobs' output
			args = append(args, "--skpViewportSize", "2048")
		}

	} else if b.GPU() {
		args = append(args, "--nocpu")

		// Add in either gles or gl configs to the canonical set based on OS
		glPrefix = "gl"
		// Use 4x MSAA for all our testing. It's more consistent and 8x MSAA is nondeterministic (by
		// design) on NVIDIA hardware. The problem is especially bad on ANGLE.  skbug.com/40038032 skbug.com/40037753
		sampleCount = 4
		if b.MatchOs("Android") || b.Os("iOS") {
			glPrefix = "gles"
			// MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
			// MSAA is disabled on Pixel5 (https://skbug.com/40042528).
			if b.Model("Pixel3a", "Pixel5") {
				sampleCount = 0
			}

			// Disable failing OpenGLES SkSL test for Pixel 10 devices (b/452352214).
			// Also see b/370739986; could be an issue on all IMG GPUs.
			if b.Model("Pixel10") {
				skip(ALL, "test", ALL, "SkSLIntrinsicModf_Ganesh")
			}
		} else if b.MatchGpu("Intel") {
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			if b.GPU("IntelIrisXe") && b.MatchOs("Win") && b.ExtraConfig("ANGLE") {
				// Make an exception for newer GPUs + D3D
				args = append(args, "--allowMSAAOnNewIntel", "true")
			} else {
				sampleCount = 0
			}
		} else if b.Os("ChromeOS") {
			glPrefix = "gles"
		}

		if b.ExtraConfig("NativeFonts") {
			configs = append(configs, glPrefix)
		} else {
			configs = append(configs, glPrefix, glPrefix+"dft")
			if sampleCount > 0 {
				configs = append(configs, fmt.Sprintf("%smsaa%d", glPrefix, sampleCount))
				// Temporarily limit the machines we test dynamic MSAA on.
				if b.GPU("QuadroP400", "MaliG77") || b.MatchOs("Mac") {
					configs = append(configs, fmt.Sprintf("%sdmsaa", glPrefix))
				}
			}
		}

		if b.ExtraConfig("Protected") {
			args = append(args, "--createProtected")
			// The Protected jobs (for now) only run the unit tests
			skip(ALL, "gm", ALL, ALL)
			skip(ALL, "image", ALL, ALL)
			skip(ALL, "lottie", ALL, ALL)
			skip(ALL, "colorImage", ALL, ALL)
			skip(ALL, "svg", ALL, ALL)
			skip(ALL, "skp", ALL, ALL)

			// These unit tests attempt to readback
			skip(ALL, "test", ALL, "ApplyGamma")
			skip(ALL, "test", ALL, "BigImageTest_Ganesh")
			skip(ALL, "test", ALL, "BigImageTest_Graphite")
			skip(ALL, "test", ALL, "BlendRequiringDstReadWithLargeCoordinates")
			skip(ALL, "test", ALL, "BlurMaskBiggerThanDest")
			skip(ALL, "test", ALL, "ClearOp")
			skip(ALL, "test", ALL, "ColorTypeBackendAllocationTest")
			skip(ALL, "test", ALL, "ComposedImageFilterBounds_Gpu")
			skip(ALL, "test", ALL, "CompressedBackendAllocationTest")
			skip(ALL, "test", ALL, "CopySurface")
			skip(ALL, "test", ALL, "crbug_1271431")
			skip(ALL, "test", ALL, "DashPathEffectTest_2PiInterval")
			skip(ALL, "test", ALL, "DeviceTestVertexTransparency")
			skip(ALL, "test", ALL, "DDLMultipleDDLs")
			skip(ALL, "test", ALL, "DefaultPathRendererTest")
			skip(ALL, "test", ALL, "DMSAA_aa_dst_read_after_dmsaa")
			skip(ALL, "test", ALL, "DMSAA_dst_read")
			skip(ALL, "test", ALL, "DMSAA_dst_read_with_existing_barrier")
			skip(ALL, "test", ALL, "DMSAA_dual_source_blend_disable")
			skip(ALL, "test", ALL, "DMSAA_preserve_contents")
			skip(ALL, "test", ALL, "EGLImageTest")
			skip(ALL, "test", ALL, "ES2BlendWithNoTexture")
			skip(ALL, "test", ALL, "ExtendedSkColorTypeTests_gpu")
			skip(ALL, "test", ALL, "F16DrawTest")
			skip(ALL, "test", ALL, "FilterResult_ganesh") // knocks out a bunch
			skip(ALL, "test", ALL, "FullScreenClearWithLayers")
			skip(ALL, "test", ALL, "GLBackendAllocationTest")
			skip(ALL, "test", ALL, "GLReadPixelsUnbindPBO")
			skip(ALL, "test", ALL, "GrAHardwareBuffer_BasicDrawTest")
			skip(ALL, "test", ALL, "GrGpuBufferTransferTest")
			skip(ALL, "test", ALL, "GrGpuBufferUpdateDataTest")
			skip(ALL, "test", ALL, "GrMeshTest")
			skip(ALL, "test", ALL, "GrPipelineDynamicStateTest")
			skip(ALL, "test", ALL, "GrTextBlobScaleAnimation")
			skip(ALL, "test", ALL, "HalfFloatAlphaTextureTest")
			skip(ALL, "test", ALL, "HalfFloatRGBATextureTest")
			skip(ALL, "test", ALL, "ImageAsyncReadPixels")
			skip(ALL, "test", ALL, "ImageAsyncReadPixelsGraphite")
			skip(ALL, "test", ALL, "ImageBackendTextureTest")
			skip(ALL, "test", ALL, "ImageEncode_Gpu")
			skip(ALL, "test", ALL, "ImageFilterFailAffectsTransparentBlack_Gpu")
			skip(ALL, "test", ALL, "ImageFilterNegativeBlurSigma_Gpu")
			skip(ALL, "test", ALL, "ImageFilterZeroBlurSigma_Gpu")
			skip(ALL, "test", ALL, "ImageLegacyBitmap_Gpu")
			skip(ALL, "test", ALL, "ImageNewShader_GPU")
			skip(ALL, "test", ALL, "ImageOriginTest_drawImage_Graphite")
			skip(ALL, "test", ALL, "ImageOriginTest_imageShader_Graphite")
			skip(ALL, "test", ALL, "ImageProviderTest_Graphite_Default")
			skip(ALL, "test", ALL, "ImageProviderTest_Graphite_Testing")
			skip(ALL, "test", ALL, "ImageReadPixels_Gpu")
			skip(ALL, "test", ALL, "ImageScalePixels_Gpu")
			skip(ALL, "test", ALL, "ImageShaderTest")
			skip(ALL, "test", ALL, "ImageWrapTextureMipmapsTest")
			skip(ALL, "test", ALL, "MatrixColorFilter_TransparentBlack")
			skip(ALL, "test", ALL, "MorphologyFilterRadiusWithMirrorCTM_Gpu")
			skip(ALL, "test", ALL, "MultisampleRetainTest")
			skip(ALL, "test", ALL, "MultisampleClearThenLoad")
			skip(ALL, "test", ALL, "MutableImagesTest")
			skip(ALL, "test", ALL, "NotifyInUseTestAsImage")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerClear")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerColor")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerColorBurn")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerColorDodge")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDarken")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDifference")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDst")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDstATop")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDstIn")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDstOut")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerDstOver")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerExclusion")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerHardLight")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerHue")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerLighten")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerLuminosity")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerModulate")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerMultiply")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerOverlay")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerPlus")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSaturation")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerScreen")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSoftLight")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSrc")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSrcATop")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSrcIn")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSrcOut")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerSrcOver")
			skip(ALL, "test", ALL, "NotifyInUseTestLayerXor")
			skip(ALL, "test", ALL, "NotifyInUseTestSnapshot")
			skip(ALL, "test", ALL, "OpsTaskFlushCount")
			skip(ALL, "test", ALL, "OverdrawSurface_Gpu")
			skip(ALL, "test", ALL, "PinnedImageTest")
			skip(ALL, "test", ALL, "RecordingOrderTest_Graphite")
			skip(ALL, "test", ALL, "RecordingSurfacesTest")
			skip(ALL, "test", ALL, "ReimportImageTextureWithMipLevels")
			skip(ALL, "test", ALL, "ReplaceSurfaceBackendTexture")
			skip(ALL, "test", ALL, "ResourceCacheCache")
			skip(ALL, "test", ALL, "SaveLayerOrigin")
			skip(ALL, "test", ALL, "ShaderTestNestedBlendsGanesh")
			skip(ALL, "test", ALL, "ShaderTestNestedBlendsGraphite")
			skip(ALL, "test", ALL, "SimplifyPaintTest")
			skip(ALL, "test", ALL, "skbug6653")
			skip(ALL, "test", ALL, "SkImage_makeNonTextureImage")
			skip(ALL, "test", ALL, "SkipCopyTaskTest")
			skip(ALL, "test", ALL, "SkipOpsTaskTest")
			skip(ALL, "test", ALL, "SkColorSpaceXform_Ganesh")
			skip(ALL, "test", ALL, "SkColorSpaceXform_Graphite")
			skip(ALL, "test", ALL, "SkRuntimeBlender_Ganesh")
			skip(ALL, "test", ALL, "SkRuntimeBlender_Graphite")
			skip(ALL, "test", ALL, "SkRuntimeEffect") // knocks out a bunch
			skip(ALL, "test", ALL, "SkRuntimeShaderImageFilter_GPU")
			skip(ALL, "test", ALL, "SkRuntimeShader_TransformedCoords_Ganesh")
			skip(ALL, "test", ALL, "SkRuntimeShader_TransformedCoords_Graphite")
			skip(ALL, "test", ALL, "SkSLCross")
			skip(ALL, "test", ALL, "SkSL") // knocks out a bunch
			skip(ALL, "test", ALL, "SpecialImage_Gpu")
			skip(ALL, "test", ALL, "SRGBReadWritePixels")
			skip(ALL, "test", ALL, "SurfaceAsyncReadPixels")
			skip(ALL, "test", ALL, "SurfaceBackendTextureTest")
			skip(ALL, "test", ALL, "SurfaceClear_Gpu")
			skip(ALL, "test", ALL, "SurfaceContextReadPixels")
			skip(ALL, "test", ALL, "SurfaceContextWritePixelsMipped")
			skip(ALL, "test", ALL, "SurfaceDrawContextTest")
			skip(ALL, "test", ALL, "SurfaceResolveTest")
			skip(ALL, "test", ALL, "SurfaceSemaphores")
			skip(ALL, "test", ALL, "TestSweepGradientZeroXGanesh")
			skip(ALL, "test", ALL, "TiledDrawCacheTest_Ganesh")
			skip(ALL, "test", ALL, "TiledDrawCacheTest_Graphite")
			skip(ALL, "test", ALL, "UnpremulTextureImage")
			skip(ALL, "test", ALL, "VkBackendAllocationTest")
			skip(ALL, "test", ALL, "VkDrawableTest")
			skip(ALL, "test", ALL, "VkDrawableImportTest")
			skip(ALL, "test", ALL, "VkYCbcrSampler_DrawImageWithYcbcrSampler")
			skip(ALL, "test", ALL, "WritePixels_Gpu")
			skip(ALL, "test", ALL, "WritePixels_Graphite")
			skip(ALL, "test", ALL, "WritePixelsMSAA_Gpu")
			skip(ALL, "test", ALL, "WritePixelsNonTexture_Gpu")
			skip(ALL, "test", ALL, "WritePixelsNonTextureMSAA_Gpu")
			skip(ALL, "test", ALL, "WritePixelsPendingIO")
			skip(ALL, "test", ALL, "XfermodeImageFilterCroppedInput_Gpu")

			// These unit tests require some debugging (skbug.com/319229312)
			skip(ALL, "test", ALL, "GrTextBlobMoveAround")          // a lot of AllocImageMemory failures
			skip(ALL, "test", ALL, "Programs")                      // Perlin Noise FP violating protected constraints
			skip(ALL, "test", ALL, "Protected_SmokeTest")           // Ganesh/Vulkan disallow creating Unprotected Image
			skip(ALL, "test", ALL, "ReadOnlyTexture")               // crashes device!
			skip(ALL, "test", ALL, "RepeatedClippedBlurTest")       // blurs not creating expected # of resources
			skip(ALL, "test", ALL, "CharacterizationVkSCBnessTest") // DDL Validation failure for Vk SCBs

			// These unit tests are very slow and probably don't benefit from Protected testing
			skip(ALL, "test", ALL, "PaintParamsKeyTest")
			skip(ALL, "test", ALL, "ProcessorCloneTest")
			skip(ALL, "test", ALL, "ProcessorOptimizationValidationTest")
			skip(ALL, "test", ALL, "TextBlobAbnormal")
			skip(ALL, "test", ALL, "TextBlobStressAbnormal")

			// b/399342221
			skip(ALL, "test", ALL, "UserDefinedStableKeyTest")
		}

		// The Tegra3 doesn't support MSAA
		if b.GPU("Tegra3") ||
			// We aren't interested in fixing msaa bugs on current iOS devices.
			b.Model("iPad4", "iPadPro", "iPhone7") ||
			// skbug.com/40036997
			b.GPU("IntelHD530", "IntelIris540") {
			configs = removeContains(configs, "msaa")
		}

		// We want to test both the OpenGL config and the GLES config on Linux Intel:
		// GL is used by Chrome, GLES is used by ChromeOS.
		if b.MatchGpu("Intel") && b.IsLinux() {
			configs = append(configs, "gles", "glesdft")
		}

		// The FailFlushTimeCallbacks tasks only run the 'gl' config
		if b.ExtraConfig("FailFlushTimeCallbacks") {
			configs = []string{"gl"}
		}

		// Graphite task *only* runs the gr*** configs
		if b.ExtraConfig("Graphite") {
			args = append(args, "--nogpu") // disable non-Graphite tests

			// This gm is just meant for local debugging
			skip(ALL, "test", ALL, "PaintParamsKeyTestReduced")

			if b.ExtraConfig("Dawn") {
				baseConfig := ""
				if b.ExtraConfig("D3D11") {
					baseConfig = "grdawn_d3d11"
				} else if b.ExtraConfig("D3D12") {
					baseConfig = "grdawn_d3d12"
				} else if b.ExtraConfig("Metal") {
					baseConfig = "grdawn_mtl"
				} else if b.ExtraConfig("Vulkan") {
					baseConfig = "grdawn_vk"
				} else if b.ExtraConfig("GL") {
					baseConfig = "grdawn_gl"
				} else if b.ExtraConfig("GLES") {
					baseConfig = "grdawn_gles"
				}

				configs = []string{baseConfig}

				if b.ExtraConfig("FakeWGPU") {
					args = append(args, "--neverYieldToWebGPU")
					args = append(args, "--useWGPUTextureView")
				}

				// Shader doesn't compile
				// https://skbug.com/40045181
				skip(ALL, "gm", ALL, "runtime_intrinsics_matrix")
				// Crashes and failures
				// https://skbug.com/40045181
				skip(ALL, "test", ALL, "BackendTextureTest")

				if b.MatchOs("Win") {
					// Enable MSAA tiling on Windows
					args = append(args, "--internalMSAATileSize", "256")
				}

				if b.MatchOs("Win10") || b.MatchGpu("Adreno620", "MaliG78", "QuadroP400") {
					// The Dawn Win10 and some Android/Linux device jobs OOMs (skbug.com/40045484, b/318725123)
					skip(ALL, "test", ALL, "BigImageTest_Graphite")
				}
				if b.MatchGpu("Adreno620") {
					// The Dawn Pixel5 device job fails one compute test (b/318725123)
					skip(ALL, "test", ALL, "Compute_AtomicOperationsOverArrayAndStructTest")
				}

				if b.ExtraConfig("GL") || b.ExtraConfig("GLES") {
					// These GMs currently have rendering issues in Dawn compat.
					skip(ALL, "gm", ALL, "glyph_pos_n_s")
					skip(ALL, "gm", ALL, "persptext")
					skip(ALL, "gm", ALL, "persptext_minimal")
					skip(ALL, "gm", ALL, "pictureshader_persp")
					skip(ALL, "gm", ALL, "wacky_yuv_formats_frompixmaps")

					// This GM is larger than Dawn compat's max texture size.
					skip(ALL, "gm", ALL, "wacky_yuv_formats_domain")

					// b/389701894 - The Dawn/GLES backend is hard crashing on this test
					skip(ALL, "test", ALL, "ThreadedPipelineCompilePurgingTest")

					// b/405970498 - The Dawn/GLES backend is failing these two tests
					skip(ALL, "test", ALL, "ThreadedPipelinePrecompileCompileTest")
					skip(ALL, "test", ALL, "ThreadedPipelinePrecompileCompilePurgingTest")

					// b/425434638 - PaintParamsKeyTest failing on Release Dawn_GLES
					skip(ALL, "test", ALL, "PaintParamsKeyTest")
				}

				if b.ExtraConfig("Vulkan") {
					// b/425434638 - PaintParamsKeyTest failing on Release Dawn_Vulkan
					skip(ALL, "test", ALL, "PaintParamsKeyTest")

					if b.ExtraConfig("TSAN") {
						// The TSAN_Graphite_Dawn_Vulkan job goes off into space on this test
						skip(ALL, "test", ALL, "BigImageTest_Graphite")
						// b/389706939 - Dawn/Vulkan reports a data race for LazyClearCountForTesting w/ TSAN
						skip(ALL, "test", ALL, "ThreadedPipelineCompilePurgingTest")
					}
				}

				if b.ExtraConfig("Metal") {
					if b.ExtraConfig("TSAN") {
						// b/389706939 - Dawn/Metal reports a data race for LazyClearCountForTesting w/ TSAN
						skip(ALL, "test", ALL, "ThreadedPipelineCompilePurgingTest")
						// The TSAN_Graphite_Dawn_Metal job seems to consistently get stuck on this unit test
						skip(ALL, "test", ALL, "BigImageTest_Graphite")
					}
				}
			} else if b.ExtraConfig("Native") {
				if b.ExtraConfig("Metal") {
					if b.ExtraConfig("TestPrecompile") {
						configs = []string{"grmtltestprecompile"}
					} else {
						configs = []string{"grmtl"}
					}

					if b.GPU("IntelIrisPlus") {
						// We get some 27/255 RGB diffs on the 45 degree
						// rotation case on this device (skbug.com/40045482)
						skip(ALL, "test", ALL, "BigImageTest_Graphite")
					}
				}
				if b.ExtraConfig("Vulkan") {
					if b.ExtraConfig("TestPrecompile") {
						configs = []string{"grvktestprecompile"}
					} else {
						configs = []string{"grvk"}
					}

					// Couldn't readback
					skip(ALL, "gm", ALL, "aaxfermodes")
					// Could not instantiate texture proxy for UploadTask!
					skip(ALL, "test", ALL, "BigImageTest_Graphite")
					// Test failures
					skip(ALL, "test", ALL, "PaintParamsKeyTest")
					if b.MatchOs("Android") {
						// Currently broken on Android Vulkan (skbug.com/310180104)
						skip(ALL, "test", ALL, "ImageAsyncReadPixelsGraphite")
						skip(ALL, "test", ALL, "SurfaceAsyncReadPixelsGraphite")
					}

					// b/380049954 Graphite Native Vulkan has a thread race issue
					skip(ALL, "test", ALL, "ThreadedPipelinePrecompileCompileTest")
					skip(ALL, "test", ALL, "ThreadedPipelinePrecompileCompilePurgingTest")
					skip(ALL, "test", ALL, "ThreadedPipelinePrecompilePurgingTest")

					if b.GPU("QuadroP400") || b.MatchOs("Ubuntu24.04") {
						// Neither the nVidia driver on the P400s nor the Ubuntu24.04 driver
						// correctly support pipeline caching control (i.e., they *never* return
						// VK_PIPELINE_COMPILE_REQUIRED from CreateGraphicsPipelines)
						skip(ALL, "test", ALL, "PersistentPipelineStorageTest")
					}
				}
			}
		} else {
			if b.GPU("QuadroP400") && b.MatchOs("Ubuntu24.04") {
				if b.ExtraConfig("Vulkan") {
					// skbug.com/40045530
					skip(ALL, "test", ALL, "VkYCbcrSampler_DrawImageWithYcbcrSampler")
				} else {
					// skbug.com/458193911
					skip(ALL, "test", ALL, "SkRuntimeBlender_Ganesh")
				}
			}

		}

		// ANGLE bot *only* runs the angle configs
		if b.ExtraConfig("ANGLE") {
			if b.MatchOs("Win") {
				configs = []string{"angle_d3d11_es2", "angle_d3d11_es3"}
				if sampleCount > 0 {
					configs = append(configs, fmt.Sprintf("angle_d3d11_es2_msaa%d", sampleCount))
					configs = append(configs, fmt.Sprintf("angle_d3d11_es2_dmsaa"))
					configs = append(configs, fmt.Sprintf("angle_d3d11_es3_msaa%d", sampleCount))
					configs = append(configs, fmt.Sprintf("angle_d3d11_es3_dmsaa"))
				}
				if !b.MatchGpu("GTX", "Quadro", "GT610") {
					// See skbug.com/40041499
					configs = append(configs, "angle_d3d9_es2")
				}
				if b.Model("NUC5i7RYH") {
					// skbug.com/40038570
					skip(ALL, "test", ALL, "ProcessorCloneTest")
				}
				if b.MatchGpu("Intel") {
					// Debug-ANGLE-All on Intel frequently timeout, and the FilterResult test suite
					// produces many test cases, that are multiplied by the number of configs (of
					// which ANGLE has many variations). There is not a lot of value gained by
					// running these if they are the source of long 'dm' time on Xe hardware given
					// many other tasks are executing them.
					skip(ALL, "test", ALL, "FilterResult")
				}
			} else if b.MatchOs("Mac") {
				configs = []string{"angle_mtl_es2", "angle_mtl_es3"}

				// anglebug.com/7245
				skip("angle_mtl_es3", "gm", ALL, "runtime_intrinsics_common_es3")

				if b.MatchGpu("AppleM") {
					// M1 Macs fail this test for sRGB color types
					// skbug.com/40044370
					skip(ALL, "test", ALL, "TransferPixelsToTextureTest")
				}
			}
		}

		if b.Model("AndroidOne", "Nexus5", "Nexus7", "JioNext") {
			// skbug.com/40040304
			skip(ALL, "test", ALL, "ProcessorCloneTest")
			skip(ALL, "test", ALL, "Programs")
			skip(ALL, "test", ALL, "ProcessorOptimizationValidationTest")
		}

		if b.Model("GalaxyS20") {
			// skbug.com/40041940
			skip(ALL, "test", ALL, "ProcessorCloneTest")
		}

		if b.Model("MotoG73") {
			// https://g-issues.skia.org/issues/370739986
			skip(ALL, "test", ALL, "SkSLSwizzleIndexStore_Ganesh")
			skip(ALL, "test", ALL, "SkSLMatrixScalarMath_Ganesh")
			skip(ALL, "test", ALL, "SkSLMatrixOpEqualsES3_Ganesh")
			skip(ALL, "test", ALL, "SkSLMatrixScalarNoOpFolding_Ganesh")

			skip(ALL, "test", ALL, "SkSLIncrementDisambiguation_Ganesh")
			skip(ALL, "test", ALL, "SkSLArrayFolding_Ganesh")
			skip(ALL, "test", ALL, "SkSLIntrinsicModf_Ganesh")
		}

		if b.Model("MacMini8.1") && b.ExtraConfig("Metal") {
			// https://g-issues.skia.org/issues/391573668
			skip(ALL, "test", ALL, "SkSLIntrinsicAll_Graphite")
			skip(ALL, "test", ALL, "SkSLIntrinsicAny_Graphite")
			skip(ALL, "test", ALL, "SkSLIntrinsicNot_Graphite")
			skip(ALL, "test", ALL, "SkSLIntrinsicMixFloatES3_Graphite")
			skip(ALL, "test", ALL, "SkSLIntrinsicAll_Ganesh")
			skip(ALL, "test", ALL, "SkSLIntrinsicAny_Ganesh")
			skip(ALL, "test", ALL, "SkSLIntrinsicNot_Ganesh")
			skip(ALL, "test", ALL, "SkSLIntrinsicMixFloatES3_Ganesh")
		}

		// skbug.com/40040327 - these devices render this test incorrectly
		// when opList splitting reduction is enabled
		if b.GPU() && b.ExtraConfig("Vulkan") && (b.GPU("RadeonR9M470X", "RadeonHD7770")) {
			skip(ALL, "tests", ALL, "VkDrawableImportTest")
		}
		if b.ExtraConfig("Vulkan") && !b.ExtraConfig("Graphite") {
			configs = []string{"vk"}
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926, skbug.com/40040308
			// MSAA4 is not supported on the MotoG73
			//     "Configuration 'vkmsaa4' sample count 4 is not a supported sample count."
			// In Ganesh we currently disable msaa on imagination GPUs. Newer GPUs
			// probably support MSAA fine, but we currently don't plan on enabling
			// MSAA at all for Ganesh to reduce correctness and performance churn on
			// clients as we're trying to move away from Ganesh.
			if !b.MatchGpu("Intel") && !b.Model("MotoG73") && !b.MatchGpu("IMG") {
				configs = append(configs, "vkmsaa4")
			}

			// Temporarily limit the machines we test dynamic MSAA on.
			if b.GPU("QuadroP400", "MaliG77") && !b.ExtraConfig("TSAN") {
				configs = append(configs, "vkdmsaa")
			}
		}
		if b.ExtraConfig("Metal") && !b.ExtraConfig("Graphite") {
			configs = []string{"mtl"}
			// MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
			if !b.MatchGpu("Intel") {
				configs = append(configs, "mtlmsaa4")
			}
		}
		if b.ExtraConfig("Slug") {
			// Test slug drawing
			configs = []string{"glslug", "glserializeslug", "glremoteslug"}
			// glremoteslug does not handle layers right. Exclude the tests for now.
			skip("glremoteslug", "gm", ALL, "rtif_distort")
			skip("glremoteslug", "gm", ALL, "textfilter_image")
			skip("glremoteslug", "gm", ALL, "textfilter_color")
			skip("glremoteslug", "gm", ALL, "savelayerpreservelcdtext")
			skip("glremoteslug", "gm", ALL, "typefacerendering_pfa")
			skip("glremoteslug", "gm", ALL, "typefacerendering_pfb")
			skip("glremoteslug", "gm", ALL, "typefacerendering")
		}
		if b.ExtraConfig("Direct3D") {
			configs = []string{"d3d"}
		}

		// Test 1010102 on our Linux/NVIDIA tasks and the persistent cache config
		// on the GL tasks.
		if b.GPU("QuadroP400") && !b.ExtraConfig("PreAbandonGpuContext") && !b.ExtraConfig("TSAN") && b.IsLinux() &&
			!b.ExtraConfig("FailFlushTimeCallbacks") && !b.ExtraConfig("Graphite") {
			if b.ExtraConfig("Vulkan") {
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
		if b.Model("Pixel2XL") && !b.ExtraConfig("Vulkan") {
			configs = append(configs, "glestestprecompile")
		}

		// Test SkSL precompile on iPhone 8 as representative iOS device
		if b.Model("iPhone8") && b.ExtraConfig("Metal") {
			configs = append(configs, "mtltestprecompile")
			// avoid tests that can generate slightly different pixels per run
			skip("mtltestprecompile", "gm", ALL, "atlastext")
			skip("mtltestprecompile", "gm", ALL, "circular_arcs_hairline")
			skip("mtltestprecompile", "gm", ALL, "dashcircle")
			skip("mtltestprecompile", "gm", ALL, "dftext")
			skip("mtltestprecompile", "gm", ALL, "encode-platform")
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
			skip("mtltestprecompile", "svg", ALL, "cartman.svg")
			skip("mtltestprecompile", "svg", ALL, "desk_motionmark_paths.svg")
			skip("mtltestprecompile", "svg", ALL, "rg1024_green_grapes.svg")
			skip("mtltestprecompile", "svg", ALL, "shapes-intro-02-f.svg")
			skip("mtltestprecompile", "svg", ALL, "tiger-8.svg")
		}
		// Test reduced shader mode on iPhone 11 as representative iOS device
		if b.Model("iPhone11") && b.ExtraConfig("Metal") && !b.ExtraConfig("Graphite") {
			configs = append(configs, "mtlreducedshaders")
		}

		if b.Model(DONT_REDUCE_OPS_TASK_SPLITTING_MODELS...) {
			args = append(args, "--dontReduceOpsTaskSplitting", "true")
		}

		// Test reduceOpsTaskSplitting fallback when over budget.
		if b.Model("NUC7i5BNK") && b.ExtraConfig("ASAN") {
			args = append(args, "--gpuResourceCacheLimit", "16777216")
		}

		// Test rendering to wrapped dsts on a few tasks
		if b.ExtraConfig("BonusConfigs") {
			configs = []string{"glbetex", "glbert", "glreducedshaders", "glr8"}
		}

		if b.Os("ChromeOS") {
			// Just run GLES for now - maybe add gles_msaa4 in the future
			configs = []string{"gles"}
		}

		// Test GPU tessellation path renderer.
		if b.ExtraConfig("GpuTess") {
			configs = []string{glPrefix + "msaa4"}
			// Use fixed count tessellation path renderers as much as possible.
			args = append(args, "--pr", "atlas", "tess")
		}

		// DDL is a GPU-only feature
		if b.ExtraConfig("DDL1") {
			// This bot generates comparison images for the large skps and the gms
			configs = filter(configs, "gl", "vk", "mtl")
			args = append(args, "--skpViewportSize", "2048")
		}
		if b.ExtraConfig("DDL3") {
			// This bot generates the real ddl images for the large skps and the gms
			configs = suffix(filter(configs, "gl", "vk", "mtl"), "ddl")
			args = append(args, "--skpViewportSize", "2048")
			args = append(args, "--gpuThreads", "0")
		}
	}

	if b.MatchExtraConfig("ColorSpaces") {
		// Here we're going to generate a bunch of configs with the format:
		//        <colorspace> <backend> <targetFormat>
		// Where: <colorspace> is one of:   "", "linear-", "narrow-", p3-", "rec2020-", "srgb2-"
		//        <backend> is one of: "gl, "gles", "mtl", "vk"
		//                             their "gr" prefixed versions
		//                             and "" (for raster)
		//        <targetFormat> is one of: "f16", { "" (for gpus) or "rgba" (for raster) }
		//
		// We also add on two configs with the format:
		//        narrow-<backend>f16norm
		//        linear-<backend>srgba
		colorSpaces := []string{"", "linear-", "narrow-", "p3-", "rec2020-", "srgb2-"}

		backendStr := ""
		if b.GPU() {
			if b.ExtraConfig("Graphite") {
				if b.ExtraConfig("GL") {
					backendStr = "grgl"
				} else if b.ExtraConfig("GLES") {
					backendStr = "grgles"
				} else if b.ExtraConfig("Metal") {
					backendStr = "grmtl"
				} else if b.ExtraConfig("Vulkan") {
					backendStr = "grvk"
				}
			} else {
				if b.ExtraConfig("GL") {
					backendStr = "gl"
				} else if b.ExtraConfig("GLES") {
					backendStr = "gles"
				} else if b.ExtraConfig("Metal") {
					backendStr = "mtl"
				} else if b.ExtraConfig("Vulkan") {
					backendStr = "vk"
				}
			}
		}

		targetFormats := []string{"f16"}
		if b.GPU() {
			targetFormats = append(targetFormats, "")
		} else {
			targetFormats = append(targetFormats, "rgba")
		}

		configs = []string{}

		for _, cs := range colorSpaces {
			for _, tf := range targetFormats {
				configs = append(configs, cs+backendStr+tf)
			}
		}

		configs = append(configs, "narrow-"+backendStr+"f16norm")
		configs = append(configs, "linear-"+backendStr+"srgba")
	}

	// Sharding.
	tf := b.Parts["test_filter"]
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
	if b.GPU() {
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
	if b.ExtraConfig("SK_FORCE_RASTER_PIPELINE_BLITTER") {
		removeFromArgs("tests")
	}

	if b.ExtraConfig("NativeFonts") { // images won't exercise native font integration :)
		removeFromArgs("image")
		removeFromArgs("colorImage")
	}

	if b.MatchExtraConfig("Graphite") {
		removeFromArgs("image")
		removeFromArgs("lottie")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	}

	if b.MatchExtraConfig("Fontations") {
		// The only fontations code is exercised via gms and tests
		removeFromArgs("image")
		removeFromArgs("lottie")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	}

	// Remove skps for all tasks except for a select few. On tasks that will run SKPs remove some of
	// their other tests.
	if b.MatchExtraConfig("DDL", "PDF") {
		// The DDL and PDF tasks just render the large skps and the gms
		removeFromArgs("tests")
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	} else if b.MatchExtraConfig("OldestSupportedSkpVersion") {
		// The OldestSupportedSkpVersion bot only renders skps.
		removeFromArgs("tests")
		removeFromArgs("gm")
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
	} else if b.MatchExtraConfig("FailFlushTimeCallbacks") {
		// The FailFlushTimeCallbacks bot only runs skps, gms and svgs
		removeFromArgs("tests")
		removeFromArgs("image")
		removeFromArgs("colorImage")
	} else if b.ExtraConfig("Protected") {
		// Currently the Protected jobs only run the unit tests
		removeFromArgs("gm")
		removeFromArgs("image")
		removeFromArgs("lottie")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
		removeFromArgs("skp")
	} else {
		// No other tasks render the .skps.
		removeFromArgs("skp")
	}

	if b.ExtraConfig("Lottie") {
		// Only run the lotties on Lottie tasks.
		removeFromArgs("tests")
		removeFromArgs("gm")
		removeFromArgs("image")
		removeFromArgs("colorImage")
		removeFromArgs("svg")
		removeFromArgs("skp")
	} else {
		removeFromArgs("lottie")
	}

	if b.ExtraConfig("TSAN") {
		// skbug.com/40042212
		removeFromArgs("svg")
		// skbug.com/40043998 avoid OOM on
		//   Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-TSAN_Vulkan
		// Avoid lots of spurious TSAN failures on
		//   Test-Debian11-Clang-NUC11TZi5-GPU-IntelIrisXe-x86_64-Release-All-TSAN_Vulkan
		//   Test-Debian11-Clang-NUC11TZi5-GPU-IntelIrisXe-x86_64-Release-All-DDL3_TSAN_Vulkan
		if b.Name == "Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-TSAN_Vulkan" ||
			b.Name == "Test-Debian11-Clang-NUC11TZi5-GPU-IntelIrisXe-x86_64-Release-All-TSAN_Vulkan" ||
			b.Name == "Test-Debian11-Clang-NUC11TZi5-GPU-IntelIrisXe-x86_64-Release-All-DDL3_TSAN_Vulkan" {
			skip("_", "test", "_", "_")
		}
	}

	// TODO: ???
	skip("f16", ALL, ALL, "dstreadshuffle")

	// --src image --config r8 means "decode into R8", which isn't supported.
	skip("r8", "image", ALL, ALL)
	skip("r8", "colorImage", ALL, ALL)

	if b.Model("TecnoSpark3Pro", "Wembley") {
		// skbug.com/40040743
		skip(ALL, "test", ALL, "InitialTextureClear")
	}

	if b.Model("Wembley", "JioNext") {
		// These tests run forever on the Wembley.
		skip(ALL, "gm", ALL, "async_rescale_and_read")
	}

	if b.Model("Wembley") {
		// These tests run forever or use too many resources on the Wembley.
		skip(ALL, "gm", ALL, "wacky_yuv_formats")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_cs")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_cubic")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_domain")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_fromimages")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_frompixmaps")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_imggen")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_limited")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_limited_cs")
		skip(ALL, "gm", ALL, "wacky_yuv_formats_limited_fromimages")
	}

	if b.Os("iOS") {
		skip(glPrefix, "skp", ALL, ALL)
	}

	if b.MatchOs("Mac", "iOS") {
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
		// skbug.com/40036984
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
	if b.MatchOs("Win") {
		skip(ALL, "image", "gen_platf", "pal8os2v2.bmp")
		skip(ALL, "image", "gen_platf", "pal8os2v2-16.bmp")
		skip(ALL, "image", "gen_platf", "rgba32abf.bmp")
		skip(ALL, "image", "gen_platf", "rgb24prof.bmp")
		skip(ALL, "image", "gen_platf", "rgb24lprof.bmp")
		skip(ALL, "image", "gen_platf", "8bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "4bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "32bpp-pixeldata-cropped.bmp")
		skip(ALL, "image", "gen_platf", "24bpp-pixeldata-cropped.bmp")
		if b.Arch("x86_64") && b.CPU() {
			// This GM triggers a SkSmallAllocator assert.
			skip(ALL, "gm", ALL, "composeshader_bitmap")
		}
	}

	if !b.MatchOs("Win", "Debian11", "Ubuntu18") || b.GPU("IntelIris655", "IntelIris540") {
		// This test requires a decent max texture size so just run it on the desktops.
		// The OS list should include Mac but Mac10.13 doesn't work correctly.
		// The IntelIris655 and IntelIris540 GPUs don't work correctly in the Vk backend
		skip(ALL, "test", ALL, "BigImageTest_Ganesh")
	}

	if b.MatchOs("Win", "Mac") {
		// WIC and CG fail on arithmetic jpegs
		skip(ALL, "image", "gen_platf", "testimgari.jpg")
		// More questionable bmps that fail on Mac, too. skbug.com/40038213
		skip(ALL, "image", "gen_platf", "rle8-height-negative.bmp")
		skip(ALL, "image", "gen_platf", "rle4-height-negative.bmp")
	}

	if b.MatchOs("Mac14") {
		// These images are very large
		skip(ALL, "image", "gen_platf", "rgb24largepal.bmp")
		skip(ALL, "image", "gen_platf", "pal8oversizepal.bmp")

		if b.ExtraConfig("ANGLE") && b.MatchGpu("IntelUHDGraphics630") {
			// b/405918638
			skip(ALL, "tests", ALL, "TransferPixelsToTextureTest")
		}
	}

	// These PNGs have CRC errors. The platform generators seem to draw
	// uninitialized memory without reporting an error, so skip them to
	// avoid lots of images on Gold.
	skip(ALL, "image", "gen_platf", "error")

	if b.MatchOs("Android") || b.Os("iOS") {
		// This test crashes the N9 (perhaps because of large malloc/frees). It also
		// is fairly slow and not platform-specific. So we just disable it on all of
		// Android and iOS. skbug.com/40036610
		skip(ALL, "test", ALL, "GrStyledShape")
	}

	if internalHardwareLabel == "5" {
		// http://b/118312149#comment9
		skip(ALL, "test", ALL, "SRGBReadWritePixels")
	}

	// skbug.com/40035245
	badSerializeGMs := []string{
		"strict_constraint_batch_no_red_allowed", // skbug.com/40041632
		"strict_constraint_no_red_allowed",       // skbug.com/40041632
		"fast_constraint_red_is_allowed",         // skbug.com/40041632
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
		"imagefilterstransformed",
	}

	// skbug.com/40036770
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

	// skbug.com/40036776
	badSerializeGMs = append(badSerializeGMs,
		"composeshader_bitmap",
		"scaled_tilemodes_npot",
		"scaled_tilemodes",
	)

	// skbug.com/40036988
	badSerializeGMs = append(badSerializeGMs, "typefacerendering_pfaMac")
	// skbug.com/40037140
	badSerializeGMs = append(badSerializeGMs, "parsedpaths")

	// these use a custom image generator which doesn't serialize
	badSerializeGMs = append(badSerializeGMs, "ImageGeneratorExternal_rect")
	badSerializeGMs = append(badSerializeGMs, "ImageGeneratorExternal_shader")

	// skbug.com/40037391
	badSerializeGMs = append(badSerializeGMs, "shadow_utils")
	badSerializeGMs = append(badSerializeGMs, "graphitestart")

	// skbug.com/40039190
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
	badSerializeGMs = append(badSerializeGMs, "image_out_of_gamut")

	// This GM forces a path to be convex. That property doesn't survive
	// serialization.
	badSerializeGMs = append(badSerializeGMs, "analytic_antialias_convex")

	for _, test := range badSerializeGMs {
		skip("serialize-8888", "gm", ALL, test)
	}

	// We skip these to avoid out-of-memory failures.
	if b.MatchOs("Win", "Android") {
		for _, test := range []string{"verylargebitmap", "verylarge_picture_image"} {
			skip("serialize-8888", "gm", ALL, test)
		}
	}
	if b.MatchOs("Mac") && b.CPU() {
		// skbug.com/40038221
		skip("pic-8888", "gm", ALL, "encode-platform")
		skip("serialize-8888", "gm", ALL, "encode-platform")
	}

	// skbug.com/40045485 -- images are visibly identical, not interested in diagnosing non-determinism here
	skip("pic-8888", "gm", ALL, "perlinnoise_layered")
	skip("serialize-8888", "gm", ALL, "perlinnoise_layered")
	if b.GPU("IntelIrisXe") && !b.ExtraConfig("Vulkan") {
		skip(ALL, "gm", ALL, "perlinnoise_layered") // skbug.com/40045485
	}

	if b.GPU("IntelIrisXe") && b.MatchOs("Win") && b.ExtraConfig("Vulkan") {
		skip(ALL, "tests", ALL, "VkYCbcrSampler_DrawImageWithYcbcrSampler") // skbug.com/40045530
	}

	// skbug.com/40035927
	skip("pic-8888", "gm", ALL, "drawfilter")

	// skbug.com/40035857
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
		"async_rescale_and_read_alpha_type",
		"blurrect_compare", // GM requires canvas->makeSurface() to return a valid surface.
		"rrect_blurs"} {
		skip("pic-8888", "gm", ALL, test)
		skip("serialize-8888", "gm", ALL, test)
	}

	// Extensions for RAW images
	r := []string{
		"arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
		"ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
	}

	// skbug.com/40035846
	// Skip RAW images (and a few large PNGs) on GPU tasks
	// until we can resolve failures.
	if b.GPU() {
		skip(ALL, "image", ALL, "interlaced1.png")
		skip(ALL, "image", ALL, "interlaced2.png")
		skip(ALL, "image", ALL, "interlaced3.png")
		for _, rawExt := range r {
			skip(ALL, "image", ALL, "."+rawExt)
		}
	}

	if b.Model("Nexus5", "Nexus5x", "JioNext") && b.GPU() {
		// skbug.com/40037071
		skip(ALL, "gm", ALL, "encode-platform")
	}

	if b.Model("AndroidOne") && b.GPU() { // skbug.com/40035852, skbug.com/40035858, skbug.com/40035849, skbug.com/40035859, skbug.com/40042506
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

	// b/416733454
	if (b.Model("AndroidOne") || b.Model("JioNext") || b.Model("GalaxyS7_G930FD")) && b.GPU() {
		skip(ALL, "svg", ALL, "desk_motionmark_paths.svg")
	}

	// b/296440036
	// disable broken tests on Adreno 5/6xx Vulkan or API30
	if b.MatchGpu("Adreno[56]") && (b.ExtraConfig("Vulkan") || b.ExtraConfig("API30")) {
		skip(ALL, "tests", ALL, "ImageAsyncReadPixels_Renderable_BottomLeft")
		skip(ALL, "tests", ALL, "ImageAsyncReadPixels_Renderable_TopLeft")
		skip(ALL, "tests", ALL, "ImageAsyncReadPixels_NonRenderable_BottomLeft")
		skip(ALL, "tests", ALL, "ImageAsyncReadPixels_NonRenderable_TopLeft")
		skip(ALL, "tests", ALL, "SurfaceAsyncReadPixels")
	}

	if b.MatchGpu("Adreno[56]") && b.ExtraConfig("Vulkan") {
		skip(ALL, "gm", ALL, "mesh_with_image")
		skip(ALL, "gm", ALL, "mesh_with_paint_color")
		skip(ALL, "gm", ALL, "mesh_with_paint_image")
	}

	if b.MatchGpu("Mali400") {
		skip(ALL, "tests", ALL, "BlendRequiringDstReadWithLargeCoordinates")
		skip(ALL, "tests", ALL, "SkSLCross") // despite the name, it's not in SkSLTest.cpp
	}

	if b.MatchOs("Mac") && (b.GPU("IntelIrisPlus") || b.GPU("IntelHD6000")) &&
		b.ExtraConfig("Metal") {
		// TODO(b/296960708): The IntelIrisPlus+Metal config hangs on this test, but passes
		// SurfaceContextWritePixelsMipped so let that one keep running.
		skip(ALL, "tests", ALL, "SurfaceContextWritePixels")
		skip(ALL, "tests", ALL, "SurfaceContextWritePixelsMipped")
		skip(ALL, "tests", ALL, "ImageAsyncReadPixels")
		skip(ALL, "tests", ALL, "SurfaceAsyncReadPixels")
	}

	if b.ExtraConfig("ANGLE") && b.MatchOs("Win") && b.MatchGpu("IntelIris(540|655|Xe)") {
		skip(ALL, "tests", ALL, "ImageFilterCropRect_Gpu") // b/294080402
	}

	if b.MatchOs("Mac15") && b.MatchGpu("IntelUHDGraphics630") {
		if !b.ExtraConfig("Graphite") {
			if b.ExtraConfig("ANGLE") {
				// b/405918638
				skip(ALL, "tests", ALL, "TransferPixelsFromTextureTest")
				skip(ALL, "tests", ALL, "ImageAsyncReadPixels_Renderable_BottomLeft")
				skip(ALL, "tests", ALL, "ImageAsyncReadPixels_Renderable_TopLeft")
				skip(ALL, "tests", ALL, "ImageAsyncReadPixels_NonRenderable_BottomLeft")
				skip(ALL, "tests", ALL, "ImageAsyncReadPixels_NonRenderable_TopLeft")
				skip(ALL, "tests", ALL, "SurfaceAsyncReadPixels")
				skip(ALL, "tests", ALL, "TransferPixelsToTextureTest")
			} else if b.ExtraConfig("Metal") {
				// b/438450848
				skip(ALL, "tests", ALL, "DMSAA_aa_dst_read_after_dmsaa")
				skip(ALL, "tests", ALL, "DMSAA_dst_read")
				skip(ALL, "tests", ALL, "SurfacePartialDraw_Gpu")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_RescaleWithColorFilter")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_RescaleWithTransform")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_RescaleWithTileMode")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_ColorFilterBetweenCrops")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_TransformAndTile")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_PeriodicTileCrops")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_IntersectingCrops")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_CropDisjointFromSourceAndOutput")
				skip(ALL, "tests", ALL, "FilterResult_ganesh_Crop")
			} else {
				// These two are also broken for OpenGL configs b/405918638
				skip(ALL, "tests", ALL, "TransferPixelsFromTextureTest")
				skip(ALL, "tests", ALL, "TransferPixelsToTextureTest")
			}
		} else {
			if b.ExtraConfig("Metal") {
				skip(ALL, "tests", ALL, "NotifyInUseTestLayerDarken")  // b/449171614
				skip(ALL, "tests", ALL, "NotifyInUseTestLayerLighten") // b/449171614
			}
		}
	}

	if b.GPU("RTX3060") && b.ExtraConfig("Vulkan") && b.MatchOs("Win") {
		skip(ALL, "gm", ALL, "blurcircles2") // skbug.com/40044425
	}

	if b.GPU("QuadroP400") && b.MatchOs("Win10") && b.MatchModel("Golo") {
		// Times out with driver 30.0.15.1179
		skip("vkmsaa4", "gm", ALL, "shadow_utils")
	}

	if b.GPU("RadeonR9M470X") && !b.ExtraConfig("Graphite") {
		// Currently, RadeonR9M470X implies Win11/AlphaR2
		if b.ExtraConfig("ANGLE") {
			// skbug.com/40045379 - ANGLE D3D9 ES2 has flaky texture sampling that leads to fuzzy diff errors
			skip(ALL, "tests", ALL, "FilterResult")
			// skbug.com/40044914 - Flaky failures on ANGLE D3D9 ES2
			skip(ALL, "tests", ALL, "SkRuntimeEffectSimple_Ganesh")
			skip(ALL, "tests", ALL, "TestSweepGradientZeroXGanesh")

			// b/438680092
			skip(ALL, "tests", ALL, "SkSLPrefixExpressionsES2_Ganesh")
			skip(ALL, "tests", ALL, "SkSLForLoopMultipleInitES3_Ganesh")
			skip(ALL, "tests", ALL, "SkSLLoopFloat_Ganesh")
		} else if b.ExtraConfig("Vulkan") {
			// No suppressions for Vulkan yet
		} else {
			// b/438680092
			skip(ALL, "tests", ALL, "SkSLPrefixExpressionsES2_Ganesh")
			skip(ALL, "tests", ALL, "SkSLForLoopMultipleInitES3_Ganesh")
			skip(ALL, "tests", ALL, "SkSLLoopFloat_Ganesh")
		}
	}

	if b.ExtraConfig("Vulkan") && b.GPU("RadeonVega6") {
		skip(ALL, "gm", ALL, "ycbcrimage")                                 // skbug.com/40044345
		skip(ALL, "test", ALL, "VkYCbcrSampler_DrawImageWithYcbcrSampler") // skbug.com/40044345
	}

	match := []string{}

	if b.ExtraConfig("Graphite") {
		// Graphite doesn't do auto-image-tiling so these GMs should remain disabled
		match = append(match, "~^verylarge_picture_image$")
		match = append(match, "~^verylargebitmap$")
		match = append(match, "~^path_huge_aa$")
		match = append(match, "~^fast_constraint_red_is_allowed$")
		match = append(match, "~^strict_constraint_batch_no_red_allowed$")
		match = append(match, "~^strict_constraint_no_red_allowed$")
	}

	if b.Model("AndroidOne") {
		match = append(match, "~WritePixels")                             // skbug.com/40035865
		match = append(match, "~PremulAlphaRoundTrip_Gpu")                // skbug.com/40038744
		match = append(match, "~ReimportImageTextureWithMipLevels")       // skbug.com/40039349
		match = append(match, "~MorphologyFilterRadiusWithMirrorCTM_Gpu") // skbug.com/40041719
	}

	if b.GPU("IntelIrisXe") {
		match = append(match, "~ReimportImageTextureWithMipLevels")
	}

	if b.ExtraConfig("MSAN") {
		match = append(match, "~Once", "~Shared") // Not sure what's up with these tests.
	}

	// By default, we test with GPU threading enabled, unless specifically
	// disabled.
	if b.ExtraConfig("NoGPUThreads") {
		args = append(args, "--gpuThreads", "0")
	}

	if b.ExtraConfig("Vulkan") && b.GPU("Adreno530") {
		// skbug.com/40036986
		match = append(match, "~CopySurface")
	}

	// Pixel4XL on the tree is still on Android 10 (Q), and the vulkan drivers
	// crash during this GM. It works correctly on newer versions of Android.
	// The Pixel3a is also failing on this GM with an invalid return value from
	// vkCreateGraphicPipelines.
	if b.ExtraConfig("Vulkan") && (b.Model("Pixel4XL") || b.Model("Pixel3a")) {
		skip("vk", "gm", ALL, "custommesh_cs_uniforms")
	}

	if b.ExtraConfig("Vulkan") && b.MatchGpu("Adreno") {
		// skbug.com/40038922
		match = append(match, "~WritePixelsNonTextureMSAA_Gpu")
		match = append(match, "~WritePixelsMSAA_Gpu")
	}

	if b.ExtraConfig("Vulkan") && b.IsLinux() && b.GPU("IntelIris640") {
		match = append(match, "~VkHeapTests") // skbug.com/40037445
	}

	if b.IsLinux() && b.GPU("IntelIris640") {
		match = append(match, "~Programs") // skbug.com/40039101
	}

	if b.Model("TecnoSpark3Pro", "Wembley") {
		// skbug.com/40041143
		match = append(match, "~Programs")
		match = append(match, "~ProcessorCloneTest")
		match = append(match, "~ProcessorOptimizationValidationTest")
	}

	if b.GPU("IntelIris640", "IntelHD615", "IntelHDGraphics615") {
		match = append(match, "~^SRGBReadWritePixels$") // skbug.com/40040526
	}

	if b.ExtraConfig("Vulkan") && b.IsLinux() && b.GPU("IntelHD405") {
		// skbug.com/40038567
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

	if b.ExtraConfig("Metal") && !b.ExtraConfig("Graphite") && b.GPU("RadeonHD8870M") && b.MatchOs("Mac") {
		// skbug.com/40040558
		match = append(match, "~WritePixelsNonTextureMSAA_Gpu")
		// skbug.com/40042739
		match = append(match, "~SurfacePartialDraw_Gpu")
	}

	if b.ExtraConfig("Metal") && !b.ExtraConfig("Graphite") && b.GPU("PowerVRGX6450") && b.MatchOs("iOS") {
		// skbug.com/40042979
		match = append(match, "~flight_animated_image")
	}

	if b.ExtraConfig("ANGLE") {
		// skbug.com/40039089
		match = append(match, "~BlurMaskBiggerThanDest")
	}

	if b.GPU("IntelIris6100") && b.ExtraConfig("ANGLE") && !b.Debug() {
		// skbug.com/40038570
		match = append(match, "~^ProcessorOptimizationValidationTest$")
	}

	if b.GPU("IntelIris6100", "IntelHD4400") && b.ExtraConfig("ANGLE") {
		// skbug.com/40038077
		skip("angle_d3d9_es2", "gm", ALL, "lighting")
	}

	if b.GPU("PowerVRGX6250") {
		match = append(match, "~gradients_view_perspective_nodither") //skbug.com/40038203
	}

	if b.Arch("arm") && b.ExtraConfig("ASAN") {
		// TODO: can we run with env allocator_may_return_null=1 instead?
		match = append(match, "~BadImage")
	}

	if b.Arch("arm64") && b.ExtraConfig("ASAN") {
		// skbug.com/40044239 the use of longjmp may cause ASAN stack check issues.
		skip(ALL, "test", ALL, "SkPDF_JpegIdentification")
	}

	if b.ExtraConfig("HWASAN") {
		// HWASAN adds tag bytes to pointers. That's incompatible with this test -- it compares
		// pointers from unrelated stack frames to check that RP isn't growing the stack.
		skip(ALL, "test", ALL, "SkRasterPipeline_stack_rewind")
	}

	if b.MatchOs("Mac") && b.GPU("IntelHD6000") {
		// skbug.com/40038821
		match = append(match, "~^ProcessorCloneTest$")
		match = append(match, "~^GrMeshTest$")
	}

	if b.MatchOs("Mac") && b.GPU("IntelHD615") {
		// skbug.com/40038854
		match = append(match, "~^GrMeshTest$")
	}

	if b.MatchOs("Mac") && b.GPU("IntelIrisPlus") {
		// skbug.com/40038854
		match = append(match, "~^GrMeshTest$")
	}

	if b.MatchOs("Mac") && b.GPU("IntelUHDGraphics630") {
		// skbug.com/40038854
		match = append(match, "~^GrMeshTest$")
	}

	if b.ExtraConfig("Vulkan") && b.Model("GalaxyS20") {
		// skbug.com/40041601
		match = append(match, "~VkPrepareForExternalIOQueueTransitionTest")
	}
	if b.MatchExtraConfig("Graphite") {
		// skbug.com/40043905
		match = append(match, "~async_rescale_and_read")
	}

	if b.MatchExtraConfig("ColorSpaces") {
		// Here we reset the 'match' and 'skipped' strings bc the ColorSpaces job only runs
		// a very specific subset of the GMs.
		skipped = []string{}
		match = []string{}
		match = append(match, "async_rescale_and_read_dog_up")
		match = append(match, "bug6783")
		match = append(match, "colorspace")
		match = append(match, "colorspace2")
		match = append(match, "coloremoji")
		match = append(match, "composeCF")
		match = append(match, "crbug_224618")
		match = append(match, "drawlines_with_local_matrix")
		match = append(match, "gradients_interesting")
		match = append(match, "manypathatlases_2048")
		match = append(match, "custommesh_cs_uniforms")
		match = append(match, "paint_alpha_normals_rt")
		match = append(match, "runtimefunctions")
		match = append(match, "savelayer_f16")
		match = append(match, "spiral_rt")
		match = append(match, "srgb_colorfilter")
		match = append(match, "strokedlines")
		match = append(match, "sweep_tiling")
	}

	if b.MatchExtraConfig("RustPNG") {
		// TODO(b/356875275) many PNG decoding tests still fail (e.g. those with SkAndroidCodec
		// or some from DM's image source). For now, just opt-in the tests we know pass and
		// eventually remove this special handling to run all image tests.
		skipped = []string{}
		match = []string{
			"RustPngCodec",
			"RustEncodePng",
		}
	}

	if len(skipped) > 0 {
		args = append(args, "--skip")
		args = append(args, skipped...)
	}

	if len(match) > 0 {
		args = append(args, "--match")
		args = append(args, match...)
	}

	// These devices run out of memory running RAW codec tests. Do not run them in
	// parallel
	// TODO(borenet): Previously this was `'Nexus5' in bot or 'Nexus9' in bot`
	// which also matched 'Nexus5x'. I added That here to maintain the
	// existing behavior, but we should verify that it's needed.
	if b.Model("Nexus5", "Nexus5x", "Nexus9", "JioNext") {
		args = append(args, "--noRAW_threading")
	}

	if b.ExtraConfig("NativeFonts") {
		args = append(args, "--nativeFonts")
		if !b.MatchOs("Android") {
			args = append(args, "--paragraph_fonts", "extra_fonts")
			args = append(args, "--norun_paragraph_tests_needing_system_fonts")
		}
	} else {
		args = append(args, "--nonativeFonts")
	}
	if b.ExtraConfig("GDI") {
		args = append(args, "--gdi")
	}
	if b.ExtraConfig("Fontations") {
		args = append(args, "--fontations")
	}
	if b.ExtraConfig("AndroidNDKFonts") {
		args = append(args, "--androidndkfonts")
	}

	// Let's make all tasks produce verbose output by default.
	args = append(args, "--verbose")

	// See skbug.com/40033899.
	if b.ExtraConfig("AbandonGpuContext") {
		args = append(args, "--abandonGpuContext")
	}
	if b.ExtraConfig("PreAbandonGpuContext") {
		args = append(args, "--preAbandonGpuContext")
	}
	if b.ExtraConfig("ReleaseAndAbandonGpuContext") {
		args = append(args, "--releaseAndAbandonGpuContext")
	}

	if b.ExtraConfig("NeverYield") {
		args = append(args, "--neverYieldToWebGPU")
	}

	if b.ExtraConfig("FailFlushTimeCallbacks") {
		args = append(args, "--failFlushTimeCallbacks")
	}

	// Finalize the DM flags and properties.
	b.recipeProp("dm_flags", marshalJson(args))
	b.recipeProp("dm_properties", marshalJson(properties))

	// Add properties indicating which assets the task should use.
	if b.MatchExtraConfig("Lottie") {
		b.asset("lottie-samples")
		b.recipeProp("lotties", "true")
	} else if b.MatchExtraConfig("OldestSupportedSkpVersion") {
		b.recipeProp("skps", "true")
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

// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"flag"
	"fmt"
	"os"
	"runtime/pprof"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter"
	"go.skia.org/skia/bazel/exporter/interfaces"
)

var gniExportDescs = []exporter.GNIExportDesc{
	{GNI: "gn/codec.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_codec_core",
			Rules: []string{
				"//src/codec:core_hdrs",
				"//src/codec:core_srcs",
			},
		},
		{Var: "skia_codec_decode_bmp",
			Rules: []string{
				"//src/codec:decode_bmp_hdrs",
				"//src/codec:decode_bmp_srcs",
			},
		},
	}},
	{GNI: "gn/core.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_core_public",
			Rules: []string{"//include/core:public_hdrs"}},
		{Var: "skia_core_sources",
			Rules: []string{
				"//include/private:private_hdrs",
				"//include/private/base:private_hdrs",
				"//include/private/base:shared_gpu_private_hdrs",
				"//include/private/chromium:private_hdrs",
				"//include/private/chromium:shared_private_hdrs",
				"//src/base:private_hdrs",
				"//src/base:skslc_srcs",
				"//src/base:srcs",
				"//src/core:core_hdrs",
				"//src/core:core_skslc_hdrs",
				"//src/core:core_skslc_srcs",
				"//src/core:core_srcs",
				"//src/core:sksl_hdrs",
				"//src/core:sksl_srcs",
				"//src/image:core_hdrs",
				"//src/image:core_srcs",
				"//src/lazy:lazy_hdrs",
				"//src/lazy:lazy_srcs",
				"//src/opts:private_hdrs",
				"//src/shaders:shader_hdrs",
				"//src/shaders:shader_srcs",
				"//src/text:text_hdrs",
				"//src/text:text_srcs",
			}},
		{Var: "skia_pathops_public",
			Rules: []string{"//include/pathops:public_hdrs"}},
		{Var: "skia_pathops_sources",
			Rules: []string{
				"//src/pathops:pathops_hdrs",
				"//src/pathops:pathops_srcs",
			}},
		{Var: "skia_skpicture_public",
			Rules: []string{"//include/core:skpicture_public_hdrs"}},
		{Var: "skia_skpicture_sources",
			Rules: []string{
				"//src/core:skpicture_hdrs",
				"//src/core:skpicture_srcs",
				"//src/shaders:skpicture_srcs",
			}},
		{Var: "src_images_srcs",
			Rules: []string{"//src/encode:srcs"}},
		{Var: "skia_encode_srcs",
			Rules: []string{"//src/encode:srcs"}},
		{Var: "skia_encode_jpeg_srcs",
			Rules: []string{"//src/encode:jpeg_encode_srcs", "//src/encode:jpeg_encode_hdrs"}},
		{Var: "skia_encode_png_srcs",
			Rules: []string{"//src/encode:png_encode_srcs"}},
		{Var: "skia_encode_webp_srcs",
			Rules: []string{"//src/encode:webp_encode_srcs"}},
		{Var: "skia_discardable_memory_chromium",
			Rules: []string{"//include/private/chromium:discardable_memory_hdrs"}},
	},
	},
	{GNI: "gn/effects.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_effects_public",
			Rules: []string{
				"//include/effects:public_hdrs",
			}},
		{Var: "skia_effects_sources",
			Rules: []string{
				"//src/effects:effects_hdrs",
				"//src/effects:effects_srcs",
				"//src/shaders/gradients:gradient_hdrs",
				"//src/shaders/gradients:gradient_srcs",
			}}},
	},
	{GNI: "gn/effects_imagefilters.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_effects_imagefilter_public",
			Rules: []string{
				"//include/effects:public_imagefilters_hdrs",
			}},
		{Var: "skia_effects_imagefilter_sources",
			Rules: []string{
				"//src/effects/imagefilters:imagefilters_srcs",
			}}},
	},
	{GNI: "gn/pdf.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_pdf_public",
			Rules: []string{"//include/docs:public_hdrs"}},
		{Var: "skia_pdf_sources",
			Rules: []string{
				"//src/pdf:pdf_hdrs",
				"//src/pdf:pdf_srcs",
			}}},
	},
	{GNI: "gn/sksl.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_sksl_sources",
			Rules: []string{
				"//include/private:sksl_private_hdrs",
				"//include/sksl:public_hdrs",
				"//src/sksl:core_hdrs",
				"//src/sksl:core_srcs",
				"//src/sksl/analysis:analysis_hdrs",
				"//src/sksl/analysis:analysis_srcs",
				"//src/sksl/codegen:core_srcs",
				"//src/sksl/codegen:private_hdrs",
				"//src/sksl/dsl:srcs",
				"//src/sksl/dsl/priv:private_hdrs",
				"//src/sksl/dsl/priv:srcs",
				"//src/sksl/ir:ir_hdrs",
				"//src/sksl/ir:ir_srcs",
				"//src/sksl/tracing:private_hdrs",
				"//src/sksl/tracing:srcs",
				"//src/sksl/transform:transform_hdrs",
				"//src/sksl/transform:transform_srcs",
			}},
		{Var: "skia_sksl_tracing_sources",
			Rules: []string{
				"//src/sksl/tracing:enabled_hdrs",
				"//src/sksl/tracing:enabled_srcs",
			}},
		{Var: "skia_sksl_gpu_sources",
			Rules: []string{
				"//src/sksl/codegen:gpu_hdrs",
				"//src/sksl/codegen:gpu_srcs",
			}},
		{Var: "skslc_deps",
			Rules: []string{
				"//src/base:skslc_srcs",
				"//src/core:core_skslc_srcs",
				"//src/gpu/ganesh:core_skslc_srcs",
				"//src/ports:malloc",
				"//src/ports:osfile",
				"//src/utils:core_skslc_srcs",
				"//src/utils:json_srcs",
				"//src/utils:sksl_srcs",
			}}},
	},
	{GNI: "gn/sksl_tests.gni", Vars: []exporter.GNIFileListExportDesc{
		// This order was the order the original file was in. It could be alphabetized if we like.
		{Var: "sksl_error_tests", Rules: []string{"//resources/sksl:sksl_error_tests"}},
		{Var: "sksl_glsl_tests", Rules: []string{"//resources/sksl:sksl_glsl_tests"}},
		{Var: "sksl_metal_tests", Rules: []string{"//resources/sksl:sksl_metal_tests"}},
		{Var: "sksl_spirv_tests", Rules: []string{"//resources/sksl:sksl_spirv_tests"}},
		{Var: "sksl_wgsl_tests", Rules: []string{"//resources/sksl:sksl_wgsl_tests"}},
		{Var: "sksl_inverse_hyperbolic_intrinsics_tests",
			Rules: []string{"//resources/sksl:sksl_inverse_hyperbolic_intrinsics_tests"}},
		{Var: "sksl_shared_tests", Rules: []string{"//resources/sksl:sksl_shared_tests"}},
		{Var: "sksl_folding_tests", Rules: []string{"//resources/sksl:sksl_folding_tests"}},
		{Var: "sksl_inliner_tests", Rules: []string{"//resources/sksl:sksl_inliner_tests"}},
		{Var: "sksl_blend_tests", Rules: []string{"//resources/sksl:sksl_blend_tests"}},
		{Var: "sksl_settings_tests", Rules: []string{"//resources/sksl:sksl_settings_tests"}},
		{Var: "sksl_rte_tests", Rules: []string{"//resources/sksl:sksl_rte_tests"}},
		{Var: "sksl_rte_error_tests", Rules: []string{"//resources/sksl:sksl_rte_error_tests"}},
	}},
	{GNI: "gn/utils.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_utils_public",
			Rules: []string{
				"//include/utils:public_hdrs",
				"//include/utils/mac:public_hdrs"}},
		{Var: "skia_utils_chromium",
			Rules: []string{
				"//src/utils:chromium_hdrs"}},
		{Var: "skia_utils_private",
			Rules: []string{
				"//src/utils:core_hdrs",
				"//src/utils:core_skslc_hdrs",
				"//src/utils:core_skslc_srcs",
				"//src/utils:core_srcs",
				"//src/utils:json_hdrs",
				"//src/utils:json_srcs",
				"//src/utils:sksl_hdrs",
				"//src/utils:sksl_srcs",
				"//src/utils/mac:core_hdrs",
				"//src/utils/mac:core_srcs",
				"//src/utils/win:core_hdrs",
				"//src/utils/win:core_srcs",
			}}},
	},
	{GNI: "gn/xps.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_xps_public",
			Rules: []string{"//include/docs:xps_hdrs"}},
		{Var: "skia_xps_sources",
			Rules: []string{
				"//src/xps:core_hdrs",
				"//src/xps:core_srcs",
			}}},
	},
	{GNI: "gn/gpu.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_gpu_public",
			Rules: []string{
				"//include/gpu/mock:public_hdrs",
				"//include/gpu:public_hdrs",
				"//include/gpu:shared_public_hdrs",
			}},
		{Var: "skia_gpu_private",
			Rules: []string{
				"//include/private/gpu/ganesh:private_hdrs",
				"//src/gpu:core_hdrs",
				"//src/gpu/ganesh:core_hdrs",
				"//src/gpu/ganesh:core_skslc_hdrs",
				"//src/gpu/ganesh:core_skslc_srcs",
				"//src/gpu/ganesh:core_srcs",
				"//src/gpu/ganesh/effects:effects_hdrs",
				"//src/gpu/ganesh/effects:effects_srcs",
				"//src/gpu/ganesh/geometry:geometry_hdrs",
				"//src/gpu/ganesh/geometry:geometry_srcs",
				"//src/gpu/ganesh/glsl:glsl_hdrs",
				"//src/gpu/ganesh/glsl:glsl_srcs",
				"//src/gpu/ganesh/gradients:gradient_hdrs",
				"//src/gpu/ganesh/gradients:gradient_srcs",
				"//src/gpu/ganesh/mock:mock_hdrs",
				"//src/gpu/ganesh/mock:mock_srcs",
				"//src/gpu/ganesh/tessellate:tessellate_hdrs",
				"//src/gpu/ganesh/tessellate:tessellate_srcs",
				"//src/gpu/ganesh/text:private_hdrs",
				"//src/gpu/ganesh/text:srcs",
				"//src/image:gpu_hdrs",
				"//src/image:gpu_srcs",
				"//src/gpu/ganesh/ops:ops_hdrs",
				"//src/gpu/ganesh/ops:ops_srcs",
			}},
		{Var: "skia_gpu_chromium_public",
			Rules: []string{
				"//include/private/chromium:gpu_private_hdrs",
			}},
		{Var: "skia_gpu_gl_public",
			Rules: []string{
				"//include/gpu/gl:public_hdrs",
			}},
		{Var: "skia_gpu_gl_private",
			Rules: []string{
				"//src/gpu/ganesh/gl:core_hdrs",
				"//src/gpu/ganesh/gl:core_srcs",
				"//src/gpu/ganesh/gl/builders:builder_hdrs",
				"//src/gpu/ganesh/gl/builders:builder_srcs",
				"//src/gpu/gl:gl_hdrs",
			}},
		{Var: "skia_null_gpu_sources",
			Rules: []string{
				"//src/gpu/ganesh/gl:native_interface_none",
			}},
		{Var: "skia_skgpu_v1_sources",
			Rules: []string{
				"//src/gpu/ganesh/ops:ops_hdrs",
				"//src/gpu/ganesh/ops:ops_srcs",
			}},
		{Var: "skia_gpu_vk_public",
			Rules: []string{
				"//include/gpu/vk:public_hdrs",
			}},
		{Var: "skia_gpu_vk_chromium_public",
			Rules: []string{
				"//include/private/chromium:vk_chromium_hdrs",
			}},
		{Var: "skia_gpu_vk_private",
			Rules: []string{
				"//include/gpu/vk:public_hdrs",
				"//include/private/gpu/ganesh:vk_private_hdrs",
				"//src/gpu/ganesh/vk:vk_hdrs",
				"//src/gpu/ganesh/vk:vk_srcs",
			}},
		{Var: "skia_gpu_vk_chromium_private",
			Rules: []string{
				"//src/gpu/ganesh/vk:vk_chromium_srcs",
			}},
		{Var: "skia_direct3d_sources",
			Rules: []string{
				"//include/gpu/d3d:public_hdrs",
				"//include/private/gpu/ganesh:d3d_private_hdrs",
				"//src/gpu/ganesh/d3d:d3d_hdrs",
				"//src/gpu/ganesh/d3d:d3d_srcs",
			}},
		{Var: "skia_gpu_dawn_public",
			Rules: []string{
				"//include/gpu/dawn:public_hdrs",
			}},
		{Var: "skia_gpu_dawn_private",
			Rules: []string{
				"//include/private/gpu/ganesh:dawn_private_hdrs",
				"//src/gpu/ganesh/dawn:dawn_hdrs",
				"//src/gpu/ganesh/dawn:dawn_srcs",
			}},
		{Var: "skia_gpu_metal_public",
			Rules: []string{
				"//include/gpu/mtl:public_hdrs",
			}},
		{Var: "skia_gpu_metal_private",
			Rules: []string{
				"//src/gpu/ganesh/mtl:mtl_hdrs",
				"//src/gpu/ganesh/mtl:mtl_srcs",
				"//src/image:mtl_srcs",
			}},
		{Var: "skia_gpu_metal_cpp",
			Rules: []string{
				"//src/gpu/ganesh/mtl:mtl_cpp_hdrs",
			}},
		{Var: "skia_native_gpu_sources",
			Rules: []string{
				"//include/gpu/gl/egl:public_hdrs",
				"//include/gpu/gl/glx:public_hdrs",
				"//src/gpu/ganesh/gl/android:srcs",
				"//src/gpu/ganesh/gl/egl:srcs",
				"//src/gpu/ganesh/gl/glx:srcs",
				"//src/gpu/ganesh/gl/iOS:srcs",
				"//src/gpu/ganesh/gl/mac:srcs",
				"//src/gpu/ganesh/gl/win:srcs",
			}},
		{Var: "skia_shared_gpu_sources",
			Rules: []string{
				"//include/gpu:shared_public_hdrs",
				"//include/private/base:shared_gpu_private_hdrs",
				"//include/private/chromium:shared_private_hdrs",
				"//src/gpu:core_hdrs",
				"//src/gpu:core_srcs",
				"//src/gpu:shared_hdrs",
				"//src/gpu:shared_srcs",
				"//src/gpu/tessellate:tessellate_hdrs",
				"//src/gpu/tessellate:tessellate_srcs",
				"//src/text/gpu:gpu_hdrs",
				"//src/text/gpu:gpu_srcs",
			}},
		{Var: "skia_shared_vk_sources",
			Rules: []string{
				"//include/gpu/vk:shared_public_hdrs",
				"//include/private/gpu/vk:private_hdrs",
				"//src/gpu/vk:vk_hdrs",
				"//src/gpu/vk:vk_srcs"}},
		{Var: "skia_shared_mtl_sources",
			Rules: []string{
				"//include/gpu/mtl:shared_public_hdrs",
				"//src/gpu/mtl:shared_srcs",
			}},
		{Var: "skia_piet_gpu_sources",
			Rules: []string{
				"//src/gpu/piet:piet_hdrs",
				"//src/gpu/piet:piet_srcs",
			}},
	}},
	{GNI: "modules/svg/svg.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_svg_public",
			Rules: []string{"//modules/svg/include:public_hdrs"}},
		{Var: "skia_svg_sources",
			Rules: []string{
				"//modules/svg/src:private_hdrs",
				"//modules/svg/src:srcs",
			}},
	}},
	{GNI: "modules/skparagraph/skparagraph.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skparagraph_public",
			Rules: []string{
				"//modules/skparagraph/include:hdrs",
				"//modules/skparagraph/utils:utils_hdrs"}},
		{Var: "skparagraph_sources",
			Rules: []string{
				"//modules/skparagraph/src:srcs",
				"//modules/skparagraph/utils:utils_srcs"}},
		{Var: "skparagraph_utils",
			Rules: []string{
				"//modules/skparagraph/utils:utils_hdrs",
				"//modules/skparagraph/utils:utils_srcs",
			}},
		{Var: "skparagraph_tests",
			Rules: []string{
				"//modules/skparagraph/tests:tests_hdrs",
				"//modules/skparagraph/tests:tests_srcs",
			}},
	}},
	{GNI: "modules/skresources/skresources.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_skresources_public",
			Rules: []string{"//modules/skresources/include:hdrs"}},
		{Var: "skia_skresources_sources",
			Rules: []string{"//modules/skresources/src:srcs"}},
	}},
	{GNI: "modules/skshaper/skshaper.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_shaper_public",
			Rules: []string{"//modules/skshaper/include:hdrs"}},
		{Var: "skia_shaper_primitive_sources",
			Rules: []string{"//modules/skshaper/src:base_srcs"}},
		{Var: "skia_shaper_harfbuzz_sources",
			Rules: []string{"//modules/skshaper/src:harfbuzz_srcs"}},
		{Var: "skia_shaper_coretext_sources",
			Rules: []string{"//modules/skshaper/src:coretext_srcs"}},
		{Var: "skia_shaper_tests",
			Rules: []string{"//modules/skshaper/tests:tests_srcs"}},
	}},
	{GNI: "modules/skunicode/skunicode.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_unicode_public",
			Rules: []string{"//modules/skunicode/include:hdrs"}},
		{Var: "skia_unicode_sources",
			Rules: []string{"//modules/skunicode/src:srcs"}},
		{Var: "skia_unicode_icu_sources",
			Rules: []string{"//modules/skunicode/src:icu_srcs"}},
		{Var: "skia_unicode_client_icu_sources",
			Rules: []string{"//modules/skunicode/src:client_srcs"}},
		{Var: "skia_unicode_builtin_icu_sources",
			Rules: []string{"//modules/skunicode/src:builtin_srcs"}},
		{Var: "skia_unicode_runtime_icu_sources",
			Rules: []string{"//modules/skunicode/src:runtime_srcs"}},
	}},
	{GNI: "modules/sksg/sksg.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_sksg_sources",
			Rules: []string{"//modules/sksg/src:srcs"}},
	}},
	{GNI: "modules/skottie/skottie.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_skottie_public",
			Rules: []string{"//modules/skottie/include:hdrs"}},
		{Var: "skia_skottie_sources",
			Rules: []string{
				"//modules/skottie/src:srcs",
				"//modules/skottie/src/animator:srcs",
				"//modules/skottie/src/effects:srcs",
				"//modules/skottie/src/layers:srcs",
				"//modules/skottie/src/layers/shapelayer:srcs",
				"//modules/skottie/src/text:srcs",
			}},
	}},
	{GNI: "modules/skcms/skcms.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skcms_public_headers",
			Rules: []string{"//modules/skcms:public_hdrs"}},
		{Var: "skcms_sources",
			Rules: []string{
				"//modules/skcms:srcs",
				"//modules/skcms:textual_hdrs",
			}},
	}},
}

const (
	unknownErr    = 1
	invalidArgErr = 2
	exportErr     = 3
	verifyErr     = 4
	profilerErr   = 5
)

type fileSystem struct {
	workspaceDir string
	outFormat    string
	openFiles    []*os.File
}

func (fs *fileSystem) OpenFile(path string) (interfaces.Writer, error) {
	f, err := os.Create(path)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	fs.openFiles = append(fs.openFiles, f)
	return f, nil
}

func (fs *fileSystem) ReadFile(filename string) ([]byte, error) {
	return os.ReadFile(filename)
}

func (fs *fileSystem) Shutdown() {
	for _, f := range fs.openFiles {
		f.Close() // Ignore error.
	}
}

// Make sure fileSystem fulfills the FileSystem interface.
var _ interfaces.FileSystem = (*fileSystem)(nil)

func createExporter(projName, cmakeFileName string, fs *fileSystem) interfaces.Exporter {
	if fs.outFormat == "cmake" {
		return exporter.NewCMakeExporter(projName, fs.workspaceDir, cmakeFileName, fs)
	}
	params := exporter.GNIExporterParams{
		WorkspaceDir: fs.workspaceDir,
		ExportDescs:  gniExportDescs,
	}
	return exporter.NewGNIExporter(params, fs)
}

func doExport(qr interfaces.QueryCommand, exp interfaces.Exporter, outFormat string) {
	err := exp.Export(qr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error exporting to %s: %v\n", outFormat, err)
		os.Exit(exportErr)
	}
}

func main() {
	var (
		queryRules    = common.NewMultiStringFlag("rule", nil, "Bazel rule (may be repeated).")
		outFormat     = flag.String("output_format", "", "Desired output format. One of cmake or gni.")
		cmakeFileName = flag.String("out", "CMakeLists.txt", "CMake output file")
		projName      = flag.String("proj_name", "", "CMake project name")
		cpuprofile    = flag.String("cpuprofile", "", "write cpu profile to file")
	)
	flag.Parse()
	if *outFormat != "cmake" && *outFormat != "gni" {
		if *outFormat == "" {
			fmt.Fprintln(os.Stderr, "Output format required")
		} else {
			fmt.Fprintf(os.Stderr, "Incorrect output format: \"%s\"\n", *outFormat)
		}
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(invalidArgErr)
	}
	if *cmakeFileName == "" || *projName == "" {
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(invalidArgErr)
	}
	workspaceDir, err := os.Getwd()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error getting pwd: %v", err)
		os.Exit(unknownErr)
	}
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			fmt.Fprintf(os.Stderr, "unable to create %q: %v\n", *cpuprofile, err)
			os.Exit(profilerErr)
		}
		defer f.Close()
		if err = pprof.StartCPUProfile(f); err != nil {
			fmt.Fprintf(os.Stderr, "error starting CPU profile: %v\n", err)
			os.Exit(profilerErr)
		}
		defer pprof.StopCPUProfile()
	}
	var qcmd *exporter.BazelQueryCommand
	switch *outFormat {
	case "gni":
		qcmd = exporter.NewBazelGNIQueryCommand(*queryRules, workspaceDir)
	case "cmake":
		qcmd = exporter.NewBazelCMakeQueryCommand(*queryRules, workspaceDir)
	}
	fs := fileSystem{workspaceDir: workspaceDir, outFormat: *outFormat}
	defer fs.Shutdown()
	var exp interfaces.Exporter = createExporter(*projName, *cmakeFileName, &fs)
	doExport(qcmd, exp, *outFormat)
}

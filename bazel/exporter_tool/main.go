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
		{Var: "skia_codec_public",
			Rules: []string{
				"//include/codec:any_codec_hdrs",
				"//include/codec:core_hdrs",
			},
		},
		{Var: "skia_codec_shared",
			Rules: []string{
				"//src/codec:any_decoder",
				"//include/codec:any_codec_hdrs",
			},
		},
		{Var: "skia_codec_decode_bmp",
			Rules: []string{
				"//src/codec:bmp_decode",
				"//src/codec:wbmp_decode",
			},
		},
		{Var: "skia_codec_xmp",
			Rules: []string{
				"//src/codec:xmp_srcs",
			},
		},
		{Var: "skia_codec_jpeg_xmp",
			Rules: []string{
				"//src/codec:jpeg_xmp_hdrs",
				"//src/codec:jpeg_xmp_srcs",
			},
		},
		{Var: "skia_codec_png_base",
			Rules: []string{
				"//src/codec:png_codec_base_hdrs",
				"//src/codec:png_codec_base_srcs",
			},
		},
		{Var: "skia_codec_libpng_srcs",
			Rules: []string{
				"//src/codec:buffet_libpng_srcs",
				"//src/codec:common_libpng_srcs",
			},
		},
		{Var: "skia_codec_rust_png_public",
			Rules: []string{
				"//experimental/rust_png/decoder:hdrs",
				"//include/encode:rust_png_public_hdrs",
			},
		},
		{Var: "skia_codec_rust_png",
			Rules: []string{
				"//experimental/rust_png/decoder:srcs",
				"//experimental/rust_png/ffi:utils",
			},
		},
		{Var: "skia_codec_rust_png_ffi_rs_srcs",
			Rules: []string{
				"//experimental/rust_png/ffi:rs_srcs",
			},
		},
		{Var: "skia_codec_rust_png_ffi_cxx_bridge_srcs",
			Rules: []string{
				"//experimental/rust_png/ffi:cxx_bridge_srcs",
			},
		},
		{Var: "skia_codec_rust_png_ffi_cpp_hdrs",
			Rules: []string{
				"//experimental/rust_png/ffi:ffi_cpp",
			},
		},
	}},
	{GNI: "gn/core.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_core_public",
			Rules: []string{
				"//include/core:core_hdrs",
			}},
		{Var: "skia_core_sources",
			Rules: []string{
				"//include/private:core_priv_hdrs",
				"//include/private/base:private_hdrs",
				"//include/private/base:shared_gpu_private_hdrs",
				"//include/private/chromium:core_hdrs",
				"//include/private/chromium:shared_private_hdrs",
				"//src/base:private_hdrs",
				"//src/base:skslc_srcs",
				"//src/base:srcs",
				"//src/capture:capture_hdrs",
				"//src/capture:capture_srcs",
				"//src/core:core_priv_hdrs",
				"//src/core:core_priv_srcs",
				"//src/core:core_srcs",
				"//src/core:textual_hdrs",
				"//src/image:image_hdrs",
				"//src/image:image_srcs",
				"//src/lazy:lazy_hdrs",
				"//src/lazy:lazy_srcs",
				"//src/opts:textual_hdrs",
				"//src/shaders:shader_hdrs",
				"//src/shaders:shader_srcs",
				"//src/text:text_hdrs",
				"//src/text:text_srcs",
			}},
		{Var: "skia_encode_public",
			Rules: []string{"//include/encode:encode_hdrs"}},
		{Var: "skia_encode_srcs",
			Rules: []string{
				"//src/encode:encoder_common",
				"//src/encode:icc_support",
			}},
		{Var: "skia_encode_jpeg_public",
			Rules: []string{"//include/encode:jpeg_hdrs"}},
		{Var: "skia_encode_jpeg_srcs",
			Rules: []string{"//src/encode:jpeg_encode_srcs",
				"//src/encode:jpeg_encode_hdrs"}},
		{Var: "skia_encode_rust_png_public",
			Rules: []string{
				"//experimental/rust_png/encoder:hdrs",
				"//include/codec:rust_png_public_hdrs",
			}},
		{Var: "skia_encode_rust_png_srcs",
			Rules: []string{
				"//experimental/rust_png/encoder:srcs",
				"//experimental/rust_png/ffi:utils",
			}},
		{Var: "skia_encode_png_base",
			Rules: []string{
				"//src/encode:png_encode_base_srcs",
				"//src/encode:png_encode_base_hdrs",
			}},
		{Var: "skia_encode_libpng_srcs",
			Rules: []string{
				"//src/encode:png_encode_srcs",
				"//src/encode:png_encode_hdrs",
			}},
		// TODO(https://crbug.com/381900683): Rename this list.
		{Var: "skia_encode_png_public",
			Rules: []string{"//include/encode:png_hdrs"}},
		{Var: "skia_encode_webp_public",
			Rules: []string{"//include/encode:webp_hdrs"}},
		{Var: "skia_encode_webp_srcs",
			Rules: []string{"//src/encode:webp_encode_srcs"}},
		{Var: "skia_no_encode_jpeg_srcs",
			Rules: []string{"//src/encode:no_jpeg_encode_srcs"}},
		{Var: "skia_no_encode_png_srcs",
			Rules: []string{"//src/encode:no_png_encode_srcs"}},
		{Var: "skia_no_encode_webp_srcs",
			Rules: []string{"//src/encode:no_webp_encode_srcs"}},
		{Var: "skia_discardable_memory_chromium",
			Rules: []string{"//include/private/chromium:discardable_memory_hdrs"}},
		{Var: "skia_no_slug_srcs",
			Rules: []string{}},
	},
	},
	{GNI: "gn/effects.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_effects_public",
			Rules: []string{
				"//include/effects:core_hdrs",
			}},
		{Var: "skia_effects_sources",
			Rules: []string{
				"//src/effects:effects_hdrs",
				"//src/effects:effects_srcs",
				"//src/shaders/gradients:gradient_hdrs",
				"//src/shaders/gradients:gradient_srcs",
			}},
		{Var: "skia_colorfilters_sources",
			Rules: []string{
				"//src/effects/colorfilters:colorfilter_srcs",
				"//src/effects/colorfilters:colorfilter_hdrs",
			}},
	}},
	{GNI: "gn/effects_imagefilters.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_effects_imagefilter_public",
			Rules: []string{
				"//include/effects:public_imagefilters_hdrs",
			}},
		{Var: "skia_effects_imagefilter_sources",
			Rules: []string{
				"//src/effects/imagefilters:srcs",
			}}},
	},
	{GNI: "gn/graphite.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_graphite_public",
			Rules: []string{
				"//include/gpu/graphite:public_hdrs",
			}},
		{Var: "skia_graphite_sources",
			Rules: []string{
				"//src/gpu/graphite/compute:core_hdrs",
				"//src/gpu/graphite/compute:core_srcs",
				"//src/gpu/graphite/geom:core_hdrs",
				"//src/gpu/graphite/geom:core_srcs",
				"//src/gpu/graphite/render:core_hdrs",
				"//src/gpu/graphite/render:core_srcs",
				"//src/gpu/graphite/task:core_hdrs",
				"//src/gpu/graphite/task:core_srcs",
				"//src/gpu/graphite/text:core_hdrs",
				"//src/gpu/graphite/text:core_srcs",
				"//src/gpu/graphite:_graphite_hdrs",
				"//src/gpu/graphite:_graphite_srcs",
			}},
		{Var: "skia_graphite_android_private",
			Rules: []string{
				"//src/gpu/graphite/surface:android_srcs",
				"//include/android:graphite_android_hdrs",
			}},
		{Var: "skia_graphite_dawn_public",
			Rules: []string{
				"//include/gpu/graphite/dawn:public_hdrs",
			}},
		{Var: "skia_graphite_dawn_sources",
			Rules: []string{
				"//src/gpu/graphite/dawn:_dawn_hdrs",
				"//src/gpu/graphite/dawn:_dawn_srcs",
			}},
		{Var: "skia_graphite_mtl_public",
			Rules: []string{
				"//include/gpu/graphite/mtl:public_hdrs",
			}},
		{Var: "skia_graphite_mtl_sources",
			Rules: []string{
				"//src/gpu/graphite/mtl:mtl_hdrs",
				"//src/gpu/graphite/mtl:mtl_srcs",
			}},
		{Var: "skia_graphite_vk_public",
			Rules: []string{
				"//include/gpu/graphite/vk:public_hdrs",
			}},
		{Var: "skia_graphite_vk_precompile_public",
			Rules: []string{
				"//include/gpu/graphite/vk/precompile:public_hdrs",
			}},
		{Var: "skia_graphite_vk_precompile_sources",
			Rules: []string{
				"//src/gpu/graphite/vk/precompile:vk_precompile_srcs",
			}},
		{Var: "skia_graphite_vk_sources",
			Rules: []string{
				"//src/gpu/graphite/vk:vk_hdrs",
				"//src/gpu/graphite/vk:vk_srcs",
			}},
		{Var: "skia_graphite_vello_sources",
			Rules: []string{
				"//src/gpu/graphite/compute:vello_hdrs",
				"//src/gpu/graphite/compute:vello_srcs",
			}},
		{Var: "skia_graphite_precompile_public",
			Rules: []string{
				"//include/gpu/graphite/precompile:public_hdrs",
			}},
		{Var: "skia_graphite_precompile_sources",
			Rules: []string{
				"//src/gpu/graphite/precompile:_precompile_hdrs",
				"//src/gpu/graphite/precompile:_precompile_srcs",
				"//src/gpu/graphite:precompile_srcs",
			}},
	}},
	{GNI: "gn/pathops.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_pathops_public",
			Rules: []string{
				"//include/pathops:public_hdrs",
			}},
		{Var: "skia_pathops_sources",
			Rules: []string{
				"//src/pathops:_pathops_hdrs",
				"//src/pathops:_pathops_srcs",
			}}},
	},
	{GNI: "gn/ports.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_ports_sources",
			Rules: []string{
				"//src/ports:global_init",
				"//src/ports:osfile",
			}},
		{Var: "skia_ports_freetype_sources",
			Rules: []string{
				"//src/ports:freetype_support",
			}},
		{Var: "skia_ports_fontmgr_android_parser_sources",
			Rules: []string{
				"//src/ports:fontmgr_android_parser",
			}},
		{Var: "skia_ports_fontmgr_android_public",
			Rules: []string{
				"//include/ports:android_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_android_sources",
			Rules: []string{
				"//src/ports:fontmgr_android",
			}},
		{Var: "skia_ports_fontmgr_android_ndk_public",
			Rules: []string{
				"//include/ports:android_ndk_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_android_ndk_sources",
			Rules: []string{
				"//src/ports:fontmgr_android_ndk",
			}},
		{Var: "skia_ports_fontmgr_custom_sources",
			Rules: []string{
				"//src/ports:fontmgr_custom",
			}},
		{Var: "skia_ports_fontmgr_coretext_public",
			Rules: []string{
				"//include/ports:mac_typeface",
				"//include/ports:coretext_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_coretext_sources",
			Rules: []string{
				"//src/ports:fontmgr_coretext",
				"//src/ports:typeface_mac_srcs",
				"//src/ports:typeface_mac_hdrs",
			}},
		{Var: "skia_ports_fontmgr_directory_public",
			Rules: []string{
				"//include/ports:directory_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_directory_sources",
			Rules: []string{
				"//src/ports:fontmgr_directory_freetype",
			}},
		{Var: "skia_ports_fontmgr_embedded_public",
			Rules: []string{
				"//include/ports:data_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_embedded_sources",
			Rules: []string{
				"//src/ports:fontmgr_data_freetype",
			}},
		{Var: "skia_ports_fontmgr_empty_public",
			Rules: []string{
				"//include/ports:empty_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_empty_sources",
			Rules: []string{
				"//src/ports:fontmgr_empty_freetype",
			}},
		{Var: "skia_ports_fontmgr_fontconfig_public",
			Rules: []string{
				"//include/ports:fontconfig_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_fontconfig_sources",
			Rules: []string{
				"//src/ports:fontmgr_fontconfig",
			}},
		{Var: "skia_ports_fontmgr_fontations_public",
			Rules: []string{
				"//include/ports:fontmgr_fontations_hdrs",
			}},
		{Var: "skia_ports_fontmgr_fontations_sources",
			Rules: []string{
				"//src/ports:fontmgr_fontations_empty_srcs",
			}},
		{Var: "skia_ports_fontmgr_fuchsia_public",
			Rules: []string{
				"//include/ports:fuchsia_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fontmgr_fuchsia_sources",
			Rules: []string{
				"//src/ports:fontmgr_fuchsia_srcs",
			}},
		{Var: "skia_ports_typeface_fontations_sources",
			Rules: []string{
				"//src/ports:typeface_fontations_hdrs",
				"//src/ports:typeface_fontations_srcs",
			}},
		{Var: "skia_ports_fontations_bridge_rust_side_sources",
			Rules: []string{
				"//src/ports/fontations:bridge_rust_side_srcs",
			}},
		{Var: "skia_ports_typeface_proxy_sources",
			Rules: []string{
				"//src/ports:typeface_proxy_hdrs",
				"//src/ports:typeface_proxy_srcs",
			}},
		{Var: "skia_ports_windows_sources",
			Rules: []string{
				"//src/ports:osfile_win",
			}},
		{Var: "skia_ports_windows_fonts_public",
			Rules: []string{
				"//include/ports:typeface_windows_hdrs",
			}},
		{Var: "skia_ports_windows_fonts_sources",
			Rules: []string{
				"//src/ports:windows_fonts_srcs",
			}},
		{Var: "skia_ports_fci_public",
			Rules: []string{
				"//include/ports:fci_fontmgr_hdrs",
			}},
		{Var: "skia_ports_fci_sources",
			Rules: []string{
				"//src/ports:fontconfig_interface_srcs",
			}},
		{Var: "skia_ports_fonthost_win_sources",
			Rules: []string{
				"//src/ports:fonthost_win_srcs",
			}},
	}},
	{GNI: "gn/pdf.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_pdf_public",
			Rules: []string{"//include/docs:pdf_hdrs"}},
		{Var: "skia_pdf_sources",
			Rules: []string{
				"//src/pdf:_pdf_hdrs",
				"//src/pdf:_pdf_srcs",
			}},
		{Var: "skia_pdf_jpeg_public",
			Rules: []string{"//include/docs:pdf_jpeg_hdrs"}},
	}},
	{GNI: "gn/sksl.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_sksl_core_sources",
			Rules: []string{
				"//include/sksl:core_hdrs",
				"//src/sksl/analysis:analysis_hdrs",
				"//src/sksl/analysis:analysis_srcs",
				"//src/sksl/codegen:rasterpipeline_hdrs",
				"//src/sksl/codegen:rasterpipeline_srcs",
				"//src/sksl/ir:ir_hdrs",
				"//src/sksl/ir:ir_srcs",
				"//src/sksl/tracing:private_hdrs",
				"//src/sksl/tracing:skopts_hdrs",
				"//src/sksl/tracing:srcs",
				"//src/sksl/transform:transform_hdrs",
				"//src/sksl/transform:transform_srcs",
				"//src/sksl:sksl_hdrs",
				"//src/sksl:sksl_srcs",
			}},
		{Var: "skia_sksl_core_module_sources",
			Rules: []string{
				"//src/sksl:sksl_default_module_srcs",
				"//src/sksl:sksl_graphite_modules_hdrs",
			}},
		{Var: "skia_sksl_graphite_modules_sources",
			Rules: []string{
				"//src/sksl:sksl_graphite_modules_srcs",
				"//src/sksl:sksl_graphite_modules_hdrs",
			}},
		{Var: "skia_sksl_tracing_sources",
			Rules: []string{
				"//src/sksl/tracing:enabled_hdrs",
				"//src/sksl/tracing:enabled_srcs",
			}},
		{Var: "skia_sksl_pipeline_sources",
			Rules: []string{
				"//src/sksl/codegen:gpu",
			}},
		{Var: "skia_sksl_codegen_sources",
			Rules: []string{
				"//src/sksl/codegen:codegen_shared_exported",
				"//src/sksl/codegen:codegen_shared_priv",
				"//src/sksl/codegen:glsl",
				"//src/sksl/codegen:metal",
				"//src/sksl/codegen:spirv",
				"//src/sksl/codegen:wgsl",
			}},
		{Var: "skia_sksl_hlsl_sources",
			Rules: []string{
				"//src/sksl/codegen:hlsl",
			}},
		{Var: "skia_sksl_validate_spirv_sources",
			Rules: []string{
				"//src/sksl/codegen:spirv_validator",
			}},
		{Var: "skia_sksl_validate_wgsl_sources",
			Rules: []string{
				"//src/sksl/codegen:wgsl_validator",
			}},
		{Var: "skslc_deps",
			Rules: []string{
				"//src/base:skslc_srcs",
				"//src/core:core_skslc_hdrs",
				"//src/core:core_skslc_srcs",
				"//src/gpu/ganesh:core_skslc_hdrs",
				"//src/gpu/ganesh:core_skslc_srcs",
				"//src/ports:malloc",
				"//src/ports:osfile",
				"//src/sksl:sksl_skslc_module_srcs",
				"//src/utils:utils_skslc_hdrs",
				"//src/utils:utils_skslc_srcs",
			}}},
	},
	{GNI: "gn/sksl_tests.gni", Vars: []exporter.GNIFileListExportDesc{
		// This order was the order the original file was in. It could be alphabetized if we like.
		{Var: "sksl_error_tests", Rules: []string{"//resources/sksl:sksl_error_tests"}},
		{Var: "sksl_glsl_tests", Rules: []string{"//resources/sksl:sksl_glsl_tests"}},
		{Var: "sksl_mesh_tests", Rules: []string{"//resources/sksl:sksl_mesh_tests"}},
		{Var: "sksl_mesh_error_tests", Rules: []string{"//resources/sksl:sksl_mesh_error_tests"}},
		{Var: "sksl_metal_tests", Rules: []string{"//resources/sksl:sksl_metal_tests"}},
		{Var: "sksl_spirv_tests", Rules: []string{"//resources/sksl:sksl_spirv_tests"}},
		{Var: "sksl_wgsl_tests", Rules: []string{"//resources/sksl:sksl_wgsl_tests"}},
		{Var: "sksl_shared_tests", Rules: []string{"//resources/sksl:sksl_shared_tests"}},
		{Var: "sksl_compute_tests", Rules: []string{"//resources/sksl:sksl_compute_tests"}},
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
				"//include/utils:core_hdrs",
				"//include/utils/mac:public_hdrs"}},
		{Var: "skia_utils_chromium",
			Rules: []string{
				"//include/docs:multi_picture_document_hdrs",
			}},
		{Var: "skia_utils_private",
			Rules: []string{
				"//src/utils/mac:mac_utils_priv",
				"//src/utils/mac:mac_utils",
				"//src/utils/win:core_hdrs",
				"//src/utils/win:core_srcs",
				"//src/utils:core_priv_hdrs",
				"//src/utils:core_srcs",
				"//src/utils:char_to_glyphcache",
				"//src/utils:canvas_state_utils",
				"//src/utils:multi_picture_document",
				"//src/utils:clip_stack_utils",
				"//src/utils:float_to_decimal",
				"//src/utils:utils_skslc_hdrs",
				"//src/utils:utils_skslc_srcs",
			}},
		{Var: "skia_clipstack_utils_sources",
			Rules: []string{
				"//src/utils:clip_stack_utils",
			}},
	},
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
	{GNI: "gn/xml.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_xml_sources",
			Rules: []string{
				"//src/xml:_xml_hdrs",
				"//src/xml:_xml_srcs",
			}}},
	},
	{GNI: "gn/gpu.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_gpu_public",
			Rules: []string{
				"//include/gpu/ganesh/mock:public_hdrs",
				"//include/gpu:shared_gpu_hdrs",
				"//include/gpu/ganesh:ganesh_hdrs",
			}},
		{Var: "skia_ganesh_private",
			Rules: []string{
				"//include/private/gpu/ganesh:private_hdrs",
				"//src/gpu/ganesh/effects:effects_hdrs",
				"//src/gpu/ganesh/effects:effects_srcs",
				"//src/gpu/ganesh/geometry:geometry_hdrs",
				"//src/gpu/ganesh/geometry:geometry_srcs",
				"//src/gpu/ganesh/glsl:glsl_hdrs",
				"//src/gpu/ganesh/glsl:glsl_srcs",
				"//src/gpu/ganesh/gradients:gradient_hdrs",
				"//src/gpu/ganesh/gradients:gradient_srcs",
				"//src/gpu/ganesh/image:image_hdrs",
				"//src/gpu/ganesh/image:image_srcs",
				"//src/gpu/ganesh/mock:mock_hdrs",
				"//src/gpu/ganesh/mock:mock_srcs",
				"//src/gpu/ganesh/ops:ops_hdrs",
				"//src/gpu/ganesh/ops:ops_srcs",
				"//src/gpu/ganesh/surface:surface_srcs",
				"//src/gpu/ganesh/surface:surface_srcs",
				"//src/gpu/ganesh/tessellate:tessellate_hdrs",
				"//src/gpu/ganesh/tessellate:tessellate_srcs",
				"//src/gpu/ganesh/text:private_hdrs",
				"//src/gpu/ganesh/text:srcs",
				"//src/gpu/ganesh:core_hdrs",
				"//src/gpu/ganesh:core_skslc_hdrs",
				"//src/gpu/ganesh:core_skslc_srcs",
				"//src/gpu/ganesh:core_srcs",
			}},
		{Var: "skia_gpu_android_private",
			Rules: []string{
				"//src/gpu/ganesh/surface:android_srcs",
				"//src/gpu/ganesh:android_srcs",
				"//src/image:android_srcs",
			}},
		{Var: "skia_gpu_chromium_public",
			Rules: []string{
				"//include/private/chromium:ganesh_private_hdrs",
			}},
		{Var: "skia_gpu_gl_public",
			Rules: []string{
				"//include/gpu/ganesh/gl:public_hdrs",
			}},
		{Var: "skia_gpu_gl_private",
			Rules: []string{
				"//src/gpu/ganesh/gl:core_hdrs",
				"//src/gpu/ganesh/gl:core_srcs",
				"//src/gpu/ganesh/gl/builders:builder_hdrs",
				"//src/gpu/ganesh/gl/builders:builder_srcs",
			}},
		{Var: "skia_android_gl_sources",
			Rules: []string{
				"//src/gpu/ganesh/gl:android_srcs",
			}},
		{Var: "skia_null_gpu_sources",
			Rules: []string{
				"//src/gpu/ganesh/gl:native_interface_none",
			}},
		{Var: "skia_gpu_vk_public",
			Rules: []string{
				"//include/gpu/vk:shared_public_hdrs",
				"//include/gpu/ganesh/vk:public_hdrs",
			}},
		{Var: "skia_gpu_vk_chromium_public",
			Rules: []string{
				"//include/private/chromium:vk_ganesh_hdrs",
			}},
		{Var: "skia_gpu_vk_private",
			Rules: []string{
				"//src/gpu/ganesh/vk:vk_hdrs",
				"//src/gpu/ganesh/vk:vk_srcs",
			}},
		{Var: "skia_gpu_vk_android_private",
			Rules: []string{
				"//src/gpu/ganesh/vk:android_srcs",
			}},
		{Var: "skia_gpu_vk_chromium_private",
			Rules: []string{
				"//src/gpu/ganesh/vk:vk_chromium_srcs",
			}},
		{Var: "skia_direct3d_sources",
			Rules: []string{
				"//include/private/gpu/ganesh:d3d_private_hdrs",
				"//src/gpu/ganesh/d3d:d3d_hdrs",
				"//src/gpu/ganesh/d3d:d3d_srcs",
			}},
		{Var: "skia_gpu_metal_public",
			Rules: []string{
				"//include/gpu/ganesh/mtl:public_hdrs",
			}},
		{Var: "skia_gpu_metal_private",
			Rules: []string{
				"//src/gpu/ganesh/surface:mtl_objc_srcs",
				"//src/gpu/ganesh/mtl:mtl_hdrs",
				"//src/gpu/ganesh/mtl:mtl_srcs",
			}},
		{Var: "skia_gpu_metal_cpp",
			Rules: []string{
				"//src/gpu/ganesh/mtl:mtl_cpp_hdrs",
			}},
		{Var: "skia_shared_gpu_sources",
			Rules: []string{
				"//include/gpu:shared_gpu_hdrs",
				"//include/private/base:shared_gpu_private_hdrs",
				"//include/private/chromium:shared_private_hdrs",
				"//src/gpu:shared_hdrs",
				"//src/gpu:shared_srcs",
				"//src/gpu/tessellate:tessellate_hdrs",
				"//src/gpu/tessellate:tessellate_srcs",
				"//src/text/gpu:gpu_hdrs",
				"//src/text/gpu:gpu_srcs",
			}},
		{Var: "skia_shared_vk_sources",
			Rules: []string{
				"//include/private/gpu/vk:private_hdrs",
				"//src/gpu/vk:_vk_hdrs",
				"//src/gpu/vk:_vk_srcs"}},
		{Var: "skia_vma_sources",
			Rules: []string{
				"//src/gpu/vk/vulkanmemoryallocator:vma_srcs",
			}},
		{Var: "skia_shared_mtl_sources",
			Rules: []string{
				"//include/gpu/mtl:public_hdrs",
				"//src/gpu/mtl:mtl_hdrs",
				"//src/gpu/mtl:mtl_srcs",
			}},
		{Var: "skia_shared_android_sources",
			Rules: []string{
				"//src/gpu/android:shared_android_srcs",
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
	{GNI: "modules/bentleyottmann/bentleyottmann.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "bentleyottmann_public",
			Rules: []string{
				"//modules/bentleyottmann/include:hdrs",
			}},
		{Var: "bentleyottmann_sources",
			Rules: []string{
				"//modules/bentleyottmann/src:srcs",
			}},
		{Var: "bentleyottmann_tests",
			Rules: []string{
				"//modules/bentleyottmann/tests:tests",
			}},
	}},
	{GNI: "modules/skparagraph/skparagraph.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skparagraph_core_public",
			Rules: []string{
				"//modules/skparagraph/include:hdrs"}},
		{Var: "skparagraph_core_sources",
			Rules: []string{
				"//modules/skparagraph/src:srcs"}},
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
		// TODO(kjlubick) remove after updating flutter
		{Var: "skparagraph_public",
			Rules: []string{
				"//modules/skparagraph/include:hdrs",
				"//modules/skparagraph/utils:utils_hdrs"}},
		{Var: "skparagraph_sources",
			Rules: []string{
				"//modules/skparagraph/src:srcs",
				"//modules/skparagraph/utils:utils_srcs"}},
	}},
	{GNI: "modules/skresources/skresources.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_skresources_public",
			Rules: []string{"//modules/skresources/include:hdrs"}},
		{Var: "skia_skresources_sources",
			Rules: []string{"//modules/skresources/src:srcs"}},
	}},
	{GNI: "modules/skshaper/skshaper.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_shaper_public",
			Rules: []string{"//modules/skshaper/include:hdrs",
				"//modules/skshaper/utils:core_hdrs",
			}},
		{Var: "skia_shaper_primitive_sources",
			Rules: []string{"//modules/skshaper/src:base_srcs"}},
		{Var: "skia_shaper_harfbuzz_sources",
			Rules: []string{"//modules/skshaper/src:harfbuzz_srcs"}},
		{Var: "skia_shaper_skunicode_sources",
			Rules: []string{"//modules/skshaper/src:skunicode_srcs"}},
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
		{Var: "skia_unicode_icu_bidi_sources",
			Rules: []string{"//modules/skunicode/src:icu_bidi_srcs"}},
		{Var: "skia_unicode_bidi_full_sources",
			Rules: []string{"//modules/skunicode/src:bidi_full_srcs"}},
		{Var: "skia_unicode_bidi_subset_sources",
			Rules: []string{"//modules/skunicode/src:bidi_subset_srcs"}},
		{Var: "skia_unicode_icu4x_sources",
			Rules: []string{"//modules/skunicode/src:icu4x_srcs"}},
		{Var: "skia_unicode_client_icu_sources",
			Rules: []string{"//modules/skunicode/src:client_srcs"}},
		{Var: "skia_unicode_bidi_sources",
			Rules: []string{"//modules/skunicode/src:bidi_srcs"}},
		{Var: "skia_unicode_builtin_icu_sources",
			Rules: []string{"//modules/skunicode/src:builtin_srcs"}},
		{Var: "skia_unicode_runtime_icu_sources",
			Rules: []string{"//modules/skunicode/src:runtime_srcs"}},
		{Var: "skia_unicode_libgrapheme_sources",
			Rules: []string{"//modules/skunicode/src:libgrapheme_srcs"}},
		{Var: "skia_unicode_tests",
			Rules: []string{"//modules/skunicode/tests:tests"}},
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
				"//modules/skottie/src:core_hdrs",
				"//modules/skottie/src:core_srcs",
				"//modules/skottie/src/animator:srcs",
				"//modules/skottie/src/effects:srcs",
				"//modules/skottie/src/layers:srcs",
				"//modules/skottie/src/layers/shapelayer:srcs",
				"//modules/skottie/src/text:text_hdrs",
				"//modules/skottie/src/text:text_srcs",
				"//modules/skottie/src/text:text_shaper_srcs",
			}},
	}},
	{GNI: "modules/skcms/skcms.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skcms_public_headers",
			Rules: []string{"//modules/skcms:public_hdrs"}},
		{Var: "skcms_public",
			Rules: []string{
				"//modules/skcms:skcms_public",
			}},
		{Var: "skcms_TransformBaseline",
			Rules: []string{
				"//modules/skcms:skcms_TransformBaseline",
			}},
		{Var: "skcms_TransformHsw",
			Rules: []string{
				"//modules/skcms:skcms_TransformHsw",
			}},
		{Var: "skcms_TransformSkx",
			Rules: []string{
				"//modules/skcms:skcms_TransformSkx",
			}},
	}},
	{GNI: "modules/jsonreader/jsonreader.gni", Vars: []exporter.GNIFileListExportDesc{
		{Var: "skia_jsonreader_sources",
			Rules: []string{"//modules/jsonreader:jsonreader"}},
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

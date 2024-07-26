/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipelineOpList_DEFINED
#define SkRasterPipelineOpList_DEFINED

// `SK_RASTER_PIPELINE_OPS_LOWP` defines ops that have parallel lowp and highp implementations.
#define SK_RASTER_PIPELINE_OPS_LOWP(M)                             \
    M(move_src_dst) M(move_dst_src) M(swap_src_dst)                \
    M(clamp_01) M(clamp_a_01) M(clamp_gamut)                       \
    M(premul) M(premul_dst)                                        \
    M(force_opaque) M(force_opaque_dst)                            \
    M(set_rgb) M(swap_rb) M(swap_rb_dst)                           \
    M(black_color) M(white_color)                                  \
    M(uniform_color) M(uniform_color_dst)                          \
    M(seed_shader)                                                 \
    M(load_a8)     M(load_a8_dst)   M(store_a8)    M(gather_a8)    \
    M(load_565)    M(load_565_dst)  M(store_565)   M(gather_565)   \
    M(load_4444)   M(load_4444_dst) M(store_4444)  M(gather_4444)  \
    M(load_8888)   M(load_8888_dst) M(store_8888)  M(gather_8888)  \
    M(load_rg88)   M(load_rg88_dst) M(store_rg88)  M(gather_rg88)  \
    M(store_r8)                                                    \
    M(alpha_to_gray) M(alpha_to_gray_dst)                          \
    M(alpha_to_red) M(alpha_to_red_dst)                            \
    M(bt709_luminance_or_luma_to_alpha) M(bt709_luminance_or_luma_to_rgb) \
    M(bilerp_clamp_8888)                                           \
    M(load_src) M(store_src) M(store_src_a) M(load_dst) M(store_dst) \
    M(scale_u8) M(scale_565) M(scale_1_float) M(scale_native)      \
    M( lerp_u8) M( lerp_565) M( lerp_1_float) M(lerp_native)       \
    M(dstatop) M(dstin) M(dstout) M(dstover)                       \
    M(srcatop) M(srcin) M(srcout) M(srcover)                       \
    M(clear) M(modulate) M(multiply) M(plus_) M(screen) M(xor_)    \
    M(darken) M(difference)                                        \
    M(exclusion) M(hardlight) M(lighten) M(overlay)                \
    M(srcover_rgba_8888)                                           \
    M(matrix_translate) M(matrix_scale_translate)                  \
    M(matrix_2x3)                                                  \
    M(matrix_perspective)                                          \
    M(decal_x)    M(decal_y)   M(decal_x_and_y)                    \
    M(check_decal_mask)                                            \
    M(clamp_x_1) M(mirror_x_1) M(repeat_x_1)                       \
    M(clamp_x_and_y)                                               \
    M(evenly_spaced_gradient)                                      \
    M(gradient)                                                    \
    M(evenly_spaced_2_stop_gradient)                               \
    M(xy_to_unit_angle)                                            \
    M(xy_to_radius)                                                \
    M(emboss)                                                      \
    M(swizzle)

/**
 * `SK_RASTER_PIPELINE_OPS_SKSL` defines ops used by SkSL.
 *
 * Design docs for SkSL in Raster Pipeline: go/sksl-rp
 * https://docs.google.com/document/d/1GCQeAGVGHubOCbmULVdXUkNiXdw9J4umai_M5X3JGS4/edit?usp=sharing
 */
#define SK_RASTER_PIPELINE_OPS_SKSL(M)                                                          \
    M(init_lane_masks) M(store_device_xy01) M(exchange_src)                                     \
    M(load_condition_mask)  M(store_condition_mask)                                             \
    M(merge_condition_mask) M(merge_inv_condition_mask)                                         \
    M(load_loop_mask)       M(store_loop_mask)      M(mask_off_loop_mask)                       \
    M(reenable_loop_mask)   M(merge_loop_mask)      M(case_op)   M(continue_op)                 \
    M(load_return_mask)     M(store_return_mask)    M(mask_off_return_mask)                     \
    M(branch_if_all_lanes_active) M(branch_if_any_lanes_active) M(branch_if_no_lanes_active)    \
    M(branch_if_no_active_lanes_eq) M(jump)                                                     \
    M(bitwise_and_imm_4_ints)                                                                   \
        M(bitwise_and_imm_3_ints) M(bitwise_and_imm_2_ints) M(bitwise_and_imm_int)              \
        M(bitwise_and_n_ints)     M(bitwise_and_int)        M(bitwise_and_2_ints)               \
        M(bitwise_and_3_ints)     M(bitwise_and_4_ints)                                         \
    M(bitwise_or_n_ints)                                                                        \
        M(bitwise_or_int)  M(bitwise_or_2_ints)  M(bitwise_or_3_ints)  M(bitwise_or_4_ints)     \
    M(bitwise_xor_imm_int)                                                                      \
        M(bitwise_xor_n_ints) M(bitwise_xor_int) M(bitwise_xor_2_ints)                          \
        M(bitwise_xor_3_ints) M(bitwise_xor_4_ints)                                             \
    M(cast_to_float_from_int)     M(cast_to_float_from_2_ints)                                  \
    M(cast_to_float_from_3_ints)  M(cast_to_float_from_4_ints)                                  \
    M(cast_to_float_from_uint)    M(cast_to_float_from_2_uints)                                 \
    M(cast_to_float_from_3_uints) M(cast_to_float_from_4_uints)                                 \
    M(cast_to_int_from_float)     M(cast_to_int_from_2_floats)                                  \
    M(cast_to_int_from_3_floats)  M(cast_to_int_from_4_floats)                                  \
    M(cast_to_uint_from_float)    M(cast_to_uint_from_2_floats)                                 \
    M(cast_to_uint_from_3_floats) M(cast_to_uint_from_4_floats)                                 \
    M(abs_int)          M(abs_2_ints)          M(abs_3_ints)          M(abs_4_ints)             \
    M(floor_float)      M(floor_2_floats)      M(floor_3_floats)      M(floor_4_floats)         \
    M(ceil_float)       M(ceil_2_floats)       M(ceil_3_floats)       M(ceil_4_floats)          \
    M(invsqrt_float)    M(invsqrt_2_floats)    M(invsqrt_3_floats)    M(invsqrt_4_floats)       \
    M(inverse_mat2)     M(inverse_mat3)        M(inverse_mat4)                                  \
    M(sin_float)        M(cos_float)           M(tan_float)                                     \
    M(asin_float)       M(acos_float)          M(atan_float)          M(atan2_n_floats)         \
    M(sqrt_float)       M(pow_n_floats)        M(exp_float)           M(exp2_float)             \
    M(log_float)        M(log2_float)          M(refract_4_floats)                              \
    M(copy_uniform)     M(copy_2_uniforms)     M(copy_3_uniforms)     M(copy_4_uniforms)        \
    M(copy_constant)    M(splat_2_constants)   M(splat_3_constants)   M(splat_4_constants)      \
    M(copy_slot_masked) M(copy_2_slots_masked) M(copy_3_slots_masked) M(copy_4_slots_masked)    \
    M(copy_from_indirect_unmasked) M(copy_from_indirect_uniform_unmasked)                       \
    M(copy_to_indirect_masked)     M(swizzle_copy_to_indirect_masked)                           \
    M(copy_slot_unmasked)          M(copy_2_slots_unmasked)                                     \
    M(copy_3_slots_unmasked)       M(copy_4_slots_unmasked)                                     \
    M(copy_immutable_unmasked)     M(copy_2_immutables_unmasked)                                \
    M(copy_3_immutables_unmasked)  M(copy_4_immutables_unmasked)                                \
    M(swizzle_copy_slot_masked)    M(swizzle_copy_2_slots_masked)                               \
    M(swizzle_copy_3_slots_masked) M(swizzle_copy_4_slots_masked)                               \
    M(swizzle_1) M(swizzle_2) M(swizzle_3) M(swizzle_4) M(shuffle)                              \
    M(matrix_multiply_2) M(matrix_multiply_3) M(matrix_multiply_4)                              \
    M(smoothstep_n_floats) M(dot_2_floats) M(dot_3_floats) M(dot_4_floats)                      \
    M(add_imm_float)                                                                            \
        M(add_n_floats)   M(add_float)    M(add_2_floats)   M(add_3_floats)   M(add_4_floats)   \
    M(add_imm_int)                                                                              \
        M(add_n_ints)     M(add_int)      M(add_2_ints)     M(add_3_ints)     M(add_4_ints)     \
    M(sub_n_floats)       M(sub_float)    M(sub_2_floats)   M(sub_3_floats)   M(sub_4_floats)   \
    M(sub_n_ints)         M(sub_int)      M(sub_2_ints)     M(sub_3_ints)     M(sub_4_ints)     \
    M(mul_imm_float)                                                                            \
        M(mul_n_floats)   M(mul_float)    M(mul_2_floats)   M(mul_3_floats)   M(mul_4_floats)   \
    M(mul_imm_int)                                                                              \
        M(mul_n_ints)     M(mul_int)      M(mul_2_ints)     M(mul_3_ints)     M(mul_4_ints)     \
    M(div_n_floats)       M(div_float)    M(div_2_floats)   M(div_3_floats)   M(div_4_floats)   \
    M(div_n_ints)         M(div_int)      M(div_2_ints)     M(div_3_ints)     M(div_4_ints)     \
    M(div_n_uints)        M(div_uint)     M(div_2_uints)    M(div_3_uints)    M(div_4_uints)    \
    M(max_imm_float)                                                                            \
        M(max_n_floats)   M(max_float)    M(max_2_floats)   M(max_3_floats)   M(max_4_floats)   \
    M(max_n_ints)         M(max_int)      M(max_2_ints)     M(max_3_ints)     M(max_4_ints)     \
    M(max_n_uints)        M(max_uint)     M(max_2_uints)    M(max_3_uints)    M(max_4_uints)    \
    M(min_imm_float)                                                                            \
        M(min_n_floats)   M(min_float)    M(min_2_floats)   M(min_3_floats)   M(min_4_floats)   \
    M(min_n_ints)         M(min_int)      M(min_2_ints)     M(min_3_ints)     M(min_4_ints)     \
    M(min_n_uints)        M(min_uint)     M(min_2_uints)    M(min_3_uints)    M(min_4_uints)    \
    M(mod_n_floats)       M(mod_float)    M(mod_2_floats)   M(mod_3_floats)   M(mod_4_floats)   \
    M(mix_n_floats)       M(mix_float)    M(mix_2_floats)   M(mix_3_floats)   M(mix_4_floats)   \
    M(mix_n_ints)         M(mix_int)      M(mix_2_ints)     M(mix_3_ints)     M(mix_4_ints)     \
    M(cmplt_imm_float)                                                                          \
        M(cmplt_n_floats) M(cmplt_float)  M(cmplt_2_floats) M(cmplt_3_floats) M(cmplt_4_floats) \
    M(cmplt_imm_int)                                                                            \
        M(cmplt_n_ints)   M(cmplt_int)    M(cmplt_2_ints)   M(cmplt_3_ints)   M(cmplt_4_ints)   \
    M(cmplt_imm_uint)                                                                           \
        M(cmplt_n_uints)  M(cmplt_uint)   M(cmplt_2_uints)  M(cmplt_3_uints)  M(cmplt_4_uints)  \
    M(cmple_imm_float)                                                                          \
        M(cmple_n_floats) M(cmple_float)  M(cmple_2_floats) M(cmple_3_floats) M(cmple_4_floats) \
    M(cmple_imm_int)                                                                            \
        M(cmple_n_ints)   M(cmple_int)    M(cmple_2_ints)   M(cmple_3_ints)   M(cmple_4_ints)   \
    M(cmple_imm_uint)                                                                           \
        M(cmple_n_uints)  M(cmple_uint)   M(cmple_2_uints)  M(cmple_3_uints)  M(cmple_4_uints)  \
    M(cmpeq_imm_float)                                                                          \
        M(cmpeq_n_floats) M(cmpeq_float)  M(cmpeq_2_floats) M(cmpeq_3_floats) M(cmpeq_4_floats) \
    M(cmpeq_imm_int)                                                                            \
        M(cmpeq_n_ints)   M(cmpeq_int)    M(cmpeq_2_ints)   M(cmpeq_3_ints)   M(cmpeq_4_ints)   \
    M(cmpne_imm_float)                                                                          \
        M(cmpne_n_floats) M(cmpne_float)  M(cmpne_2_floats) M(cmpne_3_floats) M(cmpne_4_floats) \
    M(cmpne_imm_int)                                                                            \
        M(cmpne_n_ints)   M(cmpne_int)    M(cmpne_2_ints)   M(cmpne_3_ints)   M(cmpne_4_ints)   \
    M(trace_line)         M(trace_var)    M(trace_enter)    M(trace_exit)     M(trace_scope)

// `SK_RASTER_PIPELINE_OPS_HIGHP_ONLY` defines ops that are only available in highp; this subset
// includes all of SkSL.
#define SK_RASTER_PIPELINE_OPS_HIGHP_ONLY(M)                                   \
    M(callback)                                                                \
    M(stack_checkpoint) M(stack_rewind)                                        \
    M(unbounded_set_rgb) M(unbounded_uniform_color)                            \
    M(unpremul) M(unpremul_polar) M(dither)                                    \
    M(load_16161616) M(load_16161616_dst) M(store_16161616) M(gather_16161616) \
    M(load_a16)    M(load_a16_dst)  M(store_a16)   M(gather_a16)               \
    M(load_rg1616) M(load_rg1616_dst) M(store_rg1616) M(gather_rg1616)         \
    M(load_f16)    M(load_f16_dst)  M(store_f16)   M(gather_f16)               \
    M(load_af16)   M(load_af16_dst) M(store_af16)  M(gather_af16)              \
    M(load_rgf16)  M(load_rgf16_dst) M(store_rgf16) M(gather_rgf16)            \
    M(load_f32)    M(load_f32_dst)  M(store_f32)   M(gather_f32)               \
    M(load_1010102) M(load_1010102_dst) M(store_1010102) M(gather_1010102)     \
    M(load_1010102_xr) M(load_1010102_xr_dst) M(store_1010102_xr)              \
    M(gather_1010102_xr)                                                       \
    M(load_10x6) M(load_10x6_dst) M(store_10x6) M(gather_10x6)                 \
    M(gather_10101010_xr) M(load_10101010_xr) M(load_10101010_xr_dst)          \
    M(store_10101010_xr)                                                       \
    M(store_src_rg) M(load_src_rg)                                             \
    M(byte_tables)                                                             \
    M(colorburn) M(colordodge) M(softlight)                                    \
    M(hue) M(saturation) M(color) M(luminosity)                                \
    M(matrix_3x3) M(matrix_3x4) M(matrix_4x5) M(matrix_4x3)                    \
    M(parametric) M(gamma_) M(PQish) M(HLGish) M(HLGinvish)                    \
    M(rgb_to_hsl) M(hsl_to_rgb)                                                \
    M(css_lab_to_xyz) M(css_oklab_to_linear_srgb)                              \
    M(css_oklab_gamut_map_to_linear_srgb)                                      \
    M(css_hcl_to_lab)                                                          \
    M(css_hsl_to_srgb) M(css_hwb_to_srgb)                                      \
    M(gauss_a_to_rgba)                                                         \
    M(mirror_x)   M(repeat_x)                                                  \
    M(mirror_y)   M(repeat_y)                                                  \
    M(negate_x)                                                                \
    M(bicubic_clamp_8888)                                                      \
    M(bilinear_setup)                                                          \
    M(bilinear_nx) M(bilinear_px) M(bilinear_ny) M(bilinear_py)                \
    M(bicubic_setup)                                                           \
    M(bicubic_n3x) M(bicubic_n1x) M(bicubic_p1x) M(bicubic_p3x)                \
    M(bicubic_n3y) M(bicubic_n1y) M(bicubic_p1y) M(bicubic_p3y)                \
    M(accumulate)                                                              \
    M(perlin_noise)                                                            \
    M(mipmap_linear_init) M(mipmap_linear_update) M(mipmap_linear_finish)      \
    M(xy_to_2pt_conical_strip)                                                 \
    M(xy_to_2pt_conical_focal_on_circle)                                       \
    M(xy_to_2pt_conical_well_behaved)                                          \
    M(xy_to_2pt_conical_smaller)                                               \
    M(xy_to_2pt_conical_greater)                                               \
    M(alter_2pt_conical_compensate_focal)                                      \
    M(alter_2pt_conical_unswap)                                                \
    M(mask_2pt_conical_nan)                                                    \
    M(mask_2pt_conical_degenerates) M(apply_vector_mask)                       \
    M(set_base_pointer)                                                        \
    SK_RASTER_PIPELINE_OPS_SKSL(M)

// The combined set of all RasterPipeline ops:
#define SK_RASTER_PIPELINE_OPS_ALL(M)    \
    SK_RASTER_PIPELINE_OPS_LOWP(M)       \
    SK_RASTER_PIPELINE_OPS_HIGHP_ONLY(M)

// An enumeration of every RasterPipeline op:
enum class SkRasterPipelineOp {
#define M(op) op,
    SK_RASTER_PIPELINE_OPS_ALL(M)
#undef M
};

// A count of raster pipeline ops:
#define M(st) +1
    static constexpr int kNumRasterPipelineLowpOps  = SK_RASTER_PIPELINE_OPS_LOWP(M);
    static constexpr int kNumRasterPipelineHighpOps = SK_RASTER_PIPELINE_OPS_ALL(M);
#undef M

#endif  // SkRasterPipelineOpList_DEFINED

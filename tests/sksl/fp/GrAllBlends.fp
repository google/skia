in uniform half4 src;
in uniform half4 dst;

void main() {
    sk_OutColor = blend_clear(src, dst);
    sk_OutColor = blend_src(src, dst);
    sk_OutColor = blend_dst(src, dst);
    sk_OutColor = blend_src_over(src, dst);
    sk_OutColor = blend_dst_over(src, dst);
    sk_OutColor = blend_src_in(src, dst);
    sk_OutColor = blend_dst_in(src, dst);
    sk_OutColor = blend_src_out(src, dst);
    sk_OutColor = blend_dst_out(src, dst);
    sk_OutColor = blend_src_atop(src, dst);
    sk_OutColor = blend_dst_atop(src, dst);
    sk_OutColor = blend_xor(src, dst);
    sk_OutColor = blend_plus(src, dst);
    sk_OutColor = blend_modulate(src, dst);
    sk_OutColor = blend_screen(src, dst);
    sk_OutColor = blend_overlay(src, dst);
    sk_OutColor = blend_darken(src, dst);
    sk_OutColor = blend_lighten(src, dst);
    sk_OutColor = blend_color_dodge(src, dst);
    sk_OutColor = blend_color_burn(src, dst);
    sk_OutColor = blend_hard_light(src, dst);
    sk_OutColor = blend_soft_light(src, dst);
    sk_OutColor = blend_difference(src, dst);
    sk_OutColor = blend_exclusion(src, dst);
    sk_OutColor = blend_multiply(src, dst);
    sk_OutColor = blend_hue(src, dst);
    sk_OutColor = blend_saturation(src, dst);
    sk_OutColor = blend_color(src, dst);
    sk_OutColor = blend_luminosity(src, dst);
}

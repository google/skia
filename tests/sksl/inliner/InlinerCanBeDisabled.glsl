
const float sk_PrivGuardedDivideEpsilon = false ? 9.9999999392252903e-09 : 0.0;
out vec4 sk_FragColor;
uniform vec4 color;
float blend_color_luminance_Qhh3(vec3 color);
float blend_color_saturation_Qhh3(vec3 color);
float guarded_divide_Qhhh(float n, float d);
vec3 guarded_divide_Qh3h3h(vec3 n, float d);
vec3 blend_set_color_luminance_Qh3h3hh3(vec3 hueSatColor, float alpha, vec3 lumColor);
vec3 blend_set_color_saturation_Qh3h3h3(vec3 color, vec3 satColor);
vec4 blend_hslc_h4h4h4h2(vec4 src, vec4 dst, vec2 flipSat);
vec4 blend_dst_in_h4h4h4(vec4 src, vec4 dst);
vec4 blend_hue_h4h4h4(vec4 src, vec4 dst);
vec4 blend_src_in_h4h4h4(vec4 src, vec4 dst);
float blend_color_luminance_Qhh3(vec3 color) {
    return dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
float blend_color_saturation_Qhh3(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
float guarded_divide_Qhhh(float n, float d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
vec3 guarded_divide_Qh3h3h(vec3 n, float d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
vec3 blend_set_color_luminance_Qh3h3hh3(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float lum = blend_color_luminance_Qhh3(lumColor);
    vec3 result = (lum - blend_color_luminance_Qhh3(hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + (result - lum) * guarded_divide_Qhhh(lum, lum - minComp);
    }
    if (maxComp > alpha && maxComp != lum) {
        result = lum + guarded_divide_Qh3h3h((result - lum) * (alpha - lum), maxComp - lum);
    }
    return result;
}
vec3 blend_set_color_saturation_Qh3h3h3(vec3 color, vec3 satColor) {
    float mn = min(min(color.x, color.y), color.z);
    float mx = max(max(color.x, color.y), color.z);
    return mx > mn ? ((color - mn) * blend_color_saturation_Qhh3(satColor)) / (mx - mn) : vec3(0.0);
}
vec4 blend_hslc_h4h4h4h2(vec4 src, vec4 dst, vec2 flipSat) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = bool(flipSat.x) ? dsa : sda;
    vec3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        l = blend_set_color_saturation_Qh3h3h3(l, r);
        r = dsa;
    }
    return vec4((((blend_set_color_luminance_Qh3h3hh3(l, alpha, r) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
vec4 blend_dst_in_h4h4h4(vec4 src, vec4 dst) {
    return dst * src.w;
}
vec4 blend_hue_h4h4h4(vec4 src, vec4 dst) {
    return blend_hslc_h4h4h4h2(src, dst, vec2(0.0, 1.0));
}
vec4 blend_src_in_h4h4h4(vec4 src, vec4 dst) {
    return src * dst.w;
}
float singleuse_h() {
    return 1.25;
}
float add_hhh(float a, float b) {
    float c = a + b;
    return c;
}
float mul_hhh(float a, float b) {
    return a * b;
}
float fused_multiply_add_hhhh(float a, float b, float c) {
    return add_hhh(mul_hhh(a, b), c);
}
void main() {
    sk_FragColor = vec4(fused_multiply_add_hhhh(color.x, color.y, color.z));
    sk_FragColor *= singleuse_h();
    sk_FragColor *= blend_src_in_h4h4h4(color.xxyy, color.zzww);
    sk_FragColor *= blend_dst_in_h4h4h4(color.xxyy, color.zzww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wwww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wzyx);
}


out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_color_luminance(vec3 color) {
    return dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _0_blend_color_luminance;
    {
        _0_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _0_blend_color_luminance;

    float _1_blend_color_luminance;
    {
        _1_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    vec3 result = (lum - _1_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
vec4 blend_luminosity(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(dsa, alpha, sda) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    vec4 _2_blend_luminosity;
    {
        float _3_alpha = dst.w * src.w;
        vec3 _4_sda = src.xyz * dst.w;
        vec3 _5_dsa = dst.xyz * src.w;
        _2_blend_luminosity = vec4((((_blend_set_color_luminance(_5_dsa, _3_alpha, _4_sda) + dst.xyz) - _5_dsa) + src.xyz) - _4_sda, (src.w + dst.w) - _3_alpha);
    }
    sk_FragColor = _2_blend_luminosity;

}

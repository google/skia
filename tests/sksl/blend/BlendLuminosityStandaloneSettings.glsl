
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec3 blend_set_color_luminance_Qh3h3hh3(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    vec3 result = (lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + (result - lum) * (lum / (lum - minComp));
    }
    if (maxComp > alpha && maxComp != lum) {
        return lum + ((result - lum) * (alpha - lum)) / (maxComp - lum);
    } else {
        return result;
    }
}
void main() {
    float _0_alpha = dst.w * src.w;
    vec3 _1_sda = src.xyz * dst.w;
    vec3 _2_dsa = dst.xyz * src.w;
    sk_FragColor = vec4((((blend_set_color_luminance_Qh3h3hh3(_2_dsa, _0_alpha, _1_sda) + dst.xyz) - _2_dsa) + src.xyz) - _1_sda, (src.w + dst.w) - _0_alpha);
}

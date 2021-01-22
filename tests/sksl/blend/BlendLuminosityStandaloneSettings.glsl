
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    float _1_alpha = dst.w * src.w;
    vec3 _2_sda = src.xyz * dst.w;
    vec3 _3_dsa = dst.xyz * src.w;
    float _5_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _2_sda);

    vec3 _6_result = (_5_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa)) + _3_dsa;

    float _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    float _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0 && _5_lum != _7_minComp) {
        _6_result = _5_lum + ((_6_result - _5_lum) * _5_lum) / (_5_lum - _7_minComp);
    }
    sk_FragColor = vec4(((((_8_maxComp > _1_alpha && _8_maxComp != _5_lum ? _5_lum + ((_6_result - _5_lum) * (_1_alpha - _5_lum)) / (_8_maxComp - _5_lum) : _6_result) + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


}

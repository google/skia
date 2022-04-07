
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_hslc_h4h4h4bb(vec4 src, vec4 dst, bool flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = flip ? dsa : sda;
    vec3 r = flip ? sda : dsa;
    if (saturate) {
        vec3 _2_hueLumColor = l;
        vec3 _3_midPt = vec3(0.0);
        vec3 _4_satPt = vec3(0.0);
        if (_2_hueLumColor.x <= _2_hueLumColor.y) {
            if (_2_hueLumColor.y <= _2_hueLumColor.z) {
                _2_hueLumColor -= _2_hueLumColor.xxx;
                _3_midPt.y = (_4_satPt.z = 1.0);
            } else if (_2_hueLumColor.x <= _2_hueLumColor.z) {
                _2_hueLumColor = _2_hueLumColor.xzy - _2_hueLumColor.xxx;
                _3_midPt.z = (_4_satPt.y = 1.0);
            } else {
                _2_hueLumColor = _2_hueLumColor.zxy - _2_hueLumColor.zzz;
                _3_midPt.x = (_4_satPt.y = 1.0);
            }
        } else if (_2_hueLumColor.x <= _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.yxz - _2_hueLumColor.yyy;
            _3_midPt.x = (_4_satPt.z = 1.0);
        } else if (_2_hueLumColor.y <= _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.yzx - _2_hueLumColor.yyy;
            _3_midPt.z = (_4_satPt.x = 1.0);
        } else {
            _2_hueLumColor = _2_hueLumColor.zyx - _2_hueLumColor.zzz;
            _3_midPt.y = (_4_satPt.x = 1.0);
        }
        if (_2_hueLumColor.z > 0.0) {
            _3_midPt *= _2_hueLumColor.y / _2_hueLumColor.z;
            _2_hueLumColor = (_3_midPt + _4_satPt) * (max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z));
        }
        l = _2_hueLumColor;
        r = dsa;
    }
    float _5_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _6_result = (_5_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    float _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0 && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (_5_lum - _7_minComp));
    }
    if (_8_maxComp > alpha && _8_maxComp != _5_lum) {
        _6_result = _5_lum + ((_6_result - _5_lum) * (alpha - _5_lum)) / (_8_maxComp - _5_lum);
    }
    return vec4((((_6_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = blend_hslc_h4h4h4bb(src, dst, false, true);
}

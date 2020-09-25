
out vec4 sk_FragColor;
in vec4 src, dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    float _0_blend_overlay_component;
    vec2 _1_s = src.xw;
    vec2 _2_d = dst.xw;
    {
        _0_blend_overlay_component = 2.0 * _2_d.x <= _2_d.y ? (2.0 * _1_s.x) * _2_d.x : _1_s.y * _2_d.y - (2.0 * (_2_d.y - _2_d.x)) * (_1_s.y - _1_s.x);
    }
    float _3_blend_overlay_component;
    vec2 _4_s = src.yw;
    vec2 _5_d = dst.yw;
    {
        _3_blend_overlay_component = 2.0 * _5_d.x <= _5_d.y ? (2.0 * _4_s.x) * _5_d.x : _4_s.y * _5_d.y - (2.0 * (_5_d.y - _5_d.x)) * (_4_s.y - _4_s.x);
    }
    float _6_blend_overlay_component;
    vec2 _7_s = src.zw;
    vec2 _8_d = dst.zw;
    {
        _6_blend_overlay_component = 2.0 * _8_d.x <= _8_d.y ? (2.0 * _7_s.x) * _8_d.x : _7_s.y * _8_d.y - (2.0 * (_8_d.y - _8_d.x)) * (_7_s.y - _7_s.x);
    }
    vec4 result = vec4(_0_blend_overlay_component, _3_blend_overlay_component, _6_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
void main() {
    sk_FragColor = blend_overlay(src, dst);
}


out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    vec4 result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
void main() {
    vec4 _0_blend_overlay;
    {
        float _2_blend_overlay_component;
        {
            _2_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
        }
        float _3_blend_overlay_component;
        {
            _3_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
        }
        float _4_blend_overlay_component;
        {
            _4_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
        }
        vec4 _1_result = vec4(_2_blend_overlay_component, _3_blend_overlay_component, _4_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



        _1_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
        _0_blend_overlay = _1_result;
    }

    sk_FragColor = _0_blend_overlay;

}


out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_overlay;
    {
        float _1_1_blend_overlay_component;
        {
            _1_1_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
        }
        float _2_75_blend_overlay_component;
        {
            _2_75_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
        }
        float _3_79_blend_overlay_component;
        {
            _3_79_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
        }
        vec4 _4_result = vec4(_1_1_blend_overlay_component, _2_75_blend_overlay_component, _3_79_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



        _4_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
        _0_blend_overlay = _4_result;
    }
    sk_FragColor = _0_blend_overlay;

}

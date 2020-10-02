
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _1_blend_overlay;
    {
        float _3_blend_overlay_component;
        {
            _3_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
        }
        float _4_blend_overlay_component;
        {
            _4_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
        }
        float _5_blend_overlay_component;
        {
            _5_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
        }
        vec4 _2_result = vec4(_3_blend_overlay_component, _4_blend_overlay_component, _5_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



        _2_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
        _1_blend_overlay = _2_result;
    }
    sk_FragColor = _1_blend_overlay;

}

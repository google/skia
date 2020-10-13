
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    float _1_blend_overlay_component;
    {
        _1_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
    }
    float _6_blend_overlay_component;
    {
        _6_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
    }
    float _9_blend_overlay_component;
    {
        _9_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
    }
    vec4 result = vec4(_1_blend_overlay_component, _6_blend_overlay_component, _9_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
vec4 blend_hard_light(vec4 src, vec4 dst) {
    vec4 _2_blend_overlay;
    {
        float _7_blend_overlay_component;
        {
            _7_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
        }
        float _10_blend_overlay_component;
        {
            _10_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
        }
        float _12_blend_overlay_component;
        {
            _12_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
        }
        vec4 _3_result = vec4(_7_blend_overlay_component, _10_blend_overlay_component, _12_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



        _3_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
        _2_blend_overlay = _3_result;
    }
    return _2_blend_overlay;

}
void main() {
    vec4 _0_blend_hard_light;
    {
        vec4 _4_blend_overlay;
        {
            float _8_blend_overlay_component;
            {
                _8_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
            }
            float _11_blend_overlay_component;
            {
                _11_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
            }
            float _13_blend_overlay_component;
            {
                _13_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
            }
            vec4 _5_result = vec4(_8_blend_overlay_component, _11_blend_overlay_component, _13_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _5_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _4_blend_overlay = _5_result;
        }
        _0_blend_hard_light = _4_blend_overlay;

    }

    sk_FragColor = _0_blend_hard_light;

}

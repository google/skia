#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 src;
    float4 dst;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float _color_dodge_component(float2 s, float2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _0_n = d.x * s.y;
            delta = min(d.y, _0_n / delta);
            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = float4(_color_dodge_component(_uniforms.src.xw, _uniforms.dst.xw), _color_dodge_component(_uniforms.src.yw, _uniforms.dst.yw), _color_dodge_component(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
    return _out;
}

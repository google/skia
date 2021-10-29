#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half _soft_light_component_hh2h2(half2 s, half2 d) {
    if (2.0h * s.x <= s.y) {
        return (((d.x * d.x) * (s.y - 2.0h * s.x)) / d.y + (1.0h - d.y) * s.x) + d.x * ((-s.y + 2.0h * s.x) + 1.0h);
    } else if (4.0h * d.x <= d.y) {
        half DSqd = d.x * d.x;
        half DCub = DSqd * d.x;
        half DaSqd = d.y * d.y;
        half DaCub = DaSqd * d.y;
        return (((DaSqd * (s.x - d.x * ((3.0h * s.y - 6.0h * s.x) - 1.0h)) + ((12.0h * d.y) * DSqd) * (s.y - 2.0h * s.x)) - (16.0h * DCub) * (s.y - 2.0h * s.x)) - DaCub * s.x) / DaSqd;
    } else {
        return ((d.x * ((s.y - 2.0h * s.x) + 1.0h) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0h * s.x)) - d.y * s.x;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = _uniforms.dst.w == 0.0h ? _uniforms.src : half4(_soft_light_component_hh2h2(_uniforms.src.xw, _uniforms.dst.xw), _soft_light_component_hh2h2(_uniforms.src.yw, _uniforms.dst.yw), _soft_light_component_hh2h2(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    return _out;
}

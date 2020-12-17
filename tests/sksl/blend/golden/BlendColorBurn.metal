#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float _color_burn_component(float2 s, float2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float _6_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _6_n / s.x);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(_color_burn_component(_in.src.xw, _in.dst.xw), _color_burn_component(_in.src.yw, _in.dst.yw), _color_burn_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);

    return *_out;
}

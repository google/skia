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
float _color_dodge_component(float2 s, float2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _4_n = d.x * s.y;
            delta = min(d.y, _4_n / delta);

            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(_color_dodge_component(_in.src.xw, _in.dst.xw), _color_dodge_component(_in.src.yw, _in.dst.yw), _color_dodge_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);

    return *_out;
}

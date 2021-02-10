#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float2 a;
    float2 b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float cross(float2 a, float2 b) {
    return a.x * b.y - a.y * b.x;
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = cross(_in.a, _in.b);
    return _out;
}

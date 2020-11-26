#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float4 blend_dst(float4 src, float4 dst) {
    return dst;
}
float4 live_fn(float4 a, float4 b) {
    return a + b;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    {
        _out->sk_FragColor = live_fn(float4(3.0), float4(-3.0));
    }
    {
        _out->sk_FragColor = blend_dst(float4(7.0), float4(-7.0));
    }
    return *_out;
}

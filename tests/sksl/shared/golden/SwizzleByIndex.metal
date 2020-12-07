#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 v = float4(sqrt(1.0));
    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;
    _out->sk_FragColor = float4(x, y, z, w);
    v = float4(2.0);
    x = 2.0;
    y = 2.0;
    z = 2.0;
    w = 2.0;
    _out->sk_FragColor = float4(2.0, 2.0, 2.0, 2.0);
    return *_out;
}

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
    bool4 v = bool4(sqrt(1.0) == 1.0);
    bool4 result;
    result = bool4(v.x, true, true, true);
    result = bool4(v.xy, false, true);
    result = bool4(v.x, true, true, false);
    result = bool4(false, v.y, true, true);
    result = bool4(v.xyz, true);
    result = bool4(v.xy, true, true);
    result = bool4(v.x, false, v.z, true);
    result = bool4(v.x, true, false, false);
    result = bool4(true, v.yz, false);
    result = bool4(false, v.y, true, false);
    result = bool4(true, true, v.z, false);
    result = v;
    result = bool4(v.xyz, true);
    result = bool4(v.xy, false, v.w);
    result = bool4(v.xy, true, false);
    result = bool4(v.x, true, v.zw);
    result = bool4(v.x, false, v.z, true);
    result = bool4(v.x, true, true, v.w);
    result = bool4(v.x, true, false, true);
    result = bool4(true, v.yzw);
    result = bool4(false, v.yz, true);
    result = bool4(false, v.y, true, v.w);
    result = bool4(true, v.y, true, true);
    result = bool4(false, false, v.zw);
    result = bool4(false, false, v.z, true);
    result = bool4(false, true, true, v.w);
    _out->sk_FragColor = any(result) ? float4(1.0) : float4(0.0);
    return *_out;
}

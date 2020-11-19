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
    result = bool4(bool2(v.x, true), true, false);
    result = bool4(bool2(false, v.y), true, true);
    result = bool4(v.xyz, true);
    result = bool4(bool3(v.xy, true), true);
    result = bool4(bool3(v.xz, false).xzy, true);
    result = bool4(bool3(v.x, true, false), false);
    result = bool4(bool3(v.yz, true).zxy, false);
    result = bool4(bool3(false, v.y, true), false);
    result = bool4(bool3(true, true, v.z), false);
    result = v;
    result = bool4(v.xyz, true);
    result = bool4(v.xyw, false).xywz;
    result = bool4(v.xy, true, false);
    result = bool4(v.xzw, true).xwyz;
    result = bool4(v.xz, false, true).xzyw;
    result = bool3(v.xw, true).xzzy;
    result = bool4(v.x, true, false, true);
    result = bool4(v.yzw, true).wxyz;
    result = bool4(v.yz, false, true).zxyw;
    result = bool4(v.yw, false, true).zxwy;
    result = bool4(true, v.y, true, true);
    result = bool3(v.zw, false).zzxy;
    result = bool4(false, false, v.z, true);
    result = bool4(false, true, true, v.w);
    _out->sk_FragColor = any(result) ? float4(1.0) : float4(0.0);
    return *_out;
}

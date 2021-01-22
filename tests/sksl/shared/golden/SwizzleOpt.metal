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
    float v = sqrt(1.0);
    _out->sk_FragColor = float4(v);
    _out->sk_FragColor = float4(float4(v).xyz, 0.0).wzyx;
    _out->sk_FragColor = float3(float4(v).xw, 0.0).zzxy;
    _out->sk_FragColor = float3(float3(float4(v).xw, 0.0).yx, 1.0).zzxy;
    _out->sk_FragColor = float3(float4(v).zy, 1.0).xyzz;
    _out->sk_FragColor = float4(v);
    _out->sk_FragColor = float4(float4(v).xx, 1.0, 1.0);
    _out->sk_FragColor = float4(v).wzwz;
    _out->sk_FragColor.xyzw = _out->sk_FragColor;
    _out->sk_FragColor.wzyx = _out->sk_FragColor;
    _out->sk_FragColor.xyzw.xw = _out->sk_FragColor.yz;
    _out->sk_FragColor.wzyx.yzw = float3(_out->sk_FragColor.ww, 1.0);
    return *_out;
}

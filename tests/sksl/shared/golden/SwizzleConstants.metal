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
    _out->sk_FragColor = float4(v.x, 1.0, 1.0, 1.0);
    _out->sk_FragColor = float4(v.xy, 1.0, 1.0);
    _out->sk_FragColor = float4(float2(v.x, 1.0), 1.0, 1.0);
    _out->sk_FragColor = float4(float2(0.0, v.y), 1.0, 1.0);
    _out->sk_FragColor = float4(v.xyz, 1.0);
    _out->sk_FragColor = float4(float3(v.xy, 1.0), 1.0);
    _out->sk_FragColor = float4(float3(v.xz, 0.0).xzy, 1.0);
    _out->sk_FragColor = float4(float3(v.x, 1.0, 0.0), 1.0);
    _out->sk_FragColor = float4(float3(v.yz, 1.0).zxy, 1.0);
    _out->sk_FragColor = float4(float3(0.0, v.y, 1.0), 1.0);
    _out->sk_FragColor = float4(float3(1.0, 1.0, v.z), 1.0);
    _out->sk_FragColor = v;
    _out->sk_FragColor = float4(v.xyz, 1.0);
    _out->sk_FragColor = float4(v.xyw, 0.0).xywz;
    _out->sk_FragColor = float4(v.xy, 1.0, 0.0);
    _out->sk_FragColor = float4(v.xzw, 1.0).xwyz;
    _out->sk_FragColor = float4(v.xz, 0.0, 1.0).xzyw;
    _out->sk_FragColor = float3(v.xw, 1.0).xzzy;
    _out->sk_FragColor = float4(v.x, 1.0, 0.0, 1.0);
    _out->sk_FragColor = float4(v.yzw, 1.0).wxyz;
    _out->sk_FragColor = float4(v.yz, 0.0, 1.0).zxyw;
    _out->sk_FragColor = float4(v.yw, 0.0, 1.0).zxwy;
    _out->sk_FragColor = float4(1.0, v.y, 1.0, 1.0);
    _out->sk_FragColor = float3(v.zw, 0.0).zzxy;
    _out->sk_FragColor = float4(0.0, 0.0, v.z, 1.0);
    _out->sk_FragColor = float4(0.0, 1.0, 1.0, v.w);
    return *_out;
}

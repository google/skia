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
    _out->sk_FragColor = float4(v.x, 1.0, 1.0, 1.0);
    _out->sk_FragColor = float4(0.0, v.y, 1.0, 1.0);
    _out->sk_FragColor = float4(v.xyz, 1.0);
    _out->sk_FragColor = float4(v.xy, 1.0, 1.0);
    _out->sk_FragColor = float4(v.x, 0.0, v.z, 1.0);
    _out->sk_FragColor = float4(v.x, 1.0, 0.0, 1.0);
    _out->sk_FragColor = float4(1.0, v.yz, 1.0);
    _out->sk_FragColor = float4(0.0, v.y, 1.0, 1.0);
    _out->sk_FragColor = float4(1.0, 1.0, v.z, 1.0);
    _out->sk_FragColor = v;
    _out->sk_FragColor = float4(v.xyz, 1.0);
    _out->sk_FragColor = float4(v.xy, 0.0, v.w);
    _out->sk_FragColor = float4(v.xy, 1.0, 0.0);
    _out->sk_FragColor = float4(v.x, 1.0, v.zw);
    _out->sk_FragColor = float4(v.x, 0.0, v.z, 1.0);
    _out->sk_FragColor = float4(v.x, 1.0, 1.0, v.w);
    _out->sk_FragColor = float4(v.x, 1.0, 0.0, 1.0);
    _out->sk_FragColor = float4(1.0, v.yzw);
    _out->sk_FragColor = float4(0.0, v.yz, 1.0);
    _out->sk_FragColor = float4(0.0, v.y, 1.0, v.w);
    _out->sk_FragColor = float4(1.0, v.y, 1.0, 1.0);
    _out->sk_FragColor = float4(0.0, 0.0, v.zw);
    _out->sk_FragColor = float4(0.0, 0.0, v.z, 1.0);
    _out->sk_FragColor = float4(0.0, 1.0, 1.0, v.w);
    return *_out;
}

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float f;
    float af[5];
    float4 h4;
    float4 ah4[5];
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 x;
    x.w = 0.0;
    x.yx = float2(0.0);
    int ai[1];
    ai[0] = 0;
    int4 ai4[1];
    ai4[0] = int4(1, 2, 3, 4);
    float2x4 ah2x4[1];
    ah2x4[0] = float2x4(float4(1.0, 2.0, 3.0, 4.0), float4(5.0, 6.0, 7.0, 8.0));
    ai[0] = 0;
    ai[ai[0]] = 0;
    float4 af4[1];
    af4[0].x = 0.0;
    af4[0].ywxz = float4(1.0);
    S s;
    s.f = 0.0;
    s.af[1] = 0.0;
    s.h4.zxy = float3(9.0);
    s.ah4[2].yw = float2(5.0);
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = float4(1.0);
    s.ah4[0] = float4(2.0);
    _out->sk_FragColor = float4(0.0);
    _out->sk_FragColor = float4(int4(1, 2, 3, 4));
    _out->sk_FragColor = float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0))[0].xxyz;
    _out->sk_FragColor = x;
    _out->sk_FragColor = float4(float(ai[0]));
    _out->sk_FragColor = float4(ai4[0]);
    _out->sk_FragColor = ah2x4[0][0];
    _out->sk_FragColor = af4[0];
    _out->sk_FragColor = float4(0.0);
    _out->sk_FragColor = float4(s.f);
    _out->sk_FragColor = float4(s.af[1]);
    _out->sk_FragColor = s.h4;
    _out->sk_FragColor = s.ah4[0];
    return *_out;
}

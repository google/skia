#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float2 h2;
    float3 h3;
    float4 h4;
    float2 f2;
    float3 f3;
    float4 f4;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};






fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = outerProduct(_in.f2, _in.f2)[1].xyyy;
    _out->sk_FragColor = outerProduct(_in.f3, _in.f3)[2].xyzz;
    _out->sk_FragColor = outerProduct(_in.f4, _in.f4)[3];
    _out->sk_FragColor = outerProduct(_in.f3, _in.f2)[1].xyzz;
    _out->sk_FragColor = outerProduct(_in.f2, _in.f3)[2].xyyy;
    _out->sk_FragColor = outerProduct(_in.f4, _in.f2)[1];
    _out->sk_FragColor = outerProduct(_in.f2, _in.f4)[3].xyyy;
    _out->sk_FragColor = outerProduct(_in.f4, _in.f3)[2];
    _out->sk_FragColor = outerProduct(_in.f3, _in.f4)[3].xyzz;
    _out->sk_FragColor = outerProduct(_in.h2, _in.h2)[1].xyyy;
    _out->sk_FragColor = outerProduct(_in.h3, _in.h3)[2].xyzz;
    _out->sk_FragColor = outerProduct(_in.h4, _in.h4)[3];
    _out->sk_FragColor = outerProduct(_in.h3, _in.h2)[1].xyzz;
    _out->sk_FragColor = outerProduct(_in.h2, _in.h3)[2].xyyy;
    _out->sk_FragColor = outerProduct(_in.h4, _in.h2)[1];
    _out->sk_FragColor = outerProduct(_in.h2, _in.h4)[3].xyyy;
    _out->sk_FragColor = outerProduct(_in.h4, _in.h3)[2];
    _out->sk_FragColor = outerProduct(_in.h3, _in.h4)[3].xyzz;
    return *_out;
}

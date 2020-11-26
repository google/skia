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
    {
        _out->sk_FragColor = float4(1.0);
    }
    {
        _out->sk_FragColor = float4(2.0);
    }
    {
        _out->sk_FragColor = float4(6.0);
    }
    _out->sk_FragColor = float4(7.0);
    _out->sk_FragColor = float4(-8.0);
    _out->sk_FragColor = float4(-9.0);
    _out->sk_FragColor = float4(10.0);
    {
        _out->sk_FragColor = float4(11.0);
    }
    {
        _out->sk_FragColor = float4(13.0);
    }
    {
        _out->sk_FragColor = float4(15.0);
    }
    {
        _out->sk_FragColor = float4(16.0);
    }
    {
        _out->sk_FragColor = float4(18.0);
    }
    _out->sk_FragColor = float4(19.0);
    _out->sk_FragColor = float4(20.0);
    {
        _out->sk_FragColor = float4(21.0);
    }
    return *_out;
}

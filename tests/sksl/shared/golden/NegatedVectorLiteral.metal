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
        _out->sk_FragColor.x = 1.0;
        _out->sk_FragColor.y = 1.0;
        _out->sk_FragColor.z = 1.0;
        _out->sk_FragColor.w = 1.0;
    }

    {
        _out->sk_FragColor.x = float(all(int4(-1) == int4(int2(-1), int2(-1))) ? 1 : 0);
        _out->sk_FragColor.y = float(any(int4(1) != int4(-1)) ? 1 : 0);
        _out->sk_FragColor.z = float(all(int4(-2) == int4(-2, int3(-2))) ? 1 : 0);
        _out->sk_FragColor.w = float(all(int2(1, -2) == int2(1, -2)) ? 1 : 0);
    }

    return *_out;
}

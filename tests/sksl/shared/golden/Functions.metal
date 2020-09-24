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
    float x = 10.0;
    {
        float _1_y[2], _2_z;
        _1_y[0] = 10.0;
        _1_y[1] = 20.0;
        float _3_0_foo;
        {
            _3_0_foo = _1_y[0] * _1_y[1];
        }

        _2_z = _3_0_foo;

        x = _2_z;
    }


    _out->sk_FragColor = float4(x);
    return *_out;
}

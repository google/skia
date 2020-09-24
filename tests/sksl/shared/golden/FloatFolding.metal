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
    _out->sk_FragColor.x = 34.0;
    _out->sk_FragColor.x = 30.0;
    _out->sk_FragColor.x = 64.0;
    _out->sk_FragColor.x = 16.0;
    _out->sk_FragColor.x = 19.0;
    _out->sk_FragColor.x = 1.0;
    _out->sk_FragColor.x = -2.0;
    _out->sk_FragColor.x = 3.0;
    _out->sk_FragColor.x = -4.0;
    _out->sk_FragColor.x = 5.0;
    _out->sk_FragColor.x = -6.0;
    _out->sk_FragColor.x = 7.0;
    _out->sk_FragColor.x = -8.0;
    _out->sk_FragColor.x = 9.0;
    _out->sk_FragColor.x = -10.0;
    _out->sk_FragColor.x = 11.0;
    _out->sk_FragColor.x = -12.0;
    _out->sk_FragColor.x = sqrt(1.0);
    _out->sk_FragColor.x = sqrt(2.0);
    _out->sk_FragColor.x = sqrt(3.0);
    _out->sk_FragColor.x = 0.0;
    _out->sk_FragColor.x = sqrt(5.0);
    _out->sk_FragColor.x = sqrt(6.0);
    _out->sk_FragColor.x = 0.0;
    _out->sk_FragColor.x = sqrt(8.0);
    _out->sk_FragColor.x = 0.0;
    _out->sk_FragColor.x = _out->sk_FragColor.x + 1.0;
    _out->sk_FragColor.x = _out->sk_FragColor.x - 1.0;
    _out->sk_FragColor.x = _out->sk_FragColor.x * 2.0;
    _out->sk_FragColor.x = _out->sk_FragColor.x / 2.0;
    return *_out;
}

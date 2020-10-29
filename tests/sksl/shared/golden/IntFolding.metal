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
    _out->sk_FragColor.x = 14.0;
    _out->sk_FragColor.x = 6.0;
    _out->sk_FragColor.x = 5.0;
    _out->sk_FragColor.x = 16.0;
    _out->sk_FragColor.x = 32.0;
    _out->sk_FragColor.x = 33.0;
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
    _out->sk_FragColor.x = float(int(sqrt(1.0)));
    _out->sk_FragColor.x = float(int(sqrt(2.0)));
    _out->sk_FragColor.x = float(int(sqrt(3.0)));
    _out->sk_FragColor.x = 0.0;
    _out->sk_FragColor.x = float(int(sqrt(5.0)));
    _out->sk_FragColor.x = float(int(sqrt(6.0)));
    _out->sk_FragColor.x = 0.0;
    _out->sk_FragColor.x = float(int(sqrt(8.0)));
    _out->sk_FragColor.x = 0.0;
    int x = int(sqrt(2.0));
    x += 1;
    x -= 1;
    x *= 2;
    x /= 2;
    _out->sk_FragColor.x = float(x);
    return *_out;
}

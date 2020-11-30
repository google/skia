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
    float x = 1.0;
    float y = 2.0;

    int z = 3;
    x = -6.0;
    y = -1.0;
    z = 8;
    bool b = false == true || 2.0 >= sqrt(2.0);
    bool c = sqrt(2.0) > 2.0;
    bool d = b != c;
    bool e = b && c;
    bool f = b || c;
    x += 12.0;
    x -= 12.0;
    x *= (y /= float(z = 10));
    z |= 0;
    z &= -1;
    z ^= 0;
    z >>= 2;
    z <<= 4;
    z %= 5;
    x = float((float2(sqrt(1.0)) , 6));
    y = (((((b ? 1.0 : 0.0) * (c ? 1.0 : 0.0)) * (d ? 1.0 : 0.0)) * (e ? 1.0 : 0.0)) * (f ? 1.0 : 0.0) , 6.0);
    z = (float2(sqrt(1.0)) , 6);
    return *_out;
}

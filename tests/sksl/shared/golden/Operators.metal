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
    float x = 1.0, y = 2.0;
    int z = 3;
    x = (x - x) + (6.0 * x) * -1.0;
    y = (x / y) / 3.0;
    z = (((z / 2) % 3 << 4) >> 2) << 1;
    bool b = x > 4.0 == x < 2.0 || 2.0 >= sqrt(2.0) && y <= float(z);
    x += 12.0;
    x -= 12.0;
    x *= (y /= float(z = 10));
    b ||= false;
    b &&= true;
    b ^^= false;
    z |= 0;
    z &= -1;
    z ^= 0;
    z >>= 2;
    z <<= 4;
    z %= 5;
    x = float((float2(sqrt(1.0)) , 6));
    z = (float2(sqrt(1.0)) , 6);
    return *_out;
}

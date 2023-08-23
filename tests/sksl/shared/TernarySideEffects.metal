#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half x = 1.0h;
    half y = 1.0h;
    x == y ? (x += 1.0h) : (y += 1.0h);
    x == y ? (x += 3.0h) : (y += 3.0h);
    x < y ? (x += 5.0h) : (y += 5.0h);
    y >= x ? (x += 9.0h) : (y += 9.0h);
    x != y ? (x += 1.0h) : y;
    x == y ? (x += 2.0h) : y;
    x != y ? x : (y += 3.0h);
    x == y ? x : (y += 4.0h);
    bool b = true;
    bool c = (b = false) ? false : b;
    _out.sk_FragColor = c ? _uniforms.colorRed : (x == 8.0h && y == 17.0h ? _uniforms.colorGreen : _uniforms.colorRed);
    return _out;
}

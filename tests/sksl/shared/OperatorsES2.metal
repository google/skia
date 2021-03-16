#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = 1.0;
    float y = 2.0;
    int z = 3;
    x = (x - x) + ((y * x) * x) * (y - x);
    y = (x / y) / x;
    z = ((z / 2) * 3 + 4) - 2;
    bool b = x > 4.0 == x < 2.0 || 2.0 >= sqrt(2.0) && y <= x;
    bool c = sqrt(2.0) > 2.0;
    bool d = b != c;
    bool e = b && c;
    bool f = b || c;
    x += 12.0;
    x -= 12.0;
    x *= (y /= 10.0);
    x = 6.0;
    y = (((float(b) * float(c)) * float(d)) * float(e)) * float(f);
    y = 6.0;
    z = z - 1;
    z = 6;
    _out.sk_FragColor = (x == 6.0 && y == 6.0) && z == 6 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

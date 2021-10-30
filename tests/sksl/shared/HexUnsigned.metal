#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    uint u1 = 0u;
    u1++;
    uint u2 = 305441741u;
    u2++;
    uint u3 = 2147483646u;
    u3++;
    uint u4 = 4294967294u;
    u4++;
    ushort u5 = 65534u;
    u5++;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}

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
    bool ok = true;
    bool TRUE = bool(_uniforms.colorGreen.y);
    ok = ok && 1 == int(TRUE);
    ok = ok && 1.0 == float(TRUE);
    ok = ok && TRUE;
    ok = ok && 1 == int(TRUE);
    ok = ok && 1.0 == float(TRUE);
    ok = ok && all(bool2(true) == bool2(TRUE));
    ok = ok && all(int2(1) == int2(int(TRUE)));
    ok = ok && all(float2(1.0) == float2(float(TRUE)));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

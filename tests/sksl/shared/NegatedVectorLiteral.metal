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

bool test_float() {
    const float one = 1.0;
    float two = 2.0;
    float4 result;
    result.x = 1.0;
    result.y = 1.0;
    result.z = float(all(-float4(two) == float4(-two, float3(-two))) ? 1 : 0);
    result.w = float(all(float2(1.0, -2.0) == -float2(one - two, two)) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
bool test_int() {
    int one = 1;
    const int two = 2;
    int4 result;
    result.x = 1;
    result.y = 1;
    result.z = int(all(-int4(two) == int4(-2, int3(-2))) ? 1 : 0);
    result.w = int(all(-int2(-one, one + one) == -int2(one - two, two)) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = test_float() && test_int() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

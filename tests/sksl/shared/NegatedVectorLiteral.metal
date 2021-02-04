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


bool test_int() {
    int4 result;
    result.x = 1;
    result.y = 1;
    result.z = 1;
    result.w = 1;
    return bool(((result.x * result.y) * result.z) * result.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _1_result;
    _1_result.x = 1.0;
    _1_result.y = 1.0;
    _1_result.z = 1.0;
    _1_result.w = 1.0;
    _out.sk_FragColor = bool(((_1_result.x * _1_result.y) * _1_result.z) * _1_result.w) && test_int() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;

}

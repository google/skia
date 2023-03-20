#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float3x3 testMatrix3x3;
    float4x4 testMatrix4x4;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool test3x3_b(Uniforms _uniforms) {
    float3 expected = float3(3.0, 2.0, 1.0);
    for (int c = 0;c < 3; ++c) {
        float3 vec = _uniforms.testMatrix3x3[c];
        for (int r = 0;r < 3; ++r) {
            if (vec[uint3(2, 1, 0)[r]] != expected[r]) {
                return false;
            }
        }
        expected += 3.0;
    }
    return true;
}
bool test4x4_b(Uniforms _uniforms) {
    float4 expected = float4(4.0, 3.0, 2.0, 1.0);
    for (int c = 0;c < 4; ++c) {
        float4 vec = _uniforms.testMatrix4x4[c];
        for (int r = 0;r < 4; ++r) {
            if (vec[uint4(3, 2, 1, 0)[r]] != expected[r]) {
                return false;
            }
        }
        expected += 4.0;
    }
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = test3x3_b(_uniforms) && test4x4_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

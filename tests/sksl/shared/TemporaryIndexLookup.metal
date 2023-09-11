#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
float3x3 GetTestMatrix_f33(Uniforms _uniforms) {
    return _uniforms.testMatrix3x3;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float expected = 0.0;
    for (int i = 0;i < 3; ++i) {
        for (int j = 0;j < 3; ++j) {
            expected += 1.0;
            if (GetTestMatrix_f33(_uniforms)[i][j] != expected) {
                _out.sk_FragColor = _uniforms.colorRed;
                return _out;
            }
        }
    }
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}

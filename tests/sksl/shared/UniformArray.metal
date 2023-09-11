#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    array<float, 5> testArray;
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
    for (int index = 0;index < 5; ++index) {
        if (_uniforms.testArray[index] != float(index + 1)) {
            _out.sk_FragColor = _uniforms.colorRed;
            return _out;
        }
    }
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}

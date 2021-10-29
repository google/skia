#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    array<float, 5> testArray;
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
    for (int index = 0;index < 5; ++index) {
        if (_uniforms.testArray[index] != float(index + 1)) {
            _out.sk_FragColor = _uniforms.colorRed;
            return _out;
        }
    }
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}

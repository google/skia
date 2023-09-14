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
    int counter = 0;
    const int increment = 1;
    for (int i = 0;i < 10; i += increment) {
        const int increment = 10;
        if (i == 0) {
            continue;
        }
        counter += increment;
    }
    _out.sk_FragColor = counter == 90 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

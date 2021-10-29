#include <metal_stdlib>
#include <simd/simd.h>
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
    float result = 0.0;
    for (int x = 0;x <= 1; x++) {
        {
            result = 2.0;
            continue;
        }
    }
    _out.sk_FragColor = result == 2.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

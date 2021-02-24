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
float4 unpremul(float4 color) {
    return float4(color.xyz / max(color.w, 9.9999997473787516e-05), color.w);
}
float4 unpremul_float(float4 color) {
    return float4(color.xyz / max(color.w, 9.9999997473787516e-05), color.w);
}


float4 dead_fn(float4 a, float4 b) {
    return a * b;
}
float4 live_fn(float4 a, float4 b) {
    return a + b;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool TRUE = true;
    bool FALSE = false;

    float4 a;
    float4 b;

    if (FALSE) {
        float4 unused = dead_fn(float4(0.5), float4(2.0));
    } else {
        a = live_fn(float4(3.0), float4(-5.0));
    }
    if (TRUE) {
        b = unpremul(float4(1.0));
    } else {
        float4 unused = unpremul_float(float4(-1.0));
    }
    _out.sk_FragColor = any(a != float4(0.0)) && any(b != float4(0.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

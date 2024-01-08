#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half4 ifElseTest_h4h4h4h4(Uniforms _uniforms, half4 colorBlue, half4 colorGreen, half4 colorRed) {
    half4 result = half4(0.0h);
    if (any(_uniforms.colorWhite != colorBlue)) {
        if (all(colorGreen == colorRed)) {
            result = colorRed;
        } else {
            result = colorGreen;
        }
    } else {
        if (any(colorRed != colorGreen)) {
            result = colorBlue;
        } else {
            result = _uniforms.colorWhite;
        }
    }
    if (all(colorRed == colorBlue)) {
        return _uniforms.colorWhite;
    }
    if (any(colorRed != colorGreen)) {
        return result;
    }
    if (all(colorRed == _uniforms.colorWhite)) {
        return colorBlue;
    }
    return colorRed;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = ifElseTest_h4h4h4h4(_uniforms, half4(0.0h, 0.0h, _uniforms.colorWhite.z, 1.0h), half4(0.0h, _uniforms.colorWhite.y, 0.0h, 1.0h), half4(_uniforms.colorWhite.x, 0.0h, 0.0h, 1.0h));
    return _out;
}

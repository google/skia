#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int zero = int(_uniforms.colorGreen.x);
    int one = int(_uniforms.colorGreen.y);
    for (int x = zero;x < 100; ++x) {
        for (int y = one;y < 100; ++y) {
            int _0_x = x;
            int _1_result = 0;
            while (_0_x >= y) {
                ++_1_result;
                _0_x -= y;
            }
            if (x / y != _1_result) {
                _out.sk_FragColor = half4(1.0h, half(float(x) * 0.003921569), half(float(y) * 0.003921569), 1.0h);
                return _out;
            }
        }
    }
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}

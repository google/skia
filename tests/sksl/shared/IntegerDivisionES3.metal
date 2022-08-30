#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
int exact_division_iii(int x, int y) {
    int result = 0;
    while (x >= y) {
        ++result;
        x -= y;
    }
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int zero = int(_uniforms.colorGreen.x);
    int one = int(_uniforms.colorGreen.y);
    for (int x = zero;x < 100; ++x) {
        for (int y = one;y < 100; ++y) {
            if (x / y != exact_division_iii(x, y)) {
                _out.sk_FragColor = half4(1.0h, half(float(x) / 255.0), half(float(y) / 255.0), 1.0h);
                return _out;
            }
        }
    }
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}

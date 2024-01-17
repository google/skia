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
    half sumA = 0.0h;
    half sumB = 0.0h;
    {
        half a = 0.0h;
        half b = 10.0h;
        for (; a < 10.0h && b > 0.0h; (++a, --b)) {
            sumA += a;
            sumB += b;
        }
    }
    if (sumA != 45.0h || sumB != 55.0h) {
        _out.sk_FragColor = _uniforms.colorRed;
        return _out;
    }
    int sumC = 0;
    {
        int c = 0;
        for (; c < 10; ++c) {
            sumC += c;
        }
    }
    if (sumC != 45) {
        _out.sk_FragColor = _uniforms.colorRed;
        return _out;
    }
    float sumE = 0.0;
    {
        array<float, 2> d = array<float, 2>{0.0, 10.0};
        array<float, 4> e = array<float, 4>{1.0, 2.0, 3.0, 4.0};
        for (; d[0] < d[1]; ++d[0]) {
            sumE += float(half(e[0]));
        }
    }
    if (sumE != 10.0) {
        _out.sk_FragColor = _uniforms.colorRed;
        return _out;
    }
    {
        for (; ; ) break;
    }
    for (; ; ) _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
    return _out;
}

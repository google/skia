#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    int x;
    int y;
};
struct Uniforms {
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
    array<float, 4> f1;
    f1[0] = 1.0;
    f1[0] = 2.0;
    f1[0] = 3.0;
    f1[0] = 4.0;
    array<float, 4> f2;
    f2[0] = 1.0;
    f2[0] = 2.0;
    f2[0] = 3.0;
    f2[0] = 4.0;
    array<float, 4> f3;
    f3[0] = 1.0;
    f3[0] = 2.0;
    f3[0] = 3.0;
    f3[0] = -4.0;
    array<S, 3> s1;
    s1[0] = S{1, 2};
    s1[1] = S{3, 4};
    s1[2] = S{5, 6};
    array<S, 3> s2;
    s2[0] = S{1, 2};
    s2[1] = S{0, 0};
    s2[2] = S{5, 6};
    array<S, 3> s3;
    s3[0] = S{1, 2};
    s3[1] = S{3, 4};
    s3[2] = S{5, 6};
    _out.sk_FragColor = ((f1 == f2 && f1 != f3) && s1 != s2) && s3 == s1 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

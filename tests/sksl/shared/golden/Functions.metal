#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float arr(float v[3][2]) {
    return v[0][0] * v[1][2];
}
float foo(float v[2]) {
    return v[0] * v[1];
}
void bar(thread float* x) {
    float y[2];
    float z;

    y[0] = x;
    y[1] = x * 2.0;
    z = foo(y);
    float a[2][3];
    a[0][0] = 123.0;
    a[1][2] = 456.0;
    *x = z + arr(a);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float x = 10.0;
    bar(&x);
    _out->sk_FragColor = float4(x);
    return *_out;
}

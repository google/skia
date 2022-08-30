#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    int h[1];
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

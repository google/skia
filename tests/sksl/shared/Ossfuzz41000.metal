#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float x;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

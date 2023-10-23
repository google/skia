#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct InterfaceBlockIn {
    int x;
} i[3];
thread struct InterfaceBlockOut {
    int x;
} o[3];
struct Globals {
    constant InterfaceBlockIn* i;
    constant InterfaceBlockOut* o;
};

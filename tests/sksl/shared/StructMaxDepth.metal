#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S1 {
    int x;
};
struct S2 {
    S1 x;
};
struct S3 {
    S2 x;
};
struct S4 {
    S3 x;
};
struct S5 {
    S4 x;
};
struct S6 {
    S5 x;
};
struct S7 {
    S6 x;
};
struct S8 {
    S7 x;
};
struct SA1 {
    array<int, 8> x;
};
struct SA2 {
    array<SA1, 7> x;
};
struct SA3 {
    array<SA2, 6> x;
};
struct SA4 {
    array<SA3, 5> x;
};
struct SA5 {
    array<SA4, 4> x;
};
struct SA6 {
    array<SA5, 3> x;
};
struct SA7 {
    array<SA6, 2> x;
};
struct SA8 {
    array<SA7, 1> x;
};
struct Inputs {
    S8 s8;
    array<SA8, 9> sa8;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


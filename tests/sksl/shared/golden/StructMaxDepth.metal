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
    int x[1][2][3][4][5][6][7][8];
};
struct SA2 {
    SA1 x[1][1][1][1][1][1][1][7];
};
struct SA3 {
    SA2 x[1][1][1][1][1][1][1][6];
};
struct SA4 {
    SA3 x[1][1][1][1][1][1][1][5];
};
struct SA5 {
    SA4 x[1][1][1][1][1][1][1][4];
};
struct SA6 {
    SA5 x[1][1][1][1][1][1][1][3];
};
struct SA7 {
    SA6 x[1][1][1][1][1][1][1][2];
};
struct SA8 {
    SA7 x[1][1][1][1][1][1][1][1];
};
struct Inputs {
    S8 s8;
    SA8[1][2][3][4][5][6][7][8] sa8;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};



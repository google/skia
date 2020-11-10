#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    struct S8 {
    struct S7 {
        struct S6 {
            struct S5 {
                struct S4 {
                    struct S3 {
                        struct S2 {
                            struct S1 {
                                int x;
                            } x;
                        } x;
                    } x;
                } x;
            } x;
        } x;
    } x;
} s8;
    SA8[1][2][3][4][5][6][7][8] sa8;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};



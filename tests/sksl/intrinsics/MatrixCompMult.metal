#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float3x3 a;
    float3x3 b;
    float4x4 c;
    float4x4 d;
};

template <int C, int R>
matrix<float, C, R> matrixCompMult(matrix<float, C, R> a, matrix<float, C, R> b) {
    matrix<float, C, R> result;
    for (int c = 0; c < C; ++c) {
        result[c] = a[c] * b[c];
    }
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.xyz = matrixCompMult(_globals.a, _globals.b)[0];
    _out.sk_FragColor = matrixCompMult(_globals.c, _globals.d)[0];
    return _out;
}

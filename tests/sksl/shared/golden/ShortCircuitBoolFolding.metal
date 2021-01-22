#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    bool expr1 = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).x > 0.0;
    bool expr2 = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).y > 0.0;
    if (expr1) {
        _out->sk_FragColor.x = 1.0;
    } else if (!expr1) {
        _out->sk_FragColor.x = 3.0;
    } else if (expr2) {
        _out->sk_FragColor.x = 4.0;
    } else if (expr2) {
        _out->sk_FragColor.x = 5.0;
    } else {
        _out->sk_FragColor.x = 6.0;
    }
    if (expr1) {
        _out->sk_FragColor.x = 1.0;
    } else if (!expr1) {
        _out->sk_FragColor.x = 3.0;
    } else if (expr2) {
        _out->sk_FragColor.x = 4.0;
    } else if (expr2) {
        _out->sk_FragColor.x = 5.0;
    } else {
        _out->sk_FragColor.x = 6.0;
    }
    return *_out;
}

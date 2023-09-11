#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    int gAccessCount;
};
int Z_i(thread Globals& _globals) {
    ++_globals.gAccessCount;
    return 0;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    int _skTemp0;
    int _skTemp1;
    int _skTemp2;
    int _skTemp3;
    array<float4, 1> array;
    array[Z_i(_globals)] = float4(_uniforms.colorGreen) * 0.5;
    array[Z_i(_globals)].w = 2.0;
(_skTemp0 = Z_i(_globals),     array[_skTemp0].y = array[_skTemp0].y * 4.0);
(_skTemp1 = Z_i(_globals),     array[_skTemp1].yzw = array[_skTemp1].yzw * float3x3(0.5));
(_skTemp2 = Z_i(_globals),     array[_skTemp2].zywx = array[_skTemp2].zywx + float4(0.25, 0.0, 0.0, 0.75));
(_skTemp3 = Z_i(_globals),     array[_skTemp3].x = array[_skTemp3].x + (array[Z_i(_globals)].w <= 1.0 ? array[Z_i(_globals)].z : float(Z_i(_globals))));
    _out.sk_FragColor = _globals.gAccessCount == 8 && all(array[0] == float4(1.0, 1.0, 0.25, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

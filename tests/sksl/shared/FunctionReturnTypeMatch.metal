#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


bool3 returns_bool3() {
    return bool3(true);
}
bool4 returns_bool4() {
    return bool4(true);
}
int returns_int() {
    return 1;
}
int2 returns_int2() {
    return int2(2);
}
int3 returns_int3() {
    return int3(3);
}
int4 returns_int4() {
    return int4(4);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = (((((all(bool2(true) == bool2(true)) && all(bool3(true) == returns_bool3())) && all(bool4(true) == returns_bool4())) && 1 == returns_int()) && all(int2(2) == returns_int2())) && all(int3(3) == returns_int3())) && all(int4(4) == returns_int4()) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
















}

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
float this_function_is_prototyped_at_the_start_and_never_defined_f();
half4 this_function_is_defined_before_use_h4h4(half4 x);
half4 this_function_is_defined_after_use_h4h4(half4 x);
half4 this_function_is_defined_near_the_end_h4h4(half4 x);
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]);
half4 this_function_is_defined_before_use_h4h4(half4 x) {
    return -x;
}
bool this_function_is_prototyped_in_the_middle_and_never_defined_bf44(float4x4 a);
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = this_function_is_defined_before_use_h4h4(-_uniforms.colorGreen);
    return _out;
}
int3 this_function_is_prototyped_at_the_very_end_and_never_defined_i3h22b2(half2x2 x, bool2 y);

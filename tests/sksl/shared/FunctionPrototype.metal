#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float this_function_is_prototyped_at_the_start_and_never_defined();
float4 this_function_is_defined_before_use(float4 x);
float4 this_function_is_defined_after_use(float4 x);
bool this_function_is_prototyped_in_the_middle_and_never_defined(float4x4 a);
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
int3 this_function_is_prototyped_at_the_very_end_and_never_defined(float2x2 x, bool2 y);

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float this_function_is_prototyped_at_the_start_and_never_defined();
float4 this_function_is_defined_before_use();
float4 this_function_is_defined_after_use();
float4 this_function_is_defined_before_use() {
    return float4(1.0);
}
bool this_function_is_prototyped_in_the_middle_and_never_defined(float4x4 a);
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = this_function_is_defined_before_use();
    _out->sk_FragColor = this_function_is_defined_after_use();
    return *_out;
}
float4 this_function_is_defined_after_use() {
    return float4(2.0);
}
int3 this_function_is_prototyped_at_the_very_end_and_never_defined(float2x3 x, bool2 y);

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float4 this_function_is_defined_before_use() {
    return float4(1.0);
}
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

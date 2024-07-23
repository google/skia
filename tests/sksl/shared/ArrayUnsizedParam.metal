#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    float y;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct testStorageBuffer {
    float testArr[1];
};
struct testStorageBufferStruct {
    S testArrStruct[1];
};
struct Globals {
    const device testStorageBuffer* _anonInterface0;
    const device testStorageBufferStruct* _anonInterface1;
};
float unsizedInParameterA_ff(const device float* x) {
    return x[0];
}
float unsizedInParameterB_fS(const device S* x) {
    return x[0].y;
}
float unsizedInParameterC_ff(const device float* x) {
    return x[0];
}
float unsizedInParameterD_fS(const device S* x) {
    return x[0].y;
}
float unsizedInParameterE_ff(const device float* ) {
    return 0.0;
}
float unsizedInParameterF_fS(const device S* ) {
    return 0.0;
}
half4 getColor_h4f(const device float* arr) {
    return half4(half(arr[0]), half(arr[1]), half(arr[2]), half(arr[3]));
}
half4 getColor_helper_h4f(const device float* arr) {
    return getColor_h4f(arr);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device testStorageBuffer& _anonInterface0 [[buffer(0)]], const device testStorageBufferStruct& _anonInterface1 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = getColor_helper_h4f(_globals._anonInterface0->testArr);
    unsizedInParameterA_ff(_globals._anonInterface0->testArr);
    unsizedInParameterB_fS(_globals._anonInterface1->testArrStruct);
    unsizedInParameterC_ff(_globals._anonInterface0->testArr);
    unsizedInParameterD_fS(_globals._anonInterface1->testArrStruct);
    unsizedInParameterE_ff(_globals._anonInterface0->testArr);
    unsizedInParameterF_fS(_globals._anonInterface1->testArrStruct);
    return _out;
}

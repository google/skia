#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct inputs {
    float inputBuffer[1];
};
struct outputs {
    float4 floatOut[1];
};
struct intOutputs {
    int4 intOut[1];
};
struct uintOutputs {
    uint4 uintOut[1];
};
struct Globals {
    device inputs* _anonInterface0;
    device outputs* _anonInterface1;
    device intOutputs* _anonInterface2;
    device uintOutputs* _anonInterface3;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], device inputs& _anonInterface0 [[buffer(0)]], device outputs& _anonInterface1 [[buffer(1)]], device intOutputs& _anonInterface2 [[buffer(2)]], device uintOutputs& _anonInterface3 [[buffer(3)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1, &_anonInterface2, &_anonInterface3};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    uint idx = _in.sk_GlobalInvocationID.x;
    float f = _globals._anonInterface0->inputBuffer[idx];
    int i = int(f);
    uint u = idx;
    _globals._anonInterface1->floatOut[idx] = float4(simd_sum(f), simd_min(f), simd_max(f), 0.0) + float4(simd_sum(half4(half(f))));
    _globals._anonInterface2->intOut[idx] = int4(simd_sum(i), simd_min(i), simd_max(i), 0) + simd_max(int4(i, -i, i, -i));
    _globals._anonInterface3->uintOut[idx] = uint4(simd_sum(u), simd_min(u), simd_max(u), 0u) + simd_min(uint4(u));
    _globals._anonInterface1->floatOut[idx].xy = _globals._anonInterface1->floatOut[idx].xy + float2(simd_prefix_inclusive_sum(f), simd_prefix_exclusive_sum(f));
    _globals._anonInterface1->floatOut[idx].zw = _globals._anonInterface1->floatOut[idx].zw + float2(simd_prefix_inclusive_sum(half2(half(f))));
    _globals._anonInterface2->intOut[idx].xy = _globals._anonInterface2->intOut[idx].xy + int2(simd_prefix_inclusive_sum(i), simd_prefix_exclusive_sum(i));
    _globals._anonInterface3->uintOut[idx] += simd_prefix_exclusive_sum(uint4(u));
    return;
}

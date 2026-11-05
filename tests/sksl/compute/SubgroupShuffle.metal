#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
    uint sk_SubgroupInvocationID;
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
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], uint sk_SubgroupInvocationID [[thread_index_in_simdgroup]], device inputs& _anonInterface0 [[buffer(0)]], device outputs& _anonInterface1 [[buffer(1)]], device intOutputs& _anonInterface2 [[buffer(2)]], device uintOutputs& _anonInterface3 [[buffer(3)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1, &_anonInterface2, &_anonInterface3};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID, sk_SubgroupInvocationID };
    uint idx = _in.sk_GlobalInvocationID.x;
    float f = _globals._anonInterface0->inputBuffer[idx];
    int i = int(f);
    uint u = idx;
    uint neighbor = _in.sk_SubgroupInvocationID ^ 1u;
    const uint kLane = 5u;
    _globals._anonInterface1->floatOut[idx] = float4(simd_shuffle(f, ushort(neighbor)), simd_shuffle(f, ushort(kLane)), 0.0, 0.0) + float4(simd_shuffle(half4(half(f)), ushort(neighbor)));
    _globals._anonInterface2->intOut[idx] = int4(simd_shuffle(i, ushort(neighbor)), 0, 0, 0) + simd_shuffle(int4(i, -i, i, -i), ushort(uint(i) & 7u));
    _globals._anonInterface3->uintOut[idx] = uint4(simd_shuffle(u, ushort(63u - _in.sk_SubgroupInvocationID)), 0u, 0u, 0u) + simd_shuffle(uint4(u), ushort(0u));
    return;
}

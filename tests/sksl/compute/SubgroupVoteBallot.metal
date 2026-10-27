#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const uint kLane = 3u;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct inputs {
    float inputBuffer[1];
};
struct outputs {
    uint4 outputBuffer[1];
};
struct Globals {
    device inputs* _anonInterface0;
    device outputs* _anonInterface1;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], device inputs& _anonInterface0 [[buffer(0)]], device outputs& _anonInterface1 [[buffer(1)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    uint idx = _in.sk_GlobalInvocationID.x;
    float f = _globals._anonInterface0->inputBuffer[idx];
    bool positive = f > 0.0;
    uint4 result = uint4(as_type<uint2>(static_cast<simd_vote::vote_t>(simd_ballot(positive))), 0u, 0u);
    result.x = result.x + uint(simd_all(positive));
    result.y = result.y + uint(simd_any(positive));
    result.z = result.z + uint(simd_broadcast(f, ushort(0)));
    result.w = result.w + uint(simd_broadcast(half(f), ushort(3)));
    result.xy = result.xy + uint2(simd_broadcast(int2(int(idx)), ushort(2)));
    result += simd_broadcast(uint4(idx), ushort(127));
    result.z = result.z + uint(simd_broadcast_first(f));
    result.w = result.w + uint(simd_broadcast_first(half3(half(f))).x);
    result.xy = result.xy + uint2(simd_broadcast_first(int2(int(idx))));
    result += simd_broadcast_first(uint4(idx));
    _globals._anonInterface1->outputBuffer[idx] = result;
    return;
}

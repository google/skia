#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_LocalInvocationID;
};
struct outputBlock {
    uint values[1];
};
struct Globals {
    device outputBlock* _anonInterface0;
};
uint clampValue_II(uint x) {
    if (x > 8u) {
        return x - 8u;
    }
    return x;
}
kernel void computeMain(uint3 sk_LocalInvocationID [[thread_position_in_threadgroup]], device outputBlock& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Inputs _in = { sk_LocalInvocationID };
    uint id = _in.sk_LocalInvocationID.x;
    uint value = _globals._anonInterface0->values[id];
    if (id % 2u == 0u) {
        value += 1u;
    } else {
        value *= 3u;
    }
    for (uint i = 0u;i < id; ++i) {
        if (value > 100u) {
            break;
        }
        if ((i & 1u) == 1u) {
            continue;
        }
        value += i;
    }
    switch (int(value % 4u)) {
        case 0:
            value += 4u;
        case 1:
            value += 1u;
            break;
        default:
            value = clampValue_II(value);
            break;
    }
    bool small = value < 16u && id != 0u;
    bool large = value > 1000u || id == 63u;
    _globals._anonInterface0->values[id] = small ? value : (large ? 0u : value + 1u);
    return;
}

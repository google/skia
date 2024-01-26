#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct IndirectDispatchArgs {
    int x;
    int y;
    int z;
};
struct Inputs {
};
struct InputBuffer {
    IndirectDispatchArgs args;
};
struct Globals {
    device InputBuffer* _anonInterface0;
};
struct Threadgroups {
    int outX;
    int outY;
    int outZ;
};
kernel void computeMain(device InputBuffer& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    threadgroup Threadgroups _threadgroups{{}, {}, {}};
    (void)_threadgroups;
    Inputs _in = {  };
    _threadgroups.outX = _globals._anonInterface0->args.x;
    _threadgroups.outY = _globals._anonInterface0->args.y;
    _threadgroups.outZ = _globals._anonInterface0->args.z;
    return;
}

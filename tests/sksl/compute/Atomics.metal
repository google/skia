#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    atomic_uint aCount;
};
struct Inputs {
};
struct ssbo {
    atomic_uint ssboCounterA;
    atomic_uint ssboCounterB;
    S ssboStructWithCounter;
    atomic_uint ssboCounterArray[1];
};
struct Globals {
    device ssbo* _anonInterface0;
};
kernel void computeMain(device ssbo& _anonInterface0 [[buffer(0)]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Inputs _in = {  };
    return;
}

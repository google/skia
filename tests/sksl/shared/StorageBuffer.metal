#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct SomeData {
    float4 a;
    float2 b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct storageBuffer {
    uint offset;
    SomeData inputData[1];
};
struct outputBuffer {
    SomeData outputData[1];
};
struct Globals {
    const device storageBuffer* _anonInterface0;
    device outputBuffer* _anonInterface1;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device storageBuffer& _anonInterface0 [[buffer(0)]], device outputBuffer& _anonInterface1 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _globals._anonInterface1->outputData[_globals._anonInterface0->offset] = _globals._anonInterface0->inputData[_globals._anonInterface0->offset];
    _out.sk_FragColor = half4(_globals._anonInterface0->inputData[_globals._anonInterface0->offset].a * _globals._anonInterface0->inputData[_globals._anonInterface0->offset].b.x);
    return _out;
}

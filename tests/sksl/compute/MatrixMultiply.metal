#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct sizeBuffer {
    int2 sizes[1];
};
struct inputs1 {
    float data1[1];
};
struct inputs2 {
    float data2[1];
};
struct result {
    float resultData[1];
};
struct Globals {
    device sizeBuffer* _anonInterface0;
    const device inputs1* _anonInterface1;
    const device inputs2* _anonInterface2;
    device result* _anonInterface3;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], device sizeBuffer& _anonInterface0 [[buffer(0)]], const device inputs1& _anonInterface1 [[buffer(1)]], const device inputs2& _anonInterface2 [[buffer(2)]], device result& _anonInterface3 [[buffer(3)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1, &_anonInterface2, &_anonInterface3};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    _globals._anonInterface0->sizes[2] = int2(_globals._anonInterface0->sizes[0].x, _globals._anonInterface0->sizes[1].y);
    int2 resultCell = int2(int(_in.sk_GlobalInvocationID.x), int(_in.sk_GlobalInvocationID.y));
    float result = 0.0;
    for (int i = 0;i < _globals._anonInterface0->sizes[0].y; ++i) {
        int a = i + resultCell.x * _globals._anonInterface0->sizes[0].y;
        int b = resultCell.y + i * _globals._anonInterface0->sizes[1].y;
        result += _globals._anonInterface1->data1[a] * _globals._anonInterface2->data2[b];
    }
    int index = resultCell.y + resultCell.x * _globals._anonInterface0->sizes[1].y;
    _globals._anonInterface3->resultData[index] = result;
    return;
}

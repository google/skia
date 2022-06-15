#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    device array<int2, 3>& sizes;
    device float* data1;
    device float* data2;
};
struct Outputs {
    device float* resultData;
};
kernel void computeMain(device array<int2, 3> sizes, device float* data1, device float* data2, device float* resultData, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { sk_ThreadPosition, sizes, data1, data2 };
    Outputs _out = { resultData };
    _in.sizes[2] = int2(_in.sizes[0].x, _in.sizes[1].y);
    int2 resultCell = int2(int(_in.sk_ThreadPosition.x), int(_in.sk_ThreadPosition.y));
    float result = 0.0;
    for (int i = 0;i < _in.sizes[0].y; ++i) {
        int a = i + resultCell.x * _in.sizes[0].y;
        int b = resultCell.y + i * _in.sizes[1].y;
        result += _in.data1[a] * _in.data2[b];
    }
    int index = resultCell.y + resultCell.x * _in.sizes[1].y;
    _out.resultData[index] = result;
    return;
}

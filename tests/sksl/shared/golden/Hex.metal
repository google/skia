#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int i1 = 0;
    i1++;
    int i2 = 305441741;
    i2++;
    int i3 = 2147483647;
    i3++;
    int i4 = 4294967295;
    i4++;
    int i5 = -48879;
    i5++;
    uint u1 = 0u;
    u1++;
    uint u2 = 305441741u;
    u2++;
    uint u3 = 2147483647u;
    u3++;
    uint u4 = 4294967295u;
    u4++;
    ushort u5 = 65535u;
    u5++;
    return _out;
}

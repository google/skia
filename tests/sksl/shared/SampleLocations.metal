#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_Position [[position]];
    float2 vcoord_Stage0 [[user(locn1)]];
    float sk_PointSize [[point_size]];
};
vertex Outputs vertexMain(Inputs _in [[stage_in]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
    Outputs _out;
    (void)_out;
    int x = sk_InstanceID % 200;
    int y = sk_InstanceID / 200;
    int ileft = (sk_InstanceID * 929) % 17;
    int iright = (ileft + 1) + (sk_InstanceID * 1637) % (17 - ileft);
    int itop = (sk_InstanceID * 313) % 17;
    int ibot = (itop + 1) + (sk_InstanceID * 1901) % (17 - itop);
    float outset = 0.03125;
    outset = 0 == (x + y) % 2 ? -outset : outset;
    float l = float(ileft) * 0.0625 - outset;
    float r = float(iright) * 0.0625 + outset;
    float t = float(itop) * 0.0625 - outset;
    float b = float(ibot) * 0.0625 + outset;
    float2 vertexpos;
    vertexpos.x = float(x) + (0 == sk_VertexID % 2 ? l : r);
    vertexpos.y = float(y) + (0 == sk_VertexID / 2 ? t : b);
    _out.vcoord_Stage0.x = float(0 == sk_VertexID % 2 ? -1 : 1);
    _out.vcoord_Stage0.y = float(0 == sk_VertexID / 2 ? -1 : 1);
    _out.sk_Position = float4(vertexpos.x, vertexpos.y, 0.0, 1.0);
    return _out;
}

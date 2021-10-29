#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float2 a;
    float4 b;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(as_type<uint>(half2(_uniforms.a)));
    _out.sk_FragColor.x = float(pack_float_to_unorm2x16(_uniforms.a));
    _out.sk_FragColor.x = float(pack_float_to_snorm2x16(_uniforms.a));
    _out.sk_FragColor.x = float(pack_float_to_unorm4x8(_uniforms.b));
    _out.sk_FragColor.x = float(pack_float_to_snorm4x8(_uniforms.b));
    return _out;
}

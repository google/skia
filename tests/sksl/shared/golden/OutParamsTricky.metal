#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    {
        float2 _5_3_color = _out->sk_FragColor.xz;
        {
            _5_3_color.xy = _5_3_color.yx;
        }
        _out->sk_FragColor.xz = _5_3_color;


        _out->sk_FragColor.yw = float2(3.0, 5.0);
    }

    return *_out;
}

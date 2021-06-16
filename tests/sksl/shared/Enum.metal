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
    {
        _out.sk_FragColor = float4(1.0);
    }
    {
        _out.sk_FragColor = float4(2.0);
    }
    {
        _out.sk_FragColor = float4(6.0);
    }
    _out.sk_FragColor = float4(7.0);
    _out.sk_FragColor = float4(-8.0);
    _out.sk_FragColor = float4(-9.0);
    _out.sk_FragColor = float4(10.0);
    {
        _out.sk_FragColor = float4(11.0);
    }
    {
        _out.sk_FragColor = float4(13.0);
    }
    int f = 1;
    if (f == 1) {
        _out.sk_FragColor = float4(1.0);
    }
    if (f != 1) {
        _out.sk_FragColor = float4(4.0);
    }
    _out.sk_FragColor = f == 0 ? float4(7.0) : float4(-7.0);
    _out.sk_FragColor = f != 0 ? float4(8.0) : float4(-8.0);
    _out.sk_FragColor = f == 1 ? float4(9.0) : float4(-9.0);
    _out.sk_FragColor = f != 1 ? float4(10.0) : float4(-10.0);
    switch (f) {
        case 0:
            _out.sk_FragColor = float4(11.0);
            break;
        case 1:
            _out.sk_FragColor = float4(12.0);
            break;
    }
    return _out;
}

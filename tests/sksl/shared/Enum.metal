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
    int e = 0;
    if (e == 0) {
        _out.sk_FragColor = float4(1.0);
    }
    if (e == 0) {
        _out.sk_FragColor = float4(2.0);
    }
    if (e != 0) {
        _out.sk_FragColor = float4(3.0);
    }
    if (e == 1) {
        _out.sk_FragColor = float4(4.0);
    }
    if (e == 1) {
        _out.sk_FragColor = float4(5.0);
    }
    if (e != 1) {
        _out.sk_FragColor = float4(6.0);
    }
    _out.sk_FragColor = e == 0 ? float4(7.0) : float4(-7.0);
    _out.sk_FragColor = e != 0 ? float4(8.0) : float4(-8.0);
    _out.sk_FragColor = e == 1 ? float4(9.0) : float4(-9.0);
    _out.sk_FragColor = e != 1 ? float4(10.0) : float4(-10.0);
    switch (e) {
        case 0:
            _out.sk_FragColor = float4(11.0);
            break;
        case 1:
            _out.sk_FragColor = float4(12.0);
            break;
    }
    switch (e) {
        case 0:
            _out.sk_FragColor = float4(13.0);
            break;
        case 1:
            _out.sk_FragColor = float4(14.0);
            break;
    }
    int m = 0;
    if (m == 0) {
        _out.sk_FragColor = float4(15.0);
    }
    if (m == 0) {
        _out.sk_FragColor = float4(16.0);
    }
    if (m == 1) {
        _out.sk_FragColor = float4(17.0);
    }
    if (m != 2) {
        _out.sk_FragColor = float4(18.0);
    }
    _out.sk_FragColor = m == 0 ? float4(19.0) : float4(-19.0);
    _out.sk_FragColor = m != 1 ? float4(20.0) : float4(-20.0);
    switch (m) {
        case 0:
            _out.sk_FragColor = float4(21.0);
            break;
        case 1:
            _out.sk_FragColor = float4(22.0);
            break;
        case 2:
            _out.sk_FragColor = float4(23.0);
            break;
    }
    return _out;
}

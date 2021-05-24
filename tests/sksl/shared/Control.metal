#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    if (_uniforms.unknownInput > 5.0) {
        _out.sk_FragColor = float4(0.75);
    } else {
        discard_fragment();
    }
    int i = 0;
    while (i < 10) {
        _out.sk_FragColor *= 0.5;
        i++;
    }
    do {
        _out.sk_FragColor += 0.25;
    } while (_out.sk_FragColor.x < 0.75);
    for (int i = 0;i < 10; i++) {
        if (i % 2 == 1) break; else if (i > 100) return _out; else continue;
    }
    return _out;
}

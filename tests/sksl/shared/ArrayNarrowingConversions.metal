#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right);
template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right);

template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right) {
    for (size_t index = 0; index < N; ++index) {
        if (!all(left[index] == right[index])) {
            return false;
        }
    }
    return true;
}

template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<int, 2> i2 = array<int, 2>{1, 2};
    array<int, 2> s2 = array<int, 2>{1, 2};
    array<float, 2> f2 = array<float, 2>{1.0, 2.0};
    array<float, 2> h2 = array<float, 2>{1.0, 2.0};
    i2 = s2;
    s2 = i2;
    f2 = h2;
    h2 = f2;
    const array<float, 2> cf2 = array<float, 2>{1.0, 2.0};
    _out.sk_FragColor = ((i2 == s2 && f2 == h2) && i2 == array<int, 2>{1, 2}) && h2 == cf2 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

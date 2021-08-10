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

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

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
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<float, 4> f = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    array<float, 4> h = f;
    f = h;
    h = f;
    array<int3, 3> i3 = array<int3, 3>{int3(1), int3(2), int3(3)};
    array<int3, 3> s3 = i3;
    i3 = s3;
    s3 = i3;
    array<float2x2, 2> h2x2 = array<float2x2, 2>{float2x2(float2(1.0, 2.0), float2(3.0, 4.0)), float2x2(float2(5.0, 6.0), float2(7.0, 8.0))};
    array<float2x2, 2> f2x2 = h2x2;
    f2x2 = h2x2;
    h2x2 = f2x2;
    _out.sk_FragColor = (f == h && i3 == s3) && f2x2 == h2x2 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

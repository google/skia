#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    int x;
    int y;
};
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right);
template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right);

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(thread const S& left, thread const S& right);
thread bool operator!=(thread const S& left, thread const S& right);

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
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread bool operator==(thread const S& left, thread const S& right) {
    return all(left.x == right.x) &&
           all(left.y == right.y);
}
thread bool operator!=(thread const S& left, thread const S& right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<float, 4> f1 = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    array<float, 4> f2 = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    array<float, 4> f3 = array<float, 4>{1.0, 2.0, 3.0, -4.0};
    array<int3, 2> v1 = array<int3, 2>{int3(1, 2, 3), int3(4, 5, 6)};
    array<int3, 2> v2 = array<int3, 2>{int3(1, 2, 3), int3(4, 5, 6)};
    array<int3, 2> v3 = array<int3, 2>{int3(1, 2, 3), int3(4, 5, -6)};
    array<half2x2, 3> m1 = array<half2x2, 3>{half2x2(1.0h), half2x2(2.0h), half2x2(half2(3.0h, 4.0h), half2(5.0h, 6.0h))};
    array<half2x2, 3> m2 = array<half2x2, 3>{half2x2(1.0h), half2x2(2.0h), half2x2(half2(3.0h, 4.0h), half2(5.0h, 6.0h))};
    array<half2x2, 3> m3 = array<half2x2, 3>{half2x2(1.0h), half2x2(half2(2.0h, 3.0h), half2(4.0h, 5.0h)), half2x2(6.0h)};
    array<S, 3> s1 = array<S, 3>{S{1, 2}, S{3, 4}, S{5, 6}};
    array<S, 3> s2 = array<S, 3>{S{1, 2}, S{0, 0}, S{5, 6}};
    array<S, 3> s3 = array<S, 3>{S{1, 2}, S{3, 4}, S{5, 6}};
    _out.sk_FragColor = ((((((f1 == f2 && f1 != f3) && v1 == v2) && v1 != v3) && m1 == m2) && m1 != m3) && s1 != s2) && s3 == s1 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

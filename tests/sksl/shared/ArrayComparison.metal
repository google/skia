#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    int x;
    int y;
};
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    array<float, 5> testArray;
    array<float, 5> testArrayNegative;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

template <typename T1, typename T2>
bool operator==(const array_ref<T1> left, const array_ref<T2> right);
template <typename T1, typename T2>
bool operator!=(const array_ref<T1> left, const array_ref<T2> right);

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(thread const S& left, thread const S& right);
thread bool operator!=(thread const S& left, thread const S& right);

template <typename T1, typename T2>
bool operator==(const array_ref<T1> left, const array_ref<T2> right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t index = 0; index < left.size(); ++index) {
        if (!all(left[index] == right[index])) {
            return false;
        }
    }
    return true;
}

template <typename T1, typename T2>
bool operator!=(const array_ref<T1> left, const array_ref<T2> right) {
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
    array<float, 5> f1 = array<float, 5>{1.0, 2.0, 3.0, 4.0, 5.0};
    array<float, 5> f2 = array<float, 5>{1.0, 2.0, 3.0, 4.0, 5.0};
    array<float, 5> f3 = array<float, 5>{1.0, 2.0, 3.0, -4.0, 5.0};
    array<int3, 2> v1 = array<int3, 2>{int3(1, 2, 3), int3(4, 5, 6)};
    array<int3, 2> v2 = array<int3, 2>{int3(1, 2, 3), int3(4, 5, 6)};
    array<int3, 2> v3 = array<int3, 2>{int3(1, 2, 3), int3(4, 5, -6)};
    array<half2x2, 3> m1 = array<half2x2, 3>{half2x2(1.0h), half2x2(2.0h), half2x2(half2(3.0h, 4.0h), half2(5.0h, 6.0h))};
    array<half2x2, 3> m2 = array<half2x2, 3>{half2x2(1.0h), half2x2(2.0h), half2x2(half2(3.0h, 4.0h), half2(5.0h, 6.0h))};
    array<half2x2, 3> m3 = array<half2x2, 3>{half2x2(1.0h), half2x2(half2(2.0h, 3.0h), half2(4.0h, 5.0h)), half2x2(6.0h)};
    array<S, 3> s1 = array<S, 3>{S{1, 2}, S{3, 4}, S{5, 6}};
    array<S, 3> s2 = array<S, 3>{S{1, 2}, S{0, 0}, S{5, 6}};
    array<S, 3> s3 = array<S, 3>{S{1, 2}, S{3, 4}, S{5, 6}};
    _out.sk_FragColor = (((((((((((make_array_ref(f1) == make_array_ref(f2) && make_array_ref(f1) != make_array_ref(f3)) && make_array_ref(_uniforms.testArray) != make_array_ref(_uniforms.testArrayNegative)) && make_array_ref(_uniforms.testArray) == make_array_ref(f1)) && make_array_ref(_uniforms.testArray) != make_array_ref(f3)) && make_array_ref(f1) == make_array_ref(_uniforms.testArray)) && make_array_ref(f3) != make_array_ref(_uniforms.testArray)) && make_array_ref(v1) == make_array_ref(v2)) && make_array_ref(v1) != make_array_ref(v3)) && make_array_ref(m1) == make_array_ref(m2)) && make_array_ref(m1) != make_array_ref(m3)) && make_array_ref(s1) != make_array_ref(s2)) && make_array_ref(s3) == make_array_ref(s1) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

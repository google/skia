#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    int x;
    int y;
    half2x2 m;
    array<float, 5> a;
};
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    array<float, 5> testArray;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

template <typename T1, typename T2>
bool operator==(const array_ref<T1> left, const array_ref<T2> right);
template <typename T1, typename T2>
bool operator!=(const array_ref<T1> left, const array_ref<T2> right);

thread bool operator==(thread const S& left, thread const S& right);
thread bool operator!=(thread const S& left, thread const S& right);
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}

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
thread bool operator==(thread const S& left, thread const S& right) {
    return all(left.x == right.x) &&
           all(left.y == right.y) &&
           all(left.m == right.m) &&
           (make_array_ref(left.a) == make_array_ref(right.a));
}
thread bool operator!=(thread const S& left, thread const S& right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<float, 5> array = array<float, 5>{1.0, 2.0, 3.0, 4.0, 5.0};
    S s1 = S{1, 2, half2x2(1.0h), array};
    S s2 = S{1, 2, half2x2(1.0h), _uniforms.testArray};
    S s3 = S{1, 2, half2x2(2.0h), array<float, 5>{1.0, 2.0, 3.0, 4.0, 5.0}};
    _out.sk_FragColor = s1 == s2 && s1 != s3 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

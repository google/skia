#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const array<half, 5> globalArray = array<half, 5>{1.0h, 1.0h, 1.0h, 1.0h, 1.0h};
constant const half2 globalVector = half2(1.0h);
constant const half2x2 globalMatrix = half2x2(half2(1.0h, 1.0h), half2(1.0h, 1.0h));
struct Uniforms {
    half4 colorRed;
    half2x2 testMatrix2x2;
    array<half, 5> testArray;
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const array<half, 5> localArray = array<half, 5>{0.0h, 1.0h, 2.0h, 3.0h, 4.0h};
    const half2 localVector = half2(1.0h);
    const half2x2 localMatrix = half2x2(half2(0.0h, 1.0h), half2(2.0h, 3.0h));
    if (((((make_array_ref(globalArray) == make_array_ref(_uniforms.testArray) || all(globalVector == _uniforms.colorRed.xy)) || globalMatrix == _uniforms.testMatrix2x2) || make_array_ref(localArray) == make_array_ref(_uniforms.testArray)) || all(localVector == _uniforms.colorRed.xy)) || localMatrix == _uniforms.testMatrix2x2) {
        _out.sk_FragColor = _uniforms.colorRed;
        return _out;
    }
    _out.sk_FragColor = half4(0.0h, 1.0h, 0.0h, 1.0h);
    return _out;
}

#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
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

template <size_t N>
array<int, N> array_of_int_from_short(thread const array<short, N>& x) {
    array<int, N> result;
    for (int i = 0; i < N; ++i) {
        result[i] = int(x[i]);
    }
    return result;
}

template <size_t N>
array<short, N> array_of_short_from_int(thread const array<int, N>& x) {
    array<short, N> result;
    for (int i = 0; i < N; ++i) {
        result[i] = short(x[i]);
    }
    return result;
}

template <size_t N>
array<float, N> array_of_float_from_half(thread const array<half, N>& x) {
    array<float, N> result;
    for (int i = 0; i < N; ++i) {
        result[i] = float(x[i]);
    }
    return result;
}

template <size_t N>
array<half, N> array_of_half_from_float(thread const array<float, N>& x) {
    array<half, N> result;
    for (int i = 0; i < N; ++i) {
        result[i] = half(x[i]);
    }
    return result;
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<int, 2> i2 = array<int, 2>{1, 2};
    array<short, 2> s2 = array<short, 2>{1, 2};
    array<float, 2> f2 = array<float, 2>{1.0, 2.0};
    array<half, 2> h2 = array<half, 2>{1.0h, 2.0h};
    i2 = array_of_int_from_short(s2);
    s2 = array_of_short_from_int(i2);
    f2 = array_of_float_from_half(h2);
    h2 = array_of_half_from_float(f2);
    const array<float, 2> cf2 = array<float, 2>{1.0, 2.0};
    _out.sk_FragColor = ((make_array_ref(i2) == make_array_ref(array_of_int_from_short(s2)) && make_array_ref(f2) == make_array_ref(array_of_float_from_half(h2))) && make_array_ref(i2) == make_array_ref(array<int, 2>{1, 2})) && make_array_ref(array_of_float_from_half(h2)) == make_array_ref(cf2) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_eq_mat2x2f32(left: mat2x2<f32>, right: mat2x2<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
fn mat2x2f32_diagonal(x: f32) -> mat2x2<f32> {
    return mat2x2<f32>(x, 0.0, 0.0, x);
}
fn mat3x3f32_diagonal(x: f32) -> mat3x3<f32> {
    return mat3x3<f32>(x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, x);
}
fn mat3x3f32_eq_mat3x3f32(left: mat3x3<f32>, right: mat3x3<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
fn mat4x4f32_diagonal(x: f32) -> mat4x4<f32> {
    return mat4x4<f32>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn mat4x4f32_eq_mat4x4f32(left: mat4x4<f32>, right: mat4x4<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
fn test_half_b() -> bool {
    var ok: bool = true;
    var m1: mat2x2<f32> = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    ok = ok && mat2x2f32_eq_mat2x2f32(m1, mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
    var m3: mat2x2<f32> = m1;
    ok = ok && mat2x2f32_eq_mat2x2f32(m3, mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
    var m4: mat2x2<f32> = mat2x2f32_diagonal(6.0);
    ok = ok && mat2x2f32_eq_mat2x2f32(m4, mat2x2<f32>(vec2<f32>(6.0, 0.0), vec2<f32>(0.0, 6.0)));
    m3 *= m4;
    ok = ok && mat2x2f32_eq_mat2x2f32(m3, mat2x2<f32>(vec2<f32>(6.0, 12.0), vec2<f32>(18.0, 24.0)));
    var m5: mat2x2<f32> = mat2x2f32_diagonal(m1[1].y);
    ok = ok && mat2x2f32_eq_mat2x2f32(m5, mat2x2<f32>(vec2<f32>(4.0, 0.0), vec2<f32>(0.0, 4.0)));
    m1 += m5;
    ok = ok && mat2x2f32_eq_mat2x2f32(m1, mat2x2<f32>(vec2<f32>(5.0, 2.0), vec2<f32>(3.0, 8.0)));
    var m7: mat2x2<f32> = mat2x2<f32>(vec2<f32>(5.0, 6.0), vec2<f32>(7.0, 8.0));
    ok = ok && mat2x2f32_eq_mat2x2f32(m7, mat2x2<f32>(vec2<f32>(5.0, 6.0), vec2<f32>(7.0, 8.0)));
    var m9: mat3x3<f32> = mat3x3f32_diagonal(9.0);
    ok = ok && mat3x3f32_eq_mat3x3f32(m9, mat3x3<f32>(vec3<f32>(9.0, 0.0, 0.0), vec3<f32>(0.0, 9.0, 0.0), vec3<f32>(0.0, 0.0, 9.0)));
    var m10: mat4x4<f32> = mat4x4f32_diagonal(11.0);
    ok = ok && mat4x4f32_eq_mat4x4f32(m10, mat4x4<f32>(vec4<f32>(11.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 11.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 11.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 11.0)));
    var m11: mat4x4<f32> = mat4x4<f32>(vec4<f32>(20.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 20.0));
    m11 -= m10;
    ok = ok && mat4x4f32_eq_mat4x4f32(m11, mat4x4<f32>(vec4<f32>(9.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 9.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 9.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 9.0)));
    return ok;
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _0_ok: bool = true;
    var _1_m1: mat2x2<f32> = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(_1_m1, mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
    var _2_m3: mat2x2<f32> = _1_m1;
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(_2_m3, mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
    let _3_m4: mat2x2<f32> = mat2x2f32_diagonal(6.0);
    _2_m3 *= _3_m4;
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(_2_m3, mat2x2<f32>(vec2<f32>(6.0, 12.0), vec2<f32>(18.0, 24.0)));
    var _4_m5: mat2x2<f32> = mat2x2f32_diagonal(_1_m1[1].y);
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(_4_m5, mat2x2<f32>(vec2<f32>(4.0, 0.0), vec2<f32>(0.0, 4.0)));
    _1_m1 += _4_m5;
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(_1_m1, mat2x2<f32>(vec2<f32>(5.0, 2.0), vec2<f32>(3.0, 8.0)));
    let _7_m10: mat4x4<f32> = mat4x4f32_diagonal(11.0);
    var _8_m11: mat4x4<f32> = mat4x4<f32>(vec4<f32>(20.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 20.0));
    _8_m11 -= _7_m10;
    _0_ok = _0_ok && mat4x4f32_eq_mat4x4f32(_8_m11, mat4x4<f32>(vec4<f32>(9.0, 20.0, 20.0, 20.0), vec4<f32>(20.0, 9.0, 20.0, 20.0), vec4<f32>(20.0, 20.0, 9.0, 20.0), vec4<f32>(20.0, 20.0, 20.0, 9.0)));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_0_ok && test_half_b()));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}

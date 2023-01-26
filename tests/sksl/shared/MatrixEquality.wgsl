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
    testMatrix2x2: mat2x2<f32>,
    testMatrix3x3: mat3x3<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_eq_mat2x2f32(left: mat2x2<f32>, right: mat2x2<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
fn mat3x3f32_eq_mat3x3f32(left: mat3x3<f32>, right: mat3x3<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
fn mat2x2f32_diagonal(x: f32) -> mat2x2<f32> {
    return mat2x2<f32>(x, 0.0, 0.0, x);
}
fn mat3x3f32_from_mat2x2f32(x0: mat2x2<f32>) -> mat3x3<f32> {
    return mat3x3<f32>(vec3<f32>(x0[0].xy, 0.0), vec3<f32>(x0[1].xy, 0.0), vec3<f32>(0.0, 0.0, 1.0));
}
fn mat3x3f32_diagonal(x: f32) -> mat3x3<f32> {
    return mat3x3<f32>(x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, x);
}
fn mat2x2f32_from_mat3x3f32(x0: mat3x3<f32>) -> mat2x2<f32> {
    return mat2x2<f32>(vec2<f32>(x0[0].xy), vec2<f32>(x0[1].xy));
}
fn vec4f32_from_mat2x2f32(x: mat2x2<f32>) -> vec4<f32> {
    return vec4<f32>(x[0].xy, x[1].xy);
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _0_ok: bool = true;
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(_globalUniforms.testMatrix2x2, mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
    _0_ok = _0_ok && mat3x3f32_eq_mat3x3f32(_globalUniforms.testMatrix3x3, mat3x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, 9.0)));
    _0_ok = _0_ok && !mat2x2f32_eq_mat2x2f32(_globalUniforms.testMatrix2x2, mat2x2f32_diagonal(100.0));
    _0_ok = _0_ok && !mat3x3f32_eq_mat3x3f32(_globalUniforms.testMatrix3x3, mat3x3<f32>(vec3<f32>(9.0, 8.0, 7.0), vec3<f32>(6.0, 5.0, 4.0), vec3<f32>(3.0, 2.0, 1.0)));
    var _1_zero: f32 = f32(_globalUniforms.colorGreen.x);
    var _2_one: f32 = f32(_globalUniforms.colorGreen.y);
    var _3_two: f32 = 2.0 * _2_one;
    var _4_nine: f32 = 9.0 * _2_one;
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2<f32>(vec2<f32>(_2_one, _1_zero), vec2<f32>(_1_zero, _2_one)), mat2x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0)));
    _0_ok = _0_ok && !mat2x2f32_eq_mat2x2f32(mat2x2<f32>(vec2<f32>(_2_one, _1_zero), vec2<f32>(_2_one)), mat2x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0)));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_2_one), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && !mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_2_one), mat2x2f32_diagonal(0.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(-_2_one), mat2x2f32_diagonal(-1.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_1_zero), mat2x2f32_diagonal(-0.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32((-1.0 * mat2x2f32_diagonal(-_2_one)), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32((-1.0 * mat2x2f32_diagonal(_1_zero)), mat2x2f32_diagonal(-0.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_2_one), mat2x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0)));
    _0_ok = _0_ok && !mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_3_two), mat2x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0)));
    _0_ok = _0_ok && !!mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_2_one), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && !mat2x2f32_eq_mat2x2f32(mat2x2f32_diagonal(_2_one), mat2x2f32_diagonal(0.0));
    _0_ok = _0_ok && mat3x3f32_eq_mat3x3f32(mat3x3<f32>(vec3<f32>(_2_one, _1_zero, _1_zero), vec3<f32>(_1_zero, _2_one, _1_zero), vec3<f32>(_1_zero, _1_zero, _2_one)), mat3x3f32_from_mat2x2f32(mat2x2f32_diagonal(1.0)));
    _0_ok = _0_ok && mat3x3f32_eq_mat3x3f32(mat3x3<f32>(vec3<f32>(_4_nine, _1_zero, _1_zero), vec3<f32>(_1_zero, _4_nine, _1_zero), vec3<f32>(_1_zero, _1_zero, _2_one)), mat3x3f32_from_mat2x2f32(mat2x2f32_diagonal(9.0)));
    _0_ok = _0_ok && mat3x3f32_eq_mat3x3f32(mat3x3f32_diagonal(_2_one), mat3x3f32_from_mat2x2f32(mat2x2f32_diagonal(1.0)));
    _0_ok = _0_ok && mat3x3f32_eq_mat3x3f32(mat3x3<f32>(vec3<f32>(_4_nine, 0.0, 0.0), vec3<f32>(0.0, _4_nine, 0.0), vec3<f32>(0.0, 0.0, _2_one)), mat3x3f32_from_mat2x2f32(mat2x2f32_diagonal(9.0)));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2f32_from_mat3x3f32(mat3x3f32_diagonal(_2_one)), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2f32_from_mat3x3f32(mat3x3f32_diagonal(_2_one)), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2<f32>(vec2<f32>(_2_one, _1_zero), vec2<f32>(_1_zero, _2_one)), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2<f32>(vec2<f32>(_2_one, _1_zero), vec2<f32>(_1_zero, _2_one)), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && mat2x2f32_eq_mat2x2f32(mat2x2<f32>(vec2<f32>(_2_one, _1_zero), vec2<f32>(_1_zero, _2_one)), mat2x2f32_diagonal(1.0));
    _0_ok = _0_ok && all(vec4<f32>(vec4f32_from_mat2x2f32(_globalUniforms.testMatrix2x2)) * vec4<f32>(_2_one) == vec4<f32>(1.0, 2.0, 3.0, 4.0));
    _0_ok = _0_ok && all(vec4<f32>(vec4f32_from_mat2x2f32(_globalUniforms.testMatrix2x2)) * vec4<f32>(_2_one) == vec4<f32>(vec4f32_from_mat2x2f32(_globalUniforms.testMatrix2x2)));
    _0_ok = _0_ok && all(vec4<f32>(vec4f32_from_mat2x2f32(_globalUniforms.testMatrix2x2)) * vec4<f32>(_1_zero) == vec4<f32>(0.0));
    var _5_m: mat3x3<f32> = mat3x3<f32>(vec3<f32>(_2_one, _3_two, 3.0), vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, _4_nine));
    _0_ok = _0_ok && all(_5_m[0] == vec3<f32>(1.0, 2.0, 3.0));
    _0_ok = _0_ok && all(_5_m[1] == vec3<f32>(4.0, 5.0, 6.0));
    _0_ok = _0_ok && all(_5_m[2] == vec3<f32>(7.0, 8.0, 9.0));
    _0_ok = _0_ok && _5_m[0].x == 1.0;
    _0_ok = _0_ok && _5_m[0].y == 2.0;
    _0_ok = _0_ok && _5_m[0].z == 3.0;
    _0_ok = _0_ok && _5_m[1].x == 4.0;
    _0_ok = _0_ok && _5_m[1].y == 5.0;
    _0_ok = _0_ok && _5_m[1].z == 6.0;
    _0_ok = _0_ok && _5_m[2].x == 7.0;
    _0_ok = _0_ok && _5_m[2].y == 8.0;
    _0_ok = _0_ok && _5_m[2].z == 9.0;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_0_ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}

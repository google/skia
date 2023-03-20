struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    testMatrix3x3: mat3x3<f32>,
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_from_mat3x3f32(x0: mat3x3<f32>) -> mat2x2<f32> {
    return mat2x2<f32>(vec2<f32>(x0[0].xy), vec2<f32>(x0[1].xy));
}
fn mat2x2f32_eq_mat2x2f32(left: mat2x2<f32>, right: mat2x2<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
fn mat3x3f32_eq_mat3x3f32(left: mat3x3<f32>, right: mat3x3<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
fn mat3x3f32_from_mat2x2f32(x0: mat2x2<f32>) -> mat3x3<f32> {
    return mat3x3<f32>(vec3<f32>(x0[0].xy, 0.0), vec3<f32>(x0[1].xy, 0.0), vec3<f32>(0.0, 0.0, 1.0));
}
fn resizeMatrix_f22() -> mat2x2<f32> {
    return mat2x2f32_from_mat3x3f32(_globalUniforms.testMatrix3x3);
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(mat2x2f32_eq_mat2x2f32(resizeMatrix_f22(), mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(4.0, 5.0))) && mat3x3f32_eq_mat3x3f32(mat3x3f32_from_mat2x2f32(resizeMatrix_f22()), mat3x3<f32>(vec3<f32>(1.0, 2.0, 0.0), vec3<f32>(4.0, 5.0, 0.0), vec3<f32>(0.0, 0.0, 1.0)))));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
fn mat4x4f32_diagonal(x: f32) -> mat4x4<f32> {
    return mat4x4<f32>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn mat4x4f32_from_mat2x3f32(x0: mat2x3<f32>) -> mat4x4<f32> {
    return mat4x4<f32>(vec4<f32>(x0[0].xyz, 0.0), vec4<f32>(x0[1].xyz, 0.0), vec4<f32>(0.0, 0.0, 1.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 1.0));
}
fn mat2x3f32_diagonal(x: f32) -> mat2x3<f32> {
    return mat2x3<f32>(x, 0.0, 0.0, 0.0, x, 0.0);
}
fn main(_stageOut: ptr<function, FSOut>) {
    var a: mat4x4<f32> = mat4x4f32_diagonal(6.0);
    var b: mat4x4<f32> = mat4x4f32_from_mat2x3f32(mat2x3f32_diagonal(7.0));
    (*_stageOut).sk_FragColor.x = f32(select(1, 0, all(a[1] == b[1])));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
